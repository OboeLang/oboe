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
#include "codegen.h"
#include "projectjson.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <libgen.h>

static char *read_whole_file(const char *path)
{
	FILE *f = fopen(path, "rb");
	if (!f)
		return NULL;
	fseek(f, 0, SEEK_END);
	long sz = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *buf = malloc(sz + 1);
	fread(buf, 1, sz, f);
	buf[sz] = '\0';
	fclose(f);
	return buf;
}

/* The project config file: `project.jsonc` is the current name (it always
   supported `//` comments despite older docs saying `.json`), `project.json`
   is kept as a fallback so projects written before the rename still work.
   Returns NULL when neither exists. */
static const char *find_project_json(void)
{
	struct stat st;
	if (stat("project.jsonc", &st) == 0)
		return "project.jsonc";
	if (stat("project.json", &st) == 0)
		return "project.json";
	return NULL;
}

/* directory containing this executable, used to find runtime/oboe_runtime.{h,c} */
static char *oboe_home(void)
{
	char buf[4096];
	ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
	if (n < 0) {
		fprintf(stderr, "oboe: cannot locate oboe installation\n");
		exit(1);
	}
	buf[n] = '\0';
	char *dir = strdup(dirname(buf));
	return dir;
}

static char *transpile_to_c(const char *oboe_path, const char *c_out_path)
{
	char path_copy[4096];
	strncpy(path_copy, oboe_path, sizeof(path_copy) - 1);
	path_copy[sizeof(path_copy) - 1] = '\0';
	char *dir = dirname(path_copy);
	codegen_set_source_dir(dir);

	FILE *out = fopen(c_out_path, "w");
	if (!out) {
		fprintf(stderr, "oboe: cannot write '%s'\n", c_out_path);
		exit(1);
	}
	codegen_compile(oboe_path, out);
	fclose(out);
	return strdup(c_out_path);
}

static bool tool_exists(const char *tool)
{
	char cmd[512];
	snprintf(cmd, sizeof cmd, "command -v %s >/dev/null 2>&1", tool);
	return system(cmd) == 0;
}

/* extra_obj: an additional object file to link (Windows resource), or NULL.
   `cc` defaults to the host gcc; -ldl is skipped for Windows targets. */
static int compile_c_to_binary(const char *c_path, const char *out_path,
			       bool verbose, const char *cc,
			       const char *extra_obj, bool is_windows)
{
	char *home = oboe_home();
	char cmd[8192];
	snprintf(
		cmd, sizeof cmd,
		"%s -std=c11 -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE -O2 -I\"%s/../runtime\" \"%s\" \"%s/../runtime/oboe_runtime.c\"%s%s%s%s -lm -o \"%s\" 2>&1",
		cc ? cc : "gcc", home, c_path, home, extra_obj ? " \"" : "",
		extra_obj ? extra_obj : "", extra_obj ? "\"" : "",
		is_windows ? "" : " -ldl", out_path);
	if (verbose)
		printf("oboe: %s\n", cmd);
	FILE *p = popen(cmd, "r");
	if (!p)
		return 1;
	char line[1024];
	while (fgets(line, sizeof line, p))
		fputs(line, stdout);
	int status = pclose(p);
	free(home);
	return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}

