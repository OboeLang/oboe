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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <libgen.h>

static char *read_whole_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(sz + 1);
    fread(buf, 1, sz, f);
    buf[sz] = '\0';
    fclose(f);
    return buf;
}

/* directory containing this executable, used to find runtime/oboe_runtime.{h,c} */
static char *oboe_home(void) {
    char buf[4096];
    ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (n < 0) { fprintf(stderr, "oboe: cannot locate oboe installation\n"); exit(1); }
    buf[n] = '\0';
    char *dir = strdup(dirname(buf));
    return dir;
}

static char *transpile_to_c(const char *oboe_path, const char *c_out_path) {
    char path_copy[4096];
    strncpy(path_copy, oboe_path, sizeof(path_copy) - 1);
    path_copy[sizeof(path_copy) - 1] = '\0';
    char *dir = dirname(path_copy);
    codegen_set_source_dir(dir);

    FILE *out = fopen(c_out_path, "w");
    if (!out) { fprintf(stderr, "oboe: cannot write '%s'\n", c_out_path); exit(1); }
    codegen_compile(oboe_path, out);
    fclose(out);
    return strdup(c_out_path);
}

static bool tool_exists(const char *tool) {
    char cmd[512];
    snprintf(cmd, sizeof cmd, "command -v %s >/dev/null 2>&1", tool);
    return system(cmd) == 0;
}

/* extra_obj: an additional object file to link (Windows resource), or NULL.
   `cc` defaults to the host gcc; -ldl is skipped for Windows targets. */
static int compile_c_to_binary(const char *c_path, const char *out_path, bool verbose,
                               const char *cc, const char *extra_obj, bool is_windows) {
    char *home = oboe_home();
    char cmd[8192];
    snprintf(cmd, sizeof cmd,
             "%s -std=c11 -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE -O2 -I\"%s/../runtime\" \"%s\" \"%s/../runtime/oboe_runtime.c\"%s%s%s%s -o \"%s\" 2>&1",
             cc ? cc : "gcc", home, c_path, home,
             extra_obj ? " \"" : "", extra_obj ? extra_obj : "", extra_obj ? "\"" : "",
             is_windows ? "" : " -ldl", out_path);
    if (verbose) printf("oboe: %s\n", cmd);
    FILE *p = popen(cmd, "r");
    if (!p) return 1;
    char line[1024];
    while (fgets(line, sizeof line, p)) fputs(line, stdout);
    int status = pclose(p);
    free(home);
    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}

