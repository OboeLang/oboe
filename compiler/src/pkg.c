/* SPDX-License-Identifier: GPL-2.0-only
 * © 2026 Sushii64
 * © 2026 robinpie
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include "pkg.h"

#include "izim.h"
#include "kabuk.h"
#include "katare.h"
#include "projectedit.h"
#include "projectjson.h"
#include "record.h"
#include "sha256.h"

#include <dirent.h>
#include <errno.h>
#include <fnmatch.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef OBOE_DEFAULT_REGISTRY
#define OBOE_DEFAULT_REGISTRY "katare://reedbed.oboelang.org/"
#endif

#define PKG_MAX 256

/* ---- small helpers ------------------------------------------------------ */

static const char *find_project_file(void)
{
	struct stat st;

	if (stat("project.jsonc", &st) == 0)
		return "project.jsonc";
	if (stat("project.json", &st) == 0)
		return "project.json";
	return NULL;
}

static char *slurp(const char *path, size_t *len)
{
	FILE *f = fopen(path, "rb");

	if (!f)
		return NULL;
	if (fseek(f, 0, SEEK_END) != 0) {
		fclose(f);
		return NULL;
	}
	long n = ftell(f);

	if (n < 0) {
		fclose(f);
		return NULL;
	}
	rewind(f);

	char *buf = malloc((size_t)n + 1);

	if (!buf) {
		fclose(f);
		return NULL;
	}
	size_t got = fread(buf, 1, (size_t)n, f);

	fclose(f);
	buf[got] = '\0';
	if (len)
		*len = got;
	return buf;
}

static bool mkdirs(const char *path)
{
	char buf[4096];

	snprintf(buf, sizeof buf, "%s", path);
	for (char *p = buf + 1; *p; p++) {
		if (*p != '/')
			continue;
		*p = '\0';
		if (mkdir(buf, 0755) != 0 && errno != EEXIST)
			return false;
		*p = '/';
	}
	return mkdir(buf, 0755) == 0 || errno == EEXIST;
}

static void remove_tree(const char *path)
{
	char cmd[8192];

	snprintf(cmd, sizeof cmd, "rm -rf \"%s\"", path);
	if (system(cmd) != 0)
		fprintf(stderr, "oboe: could not remove '%s'\n", path);
}

/* ---- registry and credentials ------------------------------------------- */

/* Precedence: --registry, then OBOE_REGISTRY, then a top-level "registry" in
   project.jsonc, then the built-in default.
 *
 * The environment deliberately outranks the project file: the test suite and
 * any CI pointing at a mirror must be able to redirect without editing a
 * fixture, and a fixture that hardcoded a port would be unusable. */
static const char *resolve_registry(const char *flag)
{
	static char buf[1024];

	if (flag && *flag)
		return flag;

	const char *env = getenv("OBOE_REGISTRY");

	if (env && *env)
		return env;

	const char *pf = find_project_file();

	if (pf) {
		char *json = slurp(pf, NULL);

		if (json) {
			/* depth-0 lookup, so a "registry" nested inside
			   build.targets cannot answer for the project */
			char *v = json_get_string(json, "registry");

			free(json);
			if (v) {
				snprintf(buf, sizeof buf, "%s", v);
				free(v);
				return buf;
			}
		}
	}
	return OBOE_DEFAULT_REGISTRY;
}

/* ---- the resolved set --------------------------------------------------- */

struct pin {
	char izim[IZIM_MAX + 1];
	char ver[WAKTANIMRA_MAX];
	char sema[SEMA_STR_LEN];
};

struct pinset {
	struct pin p[PKG_MAX];
	int n;
};

static struct pin *pin_find(struct pinset *s, const char *izim)
{
	for (int i = 0; i < s->n; i++)
		if (strcmp(s->p[i].izim, izim) == 0)
			return &s->p[i];
	return NULL;
}

/* ---- unpacking ---------------------------------------------------------- */

/* Extracts into .oboe/libraries/<izim>/ by way of a temporary directory: a
   half-written package must never be visible to the compiler, and the old one
   must not be destroyed until the new one is known good. */
static bool install_kabuk(const char *izim, const unsigned char *body,
			  size_t len, char **err)
{
	/* bounded below the buffers built from it, so appending a filename
	   provably fits and the compiler can see that it does */
	char tmpdir[3072], dest[3072];

	snprintf(tmpdir, sizeof tmpdir, ".oboe/tmp/%.*s.%ld", IZIM_MAX, izim,
		 (long)getpid());
	snprintf(dest, sizeof dest, ".oboe/libraries/%.*s", IZIM_MAX, izim);

	remove_tree(tmpdir);
	if (!mkdirs(tmpdir)) {
		*err = strdup("cannot create .oboe/tmp");
		return false;
	}

	if (!kabuk_extract(body, len, NULL, tmpdir, err)) {
		remove_tree(tmpdir);
		return false;
	}

	/* The compiler matches a folder module on the name in its own
	   project.jsonc, so a package whose manifest disagrees with the name it
	   was fetched under would install and then fail to import. The server
	   refuses to publish one, but a mirror is not necessarily the server. */
	char pjpath[4096];
	char *pj = NULL;

	snprintf(pjpath, sizeof pjpath, "%s/project.jsonc", tmpdir);
	pj = slurp(pjpath, NULL);
	if (!pj) {
		snprintf(pjpath, sizeof pjpath, "%s/project.json", tmpdir);
		pj = slurp(pjpath, NULL);
	}
	if (pj) {
		char *proj = json_extract_object(pj, "project");
		char *name = json_get_string(proj ? proj : pj, "name");
		bool bad = name && strcmp(name, izim) != 0;

		if (bad) {
			char msg[512];

			snprintf(
				msg, sizeof msg,
				"archive declares itself '%s' but was fetched as '%s'",
				name, izim);
			*err = strdup(msg);
		}
		free(name);
		free(proj);
		free(pj);
		if (bad) {
			remove_tree(tmpdir);
			return false;
		}
	}

	if (!mkdirs(".oboe/libraries")) {
		remove_tree(tmpdir);
		*err = strdup("cannot create .oboe/libraries");
		return false;
	}
	remove_tree(dest);
	if (rename(tmpdir, dest) != 0) {
		remove_tree(tmpdir);
		*err = strdup("cannot move the package into .oboe/libraries");
		return false;
	}
	return true;
}