/* mkdir -p: creates every missing directory along the path */
static void mkdirs(const char *path)
{
	char buf[4096];
	strncpy(buf, path, sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';
	for (char *p = buf + 1; *p; p++) {
		if (*p == '/') {
			*p = '\0';
			mkdir(buf, 0755);
			*p = '/';
		}
	}
	mkdir(buf, 0755);
}

static void cmd_run_file(const char *path)
{
	char c_path[] = "/tmp/oboe_XXXXXX.c";
	int fd = mkstemps(c_path, 2);
	if (fd < 0) {
		perror("mkstemps");
		exit(1);
	}
	close(fd);
	transpile_to_c(path, c_path);

	char bin_path[] = "/tmp/oboe_bin_XXXXXX";
	int bfd = mkstemp(bin_path);
	if (bfd < 0) {
		perror("mkstemp");
		exit(1);
	}
	close(bfd);

	int rc =
		compile_c_to_binary(c_path, bin_path, false, NULL, NULL, false);
	remove(c_path);
	if (rc != 0) {
		remove(bin_path);
		exit(rc);
	}

	int run_rc = system(bin_path);
	remove(bin_path);
	exit(WIFEXITED(run_rc) ? WEXITSTATUS(run_rc) : 1);
}

static void cmd_init(const char *dir)
{
	struct stat st;
	if (dir) {
		if (stat(dir, &st) == 0) {
			if (!S_ISDIR(st.st_mode)) {
				fprintf(stderr,
					"oboe: '%s' exists and is not a directory\n",
					dir);
				exit(1);
			}
		} else {
			mkdirs(dir);
		}
		if (chdir(dir) != 0) {
			fprintf(stderr, "oboe: cannot enter directory '%s'\n",
				dir);
			exit(1);
		}
	}
	if (stat("main.oboe", &st) == 0) {
		fprintf(stderr,
			"oboe: main.oboe already exists; refusing to overwrite an existing project\n");
		exit(1);
	}
	FILE *f = fopen("main.oboe", "w");
	fprintf(f, "func main(array args) {\n    print(\"Hello, Oboe!\")\n}\n");
	fclose(f);

	mkdir(".oboe", 0755);
	mkdir(".oboe/libraries", 0755);

	if (stat(".gitignore", &st) != 0) {
		FILE *gi = fopen(".gitignore", "w");
		fprintf(gi, "dist/\n.oboe/\n");
		fclose(gi);
	}

	char cwd[4096];
	getcwd(cwd, sizeof cwd);
	char *base = strdup(basename(cwd));

	FILE *pj = fopen("project.jsonc", "w");
	fprintf(pj,
		"{\n"
		"    \"project\": {\n"
		"        \"name\": \"%s\",\n"
		"        \"version\": \"1.0.0\",\n"
		"        \"entry\": \"main.oboe\"\n"
		"    },\n"
		"    \"dependencies\": {\n"
		"        \"oboe\": \">=1.0.0\"\n"
		"    }\n"
		"}\n",
		base);
	fclose(pj);
	free(base);

	printf("Initialized an Oboe project in the current directory.\n");
}

static void cmd_run_project(void)
{
	const char *path = find_project_json();
	char *json = path ? read_whole_file(path) : NULL;
	if (!json) {
		fprintf(stderr,
			"oboe: no project.jsonc found (run 'oboe init' first, or pass a file to 'oboe run')\n");
		exit(1);
	}
	char *entry = json_extract_string_field(json, "entry");
	if (!entry)
		entry = strdup("main.oboe");
	free(json);
	cmd_run_file(entry);
}

#if defined(_WIN32)
#define HOST_OS "windows"
#elif defined(__APPLE__)
#define HOST_OS "macos"
#else
#define HOST_OS "linux"
#endif

/* Every OS a build can target. The name is what `-t` accepts and what an
   OS-specific module file is suffixed with (`foo.freebsd.oboe`); `cc` is the
   default compiler when cross-compiling to it, always overridable with --cc. */
static const struct {
	const char *name;
	const char *alias;
	const char *cc;
} k_targets[] = { { "linux", NULL, "gcc" },
		  { "windows", "nt", "x86_64-w64-mingw32-gcc" },
		  { "macos", "darwin", "o64-clang" },
		  { "freebsd", NULL, "clang" },
		  { "openbsd", NULL, "clang" },
		  { "netbsd", NULL, "clang" },
		  { NULL, NULL, NULL } };

static const char *normalize_target(const char *t)
{
	if (!t)
		return HOST_OS;
	if (strcmp(t, "osx") == 0)
		return "macos"; /* a third macOS spelling */
	for (int i = 0; k_targets[i].name; i++)
		if (strcmp(t, k_targets[i].name) == 0 ||
		    (k_targets[i].alias && strcmp(t, k_targets[i].alias) == 0))
			return k_targets[i].name;
	fprintf(stderr, "oboe: unknown build target '%s' (expected one of:", t);
	for (int i = 0; k_targets[i].name; i++)
		fprintf(stderr, " %s", k_targets[i].name);
	fprintf(stderr, ")\n");
	exit(1);
}

static const char *default_cc_for(const char *target)
{
	if (strcmp(target, HOST_OS) == 0)
		return "gcc";
	for (int i = 0; k_targets[i].name; i++)
		if (strcmp(target, k_targets[i].name) == 0)
			return k_targets[i].cc;
	return "gcc";
}

typedef struct {
	const char *file; /* explicit script, or NULL for the project entry */
	const char *output; /* -o override */
	const char *
		config; /* -t as written: a project.json target name, or an OS */
	const char *target; /* normalized target OS */
	const char *cc; /* --cc override */
	bool verbose;
	bool desktop; /* generate a .desktop file next to the output */
	const char *meta_name;
	const char *meta_version;
	const char *meta_description;
	const char *meta_icon;
} BuildOpts;

/* Embeds ProductName/ProductVersion/FileDescription into a Windows build via
   windres, when available. Returns the path of the compiled resource object,
   or NULL if windres is missing (a note is printed) or nothing to embed. */
static char *build_windows_resource(const BuildOpts *o)
{
	if (!o->meta_name && !o->meta_version && !o->meta_description)
		return NULL;
	const char *windres = "x86_64-w64-mingw32-windres";
	if (!tool_exists(windres)) {
		printf("oboe: note: %s not found; skipping Windows version metadata\n",
		       windres);
		return NULL;
	}
	int v[4] = { 0, 0, 0, 0 };
	if (o->meta_version)
		sscanf(o->meta_version, "%d.%d.%d.%d", &v[0], &v[1], &v[2],
		       &v[3]);
	char rc_path[] = "/tmp/oboe_res_XXXXXX.rc";
	int fd = mkstemps(rc_path, 3);
	if (fd < 0)
		return NULL;
	FILE *rc = fdopen(fd, "w");
	fprintf(rc,
		"1 VERSIONINFO\n"
		"FILEVERSION %d,%d,%d,%d\n"
		"PRODUCTVERSION %d,%d,%d,%d\n"
		"BEGIN\n"
		"  BLOCK \"StringFileInfo\"\n"
		"  BEGIN\n"
		"    BLOCK \"040904E4\"\n"
		"    BEGIN\n",
		v[0], v[1], v[2], v[3], v[0], v[1], v[2], v[3]);
	if (o->meta_name)
		fprintf(rc, "      VALUE \"ProductName\", \"%s\"\n",
			o->meta_name);
	if (o->meta_description)
		fprintf(rc, "      VALUE \"FileDescription\", \"%s\"\n",
			o->meta_description);
	if (o->meta_version)
		fprintf(rc, "      VALUE \"ProductVersion\", \"%s\"\n",
			o->meta_version);
	fprintf(rc, "    END\n"
		    "  END\n"
		    "  BLOCK \"VarFileInfo\"\n"
		    "  BEGIN\n"
		    "    VALUE \"Translation\", 0x409, 1252\n"
		    "  END\n"
		    "END\n");
	fclose(rc);
	static char obj_path[4096];
	snprintf(obj_path, sizeof obj_path, "%s.o", rc_path);
	char cmd[8192];
	snprintf(cmd, sizeof cmd, "%s \"%s\" -O coff -o \"%s\"", windres,
		 rc_path, obj_path);
	int rcode = system(cmd);
	remove(rc_path);
	if (rcode != 0)
		return NULL;
	return obj_path;
}

static void write_desktop_file(const BuildOpts *o, const char *out_path,
			       const char *name)
{
	char desk_path[4096];
	char dir[4096];
	strncpy(dir, out_path, sizeof(dir) - 1);
	dir[sizeof(dir) - 1] = '\0';
	char *d = dirname(dir);
	snprintf(desk_path, sizeof desk_path, "%s/%s.desktop", d, name);
	char abs[4096];
	if (!realpath(out_path, abs))
		snprintf(abs, sizeof abs, "%s", out_path);
	FILE *f = fopen(desk_path, "w");
	if (!f) {
		fprintf(stderr, "oboe: cannot write '%s'\n", desk_path);
		return;
	}
	fprintf(f, "[Desktop Entry]\nType=Application\nName=%s\nExec=%s\n",
		name, abs);
	if (o->meta_description)
		fprintf(f, "Comment=%s\n", o->meta_description);
	if (o->meta_icon)
		fprintf(f, "Icon=%s\n", o->meta_icon);
	fprintf(f, "Terminal=false\n");
	fclose(f);
	printf("Wrote %s\n", desk_path);
}

/* Wraps the built executable in a macOS .app bundle:

       <dir>/<name>.app/Contents/Info.plist
                                /MacOS/<name>      the executable, moved here
                                /Resources/<icon>  when --meta-icon was given

   This is what `--desktop` means on a macOS target, mirroring the .desktop file
   it writes on Linux. The executable is moved rather than copied, so the build
   still produces exactly one artifact. */
static void write_app_bundle(const BuildOpts *o, const char *out_path,
			     const char *name)
{
	char dir[4096];
	strncpy(dir, out_path, sizeof(dir) - 1);
	dir[sizeof(dir) - 1] = '\0';
	char *d = dirname(dir);

	/* sized above the 4096-byte `dir` they are built from, so appending the
       fixed bundle path components can never truncate */
	char contents[4200], macos[4300], resources[4300];
	snprintf(contents, sizeof contents, "%s/%s.app/Contents", d, name);
	snprintf(macos, sizeof macos, "%s/MacOS", contents);
	snprintf(resources, sizeof resources, "%s/Resources", contents);
	mkdirs(macos);
	mkdirs(resources);

	char exe[4600];
	snprintf(exe, sizeof exe, "%s/%s", macos, name);
	remove(exe);
	if (rename(out_path, exe) != 0) {
		fprintf(stderr, "oboe: cannot move '%s' into the .app bundle\n",
			out_path);
		return;
	}

	/* a bundle identifier must be reverse-DNS-ish; keep only safe characters */
	char ident[256];
	size_t n = 0;
	for (const char *c = name; *c && n < sizeof(ident) - 1; c++)
		if ((*c >= 'a' && *c <= 'z') || (*c >= 'A' && *c <= 'Z') ||
		    (*c >= '0' && *c <= '9'))
			ident[n++] = *c;
	ident[n] = '\0';
	if (n == 0)
		snprintf(ident, sizeof ident, "app");

	char *icon_base = NULL;
	if (o->meta_icon) {
		char icon_copy[4096];
		strncpy(icon_copy, o->meta_icon, sizeof(icon_copy) - 1);
		icon_copy[sizeof(icon_copy) - 1] = '\0';
		icon_base = strdup(basename(icon_copy));
		char cmd[8192];
		snprintf(cmd, sizeof cmd, "cp \"%s\" \"%s/%s\" 2>/dev/null",
			 o->meta_icon, resources, icon_base);
		if (system(cmd) != 0)
			printf("oboe: note: could not copy icon '%s' into the bundle\n",
			       o->meta_icon);
	}

	char plist[4400];
	snprintf(plist, sizeof plist, "%s/Info.plist", contents);
	FILE *f = fopen(plist, "w");
	if (!f) {
		fprintf(stderr, "oboe: cannot write '%s'\n", plist);
		free(icon_base);
		return;
	}
	fprintf(f,
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" "
		"\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
		"<plist version=\"1.0\">\n<dict>\n"
		"    <key>CFBundleName</key><string>%s</string>\n"
		"    <key>CFBundleExecutable</key><string>%s</string>\n"
		"    <key>CFBundleIdentifier</key><string>com.oboe.%s</string>\n"
		"    <key>CFBundlePackageType</key><string>APPL</string>\n"
		"    <key>CFBundleInfoDictionaryVersion</key><string>6.0</string>\n",
		name, name, ident);
	if (o->meta_version)
		fprintf(f,
			"    <key>CFBundleShortVersionString</key><string>%s</string>\n",
			o->meta_version);
	if (o->meta_description)
		fprintf(f,
			"    <key>NSHumanReadableCopyright</key><string>%s</string>\n",
			o->meta_description);
	if (icon_base)
		fprintf(f,
			"    <key>CFBundleIconFile</key><string>%s</string>\n",
			icon_base);
	fprintf(f, "</dict>\n</plist>\n");
	fclose(f);
	free(icon_base);
	printf("Wrote %s/%s.app\n", d, name);
}

static void take_string(const char **slot, const char *obj, const char *field)
{
	if (*slot || !obj)
		return;
	char *v = json_get_string(obj, field);
	if (v)
		*slot = v;
}

/* Metadata is read from a `meta` object; the older flat `meta-xxxx` keys are
   still honored as a fallback, so project.json files written before the change
   keep working without a warning. */
static void take_meta(const char **slot, const char *obj, const char *key)
{
	if (*slot || !obj)
		return;
	char *meta = json_extract_object(obj, "meta");
	if (meta) {
		char *v = json_get_string(meta, key);
		free(meta);
		if (v) {
			*slot = v;
			return;
		}
	}
	char flat[128];
	snprintf(flat, sizeof flat, "meta-%s", key);
	take_string(slot, obj, flat);
}

/* Layers project.json build settings under the CLI flags, most specific first:
   `build.targets.<config>`, then `build`. Anything already set (from the
   command line, which is parsed first) is left alone. */
static void load_build_settings(BuildOpts *o, const char *json,
				const char *config)
{
	char *build = json_extract_object(json, "build");
	char *cfg = NULL;
	if (config && build) {
		char path[512];
		snprintf(path, sizeof path, "targets.%s", config);
		cfg = json_extract_object(build, path);
	}
	const char *layers[] = { cfg, build };
	for (int i = 0; i < 2; i++) {
		if (!layers[i])
			continue;
		take_string(&o->target, layers[i], "target");
		take_string(&o->output, layers[i], "output");
		take_string(&o->cc, layers[i], "compiler");
		if (!o->desktop)
			o->desktop = json_get_bool(layers[i], "desktop");
		take_meta(&o->meta_name, layers[i], "name");
		take_meta(&o->meta_version, layers[i], "version");
		take_meta(&o->meta_description, layers[i], "description");
		take_meta(&o->meta_icon, layers[i], "icon");
	}
	/* a named target with no `target` field of its own targets the OS it is
       named after, which is why `"windows": {}` is a usable declaration */
	if (!o->target && config)
		o->target = strdup(config);

	char *project = json_extract_object(json, "project");
	if (project) {
		take_string(&o->meta_name, project, "name");
		take_string(&o->meta_version, project, "version");
		take_string(&o->meta_description, project, "description");
		free(project);
	}
	free(cfg);
	free(build);
}

/* `oboe build` builds the project entry into dist/<name>;
   `oboe build <file>` builds a single script into ./<file-without-.oboe>;
   `-o path` overrides the output location, creating missing directories. */
static void cmd_build(BuildOpts *o)
{
	const char *file = o->file;
	const char *output = o->output;
	bool verbose = o->verbose;
	char *entry = NULL;
	char out_path[4096];

	if (file) {
		entry = strdup(file);
		char base[4096];
		strncpy(base, file, sizeof(base) - 1);
		base[sizeof(base) - 1] = '\0';
		char *b = basename(base);
		char *dot = strrchr(b, '.');
		if (dot && strcmp(dot, ".oboe") == 0)
			*dot = '\0';
		snprintf(out_path, sizeof out_path, "%s", b);
	} else {
		const char *path = find_project_json();
		char *json = path ? read_whole_file(path) : NULL;
		char *name = NULL;
		if (json) {
			entry = json_extract_string_field(json, "entry");
			char *project = json_extract_object(json, "project");
			if (project) {
				name = json_get_string(project, "name");
				free(project);
			}
			load_build_settings(o, json, o->config);
			output = o->output;
			free(json);
		}
		if (!entry)
			entry = strdup("main.oboe");
		if (!name)
			name = strdup("program");
		if (!output)
			snprintf(out_path, sizeof out_path, "dist/%s", name);
		if (!o->meta_name)
			o->meta_name = strdup(name);
		free(name);
	}

	/* -t names a project.json target config when there is one, and otherwise is
       just an OS name — load_build_settings has already resolved the first case */
	if (!o->target)
		o->target = o->config;
	const char *target = normalize_target(o->target);
	o->target = target;
	bool is_windows = strcmp(target, "windows") == 0;
	codegen_set_target_os(target);

	/* pick the compiler: --cc wins, else a per-target default */
	const char *cc = o->cc ? o->cc : default_cc_for(target);
	if (!tool_exists(cc)) {
		fprintf(stderr, "oboe: compiler '%s' for target '%s' not found",
			cc, target);
		if (is_windows)
			fprintf(stderr, " (install mingw-w64, or pass --cc)");
		else if (strcmp(target, "macos") == 0)
			fprintf(stderr, " (install osxcross, or pass --cc)");
		else
			fprintf(stderr,
				" (install a cross-compiler for it, or pass --cc)");
		fprintf(stderr, "\n");
		exit(1);
	}

	if (output) {
		snprintf(out_path, sizeof out_path, "%s", output);
		char parent[4096];
		strncpy(parent, output, sizeof(parent) - 1);
		parent[sizeof(parent) - 1] = '\0';
		char *dir = dirname(parent);
		if (strcmp(dir, ".") != 0)
			mkdirs(dir);
	}
	if (is_windows) {
		size_t len = strlen(out_path);
		if (len < 4 || strcmp(out_path + len - 4, ".exe") != 0)
			snprintf(out_path + len, sizeof(out_path) - len,
				 ".exe");
	}

	char c_path[] = "/tmp/oboe_build_XXXXXX.c";
	int fd = mkstemps(c_path, 2);
	if (fd < 0) {
		perror("mkstemps");
		exit(1);
	}
	close(fd);
	/* transpile before creating dist/ so a failed build leaves nothing behind */
	transpile_to_c(entry, c_path);
	if (verbose)
		printf("oboe: transpiled %s (target: %s)\n", entry, target);

	/* macOS carries its metadata in an .app bundle's Info.plist instead of in
       the executable, so it is written by write_app_bundle under --desktop */
	char *res_obj = is_windows ? build_windows_resource(o) : NULL;

	bool made_dist = !file && !output;
	if (made_dist)
		mkdir("dist", 0755);
	int rc = compile_c_to_binary(c_path, out_path, verbose, cc, res_obj,
				     is_windows);
	remove(c_path);
	if (res_obj)
		remove(res_obj);
	if (rc != 0) {
		if (made_dist)
			rmdir("dist"); /* only removes it if empty */
		exit(rc);
	}
	printf("Built %s\n", out_path);

	/* --desktop asks for a desktop-installable artifact for this target: a
       .desktop launcher on Linux, an .app bundle on macOS */
	if (o->desktop) {
		const char *app_name = o->meta_name ? o->meta_name : "program";
		if (strcmp(target, "linux") == 0)
			write_desktop_file(o, out_path, app_name);
		else if (strcmp(target, "macos") == 0)
			write_app_bundle(o, out_path, app_name);
		else
			printf("oboe: note: --desktop has no meaning for the '%s' target; skipped\n",
			       target);
	}
	free(entry);
}

/* Target names declared under `build.targets` in project.json, in file order.
   `oboe build` with no -t builds every one of them; with no `targets` object it
   builds once with the plain `build` settings, as it always has. */
static char **declared_targets(int *out_count)
{
	*out_count = 0;
	const char *path = find_project_json();
	char *json = path ? read_whole_file(path) : NULL;
	if (!json)
		return NULL;
	char *targets = json_extract_object(json, "build.targets");
	free(json);
	if (!targets)
		return NULL;
	char **names = json_object_keys(targets, out_count);
	free(targets);
	return names;
}

/* `oboe build <file>` is always a single script. Otherwise a project build runs
   once per declared target, each with its own settings layered under the CLI
   flags — so the per-target fields must be re-resolved from a clean copy of the
   options every time round rather than accumulating across iterations. */
static void cmd_build_all(const BuildOpts *base)
{
	if (base->file || base->config) {
		BuildOpts o = *base;
		cmd_build(&o);
		return;
	}
	int count = 0;
	char **names = declared_targets(&count);
	if (count == 0) {
		free(names);
		BuildOpts o = *base;
		cmd_build(&o);
		return;
	}
	for (int i = 0; i < count; i++) {
		BuildOpts o = *base;
		o.config = names[i];
		printf("=== %s ===\n", names[i]);
		cmd_build(&o);
		free(names[i]);
	}
	free(names);
}

static void cmd_tidy(bool verbose)
{
	if (!find_project_json()) {
		/* not a project directory: tidy does nothing */
		if (verbose)
			printf("oboe: no project.jsonc here; nothing to tidy\n");
		return;
	}
	mkdir(".oboe", 0755);
	mkdir(".oboe/libraries", 0755);
	if (verbose)
		printf("oboe: ensured .oboe/libraries exists\n");
	printf("Cleaned build artifacts. No package repository is configured yet, so no dependencies were fetched.\n");
}

/* rm -rf, for a package directory under .oboe/libraries */
static void remove_tree(const char *path)
{
	char cmd[8192];
	snprintf(cmd, sizeof cmd, "rm -rf \"%s\"", path);
	if (system(cmd) != 0)
		fprintf(stderr, "oboe: could not remove '%s'\n", path);
}

/* Rewrites project.json with `pkg`'s dependency line dropped, leaving every
   other byte alone — a line-oriented edit rather than a reserialize, since the
   file is hand-written and keeping its formatting and comments matters. A
   trailing comma left dangling on the previous entry is cleaned up. */
static bool remove_dependency_line(const char *pkg)
{
	const char *path = find_project_json();
	char *json = path ? read_whole_file(path) : NULL;
	if (!json)
		return false;
	char needle[256];
	snprintf(needle, sizeof needle, "\"%s\"", pkg);

	char *out = malloc(strlen(json) + 1);
	size_t n = 0;
	bool removed = false;
	for (char *line = json; *line;) {
		char *eol = strchr(line, '\n');
		size_t len = eol ? (size_t)(eol - line) + 1 : strlen(line);
		char *hit = strstr(line, needle);
		bool is_dep_line = hit && hit < line + len &&
				   strchr(line, ':') > hit;
		if (is_dep_line && !removed) {
			removed = true;
		} else {
			memcpy(out + n, line, len);
			n += len;
		}
		line += len;
	}
	out[n] = '\0';

	/* Dropping the last entry of an object leaves the previous one ending in a
       comma before the closing brace; strip any such dangling comma. Commas
       inside strings are skipped so a value like "a,b" is never touched. */
	if (removed) {
		bool in_string = false;
		for (char *p = out; *p; p++) {
			if (in_string) {
				if (*p == '\\' && p[1])
					p++;
				else if (*p == '"')
					in_string = false;
				continue;
			}
			if (*p == '"') {
				in_string = true;
				continue;
			}
			if (*p != ',')
				continue;
			char *q = p + 1;
			while (*q == ' ' || *q == '\t' || *q == '\n' ||
			       *q == '\r')
				q++;
			if (*q == '}' || *q == ']') {
				memmove(p, p + 1, strlen(p));
				p--;
			}
		}
	}

	if (removed) {
		FILE *f = fopen(path, "w");
		if (!f) {
			fprintf(stderr, "oboe: cannot write %s\n", path);
			free(json);
			free(out);
			return false;
		}
		fputs(out, f);
		fclose(f);
	}
	free(json);
	free(out);
	return removed;
}

/* Undoes `oboe get`: drops the package's files from .oboe/libraries and its
   entry from project.json. Works on hand-placed libraries too, which is all
   there is until a package registry exists. */
static void cmd_remove(const char *name)
{
	if (!name) {
		fprintf(stderr, "oboe: 'remove' needs a package name\n");
		exit(1);
	}
	struct stat st;
	if (!find_project_json()) {
		fprintf(stderr,
			"oboe: no project.jsonc here; 'remove' only works inside a project\n");
		exit(1);
	}

	bool any = false;
	char path[4096];
	snprintf(path, sizeof path, ".oboe/libraries/%s.oboe", name);
	if (remove(path) == 0) {
		printf("Removed %s\n", path);
		any = true;
	}
	for (int i = 0; k_targets[i].name; i++) {
		snprintf(path, sizeof path, ".oboe/libraries/%s.%s.oboe", name,
			 k_targets[i].name);
		if (remove(path) == 0) {
			printf("Removed %s\n", path);
			any = true;
		}
	}
	snprintf(path, sizeof path, ".oboe/libraries/%s", name);
	if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
		remove_tree(path);
		printf("Removed %s/\n", path);
		any = true;
	}

	if (remove_dependency_line(name)) {
		printf("Removed \"%s\" from project.jsonc dependencies.\n",
		       name);
		any = true;
	}
	if (!any)
		printf("oboe: nothing to remove for '%s'.\n", name);
}