/* mkdir -p: creates every missing directory along the path */
static void mkdirs(const char *path) {
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

static void cmd_run_file(const char *path) {
    char c_path[] = "/tmp/oboe_XXXXXX.c";
    int fd = mkstemps(c_path, 2);
    if (fd < 0) { perror("mkstemps"); exit(1); }
    close(fd);
    transpile_to_c(path, c_path);

    char bin_path[] = "/tmp/oboe_bin_XXXXXX";
    int bfd = mkstemp(bin_path);
    if (bfd < 0) { perror("mkstemp"); exit(1); }
    close(bfd);

    int rc = compile_c_to_binary(c_path, bin_path, false, NULL, NULL, false);
    remove(c_path);
    if (rc != 0) { remove(bin_path); exit(rc); }

    int run_rc = system(bin_path);
    remove(bin_path);
    exit(WIFEXITED(run_rc) ? WEXITSTATUS(run_rc) : 1);
}

/* Extracts a top-level string field like "entry": "main.oboe" from project.json.
   project.json's shape is fixed for this project (see project.example.json), so a
   minimal targeted scan is used instead of a full JSON parser. */
static char *json_extract_string_field(const char *json, const char *field) {
    char needle[256];
    snprintf(needle, sizeof needle, "\"%s\"", field);
    const char *p = strstr(json, needle);
    if (!p) return NULL;
    p = strchr(p + strlen(needle), ':');
    if (!p) return NULL;
    p++;
    while (*p == ' ' || *p == '\t' || *p == '\n') p++;
    if (*p != '"') return NULL;
    p++;
    const char *end = strchr(p, '"');
    if (!end) return NULL;
    return strndup(p, end - p);
}

static void cmd_init(const char *dir) {
    struct stat st;
    if (dir) {
        if (stat(dir, &st) == 0) {
            if (!S_ISDIR(st.st_mode)) {
                fprintf(stderr, "oboe: '%s' exists and is not a directory\n", dir);
                exit(1);
            }
        } else {
            mkdirs(dir);
        }
        if (chdir(dir) != 0) {
            fprintf(stderr, "oboe: cannot enter directory '%s'\n", dir);
            exit(1);
        }
    }
    if (stat("main.oboe", &st) == 0) {
        fprintf(stderr, "oboe: main.oboe already exists; refusing to overwrite an existing project\n");
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

    FILE *pj = fopen("project.json", "w");
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
        "}\n", base);
    fclose(pj);
    free(base);

    printf("Initialized an Oboe project in the current directory.\n");
}

static void cmd_run_project(void) {
    char *json = read_whole_file("project.json");
    if (!json) {
        fprintf(stderr, "oboe: no project.json found (run 'oboe init' first, or pass a file to 'oboe run')\n");
        exit(1);
    }
    char *entry = json_extract_string_field(json, "entry");
    if (!entry) entry = strdup("main.oboe");
    free(json);
    cmd_run_file(entry);
}

/* minimal check for `"field": true` in project.json, same spirit as the
   string-field scan above */
static bool json_extract_bool_field(const char *json, const char *field) {
    char needle[256];
    snprintf(needle, sizeof needle, "\"%s\"", field);
    const char *p = strstr(json, needle);
    if (!p) return false;
    p = strchr(p + strlen(needle), ':');
    if (!p) return false;
    p++;
    while (*p == ' ' || *p == '\t' || *p == '\n') p++;
    return strncmp(p, "true", 4) == 0;
}

#if defined(_WIN32)
#define HOST_OS "windows"
#elif defined(__APPLE__)
#define HOST_OS "macos"
#else
#define HOST_OS "linux"
#endif

/* accepts the preferred names plus the kernel-flavored aliases */
static const char *normalize_target(const char *t) {
    if (!t) return HOST_OS;
    if (strcmp(t, "linux") == 0) return "linux";
    if (strcmp(t, "windows") == 0 || strcmp(t, "nt") == 0) return "windows";
    if (strcmp(t, "macos") == 0 || strcmp(t, "darwin") == 0 || strcmp(t, "osx") == 0) return "macos";
    fprintf(stderr, "oboe: unknown build target '%s' (expected linux, windows or macos)\n", t);
    exit(1);
}

typedef struct {
    const char *file;        /* explicit script, or NULL for the project entry */
    const char *output;      /* -o override */
    const char *target;      /* normalized target OS */
    const char *cc;          /* --cc override */
    bool verbose;
    bool desktop;            /* generate a .desktop file next to the output */
    const char *meta_name;
    const char *meta_version;
    const char *meta_description;
    const char *meta_icon;
} BuildOpts;

/* Embeds ProductName/ProductVersion/FileDescription into a Windows build via
   windres, when available. Returns the path of the compiled resource object,
   or NULL if windres is missing (a note is printed) or nothing to embed. */
static char *build_windows_resource(const BuildOpts *o) {
    if (!o->meta_name && !o->meta_version && !o->meta_description) return NULL;
    const char *windres = "x86_64-w64-mingw32-windres";
    if (!tool_exists(windres)) {
        printf("oboe: note: %s not found; skipping Windows version metadata\n", windres);
        return NULL;
    }
    int v[4] = {0, 0, 0, 0};
    if (o->meta_version) sscanf(o->meta_version, "%d.%d.%d.%d", &v[0], &v[1], &v[2], &v[3]);
    char rc_path[] = "/tmp/oboe_res_XXXXXX.rc";
    int fd = mkstemps(rc_path, 3);
    if (fd < 0) return NULL;
    FILE *rc = fdopen(fd, "w");
    fprintf(rc,
        "1 VERSIONINFO\n"
        "FILEVERSION %d,%d,%d,%d\n"
        "PRODUCTVERSION %d,%d,%d,%d\n"
        "BEGIN\n"
        "  BLOCK \"StringFileInfo\"\n"
        "  BEGIN\n"
        "    BLOCK \"040904E4\"\n"
        "    BEGIN\n", v[0], v[1], v[2], v[3], v[0], v[1], v[2], v[3]);
    if (o->meta_name) fprintf(rc, "      VALUE \"ProductName\", \"%s\"\n", o->meta_name);
    if (o->meta_description) fprintf(rc, "      VALUE \"FileDescription\", \"%s\"\n", o->meta_description);
    if (o->meta_version) fprintf(rc, "      VALUE \"ProductVersion\", \"%s\"\n", o->meta_version);
    fprintf(rc,
        "    END\n"
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
    snprintf(cmd, sizeof cmd, "%s \"%s\" -O coff -o \"%s\"", windres, rc_path, obj_path);
    int rcode = system(cmd);
    remove(rc_path);
    if (rcode != 0) return NULL;
    return obj_path;
}

static void write_desktop_file(const BuildOpts *o, const char *out_path, const char *name) {
    char desk_path[4096];
    char dir[4096];
    strncpy(dir, out_path, sizeof(dir) - 1);
    dir[sizeof(dir) - 1] = '\0';
    char *d = dirname(dir);
    snprintf(desk_path, sizeof desk_path, "%s/%s.desktop", d, name);
    char abs[4096];
    if (!realpath(out_path, abs)) snprintf(abs, sizeof abs, "%s", out_path);
    FILE *f = fopen(desk_path, "w");
    if (!f) { fprintf(stderr, "oboe: cannot write '%s'\n", desk_path); return; }
    fprintf(f, "[Desktop Entry]\nType=Application\nName=%s\nExec=%s\n", name, abs);
    if (o->meta_description) fprintf(f, "Comment=%s\n", o->meta_description);
    if (o->meta_icon) fprintf(f, "Icon=%s\n", o->meta_icon);
    fprintf(f, "Terminal=false\n");
    fclose(f);
    printf("Wrote %s\n", desk_path);
}

/* `oboe build` builds the project entry into dist/<name>;
   `oboe build <file>` builds a single script into ./<file-without-.oboe>;
   `-o path` overrides the output location, creating missing directories. */
static void cmd_build(BuildOpts *o) {
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
        if (dot && strcmp(dot, ".oboe") == 0) *dot = '\0';
        snprintf(out_path, sizeof out_path, "%s", b);
    } else {
        char *json = read_whole_file("project.json");
        char *name = NULL;
        if (json) {
            entry = json_extract_string_field(json, "entry");
            name = json_extract_string_field(json, "name");
            /* project.json may carry build settings; the CLI flags win */
            if (!o->target) {
                char *t = json_extract_string_field(json, "target");
                if (t) o->target = t;
            }
            if (!o->output) {
                char *op = json_extract_string_field(json, "output");
                if (op) output = op;
            }
            if (!o->desktop) o->desktop = json_extract_bool_field(json, "desktop");
            if (!o->meta_name) o->meta_name = name ? strdup(name) : NULL;
            if (!o->meta_version) o->meta_version = json_extract_string_field(json, "version");
            if (!o->meta_description) o->meta_description = json_extract_string_field(json, "description");
            if (!o->meta_icon) o->meta_icon = json_extract_string_field(json, "icon");
            free(json);
        }
        if (!entry) entry = strdup("main.oboe");
        if (!name) name = strdup("program");
        snprintf(out_path, sizeof out_path, "dist/%s", name);
        if (!o->meta_name) o->meta_name = strdup(name);
        free(name);
    }

    const char *target = normalize_target(o->target);
    o->target = target;
    bool is_windows = strcmp(target, "windows") == 0;
    codegen_set_target_os(target);

    /* pick the compiler: --cc wins, else a per-target default */
    const char *cc = o->cc;
    if (!cc) {
        if (strcmp(target, HOST_OS) == 0) cc = "gcc";
        else if (is_windows) cc = "x86_64-w64-mingw32-gcc";
        else if (strcmp(target, "macos") == 0) cc = "o64-clang";
        else cc = "gcc";
    }
    if (!tool_exists(cc)) {
        fprintf(stderr, "oboe: compiler '%s' for target '%s' not found", cc, target);
        if (is_windows) fprintf(stderr, " (install mingw-w64, or pass --cc)");
        else if (strcmp(target, "macos") == 0) fprintf(stderr, " (install osxcross, or pass --cc)");
        fprintf(stderr, "\n");
        exit(1);
    }

    if (output) {
        snprintf(out_path, sizeof out_path, "%s", output);
        char parent[4096];
        strncpy(parent, output, sizeof(parent) - 1);
        parent[sizeof(parent) - 1] = '\0';
        char *dir = dirname(parent);
        if (strcmp(dir, ".") != 0) mkdirs(dir);
    }
    if (is_windows) {
        size_t len = strlen(out_path);
        if (len < 4 || strcmp(out_path + len - 4, ".exe") != 0)
            snprintf(out_path + len, sizeof(out_path) - len, ".exe");
    }

    char c_path[] = "/tmp/oboe_build_XXXXXX.c";
    int fd = mkstemps(c_path, 2);
    if (fd < 0) { perror("mkstemps"); exit(1); }
    close(fd);
    /* transpile before creating dist/ so a failed build leaves nothing behind */
    transpile_to_c(entry, c_path);
    if (verbose) printf("oboe: transpiled %s (target: %s)\n", entry, target);

    char *res_obj = NULL;
    if (is_windows) res_obj = build_windows_resource(o);
    else if (o->meta_version || o->meta_description) {
        if (strcmp(target, "macos") == 0)
            printf("oboe: note: macOS metadata needs an .app bundle, which isn't generated yet\n");
    }

    bool made_dist = !file && !output;
    if (made_dist) mkdir("dist", 0755);
    int rc = compile_c_to_binary(c_path, out_path, verbose, cc, res_obj, is_windows);
    remove(c_path);
    if (res_obj) remove(res_obj);
    if (rc != 0) {
        if (made_dist) rmdir("dist"); /* only removes it if empty */
        exit(rc);
    }
    printf("Built %s\n", out_path);

    if (o->desktop) {
        if (strcmp(target, "linux") == 0)
            write_desktop_file(o, out_path, o->meta_name ? o->meta_name : "program");
        else
            printf("oboe: note: .desktop files only apply to Linux targets; skipped\n");
    }
    free(entry);
}

static void cmd_tidy(bool verbose) {
    struct stat st;
    if (stat("project.json", &st) != 0) {
        /* not a project directory: tidy does nothing */
        if (verbose) printf("oboe: no project.json here; nothing to tidy\n");
        return;
    }
    mkdir(".oboe", 0755);
    mkdir(".oboe/libraries", 0755);
    if (verbose) printf("oboe: ensured .oboe/libraries exists\n");
    printf("Cleaned build artifacts. No package repository is configured yet, so no dependencies were fetched.\n");
}

static void cmd_get_or_install(const char *what, const char *name) {
    fprintf(stderr, "oboe: '%s %s' is not yet implemented — there is no package registry to fetch from yet.\n", what, name ? name : "");
    exit(1);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: oboe <init|run|build|tidy|get|install> [args]\n");
        return 1;
    }
    const char *cmd = argv[1];
    if (strcmp(cmd, "init") == 0) { cmd_init(argc >= 3 ? argv[2] : NULL); return 0; }
    if (strcmp(cmd, "run") == 0) {
        if (argc >= 3) cmd_run_file(argv[2]);
        else cmd_run_project();
        return 0;
    }
    if (strcmp(cmd, "build") == 0) {
        BuildOpts o = {0};
        for (int i = 2; i < argc; i++) {
            const char *a = argv[i];
            const char **takes_value =
                (strcmp(a, "-o") == 0 || strcmp(a, "--output") == 0) ? &o.output :
                (strcmp(a, "-t") == 0 || strcmp(a, "--target") == 0) ? &o.target :
                (strcmp(a, "--cc") == 0) ? &o.cc :
                (strcmp(a, "--meta-name") == 0) ? &o.meta_name :
                (strcmp(a, "--meta-version") == 0) ? &o.meta_version :
                (strcmp(a, "--meta-description") == 0) ? &o.meta_description :
                (strcmp(a, "--meta-icon") == 0) ? &o.meta_icon : NULL;
            if (takes_value) {
                if (i + 1 >= argc) { fprintf(stderr, "oboe: %s requires a value\n", a); return 1; }
                *takes_value = argv[++i];
            }
            else if (strcmp(a, "-v") == 0 || strcmp(a, "--verbose") == 0) o.verbose = true;
            else if (strcmp(a, "--desktop") == 0) o.desktop = true;
            else if (a[0] == '-') { fprintf(stderr, "oboe: unknown build flag '%s'\n", a); return 1; }
            else o.file = a;
        }
        cmd_build(&o);
        return 0;
    }
    if (strcmp(cmd, "tidy") == 0) {
        bool verbose = false;
        for (int i = 2; i < argc; i++)
            if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) verbose = true;
        cmd_tidy(verbose);
        return 0;
    }
    if (strcmp(cmd, "get") == 0) { cmd_get_or_install("get", argc >= 3 ? argv[2] : NULL); return 0; }
    if (strcmp(cmd, "install") == 0) { cmd_get_or_install("install", argc >= 3 ? argv[2] : NULL); return 0; }

    fprintf(stderr, "oboe: unknown command '%s'\n", cmd);
    return 1;
}
