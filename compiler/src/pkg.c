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

#include <errno.h>
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
	if (!find_project_file()) {
		fprintf(stderr,
			"oboe: no project.jsonc here; '%s' only works inside a project\n",
			as_tool ? "install" : "get");
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

int cmd_publish(int argc, char **argv)
{
	fprintf(stderr, "oboe: 'publish' is not wired up yet\n");
	return 1;
}