static void cmd_get_or_install(const char *what, const char *name)
{
	fprintf(stderr,
		"oboe: '%s %s' is not yet implemented — there is no package registry to fetch from yet.\n",
		what, name ? name : "");
	exit(1);
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr,
			"usage: oboe <init|run|build|tidy|get|install|remove> [args]\n");
		return 1;
	}
	const char *cmd = argv[1];
	if (strcmp(cmd, "init") == 0) {
		cmd_init(argc >= 3 ? argv[2] : NULL);
		return 0;
	}
	if (strcmp(cmd, "run") == 0) {
		if (argc >= 3)
			cmd_run_file(argv[2]);
		else
			cmd_run_project();
		return 0;
	}
	if (strcmp(cmd, "build") == 0) {
		BuildOpts o = { 0 };
		for (int i = 2; i < argc; i++) {
			const char *a = argv[i];
			const char **takes_value =
				(strcmp(a, "-o") == 0 ||
				 strcmp(a, "--output") == 0) ?
					&o.output :
				(strcmp(a, "-t") == 0 ||
				 strcmp(a, "--target") == 0) ?
					&o.config :
				(strcmp(a, "--cc") == 0) ? &o.cc :
				(strcmp(a, "--meta-name") == 0) ?
							   &o.meta_name :
				(strcmp(a, "--meta-version") == 0) ?
							   &o.meta_version :
				(strcmp(a, "--meta-description") == 0) ?
							   &o.meta_description :
				(strcmp(a, "--meta-icon") == 0) ? &o.meta_icon :
								  NULL;
			if (takes_value) {
				if (i + 1 >= argc) {
					fprintf(stderr,
						"oboe: %s requires a value\n",
						a);
					return 1;
				}
				*takes_value = argv[++i];
			} else if (strcmp(a, "-v") == 0 ||
				   strcmp(a, "--verbose") == 0)
				o.verbose = true;
			else if (strcmp(a, "--desktop") == 0)
				o.desktop = true;
			else if (a[0] == '-') {
				fprintf(stderr,
					"oboe: unknown build flag '%s'\n", a);
				return 1;
			} else
				o.file = a;
		}
		cmd_build_all(&o);
		return 0;
	}
	if (strcmp(cmd, "tidy") == 0) {
		bool verbose = false;
		for (int i = 2; i < argc; i++)
			if (strcmp(argv[i], "-v") == 0 ||
			    strcmp(argv[i], "--verbose") == 0)
				verbose = true;
		cmd_tidy(verbose);
		return 0;
	}
	if (strcmp(cmd, "remove") == 0) {
		cmd_remove(argc >= 3 ? argv[2] : NULL);
		return 0;
	}
	if (strcmp(cmd, "get") == 0) {
		cmd_get_or_install("get", argc >= 3 ? argv[2] : NULL);
		return 0;
	}
	if (strcmp(cmd, "install") == 0) {
		cmd_get_or_install("install", argc >= 3 ? argv[2] : NULL);
		return 0;
	}

	fprintf(stderr, "oboe: unknown command '%s'\n", cmd);
	return 1;
}