/* ---- fetching ----------------------------------------------------------- */

static bool fetch_one(struct kat_conn *c, const struct pin *p, char **err)
{
	enum kat_status st =
		kat_request(c, "ko ghazema %s %s", p->izim, p->ver);

	if (st != KAT_SI) {
		char msg[256];

		snprintf(msg, sizeof msg, "%s %s: %s", p->izim, p->ver,
			 st == KAT_KERESEBYR ? "no such release" :
					       kat_status_word(st));
		*err = strdup(msg);
		return false;
	}
	if (!kat_has_body(c)) {
		*err = strdup("registry sent no archive");
		return false;
	}

	/* The digest is echoed on the status line so it can be checked while the
	   body streams. When we already know what it should be, disagreement is
	   settled before a single octet is read. */
	const char *echoed = kat_word(c, 1);

	if (!echoed || !sema_valid(echoed)) {
		*err = strdup("registry sent a malformed digest");
		return false;
	}
	if (p->sema[0] && !sema_equal(echoed, p->sema)) {
		char msg[512];

		snprintf(msg, sizeof msg,
			 "%s %s: registry offered %s but the record says %s",
			 p->izim, p->ver, echoed, p->sema);
		*err = strdup(msg);
		return false;
	}

	unsigned long long n = kat_body_len(c);

	if (!mkdirs(".oboe/tmp")) {
		*err = strdup("cannot create .oboe/tmp");
		return false;
	}

	char tmpfile[4096];

	snprintf(tmpfile, sizeof tmpfile, ".oboe/tmp/%s-%s.kabuk", p->izim,
		 p->ver);

	int fd = open(tmpfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);

	if (fd < 0) {
		*err = strdup("cannot write to .oboe/tmp");
		return false;
	}

	bool ok = kat_read_body_verified(c, fd, echoed, err);

	close(fd);
	if (!ok) {
		unlink(tmpfile);
		return false;
	}

	size_t blen = 0;
	char *body = slurp(tmpfile, &blen);

	unlink(tmpfile);
	if (!body || blen != (size_t)n) {
		free(body);
		*err = strdup("archive did not survive being written out");
		return false;
	}

	ok = install_kabuk(p->izim, (const unsigned char *)body, blen, err);
	free(body);
	return ok;
}

/* ---- resolution --------------------------------------------------------- */

/* The server's answer, when it offers one. */
static bool resolve_remote(struct kat_conn *c, const char *izim,
			   const char *cons, struct pinset *out, char **err)
{
	enum kat_status st = kat_request(c, "ko cizujo %s %s", izim, cons);

	if (st != KAT_SI) {
		char msg[256];

		snprintf(msg, sizeof msg, "cannot resolve %s %s: %s", izim,
			 cons,
			 st == KAT_KERESEBYR ? "no version set satisfies that" :
					       kat_status_word(st));
		*err = strdup(msg);
		return false;
	}

	size_t len = 0;
	char *body = kat_read_body(c, &len, err);

	if (!body)
		return false;

	for (char *line = body, *nl; line && *line; line = nl) {
		nl = strchr(line, '\n');
		if (nl)
			*nl++ = '\0';
		if (!*line)
			continue;

		char *sp1 = strchr(line, ' ');

		if (!sp1)
			continue;
		*sp1 = '\0';
		char *sp2 = strchr(sp1 + 1, ' ');

		if (!sp2)
			continue;
		*sp2 = '\0';

		if (out->n >= PKG_MAX)
			break;
		if (!izim_valid(line) || !waktanimra_valid(sp1 + 1) ||
		    !sema_valid(sp2 + 1))
			continue;

		struct pin *p = &out->p[out->n++];

		snprintf(p->izim, sizeof p->izim, "%s", line);
		snprintf(p->ver, sizeof p->ver, "%s", sp1 + 1);
		snprintf(p->sema, sizeof p->sema, "%s", sp2 + 1);
	}
	free(body);
	return out->n > 0;
}

struct want {
	char izim[IZIM_MAX + 1];
	char cons[WAKTANIMRA_MAX + 4][8];
	int ncons;
};

/* The fallback for a server that does not advertise cizujo. Walks ko besal and
   applies the rules of KATARE.md §8.4 -- highest satisfying version, yanked
   never chosen -- so that it reaches the same answer the server would. */
