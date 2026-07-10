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
#include "lexer.h"
#include "parser.h"
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
    char *src = read_whole_file(oboe_path);
    if (!src) {
        fprintf(stderr, "oboe: cannot read '%s'\n", oboe_path);
        exit(1);
    }
    int tok_count;
    Token *toks = lex_all(src, &tok_count);
    Program *prog = parse_program(toks, tok_count, oboe_path);

    char path_copy[4096];
    strncpy(path_copy, oboe_path, sizeof(path_copy) - 1);
    path_copy[sizeof(path_copy) - 1] = '\0';
    char *dir = dirname(path_copy);
    codegen_set_source_dir(dir);

    FILE *out = fopen(c_out_path, "w");
    if (!out) { fprintf(stderr, "oboe: cannot write '%s'\n", c_out_path); exit(1); }
    codegen_program(prog, out);
    fclose(out);
    free(src);
    return strdup(c_out_path);
}

static int compile_c_to_binary(const char *c_path, const char *out_path) {
    char *home = oboe_home();
    char cmd[8192];
    snprintf(cmd, sizeof cmd,
             "gcc -std=c11 -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE -O2 -I\"%s/../runtime\" \"%s\" \"%s/../runtime/oboe_runtime.c\" -o \"%s\" 2>&1",
             home, c_path, home, out_path);
    FILE *p = popen(cmd, "r");
    if (!p) return 1;
    char line[1024];
    while (fgets(line, sizeof line, p)) fputs(line, stdout);
    int status = pclose(p);
    free(home);
    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
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

    int rc = compile_c_to_binary(c_path, bin_path);
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

static void cmd_init(void) {
    struct stat st;
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

static void cmd_build(bool consolidate) {
    (void)consolidate; /* the reference implementation always produces a single executable for now */
    char *json = read_whole_file("project.json");
    char *entry = NULL;
    char *name = NULL;
    if (json) {
        entry = json_extract_string_field(json, "entry");
        name = json_extract_string_field(json, "name");
        free(json);
    }
    if (!entry) entry = strdup("main.oboe");
    if (!name) name = strdup("program");

    mkdir("dist", 0755);
    char c_path[] = "/tmp/oboe_build_XXXXXX.c";
    int fd = mkstemps(c_path, 2);
    if (fd < 0) { perror("mkstemps"); exit(1); }
    close(fd);
    transpile_to_c(entry, c_path);

    char out_path[4096];
    snprintf(out_path, sizeof out_path, "dist/%s", name);
    int rc = compile_c_to_binary(c_path, out_path);
    remove(c_path);
    if (rc != 0) exit(rc);
    printf("Built dist/%s\n", name);
}

static void cmd_tidy(void) {
    mkdir(".oboe", 0755);
    mkdir(".oboe/libraries", 0755);
    printf("Cleaned build artifacts. No package registry is configured yet, so no dependencies were fetched.\n");
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
    if (strcmp(cmd, "init") == 0) { cmd_init(); return 0; }
    if (strcmp(cmd, "run") == 0) {
        if (argc >= 3) cmd_run_file(argv[2]);
        else cmd_run_project();
        return 0;
    }
    if (strcmp(cmd, "build") == 0) {
        bool consolidate = false;
        for (int i = 2; i < argc; i++)
            if (strcmp(argv[i], "--consolidate") == 0 || strcmp(argv[i], "-c") == 0) consolidate = true;
        cmd_build(consolidate);
        return 0;
    }
    if (strcmp(cmd, "tidy") == 0) { cmd_tidy(); return 0; }
    if (strcmp(cmd, "get") == 0) { cmd_get_or_install("get", argc >= 3 ? argv[2] : NULL); return 0; }
    if (strcmp(cmd, "install") == 0) { cmd_get_or_install("install", argc >= 3 ? argv[2] : NULL); return 0; }

    fprintf(stderr, "oboe: unknown command '%s'\n", cmd);
    return 1;
}