static bool resolve_local(struct kat_conn *c, const char *izim,
			  const char *cons, struct pinset *out, char **err)
{
	static char cons_of[PKG_MAX][8][WAKTANIMRA_MAX + 4];
	static int ncons_of[PKG_MAX];
	char names[PKG_MAX][IZIM_MAX + 1];
	int nnames = 0;
	bool dirty[PKG_MAX];

	memset(ncons_of, 0, sizeof ncons_of);
	memset(dirty, 0, sizeof dirty);

	int self = nnames++;

	snprintf(names[self], sizeof names[0], "%s", izim);
	snprintf(cons_of[self][ncons_of[self]++], WAKTANIMRA_MAX + 4, "%s",
		 cons);
	dirty[self] = true;

	int budget = 4 * PKG_MAX;
	bool changed = true;

	out->n = 0;

	while (changed && budget-- > 0) {
		changed = false;

		for (int i = 0; i < nnames; i++) {
			if (!dirty[i])
				continue;
			dirty[i] = false;
			changed = true;

			enum kat_status st =
				kat_request(c, "ko besal %s", names[i]);

			if (st != KAT_SI) {
				char msg[256];

				snprintf(msg, sizeof msg, "%s: %s", names[i],
					 st == KAT_KERESEBYR ?
						 "no such package" :
						 kat_status_word(st));
				*err = strdup(msg);
				return false;
			}

			size_t len = 0;
			char *body = kat_read_body(c, &len, err);

			if (!body)
				return false;

			struct record_set rs;

			if (!record_set_parse(body, len, &rs)) {
				free(body);
				*err = strdup(
					"registry sent a malformed record");
				return false;
			}
			free(body);

			/* highest satisfying, skipping yanked */
			const char *best_ver = NULL, *best_sema = NULL;
			int best_idx = -1;

			for (int r = 0; r < rs.n; r++) {
				const char *v = record_get(&rs.records[r],
							   "waktanimra");
				const char *sm =
					record_get(&rs.records[r], "sema");

				if (!v || !sm)
					continue;
				if (record_get(&rs.records[r], "kaldy"))
					continue;

				struct waktanimra wv;

				if (!waktanimra_parse(v, &wv))
					continue;

				bool all = true;

				for (int k = 0; k < ncons_of[i] && all; k++) {
					struct constraint cc;

					if (!constraint_parse(cons_of[i][k],
							      &cc))
						all = false;
					else
						all = constraint_match(&cc,
								       &wv);
				}
				if (!all)
					continue;

				if (!best_ver ||
				    waktanimra_cmp_str(v, best_ver) > 0) {
					best_ver = v;
					best_sema = sm;
					best_idx = r;
				}
			}

			if (!best_ver) {
				char msg[256];

				snprintf(
					msg, sizeof msg,
					"no release of %s satisfies every constraint on it",
					names[i]);
				*err = strdup(msg);
				record_set_free(&rs);
				return false;
			}

			struct pin *p = pin_find(out, names[i]);

			if (!p) {
				if (out->n >= PKG_MAX) {
					record_set_free(&rs);
					*err = strdup(
						"dependency graph too large");
					return false;
				}
				p = &out->p[out->n++];
				snprintf(p->izim, sizeof p->izim, "%s",
					 names[i]);
			}
			snprintf(p->ver, sizeof p->ver, "%s", best_ver);
			snprintf(p->sema, sizeof p->sema, "%s", best_sema);

			/* widen the problem with whatever this release needs */
			int it = 0;
			const char *d;

			while ((d = record_next(&rs.records[best_idx], "cizujo",
						&it))) {
				char dn[IZIM_MAX + 1];
				const char *sp = strchr(d, ' ');

				if (!sp)
					continue;
				size_t nl = (size_t)(sp - d);

				if (nl > IZIM_MAX)
					continue;
				memcpy(dn, d, nl);
				dn[nl] = '\0';
				if (!izim_valid(dn) ||
				    !constraint_valid(sp + 1))
					continue;

				int slot = -1;

				for (int q = 0; q < nnames; q++)
					if (strcmp(names[q], dn) == 0)
						slot = q;
				if (slot < 0) {
					if (nnames >= PKG_MAX)
						continue;
					slot = nnames++;
					snprintf(names[slot], sizeof names[0],
						 "%s", dn);
					ncons_of[slot] = 0;
				}

				bool have = false;

				for (int k = 0; k < ncons_of[slot]; k++)
					if (strcmp(cons_of[slot][k], sp + 1) ==
					    0)
						have = true;
				if (have)
					continue;
				if (ncons_of[slot] >= 8)
					continue;
				snprintf(cons_of[slot][ncons_of[slot]++],
					 WAKTANIMRA_MAX + 4, "%s", sp + 1);
				/* a new constraint can invalidate an earlier
				   pick, so that package is queued again */
				dirty[slot] = true;
			}
			record_set_free(&rs);
		}
	}

	if (budget <= 0) {
		*err = strdup("could not settle on a version set");
		return false;
	}
	return out->n > 0;
}

static bool resolve(struct kat_conn *c, const char *izim, const char *cons,
		    struct pinset *out, char **err)
{
	out->n = 0;
	if (kat_has_cap(c, "cizujo"))
		return resolve_remote(c, izim, cons, out, err);
	return resolve_local(c, izim, cons, out, err);
}

/* ---- lockfile ----------------------------------------------------------- */

/* .oboe/lock pins the whole resolved set, so two tidy runs a month apart install
   the same versions and a republished archive cannot change under a project. */
static void lock_write(const struct pinset *s)
{
	if (!mkdirs(".oboe"))
		return;

	FILE *f = fopen(".oboe/lock.tmp", "w");

	if (!f)
		return;
	fputs("# written by oboe; commit this file\n", f);
	for (int i = 0; i < s->n; i++)
		fprintf(f, "%s %s %s\n", s->p[i].izim, s->p[i].ver,
			s->p[i].sema);
	fclose(f);
	if (rename(".oboe/lock.tmp", ".oboe/lock") != 0)
		unlink(".oboe/lock.tmp");
}

static void lock_read(struct pinset *s)
{
	s->n = 0;

	char *body = slurp(".oboe/lock", NULL);

	if (!body)
		return;

	for (char *line = body, *nl; line && *line; line = nl) {
		nl = strchr(line, '\n');
		if (nl)
			*nl++ = '\0';
		if (!*line || *line == '#')
			continue;

		char *sp1 = strchr(line, ' ');

		if (!sp1)
			continue;
		*sp1 = '\0';
		char *sp2 = strchr(sp1 + 1, ' ');

		if (!sp2)
			continue;
		*sp2 = '\0';

		if (s->n >= PKG_MAX)
			break;
		if (!izim_valid(line) || !waktanimra_valid(sp1 + 1) ||
		    !sema_valid(sp2 + 1))
			continue;

		struct pin *p = &s->p[s->n++];

		snprintf(p->izim, sizeof p->izim, "%s", line);
		snprintf(p->ver, sizeof p->ver, "%s", sp1 + 1);
		snprintf(p->sema, sizeof p->sema, "%s", sp2 + 1);
	}
	free(body);
}

static bool installed_at(const char *izim, const char *ver)
{
	char path[4096];

	snprintf(path, sizeof path, ".oboe/libraries/%.*s/project.jsonc",
		 IZIM_MAX, izim);

	char *pj = slurp(path, NULL);

	if (!pj) {
		snprintf(path, sizeof path, ".oboe/libraries/%.*s/project.json",
			 IZIM_MAX, izim);
		pj = slurp(path, NULL);
	}
	if (!pj)
		return false;

	char *proj = json_extract_object(pj, "project");
	char *v = json_get_string(proj ? proj : pj, "version");
	bool match = v && strcmp(v, ver) == 0;

	free(v);
	free(proj);
	free(pj);
	return match;
}

/* ---- commands ----------------------------------------------------------- */

struct getopts {
	const char *registry;
	bool verbose;
};

static bool parse_common(int argc, char **argv, struct getopts *o,
			 const char **positional)
{
	*positional = NULL;
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "--registry") == 0 && i + 1 < argc) {
			o->registry = argv[++i];
		} else if (strcmp(argv[i], "-v") == 0 ||
			   strcmp(argv[i], "--verbose") == 0) {
			o->verbose = true;
		} else if (argv[i][0] == '-') {
			fprintf(stderr, "oboe: unknown option '%s'\n", argv[i]);
			return false;
		} else if (!*positional) {
			*positional = argv[i];
		} else {
			fprintf(stderr, "oboe: unexpected argument '%s'\n",
				argv[i]);
			return false;
		}
	}
	return true;
}

/* A release's `warna`: vivlijotiki (a library) or pawi (a tool). Costs one
   extra request, because a cizujo pin line carries only the digest -- but
   installing a tool as a library, or importing a tool, fails later and much
   less clearly than refusing here. */
static bool fetch_warna(struct kat_conn *c, const char *izim, const char *ver,
			char *out, size_t cap, char **err)
{
	enum kat_status st = kat_request(c, "ko besal %s %s", izim, ver);

	if (st != KAT_SI || !kat_has_body(c)) {
		*err = strdup("registry has no record for that release");
		return false;
	}

	size_t len = 0;
	char *body = kat_read_body(c, &len, err);

	if (!body)
		return false;

	struct record_set rs;
	bool ok = false;

	if (record_set_parse(body, len, &rs)) {
		if (rs.n > 0) {
			const char *w = record_get(&rs.records[0], "warna");

			if (w) {
				snprintf(out, cap, "%s", w);
				ok = true;
			}
		}
		record_set_free(&rs);
	}
	free(body);

	if (!ok)
		*err = strdup(
			"record does not say what kind of package this is");
	return ok;
}

/* Where installed tools live: $OBOE_HOME, else ~/.oboe. */
static bool tools_home(char *out, size_t cap)
{
	const char *env = getenv("OBOE_HOME");

	if (env && *env) {
		snprintf(out, cap, "%s", env);
		return true;
	}

	const char *home = getenv("HOME");

	if (!home || !*home)
		return false;
	snprintf(out, cap, "%s/.oboe", home);
	return true;
}

static bool on_path(const char *dir)
{
	const char *path = getenv("PATH");

	if (!path)
		return false;

	size_t dlen = strlen(dir);

	for (const char *p = path; *p;) {
		const char *end = strchr(p, ':');
		size_t n = end ? (size_t)(end - p) : strlen(p);

		if (n == dlen && memcmp(p, dir, n) == 0)
			return true;
		if (!end)
			break;
		p = end + 1;
	}
	return false;
}

/* Builds an unpacked tool by re-invoking this same compiler, rather than
   reaching into its build machinery: `oboe build` already knows about targets,
   metadata and the C toolchain, and a second path through that would drift. */
static bool build_tool(const char *pkgdir, const char *binpath)
{
	char *home = oboe_home();
	char cmd[8192];

	snprintf(cmd, sizeof cmd, "cd \"%s\" && \"%s/oboe\" build -o \"%s\"",
		 pkgdir, home, binpath);
	free(home);

	return system(cmd) == 0;
}

/* Fetches a tool and everything it needs into $OBOE_HOME/pkg/<izim>/, builds it
   there, and puts the binary in $OBOE_HOME/bin/. Nothing outside that tree is
   touched -- in particular no shell profile is edited; a PATH that needs
   changing is the user's to change. */
static int install_tool(struct kat_conn *c, struct pinset *set,
			const struct pin *root, bool verbose)
{
	char home[3072];

	if (!tools_home(home, sizeof home)) {
		fprintf(stderr,
			"oboe: set OBOE_HOME or HOME to say where tools should go\n");
		kat_close(c);
		return 1;
	}

	char pkgdir[3600], bindir[3200], binpath[3600];

	snprintf(pkgdir, sizeof pkgdir, "%s/pkg/%.*s", home, IZIM_MAX,
		 root->izim);
	snprintf(bindir, sizeof bindir, "%s/bin", home);
	snprintf(binpath, sizeof binpath, "%s/%.*s", bindir, IZIM_MAX,
		 root->izim);

	if (!mkdirs(pkgdir) || !mkdirs(bindir)) {
		fprintf(stderr, "oboe: cannot create %s\n", home);
		kat_close(c);
		return 1;
	}

	/* The tool is built like any other project, so its dependencies have to
	   land where its own imports will look: its .oboe/libraries. */
	char save[4096];

	if (!getcwd(save, sizeof save)) {
		fprintf(stderr,
			"oboe: cannot determine the current directory\n");
		kat_close(c);
		return 1;
	}
	if (chdir(pkgdir) != 0) {
		fprintf(stderr, "oboe: cannot enter %s\n", pkgdir);
		kat_close(c);
		return 1;
	}

	char *err = NULL;
	int rc = 0;

	for (int i = 0; i < set->n; i++) {
		if (verbose)
			printf("Fetching %s %s\n", set->p[i].izim,
			       set->p[i].ver);
		if (!fetch_one(c, &set->p[i], &err)) {
			fprintf(stderr, "oboe: %s\n",
				err ? err : "fetch failed");
			free(err);
			err = NULL;
			rc = 1;
			break;
		}
	}
	kat_close(c);

	/* the tool itself is the program, not one of its own libraries */
	if (rc == 0) {
		char self[3600];

		snprintf(self, sizeof self, ".oboe/libraries/%.*s", IZIM_MAX,
			 root->izim);

		char cmd[8192];

		snprintf(cmd, sizeof cmd, "cp -R \"%s/.\" .", self);
		if (system(cmd) != 0) {
			fprintf(stderr, "oboe: cannot unpack %s\n", root->izim);
			rc = 1;
		} else {
			remove_tree(self);
		}
	}

	if (rc == 0 && !build_tool(".", binpath)) {
		fprintf(stderr, "oboe: could not build %s\n", root->izim);
		rc = 1;
	}

	if (chdir(save) != 0)
		fprintf(stderr, "oboe: could not return to %s\n", save);

	if (rc == 0) {
		printf("Installed %s %s to %s\n", root->izim, root->ver,
		       binpath);
		if (!on_path(bindir))
			printf("oboe: %s is not on your PATH; add it to run %s by name\n",
			       bindir, root->izim);
	}
	return rc;
}

static int do_get(int argc, char **argv, bool as_tool)
{
	struct getopts o = { 0 };
	const char *spec = NULL;

	if (!parse_common(argc, argv, &o, &spec))
		return 2;
	if (!spec) {
		fprintf(stderr, "oboe: '%s' needs a package name\n",
			as_tool ? "install" : "get");
		return 2;
	}
	/* a library is added to a project; a tool is installed for the user, so
	   only the former needs to be standing in one */
	if (!as_tool && !find_project_file()) {
		fprintf(stderr,
			"oboe: no project.jsonc here; 'get' only works inside a project\n");
		return 1;
	}

	/* name[@constraint] */
	char izim[IZIM_MAX + 1];
	char cons[WAKTANIMRA_MAX + 4] = "*";
	const char *at = strchr(spec, '@');

	if (at) {
		size_t n = (size_t)(at - spec);

		if (n > IZIM_MAX) {
			fprintf(stderr, "oboe: package name too long\n");
			return 1;
		}
		memcpy(izim, spec, n);
		izim[n] = '\0';
		snprintf(cons, sizeof cons, "%s", at + 1);
	} else {
		snprintf(izim, sizeof izim, "%s", spec);
	}

	if (!izim_valid(izim)) {
		fprintf(stderr,
			"oboe: '%s' is not a valid package name (lowercase letters, digits and _)\n",
			izim);
		return 1;
	}
	if (izim_reserved(izim)) {
		fprintf(stderr,
			"oboe: '%s' is a built-in module, not a package\n",
			izim);
		return 1;
	}
	if (!constraint_valid(cons)) {
		fprintf(stderr,
			"oboe: '%s' is not a version constraint (=X.Y.Z, >=X.Y.Z, ^X.Y.Z or *)\n",
			cons);
		return 1;
	}

	const char *uri = resolve_registry(o.registry);
	char host[256];
	int port = 0;
	char *err = NULL;

	if (!kat_parse_uri(uri, host, sizeof host, &port, NULL, 0, &err)) {
		fprintf(stderr, "oboe: %s\n", err ? err : "bad registry URL");
		free(err);
		return 1;
	}

	struct kat_conn *c = kat_connect(host, port, &err);

	if (!c) {
		fprintf(stderr, "oboe: %s\n", err ? err : "cannot connect");
		free(err);
		return 1;
	}

	struct pinset set;

	if (!resolve(c, izim, cons, &set, &err)) {
		fprintf(stderr, "oboe: %s\n", err ? err : "cannot resolve");
		free(err);
		kat_close(c);
		return 1;
	}

	struct pin *root = pin_find(&set, izim);

	if (!root) {
		fprintf(stderr, "oboe: registry resolved %s to nothing\n",
			izim);
		kat_close(c);
		return 1;
	}

	/* Refuse a mismatch rather than install it: a tool dropped into
	   .oboe/libraries is never imported, and a library built as a program
	   has no main. Both fail later and far less clearly. */
	char warna[32] = "";

	if (!fetch_warna(c, root->izim, root->ver, warna, sizeof warna, &err)) {
		fprintf(stderr, "oboe: %s\n", err ? err : "cannot read record");
		free(err);
		kat_close(c);
		return 1;
	}

	if (strcmp(warna, as_tool ? "pawi" : "vivlijotiki") != 0) {
		bool is_tool = strcmp(warna, "pawi") == 0;

		fprintf(stderr, "oboe: %s is a %s; use 'oboe %s' instead\n",
			izim, is_tool ? "tool" : "library",
			is_tool ? "install" : "get");
		kat_close(c);
		return 1;
	}

	if (as_tool)
		return install_tool(c, &set, root, o.verbose);

	int rc = 0;

	for (int i = 0; i < set.n; i++) {
		if (o.verbose)
			printf("Fetching %s %s\n", set.p[i].izim, set.p[i].ver);
		if (!fetch_one(c, &set.p[i], &err)) {
			fprintf(stderr, "oboe: %s\n",
				err ? err : "fetch failed");
			free(err);
			err = NULL;
			rc = 1;
			break;
		}
		printf("Installed %s %s\n", set.p[i].izim, set.p[i].ver);
	}
	kat_close(c);

	if (rc == 0) {
		lock_write(&set);
		/* only the package actually asked for goes in project.jsonc;
		   what it dragged in belongs to the lockfile */
		const char *pf = find_project_file();
		struct pin *self = pin_find(&set, izim);

		if (pf && self) {
			char c2[WAKTANIMRA_MAX + 4];

			snprintf(c2, sizeof c2, "%s", at ? cons : "*");
			if (!at)
				snprintf(c2, sizeof c2, "^%s", self->ver);
			if (add_dependency_line(pf, izim, c2))
				printf("Added \"%s\": \"%s\" to %s\n", izim, c2,
				       pf);
		}
	}
	return rc;
}

int cmd_get(int argc, char **argv)
{
	return do_get(argc, argv, false);
}

int cmd_install(int argc, char **argv)
{
	return do_get(argc, argv, true);
}

int pkg_tidy(bool verbose)
{
	const char *pf = find_project_file();

	if (!pf)
		return 0;

	struct pinset lock;

	lock_read(&lock);

	/* Everything the lockfile pins is already here: nothing to fetch, and
	   nothing to connect to. Working offline when it can is most of what
	   makes tidy usable. */
	int missing = 0;

	for (int i = 0; i < lock.n; i++)
		if (!installed_at(lock.p[i].izim, lock.p[i].ver))
			missing++;

	if (lock.n > 0 && missing == 0) {
		if (verbose)
			printf("oboe: all %d locked dependencies are present\n",
			       lock.n);
		return 0;
	}

	/* Anything the lock pins but that is not on disk is refetched at exactly
	   the pinned version, with no resolution at all. Most of these are
	   transitive: they appear in no project.jsonc, so the declared-dependency
	   walk below would never notice they had gone missing. */
	if (missing > 0) {
		const char *uri = resolve_registry(NULL);
		char host[256];
		int port = 0;
		char *err = NULL;

		if (!kat_parse_uri(uri, host, sizeof host, &port, NULL, 0,
				   &err)) {
			fprintf(stderr, "oboe: %s\n",
				err ? err : "bad registry URL");
			free(err);
			return 1;
		}

		struct kat_conn *c = kat_connect(host, port, &err);

		if (!c) {
			fprintf(stderr,
				"oboe: %d locked %s missing and the registry is unreachable: %s\n",
				missing,
				missing == 1 ? "dependency is" :
					       "dependencies are",
				err ? err : "cannot connect");
			free(err);
			return 1;
		}

		for (int i = 0; i < lock.n; i++) {
			if (installed_at(lock.p[i].izim, lock.p[i].ver))
				continue;
			if (!fetch_one(c, &lock.p[i], &err)) {
				fprintf(stderr, "oboe: %s\n",
					err ? err : "fetch failed");
				free(err);
				kat_close(c);
				return 1;
			}
			printf("Installed %s %s\n", lock.p[i].izim,
			       lock.p[i].ver);
		}
		kat_close(c);
	}

	char *json = slurp(pf, NULL);

	if (!json)
		return 0;

	char *deps = json_extract_object(json, "dependencies");

	free(json);
	if (!deps) {
		if (verbose)
			printf("oboe: no dependencies declared\n");
		return 0;
	}

	int ndeps = 0;
	char **keys = json_object_keys(deps, &ndeps);
	int rc = 0;

	for (int i = 0; i < ndeps; i++) {
		/* "oboe" names the toolchain, not a package */
		if (strcmp(keys[i], "oboe") == 0) {
			free(keys[i]);
			continue;
		}

		struct pin *pinned = NULL;

		for (int k = 0; k < lock.n; k++)
			if (strcmp(lock.p[k].izim, keys[i]) == 0)
				pinned = &lock.p[k];

		if (pinned && installed_at(pinned->izim, pinned->ver)) {
			free(keys[i]);
			continue;
		}

		char *cons = json_get_string(deps, keys[i]);
		char spec[512];

		snprintf(spec, sizeof spec, "%s@%s", keys[i],
			 cons && constraint_valid(cons) ? cons : "*");
		free(cons);

		char *args[] = { spec };

		if (do_get(1, args, false) != 0)
			rc = 1;
		free(keys[i]);
	}
	free(keys);
	free(deps);
	return rc;
}

int cmd_sema(int argc, char **argv)
{
	if (argc < 1) {
		fprintf(stderr, "usage: oboe sema <file>...\n");
		return 2;
	}

	int rc = 0;

	for (int i = 0; i < argc; i++) {
		/* streamed rather than slurped, so a pipe or /dev/stdin works:
		   seeking to find the length would fail on anything that is not
		   a regular file */
		FILE *f = strcmp(argv[i], "-") == 0 ? stdin :
						      fopen(argv[i], "rb");

		if (!f) {
			fprintf(stderr, "oboe: cannot read %s\n", argv[i]);
			rc = 1;
			continue;
		}

		struct sha256_ctx ctx;
		unsigned char buf[65536];
		size_t n;

		sha256_init(&ctx);
		while ((n = fread(buf, 1, sizeof buf, f)) > 0)
			sha256_update(&ctx, buf, n);

		bool bad = ferror(f) != 0;

		if (f != stdin)
			fclose(f);
		if (bad) {
			fprintf(stderr, "oboe: error reading %s\n", argv[i]);
			rc = 1;
			continue;
		}

		unsigned char d[SHA256_DIGEST_LEN];
		char out[SEMA_STR_LEN];

		sha256_final(&ctx, d);
		sema_format(d, out);
		printf("%s  %s\n", out, argv[i]);
	}
	return rc;
}

/* ---- publish ------------------------------------------------------------ */

static void credentials_path(char *out, size_t cap)
{
	const char *xdg = getenv("XDG_CONFIG_HOME");
	const char *home = getenv("HOME");

	out[0] = '\0';
	if (xdg && *xdg)
		snprintf(out, cap, "%s/oboe/credentials", xdg);
	else if (home && *home)
		snprintf(out, cap, "%s/.oboe/credentials", home);
}

/* A token for host:port. OBOE_TOKEN overrides the file, which is how CI
   publishes and how the test suite avoids touching $HOME. */
static char *find_token(const char *host, int port)
{
	const char *env = getenv("OBOE_TOKEN");

	if (env && *env)
		return strdup(env);

	char path[3072];

	credentials_path(path, sizeof path);
	if (!path[0])
		return NULL;

	struct stat st;

	if (stat(path, &st) != 0)
		return NULL;

	/* refused rather than read: a credentials file others can read is a
	   problem to tell the user about, not to quietly work around */
	if (st.st_mode & (S_IRGRP | S_IROTH | S_IWGRP | S_IWOTH)) {
		fprintf(stderr,
			"oboe: %s is readable by other users; chmod 600 it\n",
			path);
		return NULL;
	}

	char *body = slurp(path, NULL);

	if (!body)
		return NULL;

	char want[512];

	snprintf(want, sizeof want, "katare://%s:%d", host, port);

	char *found = NULL;

	for (char *line = body, *nl; line && *line && !found; line = nl) {
		nl = strchr(line, '\n');
		if (nl)
			*nl++ = '\0';

		char *sp = strchr(line, ' ');

		if (!sp)
			continue;
		*sp = '\0';
		if (strcmp(line, want) == 0)
			found = strdup(sp + 1);
	}
	free(body);
	return found;
}

struct filelist {
	char **path;
	int n, cap;
};

static bool fl_add(struct filelist *f, const char *path)
{
	if (f->n == f->cap) {
		int cap = f->cap ? f->cap * 2 : 64;
		char **p = realloc(f->path, (size_t)cap * sizeof *p);

		if (!p)
			return false;
		f->path = p;
		f->cap = cap;
	}
	f->path[f->n] = strdup(path);
	return f->path[f->n++] != NULL;
}

static void fl_free(struct filelist *f)
{
	for (int i = 0; i < f->n; i++)
		free(f->path[i]);
	free(f->path);
}

/* Patterns from .oboeignore, in order; a `!` prefix un-ignores. */
struct ignores {
	char **pat;
	int n, cap;
};

static void ignores_load(struct ignores *ig)
{
	memset(ig, 0, sizeof *ig);

	char *body = slurp(".oboeignore", NULL);

	if (!body)
		return;

	for (char *line = body, *nl; line && *line; line = nl) {
		nl = strchr(line, '\n');
		if (nl)
			*nl++ = '\0';

		while (*line == ' ' || *line == '\t')
			line++;
		size_t n = strlen(line);

		while (n > 0 && (line[n - 1] == ' ' || line[n - 1] == '\r'))
			line[--n] = '\0';
		if (!*line || *line == '#')
			continue;

		if (ig->n == ig->cap) {
			int cap = ig->cap ? ig->cap * 2 : 16;
			char **p = realloc(ig->pat, (size_t)cap * sizeof *p);

			if (!p)
				break;
			ig->pat = p;
			ig->cap = cap;
		}
		ig->pat[ig->n++] = strdup(line);
	}
	free(body);
}

static void ignores_free(struct ignores *ig)
{
	for (int i = 0; i < ig->n; i++)
		free(ig->pat[i]);
	free(ig->pat);
}

static bool ignored(const struct ignores *ig, const char *rel, bool is_dir)
{
	bool out = false;
	const char *base = strrchr(rel, '/');

	base = base ? base + 1 : rel;

	/* later patterns win, so a ! can rescue something an earlier line
	   excluded */
	for (int i = 0; i < ig->n; i++) {
		const char *p = ig->pat[i];
		bool negate = *p == '!';

		if (negate)
			p++;

		/* A trailing slash means "directories only", as in .gitignore.
		   The directory itself is offered here without one, so the
		   pattern has to lose it before matching -- otherwise the whole
		   idiom silently matches nothing. */
		char trimmed[512];
		size_t plen = strlen(p);

		if (plen > 0 && p[plen - 1] == '/') {
			if (!is_dir || plen - 1 >= sizeof trimmed)
				continue;
			memcpy(trimmed, p, plen - 1);
			trimmed[plen - 1] = '\0';
			p = trimmed;
		}

		if (fnmatch(p, rel, 0) == 0 || fnmatch(p, base, 0) == 0)
			out = !negate;
	}
	return out;
}

/* Excluded always, with no way to turn it off: build output, version control,
   editor droppings, and this toolchain's own working directory. */
static bool always_excluded(const char *name, bool toplevel)
{
	/* Tool state rather than source, and unwanted at any depth: a vendored
	   subdirectory's .git is never something to publish. */
	if (strcmp(name, ".git") == 0 || strcmp(name, ".oboe") == 0)
		return true;

	/* Build output -- but only where a build puts it. Excluding `dist` at
	   any depth would silently drop a package's own src/dist/, producing an
	   archive that is well-formed, reproducible, and missing source. */
	if (toplevel && strcmp(name, "dist") == 0)
		return true;

	if (strcmp(name, ".DS_Store") == 0)
		return true;
	if (fnmatch("*.o", name, 0) == 0 || fnmatch("*.kate-swp", name, 0) == 0)
		return true;
	/* a dotfile at the root is configuration for something else */
	if (toplevel && name[0] == '.' && strcmp(name, ".oboeignore") != 0)
		return true;
	return false;
}

static bool walk(const char *dir, const char *prefix, struct filelist *out,
		 const struct ignores *ig, char **err)
{
	DIR *d = opendir(dir[0] ? dir : ".");

	if (!d) {
		char msg[512];

		snprintf(msg, sizeof msg, "cannot read %s", dir);
		*err = strdup(msg);
		return false;
	}

	struct dirent *e;
	bool ok = true;

	while (ok && (e = readdir(d))) {
		if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0)
			continue;
		if (always_excluded(e->d_name, prefix[0] == '\0'))
			continue;

		char rel[1024], full[3072];

		snprintf(rel, sizeof rel, "%.500s%.400s", prefix, e->d_name);
		snprintf(full, sizeof full, "%s/%s", dir[0] ? dir : ".",
			 e->d_name);

		struct stat st;

		/* lstat, not stat: a symlink must be seen as itself so it can
		   be refused rather than silently followed */
		if (lstat(full, &st) != 0)
			continue;

		if (S_ISLNK(st.st_mode)) {
			char msg[512];

			snprintf(
				msg, sizeof msg,
				"%.400s is a symlink, which a kabuk cannot represent",
				rel);
			*err = strdup(msg);
			ok = false;
			break;
		}

		if (S_ISDIR(st.st_mode)) {
			char sub[1024];

			snprintf(sub, sizeof sub, "%.1000s/", rel);
			if (ignored(ig, rel, true))
				continue;
			ok = walk(full, sub, out, ig, err);
			continue;
		}

		if (!S_ISREG(st.st_mode)) {
			char msg[512];

			snprintf(
				msg, sizeof msg,
				"%.400s is not a regular file, which a kabuk cannot represent",
				rel);
			*err = strdup(msg);
			ok = false;
			break;
		}

		if (ignored(ig, rel, false))
			continue;
		if (!fl_add(out, rel)) {
			*err = strdup("out of memory");
			ok = false;
		}
	}
	closedir(d);
	return ok;
}

int cmd_publish(int argc, char **argv)
{
	struct getopts o = { 0 };
	const char *unused = NULL;
	bool dry_run = false;

	/* filtered out before the shared parser sees it, which keeps that
	   parser common to get, install and publish */
	char *filtered[64];
	int nf = 0;

	for (int i = 0;
	     i < argc && nf < (int)(sizeof filtered / sizeof *filtered); i++) {
		if (strcmp(argv[i], "--dry-run") == 0)
			dry_run = true;
		else
			filtered[nf++] = argv[i];
	}
	if (!parse_common(nf, filtered, &o, &unused))
		return 2;

	const char *pf = find_project_file();

	if (!pf) {
		fprintf(stderr,
			"oboe: no project.jsonc here; 'publish' needs a project\n");
		return 1;
	}

	char *json = slurp(pf, NULL);

	if (!json) {
		fprintf(stderr, "oboe: cannot read %s\n", pf);
		return 1;
	}

	char *proj = json_extract_object(json, "project");
	const char *scope = proj ? proj : json;
	char *name = json_get_string(scope, "name");
	char *ver = json_get_string(scope, "version");

	int rc = 1;

	if (!name || !izim_valid(name)) {
		fprintf(stderr,
			"oboe: project.name must be a valid package name (lowercase letters, digits and _)\n");
		goto out;
	}
	if (izim_reserved(name)) {
		fprintf(stderr, "oboe: '%s' is a reserved name\n", name);
		goto out;
	}
	if (!ver || !waktanimra_valid(ver)) {
		fprintf(stderr,
			"oboe: project.version must be MAJOR.MINOR.PATCH[-tag]\n");
		goto out;
	}

	struct ignores ig;
	struct filelist fl;
	char *err = NULL;

	ignores_load(&ig);
	memset(&fl, 0, sizeof fl);

	if (!walk("", "", &fl, &ig, &err)) {
		fprintf(stderr, "oboe: %s\n",
			err ? err : "cannot read the tree");
		free(err);
		ignores_free(&ig);
		fl_free(&fl);
		goto out;
	}
	ignores_free(&ig);

	if (fl.n == 0) {
		fprintf(stderr, "oboe: nothing to publish\n");
		fl_free(&fl);
		goto out;
	}

	/* byte order, not the locale's: a locale-sensitive sort would produce a
	   different archive, and so a different digest, on another machine */
	qsort(fl.path, (size_t)fl.n, sizeof *fl.path, kabuk_path_cmp);

	struct kabuk_file *files = calloc((size_t)fl.n, sizeof *files);

	if (!files) {
		fl_free(&fl);
		goto out;
	}
	for (int i = 0; i < fl.n; i++)
		files[i].path = fl.path[i];

	if (!mkdirs(".oboe/tmp")) {
		fprintf(stderr, "oboe: cannot create .oboe/tmp\n");
		free(files);
		fl_free(&fl);
		goto out;
	}

	char kpath[3072];

	snprintf(kpath, sizeof kpath, ".oboe/tmp/%.*s-%s.kabuk", IZIM_MAX, name,
		 ver);

	FILE *kf = fopen(kpath, "wb");

	if (!kf) {
		fprintf(stderr, "oboe: cannot write %s\n", kpath);
		free(files);
		fl_free(&fl);
		goto out;
	}

	bool packed = kabuk_write(kf, files, (size_t)fl.n, &err);

	fclose(kf);
	free(files);

	if (!packed) {
		fprintf(stderr, "oboe: %s\n", err ? err : "cannot pack");
		free(err);
		unlink(kpath);
		fl_free(&fl);
		goto out;
	}

	size_t klen = 0;
	char *kbody = slurp(kpath, &klen);

	unlink(kpath);
	if (!kbody) {
		fprintf(stderr, "oboe: cannot read back the archive\n");
		fl_free(&fl);
		goto out;
	}

	unsigned char digest[SHA256_DIGEST_LEN];
	char sema[SEMA_STR_LEN];

	sha256(kbody, klen, digest);
	sema_format(digest, sema);

	if (o.verbose || dry_run)
		for (int i = 0; i < fl.n; i++)
			printf("  %s\n", fl.path[i]);
	fl_free(&fl);

	printf("%s %s: %zu octets, %s\n", name, ver, klen, sema);

	if (dry_run) {
		free(kbody);
		rc = 0;
		goto out;
	}

	const char *uri = resolve_registry(o.registry);
	char host[256];
	int port = 0;

	err = NULL;
	if (!kat_parse_uri(uri, host, sizeof host, &port, NULL, 0, &err)) {
		fprintf(stderr, "oboe: %s\n", err ? err : "bad registry URL");
		free(err);
		free(kbody);
		goto out;
	}

	char *token = find_token(host, port);

	if (!token) {
		fprintf(stderr,
			"oboe: no token for %s:%d; set OBOE_TOKEN or add a line to your credentials file\n",
			host, port);
		free(kbody);
		goto out;
	}

	struct kat_conn *c = kat_connect(host, port, &err);

	if (!c) {
		fprintf(stderr, "oboe: %s\n", err ? err : "cannot connect");
		free(err);
		free(token);
		free(kbody);
		goto out;
	}

	if (klen > kat_body_cap(c)) {
		fprintf(stderr,
			"oboe: the archive is %zu octets; %s accepts at most %llu\n",
			klen, host, kat_body_cap(c));
		kat_close(c);
		free(token);
		free(kbody);
		goto out;
	}

	enum kat_status st = kat_request(c, "kalit %s", token);

	free(token);
	if (st != KAT_SI) {
		fprintf(stderr, "oboe: %s rejected the token\n", host);
		kat_close(c);
		free(kbody);
		goto out;
	}
	if (o.verbose)
		printf("Authenticated as %s\n",
		       kat_word(c, 1) ? kat_word(c, 1) : "?");

	/* The line and its body are one message. Sending the line and waiting
	   for a status would deadlock: the server cannot answer until it has
	   read the octets it was just told to expect. */
	if (!kat_send_line(c, "kango %s %s %s kyx %zu", name, ver, sema,
			   klen) ||
	    !kat_send_body(c, kbody, klen, &err)) {
		fprintf(stderr, "oboe: %s\n", err ? err : "upload failed");
		free(err);
		kat_close(c);
		free(kbody);
		goto out;
	}
	free(kbody);

	st = kat_read_status(c);
	if (st != KAT_SI) {
		if (st == KAT_SENTYRE)
			fprintf(stderr,
				"oboe: %s %s is already published; versions are immutable\n",
				name, ver);
		else if (st == KAT_EZHAZEBYR)
			fprintf(stderr, "oboe: not authorised to publish %s\n",
				name);
		else
			fprintf(stderr,
				"oboe: registry refused the publish: %s %s\n",
				kat_status_word(st), kat_reason(c));
		kat_close(c);
		goto out;
	}

	printf("Published %s %s\n", name, ver);
	kat_close(c);
	rc = 0;

out:
	free(name);
	free(ver);
	free(proj);
	free(json);
	return rc;
}
