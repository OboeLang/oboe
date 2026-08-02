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
 *
 * Prepended to bootstrap/oboec.c by `make regen-bootstrap`, which also rewrites
 * the generated HOST_OS constant to use it. selfhost/hostos.oboe is a per-OS
 * module, so without this the OS chosen on the generating machine would be
 * frozen into a file every other machine then builds -- a macOS oboec that
 * silently targets Linux, which is what this is fixing.
 *
 * This is only the default for a bare `oboec`; bin/oboe always passes
 * --target-os. The arms match legacy/codegen.c's, BSDs included: they fall
 * through to linux there too.
 *
 * Overridable with -D so the suite can build a deliberately foreign-hosted
 * oboec and check that the default really does follow the compiler, which is
 * not otherwise observable from the host it is tested on.
 */
#ifndef OBOEC_HOST_OS
#if defined(_WIN32)
#define OBOEC_HOST_OS "windows"
#elif defined(__APPLE__)
#define OBOEC_HOST_OS "macos"
#else
#define OBOEC_HOST_OS "linux"
#endif
#endif
#include "oboe_runtime.h"
#include <stdlib.h>
#include <stdio.h>

static OboeValue ob_std_os_script_file(void) { return ob_string("<generated>"); }
static OboeValue ob_std_os_script_dir(void) { return ob_string("<generated>"); }
static OboeValue ob_std_os_project_root(void) { return ob_string("<generated>"); }

typedef struct Parser Parser;
struct Parser {
    OboeObject __base;
    OboeValue filename;
    OboeValue pos;
    OboeValue count;
    OboeValue toks;
};
static const OboeClassInfo Parser__classinfo = { "Parser", NULL };

OboeValue usage();
OboeValue read_source(OboeValue path);
OboeValue oboe_user_main(OboeValue args);
OboeValue lexer__byte_at(OboeValue i);
OboeValue lexer__ch(OboeValue i);
OboeValue lexer__is_digit(OboeValue c);
OboeValue lexer__is_alpha(OboeValue c);
OboeValue lexer__is_alnum(OboeValue c);
OboeValue lexer__is_space(OboeValue c);
OboeValue lexer__is_ident_start(OboeValue c);
OboeValue lexer__is_ident_char(OboeValue c);
OboeValue lexer__is_op_char(OboeValue c);
OboeValue lexer__register_custom_op(OboeValue sym);
OboeValue lexer__prescan_ops(OboeValue src);
OboeValue lexer__match_custom_op(OboeValue pos);
OboeValue lexer__tok(OboeValue type, OboeValue text, OboeValue line);
OboeValue lexer__read_string_body();
OboeValue lexer__lex_all(OboeValue src);
OboeValue diag__fail(OboeValue msg);
OboeValue diag__die(OboeValue msg);
void Parser__init_0(Parser *this, OboeValue toks, OboeValue filename);
OboeValue Parser__new_0(OboeValue toks, OboeValue filename);
OboeValue parser__peek(OboeValue p);
OboeValue parser__peek_at(OboeValue p, OboeValue off);
OboeValue parser__advance(OboeValue p);
OboeValue parser__check(OboeValue p, OboeValue t);
OboeValue parser__match(OboeValue p, OboeValue t);
OboeValue parser__fail(OboeValue p, OboeValue msg);
OboeValue parser__expect(OboeValue p, OboeValue t, OboeValue what);
OboeValue parser__new_expr(OboeValue kind, OboeValue line);
OboeValue parser__new_stmt(OboeValue kind, OboeValue line);
OboeValue parser__parse_string_literal_parts(OboeValue p, OboeValue raw);
OboeValue parser__parse_arg_list(OboeValue p, OboeValue close, OboeValue want_names);
OboeValue parser__parse_primary(OboeValue p);
OboeValue parser__parse_postfix(OboeValue p);
OboeValue parser__parse_unary(OboeValue p);
OboeValue parser__binary_op_here(OboeValue p, OboeValue types, OboeValue ops);
OboeValue parser__parse_term(OboeValue p);
OboeValue parser__parse_additive(OboeValue p);
OboeValue parser__parse_shift(OboeValue p);
OboeValue parser__parse_is(OboeValue p);
OboeValue parser__parse_comparison(OboeValue p);
OboeValue parser__parse_equality(OboeValue p);
OboeValue parser__parse_bitand(OboeValue p);
OboeValue parser__parse_bitxor(OboeValue p);
OboeValue parser__parse_bitor(OboeValue p);
OboeValue parser__parse_custom_op(OboeValue p);
OboeValue parser__parse_and(OboeValue p);
OboeValue parser__parse_or(OboeValue p);
OboeValue parser__parse_nullish(OboeValue p);
OboeValue parser__parse_ternary(OboeValue p);
OboeValue parser__parse_assignment(OboeValue p);
OboeValue parser__parse_expression(OboeValue p);
OboeValue parser__parse_typed_name(OboeValue p);
OboeValue parser__parse_params(OboeValue p);
OboeValue parser__parse_let(OboeValue p, OboeValue is_const);
OboeValue parser__parse_if(OboeValue p);
OboeValue parser__parse_while(OboeValue p);
OboeValue parser__looks_like_call_to(OboeValue p, OboeValue name);
OboeValue parser__parse_for(OboeValue p);
OboeValue parser__parse_switch(OboeValue p);
OboeValue parser__parse_try(OboeValue p);
OboeValue parser__parse_throw(OboeValue p);
OboeValue parser__parse_statement(OboeValue p);
OboeValue parser__parse_block(OboeValue p);
OboeValue parser__parse_func(OboeValue p, OboeValue is_static, OboeValue is_private);
OboeValue parser__parse_operator_decl(OboeValue p);
OboeValue parser__parse_class(OboeValue p);
OboeValue parser__parse_import(OboeValue p);
OboeValue parser__parse_program(OboeValue tokens, OboeValue filename);
OboeValue dump__oct3(OboeValue n);
OboeValue dump__dump_escape(OboeValue s);
OboeValue dump__dump_tokens(OboeValue toks);
OboeValue dump__emit(OboeValue s);
OboeValue dump__ind(OboeValue depth);
OboeValue dump__b01(OboeValue flag);
OboeValue dump__field_str(OboeValue key, OboeValue val);
OboeValue dump__field_double(OboeValue key, OboeValue value);
OboeValue dump__dump_body(OboeValue label, OboeValue body, OboeValue depth);
OboeValue dump__dump_opt(OboeValue label, OboeValue e, OboeValue depth);
OboeValue dump__dump_params(OboeValue params, OboeValue depth);
OboeValue dump__dump_expr(OboeValue e, OboeValue depth);
OboeValue dump__dump_stmt(OboeValue s, OboeValue depth);
OboeValue dump__dump_func(OboeValue label, OboeValue f, OboeValue depth);
OboeValue dump__dump_class(OboeValue c, OboeValue depth);
OboeValue dump__dump_ast(OboeValue decls);
OboeValue codegen__std_module_members(OboeValue module);
OboeValue codegen__std_member_lookup(OboeValue module, OboeValue member, OboeValue line);
OboeValue codegen__codegen_set_output_path(OboeValue path);
OboeValue codegen__emit(OboeValue s);
OboeValue codegen__flush_out();
OboeValue codegen__codegen_error(OboeValue line, OboeValue msg);
OboeValue codegen__byte_of(OboeValue s, OboeValue i);
OboeValue codegen__oct3(OboeValue n);
OboeValue codegen__escape_c_string(OboeValue s);
OboeValue codegen__path_dirname(OboeValue p);
OboeValue codegen__file_exists(OboeValue path);
OboeValue codegen__find_project_json_in(OboeValue folder);
OboeValue codegen__walk_to_project_root(OboeValue dir);
OboeValue codegen__emit_script_path_builtins(OboeValue main_path);
OboeValue codegen__find_class(OboeValue name);
OboeValue codegen__find_field_local(OboeValue c, OboeValue name);
OboeValue codegen__find_method_local(OboeValue c, OboeValue name);
OboeValue codegen__find_field_owner(OboeValue c, OboeValue name);
OboeValue codegen__find_method_owner(OboeValue c, OboeValue name);
OboeValue codegen__find_init_owner(OboeValue c);
OboeValue codegen__find_init_index(OboeValue c, OboeValue argc);
OboeValue codegen__find_init_decl(OboeValue c, OboeValue index);
OboeValue codegen__find_user_op(OboeValue sym);
OboeValue codegen__class_op_exists(OboeValue sym);
OboeValue codegen__find_event(OboeValue name);
OboeValue codegen__find_ffi(OboeValue name);
OboeValue codegen__find_known_func_in(OboeValue name, OboeValue prefix);
OboeValue codegen__find_known_func(OboeValue name);
OboeValue codegen__register_funcs(OboeValue decls, OboeValue prefix);
OboeValue codegen__push_scope();
OboeValue codegen__pop_scope();
OboeValue codegen__define_var_full(OboeValue name, OboeValue class_name, OboeValue c_name, OboeValue prim_type);
OboeValue codegen__define_var_c(OboeValue name, OboeValue class_name, OboeValue c_name);
OboeValue codegen__define_var(OboeValue name, OboeValue class_name);
OboeValue codegen__lookup_entry(OboeValue name);
OboeValue codegen__lookup_var_prim(OboeValue name);
OboeValue codegen__lookup_var_cname(OboeValue name);
OboeValue codegen__var_in_scope(OboeValue name);
OboeValue codegen__lookup_var_class(OboeValue name);
OboeValue codegen__ind(OboeValue n);
OboeValue codegen__infer_class(OboeValue e);
OboeValue codegen__gen_string_literal(OboeValue e);
OboeValue codegen__numeric_type(OboeValue name);
OboeValue codegen__apply_numeric_coercion(OboeValue type_name, OboeValue code);
OboeValue codegen__gen_builtin_type_check(OboeValue type_name, OboeValue value);
OboeValue codegen__find_builtin_method(OboeValue name);
OboeValue codegen__gen_member_access_ex(OboeValue field_expr, OboeValue for_call, OboeValue safe, OboeValue argc);
OboeValue codegen__gen_member_access(OboeValue field_expr, OboeValue for_call, OboeValue safe);
OboeValue codegen__assign_target_prim_type(OboeValue target);
OboeValue codegen__gen_assign_target_lvalue(OboeValue target);
OboeValue codegen__param_count_of(OboeValue f);
OboeValue codegen__bind_call_args(OboeValue f, OboeValue fname, OboeValue call);
OboeValue codegen__gen_bound_arg(OboeValue f, OboeValue idx, OboeValue arg);
OboeValue codegen__gen_call_ident(OboeValue e);
OboeValue codegen__gen_call_field(OboeValue e);
OboeValue codegen__gen_expr(OboeValue e);
OboeValue codegen__gen_stmt(OboeValue s, OboeValue indent);
OboeValue codegen__gen_stmt_list(OboeValue body, OboeValue indent);
OboeValue codegen__gen_param_list(OboeValue owner, OboeValue params, OboeValue skip_this);
OboeValue codegen__bind_params(OboeValue params, OboeValue skip_this);
OboeValue codegen__has_this_param(OboeValue f);
OboeValue codegen__gen_func_def(OboeValue prefix, OboeValue owner, OboeValue f);
OboeValue codegen__emit_func_prototype(OboeValue prefix, OboeValue owner, OboeValue f);
OboeValue codegen__emit_ctor_params(OboeValue params, OboeValue void_when_empty);
OboeValue codegen__emit_class_predecls(OboeValue c);
OboeValue codegen__class_index(OboeValue c);
OboeValue codegen__emit_class_struct(OboeValue c, OboeValue idx);
OboeValue codegen__gen_class(OboeValue c);
OboeValue codegen__note_field(OboeValue c, OboeValue name, OboeValue type_hint);
OboeValue codegen__field_type_hint(OboeValue value);
OboeValue codegen__scan_expr_for_fields(OboeValue c, OboeValue e);
OboeValue codegen__scan_stmt_list_for_fields(OboeValue c, OboeValue body);
OboeValue codegen__infer_instance_fields(OboeValue c);
OboeValue codegen__add_class(OboeValue c, OboeValue unit_prefix);
OboeValue codegen__collect_classes(OboeValue decls, OboeValue unit_prefix);
OboeValue codegen__read_whole_file(OboeValue path);
OboeValue codegen__codegen_set_source_dir(OboeValue dir);
OboeValue codegen__find_unit(OboeValue module);
OboeValue codegen__codegen_set_target_os(OboeValue target);
OboeValue codegen__is_json_ws(OboeValue c);
OboeValue codegen__json_skip_ws(OboeValue s, OboeValue i);
OboeValue codegen__json_string_field(OboeValue json, OboeValue field);
OboeValue codegen__json_read_string(OboeValue s, OboeValue p);
OboeValue codegen__folder_entry_path(OboeValue folder);
OboeValue codegen__folder_project_name(OboeValue folder);
OboeValue codegen__resolve_folder_module(OboeValue dir, OboeValue module);
OboeValue codegen__codegen_set_library_root(OboeValue main_path);
OboeValue codegen__resolve_module_path(OboeValue dir, OboeValue module);
OboeValue codegen__strip_for_scan(OboeValue src);
OboeValue codegen__is_ident_char(OboeValue c);
OboeValue codegen__scan_imports_textual(OboeValue src, OboeValue dir);
OboeValue codegen__load_unit_textual(OboeValue module, OboeValue src, OboeValue path, OboeValue dir);
OboeValue codegen__unit_file_for_prefix(OboeValue prefix);
OboeValue codegen__module_is_builtin(OboeValue module);
OboeValue codegen__parse_unit(OboeValue ui);
OboeValue codegen__ensure_unit(OboeValue module, OboeValue from_dir);
OboeValue codegen__resolve_imports(OboeValue ui);
OboeValue codegen__register_event_decl(OboeValue ev, OboeValue prefix);
OboeValue codegen__collect_extras(OboeValue decls, OboeValue prefix);
OboeValue codegen__handler_event_prefix(OboeValue h);
OboeValue codegen__finalize_events();
OboeValue codegen__has_kbint_handlers();
OboeValue codegen__emit_event_params(OboeValue params);
OboeValue codegen__emit_extras_predecls();
OboeValue codegen__emit_extras_defs();
OboeValue codegen__codegen_compile(OboeValue main_path);
static OboeValue lexer__SRC;
static OboeValue lexer__SLEN;
static OboeValue lexer__POS;
static OboeValue lexer__LINE;
static OboeValue lexer__OP_CHARS;
static OboeValue lexer__KEYWORDS;
static OboeValue lexer__OPS2;
static OboeValue lexer__OPS1;
static OboeValue lexer__CUSTOM_OPS;
static OboeValue lexer__BUILTIN_OPS;
static OboeValue dump__OUT;
static OboeValue codegen__CLASSES;
static OboeValue codegen__CLASS_EMITTED;
static OboeValue codegen__SOURCE_DIR;
static OboeValue codegen__IMPORT_ALIASES;
static OboeValue codegen__IMPORT_DIRECTS;
static OboeValue codegen__USER_OPS;
static OboeValue codegen__CLASS_OPS;
static OboeValue codegen__EVENTS;
static OboeValue codegen__HANDLERS;
static OboeValue codegen__FFI;
static OboeValue codegen__KNOWN_FUNCS;
static OboeValue codegen__TRY_DEPTH;
static OboeValue codegen__LOOP_TRY_DEPTH;
static OboeValue codegen__STD_MATH;
static OboeValue codegen__STD_RANDOM;
static OboeValue codegen__STD_OS;
static OboeValue codegen__OUT;
static OboeValue codegen__OUT_PATH;
static OboeValue codegen__CURRENT_FILE;
static OboeValue codegen__CURRENT_PREFIX;
static OboeValue codegen__SCOPES;
static OboeValue codegen__CURRENT_CLASS;
static OboeValue codegen__NUMERIC_TYPES;
static OboeValue codegen__BUILTIN_METHODS;
static OboeValue codegen__BINOP_FALLBACKS;
static OboeValue codegen__PRINT_FUNCS;
static OboeValue codegen__SCAN_METHOD;
static OboeValue codegen__INFERRED;
static OboeValue codegen__UNITS;
static OboeValue codegen__TARGET_OS;
static OboeValue codegen__LIB_ROOT;
static OboeValue hostos__HOST_OS;

static void __oboe_toplevel_7(void) {
}

static void __oboe_toplevel_6(void) {
    hostos__HOST_OS = ob_interpolate(1, ob_string(OBOEC_HOST_OS));
}

OboeValue codegen__std_module_members(OboeValue module) {
    if (ob_truthy(ob_binop("==", module, ob_interpolate(1, ob_string("math")), ob_eq))) {
        return codegen__STD_MATH;
    }
    if (ob_truthy(ob_binop("==", module, ob_interpolate(1, ob_string("random")), ob_eq))) {
        return codegen__STD_RANDOM;
    }
    if (ob_truthy(ob_binop("==", module, ob_interpolate(1, ob_string("os")), ob_eq))) {
        return codegen__STD_OS;
    }
    return ob_null();
    return ob_null();
}

OboeValue codegen__std_member_lookup(OboeValue module, OboeValue member, OboeValue line) {
    OboeValue tbl = codegen__std_module_members(module);
    if (ob_truthy(ob_bool(ob_truthy(ob_binop("!=", tbl, ob_null(), ob_neq)) && ob_truthy(ob_dict_has_m(tbl, member))))) {
        return ob_index_get(tbl, member);
    }
    (void)(codegen__codegen_error(line, ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("'")), module, ob_add), ob_interpolate(1, ob_string("' has no member '")), ob_add), member, ob_add), ob_interpolate(1, ob_string("'")), ob_add)));
    return ob_int(0LL);
    return ob_null();
}

OboeValue codegen__codegen_set_output_path(OboeValue path) {
    (void)((codegen__OUT_PATH = path));
    return ob_null();
}

OboeValue codegen__emit(OboeValue s) {
    (void)(ob_arr_push(codegen__OUT, s));
    return ob_null();
}

OboeValue codegen__flush_out() {
    if (ob_truthy(ob_binop("!=", codegen__OUT_PATH, ob_null(), ob_neq))) {
        (void)(ob_std_os_write_file(codegen__OUT_PATH, ob_arr_join(codegen__OUT, ob_string(""))));
    }
    else {
        (void)((ob_write(ob_arr_join(codegen__OUT, ob_string(""))), ob_null()));
    }
    (void)((codegen__OUT = ({ OboeValue __a = ob_array_new(); __a; })));
    return ob_null();
}

OboeValue codegen__codegen_error(OboeValue line, OboeValue msg) {
    (void)(codegen__flush_out());
    OboeValue where = ob_coalesce(codegen__CURRENT_FILE, ob_interpolate(1, ob_string("<input>")));
    (void)(diag__die(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", where, ob_interpolate(1, ob_string(":")), ob_add), ob_str(line), ob_add), ob_interpolate(1, ob_string(": error: ")), ob_add), msg, ob_add)));
    return ob_null();
}

OboeValue codegen__byte_of(OboeValue s, OboeValue i) {
    if (ob_truthy(ob_bool(ob_truthy(ob_binop("<", i, ob_int(0LL), ob_lt)) || ob_truthy(ob_binop(">=", i, ob_m_len(s), ob_gte))))) {
        return ob_neg(ob_int(1LL));
    }
    return ob_ord(ob_str_substr(s, i, ob_int(1LL)));
    return ob_null();
}

OboeValue codegen__oct3(OboeValue n) {
    return ob_binop("+", ob_binop("+", ob_str(ob_binop("/", n, ob_int(64LL), ob_div)), ob_str(ob_binop("%", ob_binop("/", n, ob_int(8LL), ob_div), ob_int(8LL), ob_mod)), ob_add), ob_str(ob_binop("%", n, ob_int(8LL), ob_mod)), ob_add);
    return ob_null();
}

OboeValue codegen__escape_c_string(OboeValue s) {
    OboeValue out = ({ OboeValue __a = ob_array_new(); __a; });
    OboeValue n = ob_m_len(s);
    OboeValue i = ob_int(0LL);
    while (ob_truthy(ob_binop("<", i, n, ob_lt))) {
        OboeValue ch = ob_str_substr(s, i, ob_int(1LL));
        OboeValue c = ob_ord(ch);
        if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", c, ob_int(34LL), ob_eq)) || ob_truthy(ob_binop("==", c, ob_int(92LL), ob_eq))))) {
            (void)(ob_arr_push(out, ob_binop("+", ob_interpolate(1, ob_string("\\")), ch, ob_add)));
        }
        else {
            if (ob_truthy(ob_binop("==", c, ob_int(10LL), ob_eq))) {
                (void)(ob_arr_push(out, ob_interpolate(1, ob_string("\\n"))));
            }
            else {
                if (ob_truthy(ob_binop("==", c, ob_int(13LL), ob_eq))) {
                    (void)(ob_arr_push(out, ob_interpolate(1, ob_string("\\r"))));
                }
                else {
                    if (ob_truthy(ob_binop("==", c, ob_int(9LL), ob_eq))) {
                        (void)(ob_arr_push(out, ob_interpolate(1, ob_string("\\t"))));
                    }
                    else {
                        if (ob_truthy(ob_bool(ob_truthy(ob_binop("<", c, ob_int(32LL), ob_lt)) || ob_truthy(ob_binop("==", c, ob_int(127LL), ob_eq))))) {
                            (void)(ob_arr_push(out, ob_binop("+", ob_interpolate(1, ob_string("\\")), codegen__oct3(c), ob_add)));
                        }
                        else {
                            (void)(ob_arr_push(out, ch));
                        }
                    }
                }
            }
        }
        (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
    }
    return ob_arr_join(out, ob_string(""));
    return ob_null();
}

OboeValue codegen__path_dirname(OboeValue p) {
    OboeValue n = ob_m_len(p);
    while (ob_truthy(ob_bool(ob_truthy(ob_binop(">", n, ob_int(0LL), ob_gt)) && ob_truthy(ob_binop("==", ob_str_substr(p, ob_binop("-", n, ob_int(1LL), ob_sub), ob_int(1LL)), ob_interpolate(1, ob_string("/")), ob_eq))))) {
        (void)((n = ob_binop("-", n, ob_int(1LL), ob_sub)));
    }
    if (ob_truthy(ob_binop("==", n, ob_int(0LL), ob_eq))) {
        return (ob_truthy(ob_binop(">", ob_m_len(p), ob_int(0LL), ob_gt)) ? (ob_interpolate(1, ob_string("/"))) : (ob_interpolate(1, ob_string("."))));
    }
    OboeValue i = n;
    while (ob_truthy(ob_bool(ob_truthy(ob_binop(">", i, ob_int(0LL), ob_gt)) && ob_truthy(ob_binop("!=", ob_str_substr(p, ob_binop("-", i, ob_int(1LL), ob_sub), ob_int(1LL)), ob_interpolate(1, ob_string("/")), ob_neq))))) {
        (void)((i = ob_binop("-", i, ob_int(1LL), ob_sub)));
    }
    if (ob_truthy(ob_binop("==", i, ob_int(0LL), ob_eq))) {
        return ob_interpolate(1, ob_string("."));
    }
    while (ob_truthy(ob_bool(ob_truthy(ob_binop(">", i, ob_int(0LL), ob_gt)) && ob_truthy(ob_binop("==", ob_str_substr(p, ob_binop("-", i, ob_int(1LL), ob_sub), ob_int(1LL)), ob_interpolate(1, ob_string("/")), ob_eq))))) {
        (void)((i = ob_binop("-", i, ob_int(1LL), ob_sub)));
    }
    if (ob_truthy(ob_binop("==", i, ob_int(0LL), ob_eq))) {
        return ob_interpolate(1, ob_string("/"));
    }
    return ob_str_substr(p, ob_int(0LL), i);
    return ob_null();
}

OboeValue codegen__file_exists(OboeValue path) {
    return ob_std_os_exists(path);
    return ob_null();
}

OboeValue codegen__find_project_json_in(OboeValue folder) {
    OboeValue p = ob_binop("+", folder, ob_interpolate(1, ob_string("/project.jsonc")), ob_add);
    if (ob_truthy(codegen__file_exists(p))) {
        return p;
    }
    (void)((p = ob_binop("+", folder, ob_interpolate(1, ob_string("/project.json")), ob_add)));
    return (ob_truthy(codegen__file_exists(p)) ? (p) : (ob_null()));
    return ob_null();
}

OboeValue codegen__walk_to_project_root(OboeValue dir) {
    OboeValue root = dir;
    while (ob_truthy(ob_bool(true))) {
        if (ob_truthy(ob_binop("!=", codegen__find_project_json_in(root), ob_null(), ob_neq))) {
            break;
        }
        OboeValue parent = codegen__path_dirname(root);
        if (ob_truthy(ob_binop("==", parent, root, ob_eq))) {
            (void)((root = dir));
            break;
        }
        (void)((root = parent));
    }
    return root;
    return ob_null();
}

OboeValue codegen__emit_script_path_builtins(OboeValue main_path) {
    OboeValue abs = ob_coalesce(ob_std_os_realpath(main_path), main_path);
    OboeValue dir = codegen__path_dirname(abs);
    OboeValue root = codegen__walk_to_project_root(dir);
    (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("static OboeValue ob_std_os_script_file(void) { return ob_string(\"")), codegen__escape_c_string(abs), ob_add), ob_interpolate(1, ob_string("\"); }\n")), ob_add)));
    (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("static OboeValue ob_std_os_script_dir(void) { return ob_string(\"")), codegen__escape_c_string(dir), ob_add), ob_interpolate(1, ob_string("\"); }\n")), ob_add)));
    (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("static OboeValue ob_std_os_project_root(void) { return ob_string(\"")), codegen__escape_c_string(root), ob_add), ob_interpolate(1, ob_string("\"); }\n\n")), ob_add)));
    return ob_null();
}

OboeValue codegen__find_class(OboeValue name) {
    if (ob_truthy(ob_binop("==", name, ob_null(), ob_eq))) {
        return ob_null();
    }
    { OboeValue __it = codegen__CLASSES; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue c = ob_iter_value(__it, __i);
        if (ob_truthy(ob_binop("==", ob_index_get(c, ob_interpolate(1, ob_string("name"))), name, ob_eq))) {
            return c;
        }
    } }
    return ob_null();
    return ob_null();
}

OboeValue codegen__find_field_local(OboeValue c, OboeValue name) {
    { OboeValue __it = ob_index_get(c, ob_interpolate(1, ob_string("fields"))); int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue f = ob_iter_value(__it, __i);
        if (ob_truthy(ob_binop("==", ob_index_get(f, ob_interpolate(1, ob_string("name"))), name, ob_eq))) {
            return f;
        }
    } }
    return ob_null();
    return ob_null();
}

OboeValue codegen__find_method_local(OboeValue c, OboeValue name) {
    { OboeValue __it = ob_index_get(c, ob_interpolate(1, ob_string("methods"))); int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue m = ob_iter_value(__it, __i);
        if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", ob_index_get(m, ob_interpolate(1, ob_string("name"))), name, ob_eq)) && ob_truthy(ob_binop("!=", name, ob_interpolate(1, ob_string("init")), ob_neq))))) {
            return m;
        }
    } }
    return ob_null();
    return ob_null();
}

OboeValue codegen__find_field_owner(OboeValue c, OboeValue name) {
    OboeValue cur = c;
    while (ob_truthy(ob_binop("!=", cur, ob_null(), ob_neq))) {
        OboeValue f = codegen__find_field_local(cur, name);
        if (ob_truthy(ob_binop("!=", f, ob_null(), ob_neq))) {
            return ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("owner"))), cur); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("field"))), f); __d; });
        }
        (void)((cur = (ob_truthy(ob_binop("!=", ob_index_get(cur, ob_interpolate(1, ob_string("parent_name"))), ob_null(), ob_neq)) ? (codegen__find_class(ob_index_get(cur, ob_interpolate(1, ob_string("parent_name"))))) : (ob_null()))));
    }
    return ob_null();
    return ob_null();
}

OboeValue codegen__find_method_owner(OboeValue c, OboeValue name) {
    OboeValue cur = c;
    while (ob_truthy(ob_binop("!=", cur, ob_null(), ob_neq))) {
        OboeValue m = codegen__find_method_local(cur, name);
        if (ob_truthy(ob_binop("!=", m, ob_null(), ob_neq))) {
            return ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("owner"))), cur); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("method"))), m); __d; });
        }
        (void)((cur = (ob_truthy(ob_binop("!=", ob_index_get(cur, ob_interpolate(1, ob_string("parent_name"))), ob_null(), ob_neq)) ? (codegen__find_class(ob_index_get(cur, ob_interpolate(1, ob_string("parent_name"))))) : (ob_null()))));
    }
    return ob_null();
    return ob_null();
}

OboeValue codegen__find_init_owner(OboeValue c) {
    OboeValue cur = c;
    while (ob_truthy(ob_binop("!=", cur, ob_null(), ob_neq))) {
        { OboeValue __it = ob_index_get(cur, ob_interpolate(1, ob_string("methods"))); int64_t __n = ob_iter_len(__it);
        for (int64_t __i = 0; __i < __n; __i++) {
            OboeValue m = ob_iter_value(__it, __i);
            if (ob_truthy(ob_binop("==", ob_index_get(m, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("init")), ob_eq))) {
                return cur;
            }
        } }
        (void)((cur = (ob_truthy(ob_binop("!=", ob_index_get(cur, ob_interpolate(1, ob_string("parent_name"))), ob_null(), ob_neq)) ? (codegen__find_class(ob_index_get(cur, ob_interpolate(1, ob_string("parent_name"))))) : (ob_null()))));
    }
    return ob_null();
    return ob_null();
}

OboeValue codegen__find_init_index(OboeValue c, OboeValue argc) {
    OboeValue idx = ob_int(0LL);
    OboeValue fallback = ob_neg(ob_int(1LL));
    { OboeValue __it = ob_index_get(c, ob_interpolate(1, ob_string("methods"))); int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue m = ob_iter_value(__it, __i);
        if (ob_truthy(ob_binop("!=", ob_index_get(m, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("init")), ob_neq))) {
            continue;
        }
        OboeValue total = ob_int(0LL);
        OboeValue required = ob_int(0LL);
        { OboeValue __it = ob_index_get(m, ob_interpolate(1, ob_string("params"))); int64_t __n = ob_iter_len(__it);
        for (int64_t __i = 0; __i < __n; __i++) {
            OboeValue p = ob_iter_value(__it, __i);
            if (ob_truthy(ob_binop("==", ob_index_get(p, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("this")), ob_eq))) {
                continue;
            }
            (void)((total = ob_binop("+", total, ob_int(1LL), ob_add)));
            if (ob_truthy(ob_binop("==", ob_index_get(p, ob_interpolate(1, ob_string("default_value"))), ob_null(), ob_eq))) {
                (void)((required = ob_binop("+", required, ob_int(1LL), ob_add)));
            }
        } }
        if (ob_truthy(ob_binop("==", total, argc, ob_eq))) {
            return idx;
        }
        if (ob_truthy(ob_bool(ob_truthy(ob_bool(ob_truthy(ob_binop("<", fallback, ob_int(0LL), ob_lt)) && ob_truthy(ob_binop(">=", argc, required, ob_gte)))) && ob_truthy(ob_binop("<=", argc, total, ob_lte))))) {
            (void)((fallback = idx));
        }
        (void)((idx = ob_binop("+", idx, ob_int(1LL), ob_add)));
    } }
    return fallback;
    return ob_null();
}

OboeValue codegen__find_init_decl(OboeValue c, OboeValue index) {
    OboeValue idx = ob_int(0LL);
    { OboeValue __it = ob_index_get(c, ob_interpolate(1, ob_string("methods"))); int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue m = ob_iter_value(__it, __i);
        if (ob_truthy(ob_binop("!=", ob_index_get(m, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("init")), ob_neq))) {
            continue;
        }
        if (ob_truthy(ob_binop("==", idx, index, ob_eq))) {
            return m;
        }
        (void)((idx = ob_binop("+", idx, ob_int(1LL), ob_add)));
    } }
    return ob_null();
    return ob_null();
}

OboeValue codegen__find_user_op(OboeValue sym) {
    { OboeValue __it = codegen__USER_OPS; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue u = ob_iter_value(__it, __i);
        if (ob_truthy(ob_binop("==", ob_index_get(u, ob_interpolate(1, ob_string("symbol"))), sym, ob_eq))) {
            return u;
        }
    } }
    return ob_null();
    return ob_null();
}

OboeValue codegen__class_op_exists(OboeValue sym) {
    { OboeValue __it = codegen__CLASS_OPS; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue o = ob_iter_value(__it, __i);
        if (ob_truthy(ob_binop("==", ob_index_get(o, ob_interpolate(1, ob_string("symbol"))), sym, ob_eq))) {
            return ob_bool(true);
        }
    } }
    return ob_bool(false);
    return ob_null();
}

OboeValue codegen__find_event(OboeValue name) {
    { OboeValue __it = codegen__EVENTS; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue e = ob_iter_value(__it, __i);
        if (ob_truthy(ob_binop("==", ob_index_get(ob_index_get(e, ob_interpolate(1, ob_string("decl"))), ob_interpolate(1, ob_string("name"))), name, ob_eq))) {
            return e;
        }
    } }
    return ob_null();
    return ob_null();
}

OboeValue codegen__find_ffi(OboeValue name) {
    { OboeValue __it = codegen__FFI; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue f = ob_iter_value(__it, __i);
        if (ob_truthy(ob_binop("==", ob_index_get(f, ob_interpolate(1, ob_string("name"))), name, ob_eq))) {
            return f;
        }
    } }
    return ob_null();
    return ob_null();
}

OboeValue codegen__find_known_func_in(OboeValue name, OboeValue prefix) {
    { OboeValue __it = codegen__KNOWN_FUNCS; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue k = ob_iter_value(__it, __i);
        if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", ob_index_get(k, ob_interpolate(1, ob_string("name"))), name, ob_eq)) && ob_truthy(ob_binop("==", ob_index_get(k, ob_interpolate(1, ob_string("prefix"))), prefix, ob_eq))))) {
            return k;
        }
    } }
    return ob_null();
    return ob_null();
}

OboeValue codegen__find_known_func(OboeValue name) {
    return codegen__find_known_func_in(name, codegen__CURRENT_PREFIX);
    return ob_null();
}

OboeValue codegen__register_funcs(OboeValue decls, OboeValue prefix) {
    { OboeValue __it = decls; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue d = ob_iter_value(__it, __i);
        if (ob_truthy(ob_binop("!=", ob_index_get(d, ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("DECL_FUNC")), ob_neq))) {
            continue;
        }
        (void)(ob_arr_push(codegen__KNOWN_FUNCS, ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("name"))), ob_index_get(ob_index_get(d, ob_interpolate(1, ob_string("func"))), ob_interpolate(1, ob_string("name")))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("prefix"))), prefix); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("decl"))), ob_index_get(d, ob_interpolate(1, ob_string("func")))); __d; })));
    } }
    return ob_null();
}

OboeValue codegen__push_scope() {
    (void)(ob_arr_push(codegen__SCOPES, ({ OboeValue __a = ob_array_new(); __a; })));
    return ob_null();
}

OboeValue codegen__pop_scope() {
    (void)(ob_arr_pop(codegen__SCOPES));
    return ob_null();
}

OboeValue codegen__define_var_full(OboeValue name, OboeValue class_name, OboeValue c_name, OboeValue prim_type) {
    (void)(ob_arr_push(ob_index_get(codegen__SCOPES, ob_binop("-", ob_m_len(codegen__SCOPES), ob_int(1LL), ob_sub)), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("name"))), name); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("class_name"))), class_name); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("c_name"))), c_name); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("prim_type"))), prim_type); __d; })));
    return ob_null();
}

OboeValue codegen__define_var_c(OboeValue name, OboeValue class_name, OboeValue c_name) {
    (void)(codegen__define_var_full(name, class_name, c_name, ob_null()));
    return ob_null();
}

OboeValue codegen__define_var(OboeValue name, OboeValue class_name) {
    (void)(codegen__define_var_full(name, class_name, ob_null(), ob_null()));
    return ob_null();
}

OboeValue codegen__lookup_entry(OboeValue name) {
    OboeValue si = ob_binop("-", ob_m_len(codegen__SCOPES), ob_int(1LL), ob_sub);
    while (ob_truthy(ob_binop(">=", si, ob_int(0LL), ob_gte))) {
        OboeValue entries = ob_index_get(codegen__SCOPES, si);
        OboeValue ei = ob_binop("-", ob_m_len(entries), ob_int(1LL), ob_sub);
        while (ob_truthy(ob_binop(">=", ei, ob_int(0LL), ob_gte))) {
            if (ob_truthy(ob_binop("==", ob_index_get(ob_index_get(entries, ei), ob_interpolate(1, ob_string("name"))), name, ob_eq))) {
                return ob_index_get(entries, ei);
            }
            (void)((ei = ob_binop("-", ei, ob_int(1LL), ob_sub)));
        }
        (void)((si = ob_binop("-", si, ob_int(1LL), ob_sub)));
    }
    return ob_null();
    return ob_null();
}

OboeValue codegen__lookup_var_prim(OboeValue name) {
    OboeValue e = codegen__lookup_entry(name);
    return (ob_truthy(ob_binop("==", e, ob_null(), ob_eq)) ? (ob_null()) : (ob_index_get(e, ob_interpolate(1, ob_string("prim_type")))));
    return ob_null();
}

OboeValue codegen__lookup_var_cname(OboeValue name) {
    OboeValue e = codegen__lookup_entry(name);
    if (ob_truthy(ob_binop("==", e, ob_null(), ob_eq))) {
        return name;
    }
    return ob_coalesce(ob_index_get(e, ob_interpolate(1, ob_string("c_name"))), ob_index_get(e, ob_interpolate(1, ob_string("name"))));
    return ob_null();
}

OboeValue codegen__var_in_scope(OboeValue name) {
    return ob_binop("!=", codegen__lookup_entry(name), ob_null(), ob_neq);
    return ob_null();
}

OboeValue codegen__lookup_var_class(OboeValue name) {
    OboeValue e = codegen__lookup_entry(name);
    return (ob_truthy(ob_binop("==", e, ob_null(), ob_eq)) ? (ob_null()) : (ob_index_get(e, ob_interpolate(1, ob_string("class_name")))));
    return ob_null();
}

OboeValue codegen__ind(OboeValue n) {
    (void)(codegen__emit(ob_str_repeat(ob_interpolate(1, ob_string(" ")), n)));
    return ob_null();
}

OboeValue codegen__infer_class(OboeValue e) {
    if (ob_truthy(ob_binop("==", e, ob_null(), ob_eq))) {
        return ob_null();
    }
    OboeValue k = ob_index_get(e, ob_interpolate(1, ob_string("kind")));
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_IDENT")), ob_eq))) {
        if (ob_truthy(ob_binop("==", ob_index_get(e, ob_interpolate(1, ob_string("ident"))), ob_interpolate(1, ob_string("this")), ob_eq))) {
            return codegen__CURRENT_CLASS;
        }
        return codegen__lookup_var_class(ob_index_get(e, ob_interpolate(1, ob_string("ident"))));
    }
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_CALL")), ob_eq))) {
        OboeValue callee = ob_index_get(e, ob_interpolate(1, ob_string("callee")));
        if (ob_truthy(ob_binop("==", ob_index_get(callee, ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("EXPR_IDENT")), ob_eq))) {
            OboeValue c = codegen__find_class(ob_index_get(callee, ob_interpolate(1, ob_string("ident"))));
            if (ob_truthy(ob_binop("!=", c, ob_null(), ob_neq))) {
                return ob_index_get(c, ob_interpolate(1, ob_string("name")));
            }
            OboeValue kf = codegen__find_known_func(ob_index_get(callee, ob_interpolate(1, ob_string("ident"))));
            if (ob_truthy(ob_bool(ob_truthy(ob_bool(ob_truthy(ob_binop("!=", kf, ob_null(), ob_neq)) && ob_truthy(ob_binop("!=", ob_index_get(ob_index_get(kf, ob_interpolate(1, ob_string("decl"))), ob_interpolate(1, ob_string("return_type"))), ob_null(), ob_neq)))) && ob_truthy(ob_binop("!=", codegen__find_class(ob_index_get(ob_index_get(kf, ob_interpolate(1, ob_string("decl"))), ob_interpolate(1, ob_string("return_type")))), ob_null(), ob_neq))))) {
                return ob_index_get(ob_index_get(kf, ob_interpolate(1, ob_string("decl"))), ob_interpolate(1, ob_string("return_type")));
            }
            return ob_null();
        }
        if (ob_truthy(ob_binop("==", ob_index_get(callee, ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("EXPR_FIELD")), ob_eq))) {
            OboeValue base = codegen__infer_class(ob_index_get(callee, ob_interpolate(1, ob_string("obj"))));
            OboeValue bc = (ob_truthy(ob_binop("!=", base, ob_null(), ob_neq)) ? (codegen__find_class(base)) : (ob_null()));
            if (ob_truthy(ob_binop("==", bc, ob_null(), ob_eq))) {
                return ob_null();
            }
            OboeValue got = codegen__find_method_owner(bc, ob_index_get(callee, ob_interpolate(1, ob_string("name"))));
            if (ob_truthy(ob_bool(ob_truthy(ob_bool(ob_truthy(ob_binop("!=", got, ob_null(), ob_neq)) && ob_truthy(ob_binop("!=", ob_index_get(ob_index_get(got, ob_interpolate(1, ob_string("method"))), ob_interpolate(1, ob_string("return_type"))), ob_null(), ob_neq)))) && ob_truthy(ob_binop("!=", codegen__find_class(ob_index_get(ob_index_get(got, ob_interpolate(1, ob_string("method"))), ob_interpolate(1, ob_string("return_type")))), ob_null(), ob_neq))))) {
                return ob_index_get(ob_index_get(got, ob_interpolate(1, ob_string("method"))), ob_interpolate(1, ob_string("return_type")));
            }
        }
        return ob_null();
    }
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_FIELD")), ob_eq))) {
        OboeValue base_class = codegen__infer_class(ob_index_get(e, ob_interpolate(1, ob_string("obj"))));
        if (ob_truthy(ob_binop("==", base_class, ob_null(), ob_eq))) {
            return ob_null();
        }
        OboeValue bc = codegen__find_class(base_class);
        if (ob_truthy(ob_binop("==", bc, ob_null(), ob_eq))) {
            return ob_null();
        }
        OboeValue got = codegen__find_field_owner(bc, ob_index_get(e, ob_interpolate(1, ob_string("name"))));
        if (ob_truthy(ob_binop("!=", got, ob_null(), ob_neq))) {
            return (ob_truthy(ob_binop("!=", codegen__find_class(ob_index_get(ob_index_get(got, ob_interpolate(1, ob_string("field"))), ob_interpolate(1, ob_string("type_name")))), ob_null(), ob_neq)) ? (ob_index_get(ob_index_get(got, ob_interpolate(1, ob_string("field"))), ob_interpolate(1, ob_string("type_name")))) : (ob_null()));
        }
        return ob_null();
    }
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_BINARY")), ob_eq))) {
        OboeValue lc = codegen__infer_class(ob_index_get(e, ob_interpolate(1, ob_string("l"))));
        if (ob_truthy(ob_binop("==", lc, ob_null(), ob_eq))) {
            return ob_null();
        }
        OboeValue c = codegen__find_class(lc);
        while (ob_truthy(ob_binop("!=", c, ob_null(), ob_neq))) {
            { OboeValue __it = codegen__CLASS_OPS; int64_t __n = ob_iter_len(__it);
            for (int64_t __i = 0; __i < __n; __i++) {
                OboeValue o = ob_iter_value(__it, __i);
                if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", ob_index_get(o, ob_interpolate(1, ob_string("cls"))), c, ob_eq)) && ob_truthy(ob_binop("==", ob_index_get(o, ob_interpolate(1, ob_string("symbol"))), ob_index_get(e, ob_interpolate(1, ob_string("op"))), ob_eq))))) {
                    return lc;
                }
            } }
            (void)((c = (ob_truthy(ob_binop("!=", ob_index_get(c, ob_interpolate(1, ob_string("parent_name"))), ob_null(), ob_neq)) ? (codegen__find_class(ob_index_get(c, ob_interpolate(1, ob_string("parent_name"))))) : (ob_null()))));
        }
        return ob_null();
    }
    return ob_null();
    return ob_null();
}

OboeValue codegen__gen_string_literal(OboeValue e) {
    OboeValue parts = ob_index_get(e, ob_interpolate(1, ob_string("str_parts")));
    OboeValue count = ob_m_len(parts);
    if (ob_truthy(ob_binop("==", count, ob_int(0LL), ob_eq))) {
        return ob_interpolate(1, ob_string("ob_string(\"\")"));
    }
    OboeValue b = ({ OboeValue __a = ob_array_new(); ob_array_push(__a, ob_binop("+", ob_interpolate(1, ob_string("ob_interpolate(")), ob_str(count), ob_add)); __a; });
    { OboeValue __it = parts; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue p = ob_iter_value(__it, __i);
        if (ob_truthy(ob_index_get(p, ob_interpolate(1, ob_string("is_expr"))))) {
            (void)(ob_arr_push(b, ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string(", ob_str(")), codegen__gen_expr(ob_index_get(p, ob_interpolate(1, ob_string("expr")))), ob_add), ob_interpolate(1, ob_string(")")), ob_add)));
        }
        else {
            (void)(ob_arr_push(b, ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string(", ob_string(\"")), codegen__escape_c_string(ob_index_get(p, ob_interpolate(1, ob_string("literal")))), ob_add), ob_interpolate(1, ob_string("\")")), ob_add)));
        }
    } }
    (void)(ob_arr_push(b, ob_interpolate(1, ob_string(")"))));
    return ob_arr_join(b, ob_string(""));
    return ob_null();
}

OboeValue codegen__numeric_type(OboeValue name) {
    if (ob_truthy(ob_binop("==", name, ob_null(), ob_eq))) {
        return ob_null();
    }
    return (ob_truthy(ob_dict_has_m(codegen__NUMERIC_TYPES, name)) ? (ob_index_get(codegen__NUMERIC_TYPES, name)) : (ob_null()));
    return ob_null();
}

OboeValue codegen__apply_numeric_coercion(OboeValue type_name, OboeValue code) {
    OboeValue t = codegen__numeric_type(type_name);
    if (ob_truthy(ob_binop("==", t, ob_null(), ob_eq))) {
        return code;
    }
    if (ob_truthy(ob_index_get(t, ob_interpolate(1, ob_string("is_float"))))) {
        return ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_coerce_float(")), code, ob_add), ob_interpolate(1, ob_string(", ")), ob_add), ob_str(ob_index_get(t, ob_interpolate(1, ob_string("width")))), ob_add), ob_interpolate(1, ob_string(")")), ob_add);
    }
    if (ob_truthy(ob_binop("==", ob_index_get(t, ob_interpolate(1, ob_string("width"))), ob_int(0LL), ob_eq))) {
        return code;
    }
    return ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_coerce_int(")), code, ob_add), ob_interpolate(1, ob_string(", ")), ob_add), ob_str(ob_index_get(t, ob_interpolate(1, ob_string("width")))), ob_add), ob_interpolate(1, ob_string(", ")), ob_add), (ob_truthy(ob_index_get(t, ob_interpolate(1, ob_string("is_unsigned")))) ? (ob_interpolate(1, ob_string("true"))) : (ob_interpolate(1, ob_string("false")))), ob_add), ob_interpolate(1, ob_string(")")), ob_add);
    return ob_null();
}

OboeValue codegen__gen_builtin_type_check(OboeValue type_name, OboeValue value) {
    if (ob_truthy(ob_binop("==", type_name, ob_interpolate(1, ob_string("bool")), ob_eq))) {
        return ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_is_bool(")), value, ob_add), ob_interpolate(1, ob_string(")")), ob_add);
    }
    if (ob_truthy(ob_binop("==", type_name, ob_interpolate(1, ob_string("string")), ob_eq))) {
        return ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_is_string(")), value, ob_add), ob_interpolate(1, ob_string(")")), ob_add);
    }
    if (ob_truthy(ob_binop("==", type_name, ob_interpolate(1, ob_string("array")), ob_eq))) {
        return ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_is_array(")), value, ob_add), ob_interpolate(1, ob_string(")")), ob_add);
    }
    if (ob_truthy(ob_binop("==", type_name, ob_interpolate(1, ob_string("dict")), ob_eq))) {
        return ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_is_dict(")), value, ob_add), ob_interpolate(1, ob_string(")")), ob_add);
    }
    if (ob_truthy(ob_binop("==", type_name, ob_interpolate(1, ob_string("null")), ob_eq))) {
        return ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_is_null(")), value, ob_add), ob_interpolate(1, ob_string(")")), ob_add);
    }
    OboeValue t = codegen__numeric_type(type_name);
    if (ob_truthy(ob_binop("==", t, ob_null(), ob_eq))) {
        return ob_null();
    }
    if (ob_truthy(ob_index_get(t, ob_interpolate(1, ob_string("is_float"))))) {
        return ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_is_float(")), value, ob_add), ob_interpolate(1, ob_string(")")), ob_add);
    }
    if (ob_truthy(ob_binop("==", ob_index_get(t, ob_interpolate(1, ob_string("width"))), ob_int(0LL), ob_eq))) {
        return ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_is_int(")), value, ob_add), ob_interpolate(1, ob_string(")")), ob_add);
    }
    return ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_is_int_width(")), value, ob_add), ob_interpolate(1, ob_string(", ")), ob_add), ob_str(ob_index_get(t, ob_interpolate(1, ob_string("width")))), ob_add), ob_interpolate(1, ob_string(", ")), ob_add), (ob_truthy(ob_index_get(t, ob_interpolate(1, ob_string("is_unsigned")))) ? (ob_interpolate(1, ob_string("true"))) : (ob_interpolate(1, ob_string("false")))), ob_add), ob_interpolate(1, ob_string(")")), ob_add);
    return ob_null();
}

OboeValue codegen__find_builtin_method(OboeValue name) {
    return (ob_truthy(ob_dict_has_m(codegen__BUILTIN_METHODS, name)) ? (ob_index_get(codegen__BUILTIN_METHODS, name)) : (ob_null()));
    return ob_null();
}

OboeValue codegen__gen_member_access_ex(OboeValue field_expr, OboeValue for_call, OboeValue safe, OboeValue argc) {
    OboeValue obj = ob_index_get(field_expr, ob_interpolate(1, ob_string("obj")));
    OboeValue name = ob_index_get(field_expr, ob_interpolate(1, ob_string("name")));
    OboeValue line = ob_index_get(field_expr, ob_interpolate(1, ob_string("line")));
    if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", ob_index_get(obj, ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("EXPR_IDENT")), ob_eq)) && ob_truthy(ob_not(codegen__var_in_scope(ob_index_get(obj, ob_interpolate(1, ob_string("ident"))))))))) {
        OboeValue sc = codegen__find_class(ob_index_get(obj, ob_interpolate(1, ob_string("ident"))));
        if (ob_truthy(ob_binop("!=", sc, ob_null(), ob_neq))) {
            if (ob_truthy(for_call)) {
                OboeValue sm = ob_null();
                { OboeValue __it = ob_index_get(sc, ob_interpolate(1, ob_string("methods"))); int64_t __n = ob_iter_len(__it);
                for (int64_t __i = 0; __i < __n; __i++) {
                    OboeValue m = ob_iter_value(__it, __i);
                    if (ob_truthy(ob_binop("==", ob_index_get(m, ob_interpolate(1, ob_string("name"))), name, ob_eq))) {
                        (void)((sm = m));
                    }
                } }
                return ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("code"))), ob_binop("+", ob_binop("+", ob_binop("+", ob_index_get(sc, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("__")), ob_add), name, ob_add), ob_interpolate(1, ob_string("(")), ob_add)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("first_arg"))), ob_null()); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("decl"))), sm); __d; });
            }
            return ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("code"))), ob_binop("+", ob_binop("+", ob_index_get(sc, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("__")), ob_add), name, ob_add)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("first_arg"))), ob_null()); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("decl"))), ob_null()); __d; });
        }
        { OboeValue __it = codegen__IMPORT_ALIASES; int64_t __n = ob_iter_len(__it);
        for (int64_t __i = 0; __i < __n; __i++) {
            OboeValue ia = ob_iter_value(__it, __i);
            if (ob_truthy(ob_bool(ob_truthy(ob_binop("!=", ob_index_get(ia, ob_interpolate(1, ob_string("local_name"))), ob_index_get(obj, ob_interpolate(1, ob_string("ident"))), ob_neq)) || ob_truthy(ob_binop("!=", ob_index_get(ia, ob_interpolate(1, ob_string("owner"))), codegen__CURRENT_PREFIX, ob_neq))))) {
                continue;
            }
            OboeValue mod = ob_index_get(ia, ob_interpolate(1, ob_string("module")));
            if (ob_truthy(codegen__module_is_builtin(mod))) {
                (void)(codegen__std_member_lookup(mod, name, line));
                if (ob_truthy(ob_not(for_call))) {
                    (void)(codegen__codegen_error(line, ob_interpolate(1, ob_string("this standard-library member is a function; call it"))));
                }
                return ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("code"))), ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_std_")), mod, ob_add), ob_interpolate(1, ob_string("_")), ob_add), name, ob_add), ob_interpolate(1, ob_string("(")), ob_add)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("first_arg"))), ob_null()); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("decl"))), ob_null()); __d; });
            }
            if (ob_truthy(for_call)) {
                OboeValue mkf = codegen__find_known_func_in(name, ob_binop("+", mod, ob_interpolate(1, ob_string("__")), ob_add));
                return ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("code"))), ob_binop("+", ob_binop("+", ob_binop("+", mod, ob_interpolate(1, ob_string("__")), ob_add), name, ob_add), ob_interpolate(1, ob_string("(")), ob_add)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("first_arg"))), ob_null()); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("decl"))), (ob_truthy(ob_binop("!=", mkf, ob_null(), ob_neq)) ? (ob_index_get(mkf, ob_interpolate(1, ob_string("decl")))) : (ob_null()))); __d; });
            }
            return ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("code"))), ob_binop("+", ob_binop("+", mod, ob_interpolate(1, ob_string("__")), ob_add), name, ob_add)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("first_arg"))), ob_null()); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("decl"))), ob_null()); __d; });
        } }
    }
    if (ob_truthy(ob_bool(ob_truthy(ob_bool(ob_truthy(ob_binop("==", ob_index_get(obj, ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("EXPR_IDENT")), ob_eq)) && ob_truthy(ob_binop("==", ob_index_get(obj, ob_interpolate(1, ob_string("ident"))), ob_interpolate(1, ob_string("super")), ob_eq)))) && ob_truthy(ob_not(codegen__var_in_scope(ob_index_get(obj, ob_interpolate(1, ob_string("ident"))))))))) {
        if (ob_truthy(ob_binop("==", codegen__CURRENT_CLASS, ob_null(), ob_eq))) {
            (void)(codegen__codegen_error(line, ob_interpolate(1, ob_string("'super' used outside a class"))));
        }
        OboeValue cc = codegen__find_class(codegen__CURRENT_CLASS);
        OboeValue parent = (ob_truthy(ob_bool(ob_truthy(ob_binop("!=", cc, ob_null(), ob_neq)) && ob_truthy(ob_binop("!=", ob_index_get(cc, ob_interpolate(1, ob_string("parent_name"))), ob_null(), ob_neq)))) ? (codegen__find_class(ob_index_get(cc, ob_interpolate(1, ob_string("parent_name"))))) : (ob_null()));
        if (ob_truthy(ob_binop("==", parent, ob_null(), ob_eq))) {
            (void)(codegen__codegen_error(line, ob_interpolate(1, ob_string("'super' used in a class with no parent"))));
        }
        if (ob_truthy(for_call)) {
            OboeValue got = codegen__find_method_owner(parent, name);
            if (ob_truthy(ob_binop("==", got, ob_null(), ob_eq))) {
                (void)(codegen__codegen_error(line, ob_interpolate(1, ob_string("no such method on the parent class (or its ancestors)"))));
            }
            OboeValue owner = ob_index_get(got, ob_interpolate(1, ob_string("owner")));
            if (ob_truthy(ob_bool(ob_truthy(ob_index_get(ob_index_get(got, ob_interpolate(1, ob_string("method"))), ob_interpolate(1, ob_string("is_private")))) && ob_truthy(ob_binop("!=", codegen__CURRENT_CLASS, ob_index_get(owner, ob_interpolate(1, ob_string("name"))), ob_neq))))) {
                (void)(codegen__codegen_error(line, ob_interpolate(1, ob_string("method is private"))));
            }
            return ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("code"))), ob_binop("+", ob_binop("+", ob_binop("+", ob_index_get(owner, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("__")), ob_add), name, ob_add), ob_interpolate(1, ob_string("(")), ob_add)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("first_arg"))), ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("((")), ob_index_get(owner, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("*)(this))")), ob_add)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("decl"))), ob_index_get(got, ob_interpolate(1, ob_string("method")))); __d; });
        }
        OboeValue fgot = codegen__find_field_owner(parent, name);
        if (ob_truthy(ob_binop("==", fgot, ob_null(), ob_eq))) {
            (void)(codegen__codegen_error(line, ob_interpolate(1, ob_string("no such field on the parent class (or its ancestors)"))));
        }
        return ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("code"))), ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("((")), ob_index_get(ob_index_get(fgot, ob_interpolate(1, ob_string("owner"))), ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("*)(this))->")), ob_add), name, ob_add)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("first_arg"))), ob_null()); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("decl"))), ob_null()); __d; });
    }
    OboeValue is_this = ob_bool(ob_truthy(ob_binop("==", ob_index_get(obj, ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("EXPR_IDENT")), ob_eq)) && ob_truthy(ob_binop("==", ob_index_get(obj, ob_interpolate(1, ob_string("ident"))), ob_interpolate(1, ob_string("this")), ob_eq)));
    OboeValue base_class = codegen__infer_class(obj);
    OboeValue obj_code = (ob_truthy(is_this) ? (ob_null()) : (codegen__gen_expr(obj)));
    if (ob_truthy(ob_bool(ob_truthy(ob_bool(ob_truthy(ob_binop("==", base_class, ob_null(), ob_eq)) && ob_truthy(for_call))) && ob_truthy(ob_not(is_this))))) {
        OboeValue bm = codegen__find_builtin_method(name);
        if (ob_truthy(ob_binop("!=", bm, ob_null(), ob_neq))) {
            if (ob_truthy(ob_bool(ob_truthy(ob_binop(">=", argc, ob_int(0LL), ob_gte)) && ob_truthy(ob_binop("!=", argc, ob_index_get(bm, ob_interpolate(1, ob_string("arity"))), ob_neq))))) {
                (void)(codegen__codegen_error(line, ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("'")), name, ob_add), ob_interpolate(1, ob_string("' takes ")), ob_add), ob_str(ob_index_get(bm, ob_interpolate(1, ob_string("arity")))), ob_add), ob_interpolate(1, ob_string(" argument(s), got ")), ob_add), ob_str(argc), ob_add)));
            }
            return ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("code"))), ob_binop("+", ob_index_get(bm, ob_interpolate(1, ob_string("cfunc"))), ob_interpolate(1, ob_string("(")), ob_add)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("first_arg"))), obj_code); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("decl"))), ob_null()); __d; });
        }
    }
    if (ob_truthy(ob_binop("==", base_class, ob_null(), ob_eq))) {
        (void)(codegen__codegen_error(line, ob_interpolate(1, ob_string("cannot resolve the static type of this expression for member access (compile-time dispatch requires a known class)"))));
    }
    OboeValue bc = codegen__find_class(base_class);
    if (ob_truthy(for_call)) {
        OboeValue got = codegen__find_method_owner(bc, name);
        if (ob_truthy(ob_binop("==", got, ob_null(), ob_eq))) {
            (void)(codegen__codegen_error(line, ob_interpolate(1, ob_string("no such method on this class (or its ancestors)"))));
        }
        OboeValue owner = ob_index_get(got, ob_interpolate(1, ob_string("owner")));
        if (ob_truthy(ob_bool(ob_truthy(ob_index_get(ob_index_get(got, ob_interpolate(1, ob_string("method"))), ob_interpolate(1, ob_string("is_private")))) && ob_truthy(ob_bool(ob_truthy(ob_binop("==", codegen__CURRENT_CLASS, ob_null(), ob_eq)) || ob_truthy(ob_binop("!=", codegen__CURRENT_CLASS, ob_index_get(owner, ob_interpolate(1, ob_string("name"))), ob_neq))))))) {
            (void)(codegen__codegen_error(line, ob_interpolate(1, ob_string("method is private"))));
        }
        OboeValue ptr = (ob_truthy(is_this) ? (ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("((")), ob_index_get(owner, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("*)(this))")), ob_add)) : (ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("((")), ob_index_get(owner, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("*)((")), ob_add), obj_code, ob_add), ob_interpolate(1, ob_string(").as.obj))")), ob_add)));
        return ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("code"))), ob_binop("+", ob_binop("+", ob_binop("+", ob_index_get(owner, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("__")), ob_add), name, ob_add), ob_interpolate(1, ob_string("(")), ob_add)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("first_arg"))), ptr); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("decl"))), ob_index_get(got, ob_interpolate(1, ob_string("method")))); __d; });
    }
    OboeValue fgot2 = codegen__find_field_owner(bc, name);
    if (ob_truthy(ob_binop("==", fgot2, ob_null(), ob_eq))) {
        (void)(codegen__codegen_error(line, ob_interpolate(1, ob_string("no such field on this class (or its ancestors)"))));
    }
    OboeValue fowner = ob_index_get(fgot2, ob_interpolate(1, ob_string("owner")));
    if (ob_truthy(ob_bool(ob_truthy(ob_index_get(ob_index_get(fgot2, ob_interpolate(1, ob_string("field"))), ob_interpolate(1, ob_string("is_private")))) && ob_truthy(ob_bool(ob_truthy(ob_binop("==", codegen__CURRENT_CLASS, ob_null(), ob_eq)) || ob_truthy(ob_binop("!=", codegen__CURRENT_CLASS, ob_index_get(fowner, ob_interpolate(1, ob_string("name"))), ob_neq))))))) {
        (void)(codegen__codegen_error(line, ob_interpolate(1, ob_string("field is private"))));
    }
    OboeValue fptr = (ob_truthy(is_this) ? (ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("((")), ob_index_get(fowner, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("*)(this))")), ob_add)) : (ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("((")), ob_index_get(fowner, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("*)((")), ob_add), obj_code, ob_add), ob_interpolate(1, ob_string(").as.obj))")), ob_add)));
    OboeValue access = ob_binop("+", ob_binop("+", fptr, ob_interpolate(1, ob_string("->")), ob_add), name, ob_add);
    if (ob_truthy(ob_bool(ob_truthy(safe) && ob_truthy(ob_not(is_this))))) {
        (void)((access = ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("(ob_is_null(")), obj_code, ob_add), ob_interpolate(1, ob_string(") ? ob_null() : ")), ob_add), access, ob_add), ob_interpolate(1, ob_string(")")), ob_add)));
    }
    return ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("code"))), access); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("first_arg"))), ob_null()); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("decl"))), ob_null()); __d; });
    return ob_null();
}

OboeValue codegen__gen_member_access(OboeValue field_expr, OboeValue for_call, OboeValue safe) {
    return ob_index_get(codegen__gen_member_access_ex(field_expr, for_call, safe, ob_neg(ob_int(1LL))), ob_interpolate(1, ob_string("code")));
    return ob_null();
}

OboeValue codegen__assign_target_prim_type(OboeValue target) {
    if (ob_truthy(ob_binop("==", ob_index_get(target, ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("EXPR_IDENT")), ob_eq))) {
        return codegen__lookup_var_prim(ob_index_get(target, ob_interpolate(1, ob_string("ident"))));
    }
    if (ob_truthy(ob_binop("==", ob_index_get(target, ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("EXPR_FIELD")), ob_eq))) {
        OboeValue bc = codegen__find_class(codegen__infer_class(ob_index_get(target, ob_interpolate(1, ob_string("obj")))));
        if (ob_truthy(ob_binop("==", bc, ob_null(), ob_eq))) {
            return ob_null();
        }
        OboeValue got = codegen__find_field_owner(bc, ob_index_get(target, ob_interpolate(1, ob_string("name"))));
        if (ob_truthy(ob_binop("!=", got, ob_null(), ob_neq))) {
            return ob_index_get(ob_index_get(got, ob_interpolate(1, ob_string("field"))), ob_interpolate(1, ob_string("type_name")));
        }
    }
    return ob_null();
    return ob_null();
}

OboeValue codegen__gen_assign_target_lvalue(OboeValue target) {
    if (ob_truthy(ob_binop("==", ob_index_get(target, ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("EXPR_IDENT")), ob_eq))) {
        if (ob_truthy(ob_not(codegen__var_in_scope(ob_index_get(target, ob_interpolate(1, ob_string("ident"))))))) {
            (void)(codegen__codegen_error(ob_index_get(target, ob_interpolate(1, ob_string("line"))), ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("undefined variable '")), ob_index_get(target, ob_interpolate(1, ob_string("ident"))), ob_add), ob_interpolate(1, ob_string("'")), ob_add)));
        }
        return codegen__lookup_var_cname(ob_index_get(target, ob_interpolate(1, ob_string("ident"))));
    }
    if (ob_truthy(ob_binop("==", ob_index_get(target, ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("EXPR_FIELD")), ob_eq))) {
        return codegen__gen_member_access(target, ob_bool(false), ob_bool(false));
    }
    (void)(codegen__codegen_error(ob_index_get(target, ob_interpolate(1, ob_string("line"))), ob_interpolate(1, ob_string("invalid assignment target"))));
    return ob_null();
    return ob_null();
}

OboeValue codegen__param_count_of(OboeValue f) {
    OboeValue total = ob_int(0LL);
    OboeValue required = ob_int(0LL);
    { OboeValue __it = ob_index_get(f, ob_interpolate(1, ob_string("params"))); int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue p = ob_iter_value(__it, __i);
        if (ob_truthy(ob_binop("==", ob_index_get(p, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("this")), ob_eq))) {
            continue;
        }
        (void)((total = ob_binop("+", total, ob_int(1LL), ob_add)));
        if (ob_truthy(ob_binop("==", ob_index_get(p, ob_interpolate(1, ob_string("default_value"))), ob_null(), ob_eq))) {
            (void)((required = ob_binop("+", required, ob_int(1LL), ob_add)));
        }
    } }
    return ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("total"))), total); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("required"))), required); __d; });
    return ob_null();
}

OboeValue codegen__bind_call_args(OboeValue f, OboeValue fname, OboeValue call) {
    OboeValue args = ob_index_get(call, ob_interpolate(1, ob_string("args")));
    OboeValue names = ob_index_get(call, ob_interpolate(1, ob_string("arg_names")));
    OboeValue argc = ob_m_len(args);
    OboeValue line = ob_index_get(call, ob_interpolate(1, ob_string("line")));
    OboeValue total = ob_index_get(codegen__param_count_of(f), ob_interpolate(1, ob_string("total")));
    OboeValue params = ({ OboeValue __a = ob_array_new(); __a; });
    { OboeValue __it = ob_index_get(f, ob_interpolate(1, ob_string("params"))); int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue p = ob_iter_value(__it, __i);
        if (ob_truthy(ob_binop("!=", ob_index_get(p, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("this")), ob_neq))) {
            (void)(ob_arr_push(params, p));
        }
    } }
    OboeValue bound = ({ OboeValue __a = ob_array_new(); __a; });
    OboeValue j = ob_int(0LL);
    while (ob_truthy(ob_binop("<", j, total, ob_lt))) {
        (void)(ob_arr_push(bound, ob_null()));
        (void)((j = ob_binop("+", j, ob_int(1LL), ob_add)));
    }
    OboeValue pos = ob_int(0LL);
    OboeValue seen_named = ob_bool(false);
    OboeValue i = ob_int(0LL);
    while (ob_truthy(ob_binop("<", i, argc, ob_lt))) {
        OboeValue name = ob_index_get(names, i);
        if (ob_truthy(ob_binop("==", name, ob_null(), ob_eq))) {
            if (ob_truthy(seen_named)) {
                (void)(codegen__codegen_error(line, ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("positional argument after a named argument in call to '")), fname, ob_add), ob_interpolate(1, ob_string("'")), ob_add)));
            }
            if (ob_truthy(ob_binop(">=", pos, total, ob_gte))) {
                (void)(codegen__codegen_error(line, ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("too many arguments to '")), fname, ob_add), ob_interpolate(1, ob_string("' (expected at most ")), ob_add), ob_str(total), ob_add), ob_interpolate(1, ob_string(", got ")), ob_add), ob_str(argc), ob_add), ob_interpolate(1, ob_string(")")), ob_add)));
            }
            (void)(ob_index_set(bound, pos, ob_index_get(args, i)));
            (void)((pos = ob_binop("+", pos, ob_int(1LL), ob_add)));
            (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
            continue;
        }
        (void)((seen_named = ob_bool(true)));
        OboeValue idx = ob_neg(ob_int(1LL));
        OboeValue k = ob_int(0LL);
        while (ob_truthy(ob_binop("<", k, total, ob_lt))) {
            if (ob_truthy(ob_binop("==", ob_index_get(ob_index_get(params, k), ob_interpolate(1, ob_string("name"))), name, ob_eq))) {
                (void)((idx = k));
                break;
            }
            (void)((k = ob_binop("+", k, ob_int(1LL), ob_add)));
        }
        if (ob_truthy(ob_binop("<", idx, ob_int(0LL), ob_lt))) {
            (void)(codegen__codegen_error(line, ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("unknown parameter name '")), name, ob_add), ob_interpolate(1, ob_string("' in call to '")), ob_add), fname, ob_add), ob_interpolate(1, ob_string("'")), ob_add)));
        }
        if (ob_truthy(ob_binop("!=", ob_index_get(bound, idx), ob_null(), ob_neq))) {
            (void)(codegen__codegen_error(line, ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("argument '")), name, ob_add), ob_interpolate(1, ob_string("' given twice in call to '")), ob_add), fname, ob_add), ob_interpolate(1, ob_string("'")), ob_add)));
        }
        (void)(ob_index_set(bound, idx, ob_index_get(args, i)));
        (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
    }
    (void)((j = ob_int(0LL)));
    while (ob_truthy(ob_binop("<", j, total, ob_lt))) {
        if (ob_truthy(ob_binop("==", ob_index_get(bound, j), ob_null(), ob_eq))) {
            if (ob_truthy(ob_binop("==", ob_index_get(ob_index_get(params, j), ob_interpolate(1, ob_string("default_value"))), ob_null(), ob_eq))) {
                (void)(codegen__codegen_error(line, ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("missing required argument '")), ob_index_get(ob_index_get(params, j), ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("' in call to '")), ob_add), fname, ob_add), ob_interpolate(1, ob_string("'")), ob_add)));
            }
            (void)(ob_index_set(bound, j, ob_index_get(ob_index_get(params, j), ob_interpolate(1, ob_string("default_value")))));
        }
        (void)((j = ob_binop("+", j, ob_int(1LL), ob_add)));
    }
    return bound;
    return ob_null();
}

OboeValue codegen__gen_bound_arg(OboeValue f, OboeValue idx, OboeValue arg) {
    OboeValue seen = ob_int(0LL);
    { OboeValue __it = ob_index_get(f, ob_interpolate(1, ob_string("params"))); int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue p = ob_iter_value(__it, __i);
        if (ob_truthy(ob_binop("==", ob_index_get(p, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("this")), ob_eq))) {
            continue;
        }
        if (ob_truthy(ob_binop("!=", seen, idx, ob_neq))) {
            (void)((seen = ob_binop("+", seen, ob_int(1LL), ob_add)));
            continue;
        }
        return codegen__apply_numeric_coercion(ob_index_get(p, ob_interpolate(1, ob_string("type_name"))), codegen__gen_expr(arg));
    } }
    return codegen__gen_expr(arg);
    return ob_null();
}

OboeValue codegen__gen_call_ident(OboeValue e) {
    OboeValue callee = ob_index_get(e, ob_interpolate(1, ob_string("callee")));
    OboeValue ident = ob_index_get(callee, ob_interpolate(1, ob_string("ident")));
    OboeValue argc = ob_m_len(ob_index_get(e, ob_interpolate(1, ob_string("args"))));
    if (ob_truthy(ob_dict_has_m(codegen__PRINT_FUNCS, ident))) {
        OboeValue fn = ob_index_get(codegen__PRINT_FUNCS, ident);
        if (ob_truthy(ob_binop("==", argc, ob_int(0LL), ob_eq))) {
            return ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("(")), fn, ob_add), ob_interpolate(1, ob_string("(ob_string(\"\")), ob_null())")), ob_add);
        }
        if (ob_truthy(ob_binop("==", argc, ob_int(1LL), ob_eq))) {
            return ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("(")), fn, ob_add), ob_interpolate(1, ob_string("(")), ob_add), codegen__gen_expr(ob_index_get(ob_index_get(e, ob_interpolate(1, ob_string("args"))), ob_int(0LL))), ob_add), ob_interpolate(1, ob_string("), ob_null())")), ob_add);
        }
        OboeValue b = ({ OboeValue __a = ob_array_new(); ob_array_push(__a, ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("(")), fn, ob_add), ob_interpolate(1, ob_string("(ob_interpolate(")), ob_add), ob_str(ob_binop("-", ob_binop("*", argc, ob_int(2LL), ob_mul), ob_int(1LL), ob_sub)), ob_add)); __a; });
        OboeValue i = ob_int(0LL);
        while (ob_truthy(ob_binop("<", i, argc, ob_lt))) {
            (void)(ob_arr_push(b, ob_binop("+", ob_binop("+", (ob_truthy(ob_binop("!=", i, ob_int(0LL), ob_neq)) ? (ob_interpolate(1, ob_string(", ob_string(\" \")"))) : (ob_string(""))), ob_interpolate(1, ob_string(", ")), ob_add), codegen__gen_expr(ob_index_get(ob_index_get(e, ob_interpolate(1, ob_string("args"))), i)), ob_add)));
            (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
        }
        (void)(ob_arr_push(b, ob_interpolate(1, ob_string(")), ob_null())"))));
        return ob_arr_join(b, ob_string(""));
    }
    if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", ident, ob_interpolate(1, ob_string("input")), ob_eq)) && ob_truthy(ob_binop("==", argc, ob_int(0LL), ob_eq))))) {
        return ob_interpolate(1, ob_string("ob_input()"));
    }
    if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", ident, ob_interpolate(1, ob_string("str")), ob_eq)) && ob_truthy(ob_binop("==", argc, ob_int(1LL), ob_eq))))) {
        return ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_str(")), codegen__gen_expr(ob_index_get(ob_index_get(e, ob_interpolate(1, ob_string("args"))), ob_int(0LL))), ob_add), ob_interpolate(1, ob_string(")")), ob_add);
    }
    if (ob_truthy(ob_bool(ob_truthy(ob_bool(ob_truthy(ob_binop("==", ident, ob_interpolate(1, ob_string("ord")), ob_eq)) || ob_truthy(ob_binop("==", ident, ob_interpolate(1, ob_string("chr")), ob_eq)))) && ob_truthy(ob_binop("==", argc, ob_int(1LL), ob_eq))))) {
        return ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_")), ident, ob_add), ob_interpolate(1, ob_string("(")), ob_add), codegen__gen_expr(ob_index_get(ob_index_get(e, ob_interpolate(1, ob_string("args"))), ob_int(0LL))), ob_add), ob_interpolate(1, ob_string(")")), ob_add);
    }
    if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", ident, ob_interpolate(1, ob_string("range")), ob_eq)) && ob_truthy(ob_binop("==", argc, ob_int(2LL), ob_eq))))) {
        return ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_range((")), codegen__gen_expr(ob_index_get(ob_index_get(e, ob_interpolate(1, ob_string("args"))), ob_int(0LL))), ob_add), ob_interpolate(1, ob_string(").as.i, (")), ob_add), codegen__gen_expr(ob_index_get(ob_index_get(e, ob_interpolate(1, ob_string("args"))), ob_int(1LL))), ob_add), ob_interpolate(1, ob_string(").as.i)")), ob_add);
    }
    if (ob_truthy(ob_binop("==", ident, ob_interpolate(1, ob_string("super")), ob_eq))) {
        if (ob_truthy(ob_binop("==", codegen__CURRENT_CLASS, ob_null(), ob_eq))) {
            (void)(codegen__codegen_error(ob_index_get(e, ob_interpolate(1, ob_string("line"))), ob_interpolate(1, ob_string("'super' used outside a class"))));
        }
        OboeValue cc = codegen__find_class(codegen__CURRENT_CLASS);
        OboeValue parent = (ob_truthy(ob_bool(ob_truthy(ob_binop("!=", cc, ob_null(), ob_neq)) && ob_truthy(ob_binop("!=", ob_index_get(cc, ob_interpolate(1, ob_string("parent_name"))), ob_null(), ob_neq)))) ? (codegen__find_class(ob_index_get(cc, ob_interpolate(1, ob_string("parent_name"))))) : (ob_null()));
        if (ob_truthy(ob_binop("==", parent, ob_null(), ob_eq))) {
            (void)(codegen__codegen_error(ob_index_get(e, ob_interpolate(1, ob_string("line"))), ob_interpolate(1, ob_string("'super' used in a class with no parent"))));
        }
        OboeValue owner = codegen__find_init_owner(parent);
        if (ob_truthy(ob_binop("==", owner, ob_null(), ob_eq))) {
            if (ob_truthy(ob_binop("!=", argc, ob_int(0LL), ob_neq))) {
                (void)(codegen__codegen_error(ob_index_get(e, ob_interpolate(1, ob_string("line"))), ob_interpolate(1, ob_string("no ancestor constructor matches this argument count"))));
            }
            return ob_interpolate(1, ob_string("ob_null()"));
        }
        OboeValue chosen = codegen__find_init_index(owner, argc);
        if (ob_truthy(ob_binop("<", chosen, ob_int(0LL), ob_lt))) {
            (void)(codegen__codegen_error(ob_index_get(e, ob_interpolate(1, ob_string("line"))), ob_interpolate(1, ob_string("no ancestor constructor matches this argument count"))));
        }
        OboeValue ctor = codegen__find_init_decl(owner, chosen);
        OboeValue sb = ({ OboeValue __a = ob_array_new(); ob_array_push(__a, ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("({ ")), ob_index_get(owner, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("__init_")), ob_add), ob_str(chosen), ob_add), ob_interpolate(1, ob_string("((")), ob_add), ob_index_get(owner, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("*)this")), ob_add)); __a; });
        OboeValue bound = codegen__bind_call_args(ctor, ob_interpolate(1, ob_string("super")), e);
        OboeValue bi = ob_int(0LL);
        while (ob_truthy(ob_binop("<", bi, ob_m_len(bound), ob_lt))) {
            (void)(ob_arr_push(sb, ob_binop("+", ob_interpolate(1, ob_string(", ")), codegen__gen_bound_arg(ctor, bi, ob_index_get(bound, bi)), ob_add)));
            (void)((bi = ob_binop("+", bi, ob_int(1LL), ob_add)));
        }
        (void)(ob_arr_push(sb, ob_interpolate(1, ob_string("); ob_null(); })"))));
        return ob_arr_join(sb, ob_string(""));
    }
    OboeValue c = codegen__find_class(ident);
    if (ob_truthy(ob_binop("!=", c, ob_null(), ob_neq))) {
        OboeValue init_count = ob_int(0LL);
        { OboeValue __it = ob_index_get(c, ob_interpolate(1, ob_string("methods"))); int64_t __n = ob_iter_len(__it);
        for (int64_t __i = 0; __i < __n; __i++) {
            OboeValue m = ob_iter_value(__it, __i);
            if (ob_truthy(ob_binop("==", ob_index_get(m, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("init")), ob_eq))) {
                (void)((init_count = ob_binop("+", init_count, ob_int(1LL), ob_add)));
            }
        } }
        OboeValue cb = ({ OboeValue __a = ob_array_new(); __a; });
        OboeValue ctor = ob_null();
        if (ob_truthy(ob_binop(">", init_count, ob_int(0LL), ob_gt))) {
            OboeValue chosen = codegen__find_init_index(c, argc);
            if (ob_truthy(ob_binop("<", chosen, ob_int(0LL), ob_lt))) {
                (void)(codegen__codegen_error(ob_index_get(e, ob_interpolate(1, ob_string("line"))), ob_interpolate(1, ob_string("no matching constructor overload for this argument count"))));
            }
            (void)((ctor = codegen__find_init_decl(c, chosen)));
            (void)(ob_arr_push(cb, ob_binop("+", ob_binop("+", ob_binop("+", ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("__new_")), ob_add), ob_str(chosen), ob_add), ob_interpolate(1, ob_string("(")), ob_add)));
        }
        else {
            OboeValue owner = codegen__find_init_owner(c);
            if (ob_truthy(ob_binop("!=", owner, ob_null(), ob_neq))) {
                OboeValue ichosen = codegen__find_init_index(owner, argc);
                if (ob_truthy(ob_binop("<", ichosen, ob_int(0LL), ob_lt))) {
                    (void)(codegen__codegen_error(ob_index_get(e, ob_interpolate(1, ob_string("line"))), ob_interpolate(1, ob_string("no matching constructor overload for this argument count"))));
                }
                (void)((ctor = codegen__find_init_decl(owner, ichosen)));
                (void)(ob_arr_push(cb, ob_binop("+", ob_binop("+", ob_binop("+", ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("__new_inh_")), ob_add), ob_str(ichosen), ob_add), ob_interpolate(1, ob_string("(")), ob_add)));
            }
            else {
                if (ob_truthy(ob_binop("==", argc, ob_int(0LL), ob_eq))) {
                    (void)(ob_arr_push(cb, ob_binop("+", ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("__new_default(")), ob_add)));
                }
                else {
                    (void)(codegen__codegen_error(ob_index_get(e, ob_interpolate(1, ob_string("line"))), ob_interpolate(1, ob_string("no matching constructor overload for this argument count"))));
                }
            }
        }
        if (ob_truthy(ob_binop("!=", ctor, ob_null(), ob_neq))) {
            OboeValue cbound = codegen__bind_call_args(ctor, ob_index_get(c, ob_interpolate(1, ob_string("name"))), e);
            OboeValue ci = ob_int(0LL);
            while (ob_truthy(ob_binop("<", ci, ob_m_len(cbound), ob_lt))) {
                (void)(ob_arr_push(cb, ob_binop("+", (ob_truthy(ob_binop("!=", ci, ob_int(0LL), ob_neq)) ? (ob_interpolate(1, ob_string(", "))) : (ob_string(""))), codegen__gen_bound_arg(ctor, ci, ob_index_get(cbound, ci)), ob_add)));
                (void)((ci = ob_binop("+", ci, ob_int(1LL), ob_add)));
            }
        }
        (void)(ob_arr_push(cb, ob_interpolate(1, ob_string(")"))));
        return ob_arr_join(cb, ob_string(""));
    }
    OboeValue ffi = codegen__find_ffi(ident);
    if (ob_truthy(ob_binop("!=", ffi, ob_null(), ob_neq))) {
        OboeValue fb = ({ OboeValue __a = ob_array_new(); ob_array_push(__a, ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_ffi_call(__ffi_")), ob_index_get(ffi, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string(", ")), ob_add), ob_str(argc), ob_add)); __a; });
        OboeValue fi = ob_int(0LL);
        while (ob_truthy(ob_binop("<", fi, argc, ob_lt))) {
            (void)(ob_arr_push(fb, ob_binop("+", ob_interpolate(1, ob_string(", ")), codegen__gen_expr(ob_index_get(ob_index_get(e, ob_interpolate(1, ob_string("args"))), fi)), ob_add)));
            (void)((fi = ob_binop("+", fi, ob_int(1LL), ob_add)));
        }
        (void)(ob_arr_push(fb, ob_interpolate(1, ob_string(")"))));
        return ob_arr_join(fb, ob_string(""));
    }
    { OboeValue __it = codegen__IMPORT_DIRECTS; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue idr = ob_iter_value(__it, __i);
        if (ob_truthy(ob_bool(ob_truthy(ob_binop("!=", ob_index_get(idr, ob_interpolate(1, ob_string("local_name"))), ident, ob_neq)) || ob_truthy(ob_binop("!=", ob_index_get(idr, ob_interpolate(1, ob_string("owner"))), codegen__CURRENT_PREFIX, ob_neq))))) {
            continue;
        }
        OboeValue db = ({ OboeValue __a = ob_array_new(); __a; });
        if (ob_truthy(codegen__module_is_builtin(ob_index_get(idr, ob_interpolate(1, ob_string("module")))))) {
            OboeValue arity = codegen__std_member_lookup(ob_index_get(idr, ob_interpolate(1, ob_string("module"))), ident, ob_index_get(e, ob_interpolate(1, ob_string("line"))));
            if (ob_truthy(ob_binop("!=", arity, argc, ob_neq))) {
                (void)(codegen__codegen_error(ob_index_get(e, ob_interpolate(1, ob_string("line"))), ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("'")), ident, ob_add), ob_interpolate(1, ob_string("' takes ")), ob_add), ob_str(arity), ob_add), ob_interpolate(1, ob_string(" argument(s)")), ob_add)));
            }
            (void)(ob_arr_push(db, ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_std_")), ob_index_get(idr, ob_interpolate(1, ob_string("module"))), ob_add), ob_interpolate(1, ob_string("_")), ob_add), ident, ob_add), ob_interpolate(1, ob_string("(")), ob_add)));
            OboeValue si = ob_int(0LL);
            while (ob_truthy(ob_binop("<", si, argc, ob_lt))) {
                (void)(ob_arr_push(db, ob_binop("+", (ob_truthy(ob_binop("!=", si, ob_int(0LL), ob_neq)) ? (ob_interpolate(1, ob_string(", "))) : (ob_string(""))), codegen__gen_expr(ob_index_get(ob_index_get(e, ob_interpolate(1, ob_string("args"))), si)), ob_add)));
                (void)((si = ob_binop("+", si, ob_int(1LL), ob_add)));
            }
            (void)(ob_arr_push(db, ob_interpolate(1, ob_string(")"))));
            return ob_arr_join(db, ob_string(""));
        }
        OboeValue mkf = codegen__find_known_func_in(ident, ob_binop("+", ob_index_get(idr, ob_interpolate(1, ob_string("module"))), ob_interpolate(1, ob_string("__")), ob_add));
        if (ob_truthy(ob_binop("==", mkf, ob_null(), ob_eq))) {
            (void)(codegen__codegen_error(ob_index_get(e, ob_interpolate(1, ob_string("line"))), ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("'")), ob_index_get(idr, ob_interpolate(1, ob_string("module"))), ob_add), ob_interpolate(1, ob_string("' has no function '")), ob_add), ident, ob_add), ob_interpolate(1, ob_string("'")), ob_add)));
        }
        (void)(ob_arr_push(db, ob_binop("+", ob_binop("+", ob_binop("+", ob_index_get(idr, ob_interpolate(1, ob_string("module"))), ob_interpolate(1, ob_string("__")), ob_add), ident, ob_add), ob_interpolate(1, ob_string("(")), ob_add)));
        OboeValue mbound = codegen__bind_call_args(ob_index_get(mkf, ob_interpolate(1, ob_string("decl"))), ident, e);
        OboeValue mi = ob_int(0LL);
        while (ob_truthy(ob_binop("<", mi, ob_m_len(mbound), ob_lt))) {
            (void)(ob_arr_push(db, ob_binop("+", (ob_truthy(ob_binop("!=", mi, ob_int(0LL), ob_neq)) ? (ob_interpolate(1, ob_string(", "))) : (ob_string(""))), codegen__gen_bound_arg(ob_index_get(mkf, ob_interpolate(1, ob_string("decl"))), mi, ob_index_get(mbound, mi)), ob_add)));
            (void)((mi = ob_binop("+", mi, ob_int(1LL), ob_add)));
        }
        (void)(ob_arr_push(db, ob_interpolate(1, ob_string(")"))));
        return ob_arr_join(db, ob_string(""));
    } }
    OboeValue kf = codegen__find_known_func(ident);
    if (ob_truthy(ob_binop("==", kf, ob_null(), ob_eq))) {
        if (ob_truthy(ob_binop("==", ident, ob_interpolate(1, ob_string("input")), ob_eq))) {
            (void)(codegen__codegen_error(ob_index_get(e, ob_interpolate(1, ob_string("line"))), ob_interpolate(1, ob_string("input() takes no arguments"))));
        }
        if (ob_truthy(ob_binop("==", ident, ob_interpolate(1, ob_string("str")), ob_eq))) {
            (void)(codegen__codegen_error(ob_index_get(e, ob_interpolate(1, ob_string("line"))), ob_interpolate(1, ob_string("str() takes exactly 1 argument"))));
        }
        if (ob_truthy(ob_binop("==", ident, ob_interpolate(1, ob_string("range")), ob_eq))) {
            (void)(codegen__codegen_error(ob_index_get(e, ob_interpolate(1, ob_string("line"))), ob_interpolate(1, ob_string("range() takes exactly 2 arguments"))));
        }
        if (ob_truthy(ob_binop("==", ident, ob_interpolate(1, ob_string("ord")), ob_eq))) {
            (void)(codegen__codegen_error(ob_index_get(e, ob_interpolate(1, ob_string("line"))), ob_interpolate(1, ob_string("ord() takes exactly 1 argument"))));
        }
        if (ob_truthy(ob_binop("==", ident, ob_interpolate(1, ob_string("chr")), ob_eq))) {
            (void)(codegen__codegen_error(ob_index_get(e, ob_interpolate(1, ob_string("line"))), ob_interpolate(1, ob_string("chr() takes exactly 1 argument"))));
        }
        (void)(codegen__codegen_error(ob_index_get(e, ob_interpolate(1, ob_string("line"))), ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("unknown function or class '")), ident, ob_add), ob_interpolate(1, ob_string("'")), ob_add)));
    }
    OboeValue fname = (ob_truthy(ob_bool(ob_truthy(ob_binop("==", ob_index_get(kf, ob_interpolate(1, ob_string("prefix"))), ob_string(""), ob_eq)) && ob_truthy(ob_binop("==", ident, ob_interpolate(1, ob_string("main")), ob_eq)))) ? (ob_interpolate(1, ob_string("oboe_user_main"))) : (ob_binop("+", ob_index_get(kf, ob_interpolate(1, ob_string("prefix"))), ident, ob_add)));
    OboeValue kbound = codegen__bind_call_args(ob_index_get(kf, ob_interpolate(1, ob_string("decl"))), ident, e);
    OboeValue kb = ({ OboeValue __a = ob_array_new(); ob_array_push(__a, ob_binop("+", fname, ob_interpolate(1, ob_string("(")), ob_add)); __a; });
    OboeValue ki = ob_int(0LL);
    while (ob_truthy(ob_binop("<", ki, ob_m_len(kbound), ob_lt))) {
        (void)(ob_arr_push(kb, ob_binop("+", (ob_truthy(ob_binop("!=", ki, ob_int(0LL), ob_neq)) ? (ob_interpolate(1, ob_string(", "))) : (ob_string(""))), codegen__gen_bound_arg(ob_index_get(kf, ob_interpolate(1, ob_string("decl"))), ki, ob_index_get(kbound, ki)), ob_add)));
        (void)((ki = ob_binop("+", ki, ob_int(1LL), ob_add)));
    }
    (void)(ob_arr_push(kb, ob_interpolate(1, ob_string(")"))));
    return ob_arr_join(kb, ob_string(""));
    return ob_null();
}

OboeValue codegen__gen_call_field(OboeValue e) {
    OboeValue callee = ob_index_get(e, ob_interpolate(1, ob_string("callee")));
    OboeValue argc = ob_m_len(ob_index_get(e, ob_interpolate(1, ob_string("args"))));
    OboeValue mobj = ob_index_get(callee, ob_interpolate(1, ob_string("obj")));
    if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", ob_index_get(mobj, ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("EXPR_IDENT")), ob_eq)) && ob_truthy(ob_not(codegen__var_in_scope(ob_index_get(mobj, ob_interpolate(1, ob_string("ident"))))))))) {
        { OboeValue __it = codegen__IMPORT_ALIASES; int64_t __n = ob_iter_len(__it);
        for (int64_t __i = 0; __i < __n; __i++) {
            OboeValue ia = ob_iter_value(__it, __i);
            if (ob_truthy(ob_bool(ob_truthy(ob_bool(ob_truthy(ob_binop("==", ob_index_get(ia, ob_interpolate(1, ob_string("local_name"))), ob_index_get(mobj, ob_interpolate(1, ob_string("ident"))), ob_eq)) && ob_truthy(ob_binop("==", ob_index_get(ia, ob_interpolate(1, ob_string("owner"))), codegen__CURRENT_PREFIX, ob_eq)))) && ob_truthy(codegen__module_is_builtin(ob_index_get(ia, ob_interpolate(1, ob_string("module")))))))) {
                OboeValue arity = codegen__std_member_lookup(ob_index_get(ia, ob_interpolate(1, ob_string("module"))), ob_index_get(callee, ob_interpolate(1, ob_string("name"))), ob_index_get(e, ob_interpolate(1, ob_string("line"))));
                if (ob_truthy(ob_binop("!=", arity, argc, ob_neq))) {
                    (void)(codegen__codegen_error(ob_index_get(e, ob_interpolate(1, ob_string("line"))), ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("'")), ob_index_get(ia, ob_interpolate(1, ob_string("module"))), ob_add), ob_interpolate(1, ob_string(".")), ob_add), ob_index_get(callee, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("' takes ")), ob_add), ob_str(arity), ob_add), ob_interpolate(1, ob_string(" argument(s)")), ob_add)));
                }
            }
        } }
    }
    OboeValue got = codegen__gen_member_access_ex(callee, ob_bool(true), ob_bool(false), argc);
    OboeValue target = ob_index_get(got, ob_interpolate(1, ob_string("decl")));
    OboeValue args = ({ OboeValue __a = ob_array_new(); __a; });
    OboeValue first = ob_bool(true);
    if (ob_truthy(ob_binop("!=", ob_index_get(got, ob_interpolate(1, ob_string("first_arg"))), ob_null(), ob_neq))) {
        (void)(ob_arr_push(args, ob_index_get(got, ob_interpolate(1, ob_string("first_arg")))));
        (void)((first = ob_bool(false)));
    }
    OboeValue emit_args = ob_index_get(e, ob_interpolate(1, ob_string("args")));
    if (ob_truthy(ob_binop("!=", target, ob_null(), ob_neq))) {
        (void)((emit_args = codegen__bind_call_args(target, ob_index_get(callee, ob_interpolate(1, ob_string("name"))), e)));
    }
    OboeValue i = ob_int(0LL);
    while (ob_truthy(ob_binop("<", i, ob_m_len(emit_args), ob_lt))) {
        OboeValue a = (ob_truthy(ob_binop("!=", target, ob_null(), ob_neq)) ? (codegen__gen_bound_arg(target, i, ob_index_get(emit_args, i))) : (codegen__gen_expr(ob_index_get(emit_args, i))));
        (void)(ob_arr_push(args, ob_binop("+", (ob_truthy(first) ? (ob_string("")) : (ob_interpolate(1, ob_string(", ")))), a, ob_add)));
        (void)((first = ob_bool(false)));
        (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
    }
    return ob_binop("+", ob_binop("+", ob_index_get(got, ob_interpolate(1, ob_string("code"))), ob_arr_join(args, ob_string("")), ob_add), ob_interpolate(1, ob_string(")")), ob_add);
    return ob_null();
}

OboeValue codegen__gen_expr(OboeValue e) {
    OboeValue k = ob_index_get(e, ob_interpolate(1, ob_string("kind")));
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_INT")), ob_eq))) {
        return ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_int(")), ob_str(ob_index_get(e, ob_interpolate(1, ob_string("int_val")))), ob_add), ob_interpolate(1, ob_string("LL)")), ob_add);
    }
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_FLOAT")), ob_eq))) {
        return ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_float(")), ob_str(ob_index_get(e, ob_interpolate(1, ob_string("float_val")))), ob_add), ob_interpolate(1, ob_string(")")), ob_add);
    }
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_BOOL")), ob_eq))) {
        return ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_bool(")), (ob_truthy(ob_index_get(e, ob_interpolate(1, ob_string("bool_val")))) ? (ob_interpolate(1, ob_string("true"))) : (ob_interpolate(1, ob_string("false")))), ob_add), ob_interpolate(1, ob_string(")")), ob_add);
    }
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_NULL")), ob_eq))) {
        return ob_interpolate(1, ob_string("ob_null()"));
    }
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_STRING")), ob_eq))) {
        return codegen__gen_string_literal(e);
    }
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_IDENT")), ob_eq))) {
        if (ob_truthy(ob_not(codegen__var_in_scope(ob_index_get(e, ob_interpolate(1, ob_string("ident"))))))) {
            { OboeValue __it = codegen__IMPORT_DIRECTS; int64_t __n = ob_iter_len(__it);
            for (int64_t __i = 0; __i < __n; __i++) {
                OboeValue idr = ob_iter_value(__it, __i);
                if (ob_truthy(ob_bool(ob_truthy(ob_binop("!=", ob_index_get(idr, ob_interpolate(1, ob_string("local_name"))), ob_index_get(e, ob_interpolate(1, ob_string("ident"))), ob_neq)) || ob_truthy(ob_binop("!=", ob_index_get(idr, ob_interpolate(1, ob_string("owner"))), codegen__CURRENT_PREFIX, ob_neq))))) {
                    continue;
                }
                if (ob_truthy(codegen__module_is_builtin(ob_index_get(idr, ob_interpolate(1, ob_string("module")))))) {
                    (void)(codegen__codegen_error(ob_index_get(e, ob_interpolate(1, ob_string("line"))), ob_interpolate(1, ob_string("this standard-library member is a function; call it"))));
                }
                return ob_binop("+", ob_binop("+", ob_index_get(idr, ob_interpolate(1, ob_string("module"))), ob_interpolate(1, ob_string("__")), ob_add), ob_index_get(e, ob_interpolate(1, ob_string("ident"))), ob_add);
            } }
            (void)(codegen__codegen_error(ob_index_get(e, ob_interpolate(1, ob_string("line"))), ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("undefined variable '")), ob_index_get(e, ob_interpolate(1, ob_string("ident"))), ob_add), ob_interpolate(1, ob_string("'")), ob_add)));
        }
        return codegen__lookup_var_cname(ob_index_get(e, ob_interpolate(1, ob_string("ident"))));
    }
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_ARRAY")), ob_eq))) {
        OboeValue ab = ({ OboeValue __a = ob_array_new(); ob_array_push(__a, ob_interpolate(1, ob_string("({ OboeValue __a = ob_array_new();"))); __a; });
        { OboeValue __it = ob_index_get(e, ob_interpolate(1, ob_string("items"))); int64_t __n = ob_iter_len(__it);
        for (int64_t __i = 0; __i < __n; __i++) {
            OboeValue item = ob_iter_value(__it, __i);
            (void)(ob_arr_push(ab, ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string(" ob_array_push(__a, ")), codegen__gen_expr(item), ob_add), ob_interpolate(1, ob_string(");")), ob_add)));
        } }
        (void)(ob_arr_push(ab, ob_interpolate(1, ob_string(" __a; })"))));
        return ob_arr_join(ab, ob_string(""));
    }
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_DICT")), ob_eq))) {
        OboeValue db = ({ OboeValue __a = ob_array_new(); ob_array_push(__a, ob_interpolate(1, ob_string("({ OboeValue __d = ob_dict_new();"))); __a; });
        OboeValue i = ob_int(0LL);
        while (ob_truthy(ob_binop("<", i, ob_m_len(ob_index_get(e, ob_interpolate(1, ob_string("keys")))), ob_lt))) {
            (void)(ob_arr_push(db, ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string(" ob_dict_set(__d, ob_to_cstr(")), codegen__gen_expr(ob_index_get(ob_index_get(e, ob_interpolate(1, ob_string("keys"))), i)), ob_add), ob_interpolate(1, ob_string("), ")), ob_add), codegen__gen_expr(ob_index_get(ob_index_get(e, ob_interpolate(1, ob_string("values"))), i)), ob_add), ob_interpolate(1, ob_string(");")), ob_add)));
            (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
        }
        (void)(ob_arr_push(db, ob_interpolate(1, ob_string(" __d; })"))));
        return ob_arr_join(db, ob_string(""));
    }
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_BINARY")), ob_eq))) {
        OboeValue l = codegen__gen_expr(ob_index_get(e, ob_interpolate(1, ob_string("l"))));
        OboeValue r = codegen__gen_expr(ob_index_get(e, ob_interpolate(1, ob_string("r"))));
        OboeValue op = ob_index_get(e, ob_interpolate(1, ob_string("op")));
        if (ob_truthy(ob_binop("==", op, ob_interpolate(1, ob_string("&&")), ob_eq))) {
            return ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_bool(ob_truthy(")), l, ob_add), ob_interpolate(1, ob_string(") && ob_truthy(")), ob_add), r, ob_add), ob_interpolate(1, ob_string("))")), ob_add);
        }
        if (ob_truthy(ob_binop("==", op, ob_interpolate(1, ob_string("||")), ob_eq))) {
            return ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_bool(ob_truthy(")), l, ob_add), ob_interpolate(1, ob_string(") || ob_truthy(")), ob_add), r, ob_add), ob_interpolate(1, ob_string("))")), ob_add);
        }
        if (ob_truthy(ob_binop("==", op, ob_interpolate(1, ob_string("??")), ob_eq))) {
            return ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_coalesce(")), l, ob_add), ob_interpolate(1, ob_string(", ")), ob_add), r, ob_add), ob_interpolate(1, ob_string(")")), ob_add);
        }
        OboeValue fallback = (ob_truthy(ob_dict_has_m(codegen__BINOP_FALLBACKS, op)) ? (ob_index_get(codegen__BINOP_FALLBACKS, op)) : (ob_null()));
        if (ob_truthy(ob_binop("==", fallback, ob_null(), ob_eq))) {
            OboeValue uo = codegen__find_user_op(op);
            if (ob_truthy(ob_binop("!=", uo, ob_null(), ob_neq))) {
                (void)((fallback = ob_index_get(uo, ob_interpolate(1, ob_string("cfunc")))));
            }
            else {
                if (ob_truthy(codegen__class_op_exists(op))) {
                    (void)((fallback = ob_interpolate(1, ob_string("ob_op_missing"))));
                }
            }
        }
        if (ob_truthy(ob_binop("==", fallback, ob_null(), ob_eq))) {
            (void)(codegen__codegen_error(ob_index_get(e, ob_interpolate(1, ob_string("line"))), ob_interpolate(1, ob_string("unknown binary operator"))));
        }
        return ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_binop(\"")), op, ob_add), ob_interpolate(1, ob_string("\", ")), ob_add), l, ob_add), ob_interpolate(1, ob_string(", ")), ob_add), r, ob_add), ob_interpolate(1, ob_string(", ")), ob_add), fallback, ob_add), ob_interpolate(1, ob_string(")")), ob_add);
    }
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_UNARY")), ob_eq))) {
        OboeValue v = codegen__gen_expr(ob_index_get(e, ob_interpolate(1, ob_string("operand"))));
        OboeValue fn = (ob_truthy(ob_binop("==", ob_index_get(e, ob_interpolate(1, ob_string("op"))), ob_interpolate(1, ob_string("!")), ob_eq)) ? (ob_interpolate(1, ob_string("ob_not"))) : ((ob_truthy(ob_binop("==", ob_index_get(e, ob_interpolate(1, ob_string("op"))), ob_interpolate(1, ob_string("~")), ob_eq)) ? (ob_interpolate(1, ob_string("ob_bnot"))) : (ob_interpolate(1, ob_string("ob_neg"))))));
        return ob_binop("+", ob_binop("+", ob_binop("+", fn, ob_interpolate(1, ob_string("(")), ob_add), v, ob_add), ob_interpolate(1, ob_string(")")), ob_add);
    }
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_IS")), ob_eq))) {
        OboeValue v = codegen__gen_expr(ob_index_get(e, ob_interpolate(1, ob_string("value"))));
        OboeValue check = codegen__gen_builtin_type_check(ob_index_get(e, ob_interpolate(1, ob_string("type_name"))), v);
        if (ob_truthy(ob_binop("!=", check, ob_null(), ob_neq))) {
            return ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_bool(")), check, ob_add), ob_interpolate(1, ob_string(")")), ob_add);
        }
        return ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_bool(ob_is_object_of(")), v, ob_add), ob_interpolate(1, ob_string(", &")), ob_add), ob_index_get(e, ob_interpolate(1, ob_string("type_name"))), ob_add), ob_interpolate(1, ob_string("__classinfo))")), ob_add);
    }
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_INDEX")), ob_eq))) {
        return ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_index_get(")), codegen__gen_expr(ob_index_get(e, ob_interpolate(1, ob_string("arr")))), ob_add), ob_interpolate(1, ob_string(", ")), ob_add), codegen__gen_expr(ob_index_get(e, ob_interpolate(1, ob_string("idx")))), ob_add), ob_interpolate(1, ob_string(")")), ob_add);
    }
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_FIELD")), ob_eq))) {
        return codegen__gen_member_access(e, ob_bool(false), ob_bool(false));
    }
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_SAFE_FIELD")), ob_eq))) {
        return codegen__gen_member_access(e, ob_bool(false), ob_bool(true));
    }
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_CALL")), ob_eq))) {
        OboeValue callee = ob_index_get(e, ob_interpolate(1, ob_string("callee")));
        if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", ob_index_get(callee, ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("EXPR_IDENT")), ob_eq)) && ob_truthy(ob_not(codegen__var_in_scope(ob_index_get(callee, ob_interpolate(1, ob_string("ident"))))))))) {
            return codegen__gen_call_ident(e);
        }
        if (ob_truthy(ob_binop("==", ob_index_get(callee, ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("EXPR_FIELD")), ob_eq))) {
            return codegen__gen_call_field(e);
        }
        (void)(codegen__codegen_error(ob_index_get(e, ob_interpolate(1, ob_string("line"))), ob_interpolate(1, ob_string("unsupported call expression"))));
        return ob_null();
    }
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_TERNARY")), ob_eq))) {
        return ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("(ob_truthy(")), codegen__gen_expr(ob_index_get(e, ob_interpolate(1, ob_string("cond")))), ob_add), ob_interpolate(1, ob_string(") ? (")), ob_add), codegen__gen_expr(ob_index_get(e, ob_interpolate(1, ob_string("then_e")))), ob_add), ob_interpolate(1, ob_string(") : (")), ob_add), codegen__gen_expr(ob_index_get(e, ob_interpolate(1, ob_string("else_e")))), ob_add), ob_interpolate(1, ob_string("))")), ob_add);
    }
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_ASSIGN")), ob_eq))) {
        OboeValue target = ob_index_get(e, ob_interpolate(1, ob_string("target")));
        if (ob_truthy(ob_binop("==", ob_index_get(target, ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("EXPR_INDEX")), ob_eq))) {
            return ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_index_set(")), codegen__gen_expr(ob_index_get(target, ob_interpolate(1, ob_string("arr")))), ob_add), ob_interpolate(1, ob_string(", ")), ob_add), codegen__gen_expr(ob_index_get(target, ob_interpolate(1, ob_string("idx")))), ob_add), ob_interpolate(1, ob_string(", ")), ob_add), codegen__gen_expr(ob_index_get(e, ob_interpolate(1, ob_string("value")))), ob_add), ob_interpolate(1, ob_string(")")), ob_add);
        }
        OboeValue lvalue = codegen__gen_assign_target_lvalue(target);
        OboeValue val = codegen__gen_expr(ob_index_get(e, ob_interpolate(1, ob_string("value"))));
        (void)((val = codegen__apply_numeric_coercion(codegen__assign_target_prim_type(target), val)));
        return ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("(")), lvalue, ob_add), ob_interpolate(1, ob_string(" = ")), ob_add), val, ob_add), ob_interpolate(1, ob_string(")")), ob_add);
    }
    (void)(codegen__codegen_error(ob_index_get(e, ob_interpolate(1, ob_string("line"))), ob_interpolate(1, ob_string("unknown expression kind"))));
    return ob_null();
    return ob_null();
}

OboeValue codegen__gen_stmt(OboeValue s, OboeValue indent) {
    OboeValue k = ob_index_get(s, ob_interpolate(1, ob_string("kind")));
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("STMT_LET")), ob_eq))) {
        OboeValue class_type = ob_null();
        if (ob_truthy(ob_bool(ob_truthy(ob_binop("!=", ob_index_get(s, ob_interpolate(1, ob_string("type_name"))), ob_null(), ob_neq)) && ob_truthy(ob_binop("!=", codegen__find_class(ob_index_get(s, ob_interpolate(1, ob_string("type_name")))), ob_null(), ob_neq))))) {
            (void)((class_type = ob_index_get(s, ob_interpolate(1, ob_string("type_name")))));
        }
        else {
            if (ob_truthy(ob_binop("!=", ob_index_get(s, ob_interpolate(1, ob_string("init"))), ob_null(), ob_neq))) {
                (void)((class_type = codegen__infer_class(ob_index_get(s, ob_interpolate(1, ob_string("init"))))));
            }
        }
        (void)(codegen__define_var_full(ob_index_get(s, ob_interpolate(1, ob_string("name"))), class_type, ob_null(), ob_index_get(s, ob_interpolate(1, ob_string("type_name")))));
        OboeValue init = ob_interpolate(1, ob_string("ob_null()"));
        if (ob_truthy(ob_binop("!=", ob_index_get(s, ob_interpolate(1, ob_string("init"))), ob_null(), ob_neq))) {
            (void)((init = codegen__apply_numeric_coercion(ob_index_get(s, ob_interpolate(1, ob_string("type_name"))), codegen__gen_expr(ob_index_get(s, ob_interpolate(1, ob_string("init")))))));
        }
        (void)(codegen__ind(indent));
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", (ob_truthy(ob_index_get(s, ob_interpolate(1, ob_string("is_const")))) ? (ob_interpolate(1, ob_string("const "))) : (ob_string(""))), ob_interpolate(1, ob_string("OboeValue ")), ob_add), ob_index_get(s, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string(" = ")), ob_add), init, ob_add), ob_interpolate(1, ob_string(";\n")), ob_add)));
        return ob_null();
    }
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("STMT_EXPR")), ob_eq))) {
        OboeValue code = codegen__gen_expr(ob_index_get(s, ob_interpolate(1, ob_string("expr"))));
        (void)(codegen__ind(indent));
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("(void)(")), code, ob_add), ob_interpolate(1, ob_string(");\n")), ob_add)));
        return ob_null();
    }
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("STMT_RETURN")), ob_eq))) {
        (void)(codegen__ind(indent));
        if (ob_truthy(ob_binop(">", codegen__TRY_DEPTH, ob_int(0LL), ob_gt))) {
            if (ob_truthy(ob_binop("!=", ob_index_get(s, ob_interpolate(1, ob_string("value"))), ob_null(), ob_neq))) {
                (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("{ OboeValue __ret = ")), codegen__gen_expr(ob_index_get(s, ob_interpolate(1, ob_string("value")))), ob_add), ob_interpolate(1, ob_string("; ob_exc_stack = __frame_0.prev; return __ret; }\n")), ob_add)));
            }
            else {
                (void)(codegen__emit(ob_interpolate(1, ob_string("{ ob_exc_stack = __frame_0.prev; return ob_null(); }\n"))));
            }
        }
        else {
            if (ob_truthy(ob_binop("!=", ob_index_get(s, ob_interpolate(1, ob_string("value"))), ob_null(), ob_neq))) {
                (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("return ")), codegen__gen_expr(ob_index_get(s, ob_interpolate(1, ob_string("value")))), ob_add), ob_interpolate(1, ob_string(";\n")), ob_add)));
            }
            else {
                (void)(codegen__emit(ob_interpolate(1, ob_string("return ob_null();\n"))));
            }
        }
        return ob_null();
    }
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("STMT_IF")), ob_eq))) {
        OboeValue cond = codegen__gen_expr(ob_index_get(s, ob_interpolate(1, ob_string("cond"))));
        (void)(codegen__ind(indent));
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("if (ob_truthy(")), cond, ob_add), ob_interpolate(1, ob_string(")) {\n")), ob_add)));
        (void)(codegen__push_scope());
        (void)(codegen__gen_stmt_list(ob_index_get(s, ob_interpolate(1, ob_string("then_body"))), ob_binop("+", indent, ob_int(4LL), ob_add)));
        (void)(codegen__pop_scope());
        (void)(codegen__ind(indent));
        (void)(codegen__emit(ob_interpolate(1, ob_string("}\n"))));
        if (ob_truthy(ob_binop(">", ob_m_len(ob_index_get(s, ob_interpolate(1, ob_string("else_body")))), ob_int(0LL), ob_gt))) {
            (void)(codegen__ind(indent));
            (void)(codegen__emit(ob_interpolate(1, ob_string("else {\n"))));
            (void)(codegen__push_scope());
            (void)(codegen__gen_stmt_list(ob_index_get(s, ob_interpolate(1, ob_string("else_body"))), ob_binop("+", indent, ob_int(4LL), ob_add)));
            (void)(codegen__pop_scope());
            (void)(codegen__ind(indent));
            (void)(codegen__emit(ob_interpolate(1, ob_string("}\n"))));
        }
        return ob_null();
    }
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("STMT_WHILE")), ob_eq))) {
        OboeValue cond = codegen__gen_expr(ob_index_get(s, ob_interpolate(1, ob_string("cond"))));
        (void)(codegen__ind(indent));
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("while (ob_truthy(")), cond, ob_add), ob_interpolate(1, ob_string(")) {\n")), ob_add)));
        (void)(codegen__push_scope());
        OboeValue saved_loop = codegen__LOOP_TRY_DEPTH;
        (void)((codegen__LOOP_TRY_DEPTH = codegen__TRY_DEPTH));
        (void)(codegen__gen_stmt_list(ob_index_get(s, ob_interpolate(1, ob_string("body"))), ob_binop("+", indent, ob_int(4LL), ob_add)));
        (void)((codegen__LOOP_TRY_DEPTH = saved_loop));
        (void)(codegen__pop_scope());
        (void)(codegen__ind(indent));
        (void)(codegen__emit(ob_interpolate(1, ob_string("}\n"))));
        return ob_null();
    }
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("STMT_FOR")), ob_eq))) {
        (void)(codegen__push_scope());
        OboeValue saved_loop = codegen__LOOP_TRY_DEPTH;
        (void)((codegen__LOOP_TRY_DEPTH = codegen__TRY_DEPTH));
        if (ob_truthy(ob_binop("==", ob_index_get(s, ob_interpolate(1, ob_string("kind_iter"))), ob_interpolate(1, ob_string("FOR_RANGE")), ob_eq))) {
            OboeValue a = codegen__gen_expr(ob_index_get(s, ob_interpolate(1, ob_string("range_a"))));
            OboeValue b = codegen__gen_expr(ob_index_get(s, ob_interpolate(1, ob_string("range_b"))));
            (void)(codegen__ind(indent));
            (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("for (int64_t __i = (")), a, ob_add), ob_interpolate(1, ob_string(").as.i, __end = (")), ob_add), b, ob_add), ob_interpolate(1, ob_string(").as.i; __i < __end; __i++) {\n")), ob_add)));
            (void)(codegen__ind(ob_binop("+", indent, ob_int(4LL), ob_add)));
            (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("OboeValue ")), ob_index_get(s, ob_interpolate(1, ob_string("var_name"))), ob_add), ob_interpolate(1, ob_string(" = ob_int(__i);\n")), ob_add)));
            (void)(codegen__define_var(ob_index_get(s, ob_interpolate(1, ob_string("var_name"))), ob_null()));
            (void)(codegen__gen_stmt_list(ob_index_get(s, ob_interpolate(1, ob_string("body"))), ob_binop("+", indent, ob_int(4LL), ob_add)));
            (void)(codegen__ind(indent));
            (void)(codegen__emit(ob_interpolate(1, ob_string("}\n"))));
        }
        else {
            OboeValue iter = codegen__gen_expr(ob_index_get(s, ob_interpolate(1, ob_string("iterable"))));
            (void)(codegen__ind(indent));
            (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("{ OboeValue __it = ")), iter, ob_add), ob_interpolate(1, ob_string("; int64_t __n = ob_iter_len(__it);\n")), ob_add)));
            (void)(codegen__ind(indent));
            (void)(codegen__emit(ob_interpolate(1, ob_string("for (int64_t __i = 0; __i < __n; __i++) {\n"))));
            if (ob_truthy(ob_binop("!=", ob_index_get(s, ob_interpolate(1, ob_string("var2_name"))), ob_null(), ob_neq))) {
                (void)(codegen__ind(ob_binop("+", indent, ob_int(4LL), ob_add)));
                if (ob_truthy(ob_binop("==", ob_index_get(s, ob_interpolate(1, ob_string("kind_iter"))), ob_interpolate(1, ob_string("FOR_IPAIRS")), ob_eq))) {
                    (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("OboeValue ")), ob_index_get(s, ob_interpolate(1, ob_string("var_name"))), ob_add), ob_interpolate(1, ob_string(" = ob_int(__i);\n")), ob_add)));
                }
                else {
                    (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("OboeValue ")), ob_index_get(s, ob_interpolate(1, ob_string("var_name"))), ob_add), ob_interpolate(1, ob_string(" = ob_iter_key(__it, __i);\n")), ob_add)));
                }
                (void)(codegen__ind(ob_binop("+", indent, ob_int(4LL), ob_add)));
                (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("OboeValue ")), ob_index_get(s, ob_interpolate(1, ob_string("var2_name"))), ob_add), ob_interpolate(1, ob_string(" = ob_iter_value(__it, __i);\n")), ob_add)));
                (void)(codegen__define_var(ob_index_get(s, ob_interpolate(1, ob_string("var_name"))), ob_null()));
                (void)(codegen__define_var(ob_index_get(s, ob_interpolate(1, ob_string("var2_name"))), ob_null()));
            }
            else {
                (void)(codegen__ind(ob_binop("+", indent, ob_int(4LL), ob_add)));
                (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("OboeValue ")), ob_index_get(s, ob_interpolate(1, ob_string("var_name"))), ob_add), ob_interpolate(1, ob_string(" = ob_iter_value(__it, __i);\n")), ob_add)));
                (void)(codegen__define_var(ob_index_get(s, ob_interpolate(1, ob_string("var_name"))), ob_null()));
            }
            (void)(codegen__gen_stmt_list(ob_index_get(s, ob_interpolate(1, ob_string("body"))), ob_binop("+", indent, ob_int(4LL), ob_add)));
            (void)(codegen__ind(indent));
            (void)(codegen__emit(ob_interpolate(1, ob_string("} }\n"))));
        }
        (void)((codegen__LOOP_TRY_DEPTH = saved_loop));
        (void)(codegen__pop_scope());
        return ob_null();
    }
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("STMT_SWITCH")), ob_eq))) {
        (void)(codegen__ind(indent));
        OboeValue subj = codegen__gen_expr(ob_index_get(s, ob_interpolate(1, ob_string("subject"))));
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("{ OboeValue __subj = ")), subj, ob_add), ob_interpolate(1, ob_string(";\n")), ob_add)));
        OboeValue first = ob_bool(true);
        { OboeValue __it = ob_index_get(s, ob_interpolate(1, ob_string("cases"))); int64_t __n = ob_iter_len(__it);
        for (int64_t __i = 0; __i < __n; __i++) {
            OboeValue c = ob_iter_value(__it, __i);
            OboeValue val = codegen__gen_expr(ob_index_get(c, ob_interpolate(1, ob_string("value"))));
            (void)(codegen__ind(indent));
            (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", (ob_truthy(first) ? (ob_interpolate(1, ob_string("if"))) : (ob_interpolate(1, ob_string("else if")))), ob_interpolate(1, ob_string(" (ob_truthy(ob_eq(__subj, ")), ob_add), val, ob_add), ob_interpolate(1, ob_string("))) {\n")), ob_add)));
            (void)((first = ob_bool(false)));
            (void)(codegen__push_scope());
            (void)(codegen__gen_stmt_list(ob_index_get(c, ob_interpolate(1, ob_string("body"))), ob_binop("+", indent, ob_int(4LL), ob_add)));
            (void)(codegen__pop_scope());
            (void)(codegen__ind(indent));
            (void)(codegen__emit(ob_interpolate(1, ob_string("}\n"))));
        } }
        (void)(codegen__ind(indent));
        (void)(codegen__emit(ob_interpolate(1, ob_string("}\n"))));
        return ob_null();
    }
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("STMT_TRY")), ob_eq))) {
        OboeValue depth = codegen__TRY_DEPTH;
        OboeValue d = ob_str(depth);
        (void)(codegen__ind(indent));
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("{ OboeExceptionFrame __frame_")), d, ob_add), ob_interpolate(1, ob_string("; __frame_")), ob_add), d, ob_add), ob_interpolate(1, ob_string(".prev = ob_exc_stack; ob_exc_stack = &__frame_")), ob_add), d, ob_add), ob_interpolate(1, ob_string(";\n")), ob_add)));
        (void)(codegen__ind(indent));
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("bool __rethrow_")), d, ob_add), ob_interpolate(1, ob_string(" = false;\n")), ob_add)));
        (void)(codegen__ind(indent));
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("if (setjmp(__frame_")), d, ob_add), ob_interpolate(1, ob_string(".buf) == 0) {\n")), ob_add)));
        (void)(codegen__push_scope());
        (void)((codegen__TRY_DEPTH = ob_binop("+", codegen__TRY_DEPTH, ob_int(1LL), ob_add)));
        (void)(codegen__gen_stmt_list(ob_index_get(s, ob_interpolate(1, ob_string("body"))), ob_binop("+", indent, ob_int(4LL), ob_add)));
        (void)((codegen__TRY_DEPTH = ob_binop("-", codegen__TRY_DEPTH, ob_int(1LL), ob_sub)));
        (void)(codegen__pop_scope());
        (void)(codegen__ind(ob_binop("+", indent, ob_int(4LL), ob_add)));
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_exc_stack = __frame_")), d, ob_add), ob_interpolate(1, ob_string(".prev;\n")), ob_add)));
        (void)(codegen__ind(indent));
        (void)(codegen__emit(ob_interpolate(1, ob_string("} else {\n"))));
        OboeValue first = ob_bool(true);
        { OboeValue __it = ob_index_get(s, ob_interpolate(1, ob_string("catches"))); int64_t __n = ob_iter_len(__it);
        for (int64_t __i = 0; __i < __n; __i++) {
            OboeValue c = ob_iter_value(__it, __i);
            (void)(codegen__ind(ob_binop("+", indent, ob_int(4LL), ob_add)));
            (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", (ob_truthy(first) ? (ob_interpolate(1, ob_string("if"))) : (ob_interpolate(1, ob_string("else if")))), ob_interpolate(1, ob_string(" (ob_exception_matches(\"")), ob_add), ob_index_get(c, ob_interpolate(1, ob_string("type_name"))), ob_add), ob_interpolate(1, ob_string("\")) {\n")), ob_add)));
            (void)((first = ob_bool(false)));
            (void)(codegen__ind(ob_binop("+", indent, ob_int(8LL), ob_add)));
            (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("OboeValue ")), ob_index_get(c, ob_interpolate(1, ob_string("var_name"))), ob_add), ob_interpolate(1, ob_string(" = ob_current_exception;\n")), ob_add)));
            (void)(codegen__push_scope());
            (void)(codegen__define_var(ob_index_get(c, ob_interpolate(1, ob_string("var_name"))), ob_null()));
            (void)((codegen__TRY_DEPTH = ob_binop("+", codegen__TRY_DEPTH, ob_int(1LL), ob_add)));
            (void)(codegen__gen_stmt_list(ob_index_get(c, ob_interpolate(1, ob_string("body"))), ob_binop("+", indent, ob_int(8LL), ob_add)));
            (void)((codegen__TRY_DEPTH = ob_binop("-", codegen__TRY_DEPTH, ob_int(1LL), ob_sub)));
            (void)(codegen__pop_scope());
            (void)(codegen__ind(ob_binop("+", indent, ob_int(4LL), ob_add)));
            (void)(codegen__emit(ob_interpolate(1, ob_string("}\n"))));
        } }
        (void)(codegen__ind(ob_binop("+", indent, ob_int(4LL), ob_add)));
        if (ob_truthy(ob_binop(">", ob_m_len(ob_index_get(s, ob_interpolate(1, ob_string("catches")))), ob_int(0LL), ob_gt))) {
            (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("else { __rethrow_")), d, ob_add), ob_interpolate(1, ob_string(" = true; }\n")), ob_add)));
        }
        else {
            (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("__rethrow_")), d, ob_add), ob_interpolate(1, ob_string(" = true;\n")), ob_add)));
        }
        (void)(codegen__ind(indent));
        (void)(codegen__emit(ob_interpolate(1, ob_string("}\n"))));
        if (ob_truthy(ob_binop(">", ob_m_len(ob_index_get(s, ob_interpolate(1, ob_string("finally_body")))), ob_int(0LL), ob_gt))) {
            (void)(codegen__push_scope());
            (void)((codegen__TRY_DEPTH = ob_binop("+", codegen__TRY_DEPTH, ob_int(1LL), ob_add)));
            (void)(codegen__gen_stmt_list(ob_index_get(s, ob_interpolate(1, ob_string("finally_body"))), indent));
            (void)((codegen__TRY_DEPTH = ob_binop("-", codegen__TRY_DEPTH, ob_int(1LL), ob_sub)));
            (void)(codegen__pop_scope());
        }
        (void)(codegen__ind(indent));
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("if (__rethrow_")), d, ob_add), ob_interpolate(1, ob_string(") ob_throw(ob_current_exception_type, ob_current_exception);\n")), ob_add)));
        (void)(codegen__ind(indent));
        (void)(codegen__emit(ob_interpolate(1, ob_string("}\n"))));
        return ob_null();
    }
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("STMT_THROW")), ob_eq))) {
        OboeValue val = (ob_truthy(ob_binop("!=", ob_index_get(s, ob_interpolate(1, ob_string("value"))), ob_null(), ob_neq)) ? (codegen__gen_expr(ob_index_get(s, ob_interpolate(1, ob_string("value"))))) : (ob_interpolate(1, ob_string("ob_null()"))));
        (void)(codegen__ind(indent));
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("ob_throw(\"")), ob_index_get(s, ob_interpolate(1, ob_string("type_name"))), ob_add), ob_interpolate(1, ob_string("\", ")), ob_add), val, ob_add), ob_interpolate(1, ob_string(");\n")), ob_add)));
        return ob_null();
    }
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("STMT_BLOCK")), ob_eq))) {
        (void)(codegen__ind(indent));
        (void)(codegen__emit(ob_interpolate(1, ob_string("{\n"))));
        (void)(codegen__push_scope());
        (void)(codegen__gen_stmt_list(ob_index_get(s, ob_interpolate(1, ob_string("body"))), ob_binop("+", indent, ob_int(4LL), ob_add)));
        (void)(codegen__pop_scope());
        (void)(codegen__ind(indent));
        (void)(codegen__emit(ob_interpolate(1, ob_string("}\n"))));
        return ob_null();
    }
    if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("STMT_BREAK")), ob_eq)) || ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("STMT_CONTINUE")), ob_eq))))) {
        OboeValue kw = (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("STMT_BREAK")), ob_eq)) ? (ob_interpolate(1, ob_string("break"))) : (ob_interpolate(1, ob_string("continue"))));
        if (ob_truthy(ob_binop("<", codegen__LOOP_TRY_DEPTH, ob_int(0LL), ob_lt))) {
            (void)(codegen__codegen_error(ob_index_get(s, ob_interpolate(1, ob_string("line"))), ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("'")), kw, ob_add), ob_interpolate(1, ob_string("' outside of a loop")), ob_add)));
        }
        (void)(codegen__ind(indent));
        if (ob_truthy(ob_binop(">", codegen__TRY_DEPTH, codegen__LOOP_TRY_DEPTH, ob_gt))) {
            (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("{ ob_exc_stack = __frame_")), ob_str(codegen__LOOP_TRY_DEPTH), ob_add), ob_interpolate(1, ob_string(".prev; ")), ob_add), kw, ob_add), ob_interpolate(1, ob_string("; }\n")), ob_add)));
        }
        else {
            (void)(codegen__emit(ob_binop("+", kw, ob_interpolate(1, ob_string(";\n")), ob_add)));
        }
        return ob_null();
    }
    return ob_null();
    return ob_null();
}

OboeValue codegen__gen_stmt_list(OboeValue body, OboeValue indent) {
    { OboeValue __it = body; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue s = ob_iter_value(__it, __i);
        (void)(codegen__gen_stmt(s, indent));
    } }
    return ob_null();
}

OboeValue codegen__gen_param_list(OboeValue owner, OboeValue params, OboeValue skip_this) {
    OboeValue first = ob_bool(true);
    if (ob_truthy(ob_bool(ob_truthy(ob_binop("!=", owner, ob_null(), ob_neq)) && ob_truthy(skip_this)))) {
        (void)(codegen__emit(ob_binop("+", ob_index_get(owner, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("* this")), ob_add)));
        (void)((first = ob_bool(false)));
    }
    { OboeValue __it = params; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue p = ob_iter_value(__it, __i);
        if (ob_truthy(ob_binop("==", ob_index_get(p, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("this")), ob_eq))) {
            continue;
        }
        if (ob_truthy(ob_not(first))) {
            (void)(codegen__emit(ob_interpolate(1, ob_string(", "))));
        }
        (void)(codegen__emit(ob_binop("+", ob_interpolate(1, ob_string("OboeValue ")), ob_index_get(p, ob_interpolate(1, ob_string("name"))), ob_add)));
        (void)((first = ob_bool(false)));
    } }
    return ob_null();
}

OboeValue codegen__bind_params(OboeValue params, OboeValue skip_this) {
    { OboeValue __it = params; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue p = ob_iter_value(__it, __i);
        if (ob_truthy(ob_bool(ob_truthy(skip_this) && ob_truthy(ob_binop("==", ob_index_get(p, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("this")), ob_eq))))) {
            continue;
        }
        OboeValue pc = (ob_truthy(ob_bool(ob_truthy(ob_binop("!=", ob_index_get(p, ob_interpolate(1, ob_string("type_name"))), ob_null(), ob_neq)) && ob_truthy(ob_binop("!=", codegen__find_class(ob_index_get(p, ob_interpolate(1, ob_string("type_name")))), ob_null(), ob_neq)))) ? (ob_index_get(p, ob_interpolate(1, ob_string("type_name")))) : (ob_null()));
        (void)(codegen__define_var_full(ob_index_get(p, ob_interpolate(1, ob_string("name"))), pc, ob_null(), ob_index_get(p, ob_interpolate(1, ob_string("type_name")))));
        OboeValue coerced = codegen__apply_numeric_coercion(ob_index_get(p, ob_interpolate(1, ob_string("type_name"))), ob_index_get(p, ob_interpolate(1, ob_string("name"))));
        if (ob_truthy(ob_binop("!=", coerced, ob_index_get(p, ob_interpolate(1, ob_string("name"))), ob_neq))) {
            (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("    ")), ob_index_get(p, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string(" = ")), ob_add), coerced, ob_add), ob_interpolate(1, ob_string(";\n")), ob_add)));
        }
    } }
    return ob_null();
}

OboeValue codegen__has_this_param(OboeValue f) {
    { OboeValue __it = ob_index_get(f, ob_interpolate(1, ob_string("params"))); int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue p = ob_iter_value(__it, __i);
        if (ob_truthy(ob_binop("==", ob_index_get(p, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("this")), ob_eq))) {
            return ob_bool(true);
        }
    } }
    return ob_bool(false);
    return ob_null();
}

OboeValue codegen__gen_func_def(OboeValue prefix, OboeValue owner, OboeValue f) {
    OboeValue is_method = ob_binop("!=", owner, ob_null(), ob_neq);
    OboeValue has_this = codegen__has_this_param(f);
    OboeValue fname_prefix = ob_coalesce(prefix, ob_string(""));
    OboeValue fname = (ob_truthy(ob_bool(ob_truthy(ob_bool(ob_truthy(ob_not(is_method)) && ob_truthy(ob_binop("==", prefix, ob_null(), ob_eq)))) && ob_truthy(ob_binop("==", ob_index_get(f, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("main")), ob_eq)))) ? (ob_interpolate(1, ob_string("oboe_user_main"))) : (ob_index_get(f, ob_interpolate(1, ob_string("name")))));
    (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("OboeValue ")), fname_prefix, ob_add), (ob_truthy(is_method) ? (ob_interpolate(1, ob_string("__"))) : (ob_string(""))), ob_add), fname, ob_add), ob_interpolate(1, ob_string("(")), ob_add)));
    (void)(codegen__gen_param_list((ob_truthy(ob_bool(ob_truthy(is_method) && ob_truthy(has_this))) ? (owner) : (ob_null())), ob_index_get(f, ob_interpolate(1, ob_string("params"))), has_this));
    (void)(codegen__emit(ob_interpolate(1, ob_string(") {\n"))));
    (void)(codegen__push_scope());
    if (ob_truthy(ob_bool(ob_truthy(is_method) && ob_truthy(has_this)))) {
        (void)(codegen__define_var(ob_interpolate(1, ob_string("this")), ob_index_get(owner, ob_interpolate(1, ob_string("name")))));
    }
    (void)(codegen__bind_params(ob_index_get(f, ob_interpolate(1, ob_string("params"))), ob_bool(true)));
    OboeValue saved_class = codegen__CURRENT_CLASS;
    (void)((codegen__CURRENT_CLASS = (ob_truthy(is_method) ? (ob_index_get(owner, ob_interpolate(1, ob_string("name")))) : (ob_null()))));
    (void)(codegen__gen_stmt_list(ob_index_get(f, ob_interpolate(1, ob_string("body"))), ob_int(4LL)));
    (void)((codegen__CURRENT_CLASS = saved_class));
    (void)(codegen__pop_scope());
    (void)(codegen__emit(ob_interpolate(1, ob_string("    return ob_null();\n"))));
    (void)(codegen__emit(ob_interpolate(1, ob_string("}\n\n"))));
    return ob_null();
}

OboeValue codegen__emit_func_prototype(OboeValue prefix, OboeValue owner, OboeValue f) {
    OboeValue is_method = ob_binop("!=", owner, ob_null(), ob_neq);
    OboeValue has_this = codegen__has_this_param(f);
    OboeValue fname_prefix = ob_coalesce(prefix, ob_string(""));
    OboeValue fname = (ob_truthy(ob_bool(ob_truthy(ob_bool(ob_truthy(ob_not(is_method)) && ob_truthy(ob_binop("==", prefix, ob_null(), ob_eq)))) && ob_truthy(ob_binop("==", ob_index_get(f, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("main")), ob_eq)))) ? (ob_interpolate(1, ob_string("oboe_user_main"))) : (ob_index_get(f, ob_interpolate(1, ob_string("name")))));
    (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("OboeValue ")), fname_prefix, ob_add), (ob_truthy(is_method) ? (ob_interpolate(1, ob_string("__"))) : (ob_string(""))), ob_add), fname, ob_add), ob_interpolate(1, ob_string("(")), ob_add)));
    (void)(codegen__gen_param_list((ob_truthy(ob_bool(ob_truthy(is_method) && ob_truthy(has_this))) ? (owner) : (ob_null())), ob_index_get(f, ob_interpolate(1, ob_string("params"))), has_this));
    (void)(codegen__emit(ob_interpolate(1, ob_string(");\n"))));
    return ob_null();
}

OboeValue codegen__emit_ctor_params(OboeValue params, OboeValue void_when_empty) {
    OboeValue first = ob_bool(true);
    { OboeValue __it = params; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue p = ob_iter_value(__it, __i);
        if (ob_truthy(ob_binop("==", ob_index_get(p, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("this")), ob_eq))) {
            continue;
        }
        if (ob_truthy(ob_not(first))) {
            (void)(codegen__emit(ob_interpolate(1, ob_string(", "))));
        }
        (void)(codegen__emit(ob_binop("+", ob_interpolate(1, ob_string("OboeValue ")), ob_index_get(p, ob_interpolate(1, ob_string("name"))), ob_add)));
        (void)((first = ob_bool(false)));
    } }
    if (ob_truthy(ob_bool(ob_truthy(first) && ob_truthy(void_when_empty)))) {
        (void)(codegen__emit(ob_interpolate(1, ob_string("void"))));
    }
    return first;
    return ob_null();
}

OboeValue codegen__emit_class_predecls(OboeValue c) {
    { OboeValue __it = ob_index_get(c, ob_interpolate(1, ob_string("fields"))); int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue fd = ob_iter_value(__it, __i);
        if (ob_truthy(ob_index_get(fd, ob_interpolate(1, ob_string("is_static"))))) {
            (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("static OboeValue ")), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("__")), ob_add), ob_index_get(fd, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string(";\n")), ob_add)));
        }
    } }
    OboeValue init_count = ob_int(0LL);
    OboeValue idx = ob_int(0LL);
    { OboeValue __it = ob_index_get(c, ob_interpolate(1, ob_string("methods"))); int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue m = ob_iter_value(__it, __i);
        if (ob_truthy(ob_binop("==", ob_index_get(m, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("init")), ob_eq))) {
            (void)((init_count = ob_binop("+", init_count, ob_int(1LL), ob_add)));
        }
    } }
    if (ob_truthy(ob_binop("==", init_count, ob_int(0LL), ob_eq))) {
        OboeValue owner = codegen__find_init_owner(c);
        if (ob_truthy(ob_binop("==", owner, ob_null(), ob_eq))) {
            (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("OboeValue ")), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("__new_default(void);\n")), ob_add)));
        }
        else {
            OboeValue inh = ob_int(0LL);
            { OboeValue __it = ob_index_get(owner, ob_interpolate(1, ob_string("methods"))); int64_t __n = ob_iter_len(__it);
            for (int64_t __i = 0; __i < __n; __i++) {
                OboeValue m = ob_iter_value(__it, __i);
                if (ob_truthy(ob_binop("!=", ob_index_get(m, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("init")), ob_neq))) {
                    continue;
                }
                (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("OboeValue ")), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("__new_inh_")), ob_add), ob_str(inh), ob_add), ob_interpolate(1, ob_string("(")), ob_add)));
                (void)(codegen__emit_ctor_params(ob_index_get(m, ob_interpolate(1, ob_string("params"))), ob_bool(true)));
                (void)(codegen__emit(ob_interpolate(1, ob_string(");\n"))));
                (void)((inh = ob_binop("+", inh, ob_int(1LL), ob_add)));
            } }
        }
    }
    { OboeValue __it = ob_index_get(c, ob_interpolate(1, ob_string("methods"))); int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue m = ob_iter_value(__it, __i);
        if (ob_truthy(ob_binop("!=", ob_index_get(m, ob_interpolate(1, ob_string("op_symbol"))), ob_null(), ob_neq))) {
            { OboeValue __it = codegen__CLASS_OPS; int64_t __n = ob_iter_len(__it);
            for (int64_t __i = 0; __i < __n; __i++) {
                OboeValue o = ob_iter_value(__it, __i);
                if (ob_truthy(ob_binop("==", ob_index_get(o, ob_interpolate(1, ob_string("decl"))), m, ob_eq))) {
                    (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("static OboeValue ")), ob_index_get(o, ob_interpolate(1, ob_string("cfunc"))), ob_add), ob_interpolate(1, ob_string("(OboeValue __self, OboeValue __rhs);\n")), ob_add)));
                }
            } }
            continue;
        }
        if (ob_truthy(ob_binop("==", ob_index_get(m, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("init")), ob_eq))) {
            (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("void ")), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("__init_")), ob_add), ob_str(idx), ob_add), ob_interpolate(1, ob_string("(")), ob_add), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string(" *this")), ob_add)));
            { OboeValue __it = ob_index_get(m, ob_interpolate(1, ob_string("params"))); int64_t __n = ob_iter_len(__it);
            for (int64_t __i = 0; __i < __n; __i++) {
                OboeValue p = ob_iter_value(__it, __i);
                if (ob_truthy(ob_binop("==", ob_index_get(p, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("this")), ob_eq))) {
                    continue;
                }
                (void)(codegen__emit(ob_binop("+", ob_interpolate(1, ob_string(", OboeValue ")), ob_index_get(p, ob_interpolate(1, ob_string("name"))), ob_add)));
            } }
            (void)(codegen__emit(ob_interpolate(1, ob_string(");\n"))));
            (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("OboeValue ")), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("__new_")), ob_add), ob_str(idx), ob_add), ob_interpolate(1, ob_string("(")), ob_add)));
            (void)(codegen__emit_ctor_params(ob_index_get(m, ob_interpolate(1, ob_string("params"))), ob_bool(false)));
            (void)(codegen__emit(ob_interpolate(1, ob_string(");\n"))));
            (void)((idx = ob_binop("+", idx, ob_int(1LL), ob_add)));
        }
        else {
            if (ob_truthy(ob_index_get(m, ob_interpolate(1, ob_string("is_static"))))) {
                (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("OboeValue ")), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("__")), ob_add), ob_index_get(m, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("(")), ob_add)));
                OboeValue first = ob_bool(true);
                { OboeValue __it = ob_index_get(m, ob_interpolate(1, ob_string("params"))); int64_t __n = ob_iter_len(__it);
                for (int64_t __i = 0; __i < __n; __i++) {
                    OboeValue p = ob_iter_value(__it, __i);
                    if (ob_truthy(ob_not(first))) {
                        (void)(codegen__emit(ob_interpolate(1, ob_string(", "))));
                    }
                    (void)(codegen__emit(ob_binop("+", ob_interpolate(1, ob_string("OboeValue ")), ob_index_get(p, ob_interpolate(1, ob_string("name"))), ob_add)));
                    (void)((first = ob_bool(false)));
                } }
                (void)(codegen__emit(ob_interpolate(1, ob_string(");\n"))));
            }
            else {
                (void)(codegen__emit_func_prototype(ob_index_get(c, ob_interpolate(1, ob_string("name"))), c, m));
            }
        }
    } }
    return ob_null();
}

OboeValue codegen__class_index(OboeValue c) {
    OboeValue i = ob_int(0LL);
    while (ob_truthy(ob_binop("<", i, ob_m_len(codegen__CLASSES), ob_lt))) {
        if (ob_truthy(ob_binop("==", ob_index_get(codegen__CLASSES, i), c, ob_eq))) {
            return i;
        }
        (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
    }
    return ob_neg(ob_int(1LL));
    return ob_null();
}

OboeValue codegen__emit_class_struct(OboeValue c, OboeValue idx) {
    if (ob_truthy(ob_index_get(codegen__CLASS_EMITTED, idx))) {
        return ob_null();
    }
    OboeValue parent = (ob_truthy(ob_binop("!=", ob_index_get(c, ob_interpolate(1, ob_string("parent_name"))), ob_null(), ob_neq)) ? (codegen__find_class(ob_index_get(c, ob_interpolate(1, ob_string("parent_name"))))) : (ob_null()));
    if (ob_truthy(ob_binop("!=", parent, ob_null(), ob_neq))) {
        OboeValue pi = ob_int(0LL);
        while (ob_truthy(ob_binop("<", pi, ob_m_len(codegen__CLASSES), ob_lt))) {
            if (ob_truthy(ob_binop("==", ob_index_get(codegen__CLASSES, pi), parent, ob_eq))) {
                (void)(codegen__emit_class_struct(parent, pi));
            }
            (void)((pi = ob_binop("+", pi, ob_int(1LL), ob_add)));
        }
    }
    (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("typedef struct ")), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string(" ")), ob_add), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string(";\n")), ob_add)));
    (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("struct ")), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string(" {\n")), ob_add)));
    if (ob_truthy(ob_binop("!=", parent, ob_null(), ob_neq))) {
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("    ")), ob_index_get(parent, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string(" __parent;\n")), ob_add)));
    }
    else {
        (void)(codegen__emit(ob_interpolate(1, ob_string("    OboeObject __base;\n"))));
    }
    { OboeValue __it = ob_index_get(c, ob_interpolate(1, ob_string("fields"))); int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue fd = ob_iter_value(__it, __i);
        if (ob_truthy(ob_index_get(fd, ob_interpolate(1, ob_string("is_static"))))) {
            continue;
        }
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("    OboeValue ")), ob_index_get(fd, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string(";\n")), ob_add)));
    } }
    (void)(codegen__emit(ob_interpolate(1, ob_string("};\n"))));
    (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("static const OboeClassInfo ")), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("__classinfo = { \"")), ob_add), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("\", ")), ob_add), (ob_truthy(ob_binop("!=", parent, ob_null(), ob_neq)) ? (ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("&")), ob_index_get(parent, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("__classinfo")), ob_add)) : (ob_interpolate(1, ob_string("NULL")))), ob_add), ob_interpolate(1, ob_string(" };\n\n")), ob_add)));
    (void)(ob_index_set(codegen__CLASS_EMITTED, idx, ob_bool(true)));
    return ob_null();
}

OboeValue codegen__gen_class(OboeValue c) {
    { OboeValue __it = ob_index_get(c, ob_interpolate(1, ob_string("fields"))); int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue fd = ob_iter_value(__it, __i);
        if (ob_truthy(ob_index_get(fd, ob_interpolate(1, ob_string("is_static"))))) {
            (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("static OboeValue ")), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("__")), ob_add), ob_index_get(fd, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string(" = {0};\n")), ob_add)));
        }
    } }
    OboeValue init_count = ob_int(0LL);
    { OboeValue __it = ob_index_get(c, ob_interpolate(1, ob_string("methods"))); int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue m = ob_iter_value(__it, __i);
        if (ob_truthy(ob_binop("==", ob_index_get(m, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("init")), ob_eq))) {
            (void)((init_count = ob_binop("+", init_count, ob_int(1LL), ob_add)));
        }
    } }
    if (ob_truthy(ob_binop("==", init_count, ob_int(0LL), ob_eq))) {
        OboeValue owner = codegen__find_init_owner(c);
        if (ob_truthy(ob_binop("==", owner, ob_null(), ob_eq))) {
            (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("OboeValue ")), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("__new_default(void) {\n")), ob_add)));
            (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("    ")), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string(" *obj = calloc(1, sizeof(")), ob_add), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("));\n")), ob_add)));
            (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("    ((OboeObject*)obj)->cls = &")), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("__classinfo;\n")), ob_add)));
            (void)(codegen__emit(ob_interpolate(1, ob_string("    return ob_object_wrap(obj);\n"))));
            (void)(codegen__emit(ob_interpolate(1, ob_string("}\n\n"))));
        }
        else {
            OboeValue inh = ob_int(0LL);
            { OboeValue __it = ob_index_get(owner, ob_interpolate(1, ob_string("methods"))); int64_t __n = ob_iter_len(__it);
            for (int64_t __i = 0; __i < __n; __i++) {
                OboeValue m = ob_iter_value(__it, __i);
                if (ob_truthy(ob_binop("!=", ob_index_get(m, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("init")), ob_neq))) {
                    continue;
                }
                (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("OboeValue ")), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("__new_inh_")), ob_add), ob_str(inh), ob_add), ob_interpolate(1, ob_string("(")), ob_add)));
                (void)(codegen__emit_ctor_params(ob_index_get(m, ob_interpolate(1, ob_string("params"))), ob_bool(true)));
                (void)(codegen__emit(ob_interpolate(1, ob_string(") {\n"))));
                (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("    ")), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string(" *obj = calloc(1, sizeof(")), ob_add), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("));\n")), ob_add)));
                (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("    ((OboeObject*)obj)->cls = &")), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("__classinfo;\n")), ob_add)));
                (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("    ")), ob_index_get(owner, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("__init_")), ob_add), ob_str(inh), ob_add), ob_interpolate(1, ob_string("((")), ob_add), ob_index_get(owner, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("*)obj")), ob_add)));
                { OboeValue __it = ob_index_get(m, ob_interpolate(1, ob_string("params"))); int64_t __n = ob_iter_len(__it);
                for (int64_t __i = 0; __i < __n; __i++) {
                    OboeValue p = ob_iter_value(__it, __i);
                    if (ob_truthy(ob_binop("==", ob_index_get(p, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("this")), ob_eq))) {
                        continue;
                    }
                    (void)(codegen__emit(ob_binop("+", ob_interpolate(1, ob_string(", ")), ob_index_get(p, ob_interpolate(1, ob_string("name"))), ob_add)));
                } }
                (void)(codegen__emit(ob_interpolate(1, ob_string(");\n"))));
                (void)(codegen__emit(ob_interpolate(1, ob_string("    return ob_object_wrap(obj);\n"))));
                (void)(codegen__emit(ob_interpolate(1, ob_string("}\n\n"))));
                (void)((inh = ob_binop("+", inh, ob_int(1LL), ob_add)));
            } }
        }
    }
    OboeValue idx = ob_int(0LL);
    (void)(codegen__push_scope());
    { OboeValue __it = ob_index_get(c, ob_interpolate(1, ob_string("methods"))); int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue m = ob_iter_value(__it, __i);
        if (ob_truthy(ob_binop("!=", ob_index_get(m, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("init")), ob_neq))) {
            continue;
        }
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("void ")), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("__init_")), ob_add), ob_str(idx), ob_add), ob_interpolate(1, ob_string("(")), ob_add), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string(" *this")), ob_add)));
        { OboeValue __it = ob_index_get(m, ob_interpolate(1, ob_string("params"))); int64_t __n = ob_iter_len(__it);
        for (int64_t __i = 0; __i < __n; __i++) {
            OboeValue p = ob_iter_value(__it, __i);
            if (ob_truthy(ob_binop("==", ob_index_get(p, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("this")), ob_eq))) {
                continue;
            }
            (void)(codegen__emit(ob_binop("+", ob_interpolate(1, ob_string(", OboeValue ")), ob_index_get(p, ob_interpolate(1, ob_string("name"))), ob_add)));
        } }
        (void)(codegen__emit(ob_interpolate(1, ob_string(") {\n"))));
        (void)(codegen__push_scope());
        (void)(codegen__define_var(ob_interpolate(1, ob_string("this")), ob_index_get(c, ob_interpolate(1, ob_string("name")))));
        (void)(codegen__bind_params(ob_index_get(m, ob_interpolate(1, ob_string("params"))), ob_bool(true)));
        OboeValue saved_class = codegen__CURRENT_CLASS;
        (void)((codegen__CURRENT_CLASS = ob_index_get(c, ob_interpolate(1, ob_string("name")))));
        (void)(codegen__gen_stmt_list(ob_index_get(m, ob_interpolate(1, ob_string("body"))), ob_int(4LL)));
        (void)((codegen__CURRENT_CLASS = saved_class));
        (void)(codegen__pop_scope());
        (void)(codegen__emit(ob_interpolate(1, ob_string("}\n\n"))));
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("OboeValue ")), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("__new_")), ob_add), ob_str(idx), ob_add), ob_interpolate(1, ob_string("(")), ob_add)));
        (void)(codegen__emit_ctor_params(ob_index_get(m, ob_interpolate(1, ob_string("params"))), ob_bool(false)));
        (void)(codegen__emit(ob_interpolate(1, ob_string(") {\n"))));
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("    ")), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string(" *obj = calloc(1, sizeof(")), ob_add), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("));\n")), ob_add)));
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("    ((OboeObject*)obj)->cls = &")), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("__classinfo;\n")), ob_add)));
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("    ")), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("__init_")), ob_add), ob_str(idx), ob_add), ob_interpolate(1, ob_string("(obj")), ob_add)));
        { OboeValue __it = ob_index_get(m, ob_interpolate(1, ob_string("params"))); int64_t __n = ob_iter_len(__it);
        for (int64_t __i = 0; __i < __n; __i++) {
            OboeValue p = ob_iter_value(__it, __i);
            if (ob_truthy(ob_binop("==", ob_index_get(p, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("this")), ob_eq))) {
                continue;
            }
            (void)(codegen__emit(ob_binop("+", ob_interpolate(1, ob_string(", ")), ob_index_get(p, ob_interpolate(1, ob_string("name"))), ob_add)));
        } }
        (void)(codegen__emit(ob_interpolate(1, ob_string(");\n"))));
        (void)(codegen__emit(ob_interpolate(1, ob_string("    return ob_object_wrap(obj);\n"))));
        (void)(codegen__emit(ob_interpolate(1, ob_string("}\n\n"))));
        (void)((idx = ob_binop("+", idx, ob_int(1LL), ob_add)));
    } }
    (void)(codegen__pop_scope());
    { OboeValue __it = ob_index_get(c, ob_interpolate(1, ob_string("methods"))); int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue m = ob_iter_value(__it, __i);
        if (ob_truthy(ob_binop("==", ob_index_get(m, ob_interpolate(1, ob_string("op_symbol"))), ob_null(), ob_eq))) {
            continue;
        }
        OboeValue cfunc = ob_null();
        { OboeValue __it = codegen__CLASS_OPS; int64_t __n = ob_iter_len(__it);
        for (int64_t __i = 0; __i < __n; __i++) {
            OboeValue o = ob_iter_value(__it, __i);
            if (ob_truthy(ob_binop("==", ob_index_get(o, ob_interpolate(1, ob_string("decl"))), m, ob_eq))) {
                (void)((cfunc = ob_index_get(o, ob_interpolate(1, ob_string("cfunc")))));
            }
        } }
        OboeValue params = ob_index_get(m, ob_interpolate(1, ob_string("params")));
        if (ob_truthy(ob_bool(ob_truthy(ob_binop("!=", ob_m_len(params), ob_int(2LL), ob_neq)) || ob_truthy(ob_binop("!=", ob_index_get(ob_index_get(params, ob_int(0LL)), ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("this")), ob_neq))))) {
            (void)(codegen__codegen_error(ob_index_get(m, ob_interpolate(1, ob_string("line"))), ob_interpolate(1, ob_string("a class operator must take exactly (this, other)"))));
        }
        OboeValue rhs = ob_index_get(params, ob_int(1LL));
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("static OboeValue ")), cfunc, ob_add), ob_interpolate(1, ob_string("(OboeValue __self, OboeValue ")), ob_add), ob_index_get(rhs, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string(") {\n")), ob_add)));
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("    ")), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string(" *this = (")), ob_add), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("*)__self.as.obj;\n")), ob_add)));
        (void)(codegen__emit(ob_interpolate(1, ob_string("    (void)this;\n"))));
        (void)(codegen__push_scope());
        (void)(codegen__define_var(ob_interpolate(1, ob_string("this")), ob_index_get(c, ob_interpolate(1, ob_string("name")))));
        (void)(codegen__define_var(ob_index_get(rhs, ob_interpolate(1, ob_string("name"))), (ob_truthy(ob_bool(ob_truthy(ob_binop("!=", ob_index_get(rhs, ob_interpolate(1, ob_string("type_name"))), ob_null(), ob_neq)) && ob_truthy(ob_binop("!=", codegen__find_class(ob_index_get(rhs, ob_interpolate(1, ob_string("type_name")))), ob_null(), ob_neq)))) ? (ob_index_get(rhs, ob_interpolate(1, ob_string("type_name")))) : (ob_null()))));
        OboeValue saved_class = codegen__CURRENT_CLASS;
        (void)((codegen__CURRENT_CLASS = ob_index_get(c, ob_interpolate(1, ob_string("name")))));
        (void)(codegen__gen_stmt_list(ob_index_get(m, ob_interpolate(1, ob_string("body"))), ob_int(4LL)));
        (void)((codegen__CURRENT_CLASS = saved_class));
        (void)(codegen__pop_scope());
        (void)(codegen__emit(ob_interpolate(1, ob_string("    return ob_null();\n"))));
        (void)(codegen__emit(ob_interpolate(1, ob_string("}\n\n"))));
    } }
    { OboeValue __it = ob_index_get(c, ob_interpolate(1, ob_string("methods"))); int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue m = ob_iter_value(__it, __i);
        if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", ob_index_get(m, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("init")), ob_eq)) || ob_truthy(ob_binop("!=", ob_index_get(m, ob_interpolate(1, ob_string("op_symbol"))), ob_null(), ob_neq))))) {
            continue;
        }
        if (ob_truthy(ob_index_get(m, ob_interpolate(1, ob_string("is_static"))))) {
            (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("OboeValue ")), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("__")), ob_add), ob_index_get(m, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("(")), ob_add)));
            OboeValue first = ob_bool(true);
            { OboeValue __it = ob_index_get(m, ob_interpolate(1, ob_string("params"))); int64_t __n = ob_iter_len(__it);
            for (int64_t __i = 0; __i < __n; __i++) {
                OboeValue p = ob_iter_value(__it, __i);
                if (ob_truthy(ob_not(first))) {
                    (void)(codegen__emit(ob_interpolate(1, ob_string(", "))));
                }
                (void)(codegen__emit(ob_binop("+", ob_interpolate(1, ob_string("OboeValue ")), ob_index_get(p, ob_interpolate(1, ob_string("name"))), ob_add)));
                (void)((first = ob_bool(false)));
            } }
            (void)(codegen__emit(ob_interpolate(1, ob_string(") {\n"))));
            (void)(codegen__push_scope());
            (void)(codegen__bind_params(ob_index_get(m, ob_interpolate(1, ob_string("params"))), ob_bool(false)));
            OboeValue saved_class = codegen__CURRENT_CLASS;
            (void)((codegen__CURRENT_CLASS = ob_index_get(c, ob_interpolate(1, ob_string("name")))));
            (void)(codegen__gen_stmt_list(ob_index_get(m, ob_interpolate(1, ob_string("body"))), ob_int(4LL)));
            (void)((codegen__CURRENT_CLASS = saved_class));
            (void)(codegen__pop_scope());
            (void)(codegen__emit(ob_interpolate(1, ob_string("    return ob_null();\n"))));
            (void)(codegen__emit(ob_interpolate(1, ob_string("}\n\n"))));
        }
        else {
            (void)(codegen__gen_func_def(ob_index_get(c, ob_interpolate(1, ob_string("name"))), c, m));
        }
    } }
    return ob_null();
}

OboeValue codegen__note_field(OboeValue c, OboeValue name, OboeValue type_hint) {
    OboeValue got = codegen__find_field_owner(c, name);
    if (ob_truthy(ob_binop("!=", got, ob_null(), ob_neq))) {
        if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", ob_index_get(ob_index_get(got, ob_interpolate(1, ob_string("field"))), ob_interpolate(1, ob_string("type_name"))), ob_null(), ob_eq)) && ob_truthy(ob_binop("!=", type_hint, ob_null(), ob_neq))))) {
            (void)(ob_index_set(ob_index_get(got, ob_interpolate(1, ob_string("field"))), ob_interpolate(1, ob_string("type_name")), type_hint));
        }
        return ob_null();
    }
    (void)(ob_arr_insert(ob_index_get(c, ob_interpolate(1, ob_string("fields"))), ob_int(0LL), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("name"))), name); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("type_name"))), type_hint); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_static"))), ob_bool(false)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_private"))), ob_bool(false)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_const"))), ob_bool(false)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("init"))), ob_null()); __d; })));
    return ob_null();
}

OboeValue codegen__field_type_hint(OboeValue value) {
    if (ob_truthy(ob_binop("==", value, ob_null(), ob_eq))) {
        return ob_null();
    }
    if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", ob_index_get(value, ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("EXPR_IDENT")), ob_eq)) && ob_truthy(ob_binop("!=", codegen__SCAN_METHOD, ob_null(), ob_neq))))) {
        { OboeValue __it = ob_index_get(codegen__SCAN_METHOD, ob_interpolate(1, ob_string("params"))); int64_t __n = ob_iter_len(__it);
        for (int64_t __i = 0; __i < __n; __i++) {
            OboeValue p = ob_iter_value(__it, __i);
            if (ob_truthy(ob_binop("==", ob_index_get(p, ob_interpolate(1, ob_string("name"))), ob_index_get(value, ob_interpolate(1, ob_string("ident"))), ob_eq))) {
                return (ob_truthy(ob_bool(ob_truthy(ob_binop("!=", ob_index_get(p, ob_interpolate(1, ob_string("type_name"))), ob_null(), ob_neq)) && ob_truthy(ob_binop("!=", codegen__find_class(ob_index_get(p, ob_interpolate(1, ob_string("type_name")))), ob_null(), ob_neq)))) ? (ob_index_get(p, ob_interpolate(1, ob_string("type_name")))) : (ob_null()));
            }
        } }
    }
    if (ob_truthy(ob_bool(ob_truthy(ob_bool(ob_truthy(ob_binop("==", ob_index_get(value, ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("EXPR_CALL")), ob_eq)) && ob_truthy(ob_binop("==", ob_index_get(ob_index_get(value, ob_interpolate(1, ob_string("callee"))), ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("EXPR_IDENT")), ob_eq)))) && ob_truthy(ob_binop("!=", codegen__find_class(ob_index_get(ob_index_get(value, ob_interpolate(1, ob_string("callee"))), ob_interpolate(1, ob_string("ident")))), ob_null(), ob_neq))))) {
        return ob_index_get(ob_index_get(value, ob_interpolate(1, ob_string("callee"))), ob_interpolate(1, ob_string("ident")));
    }
    return ob_null();
    return ob_null();
}

OboeValue codegen__scan_expr_for_fields(OboeValue c, OboeValue e) {
    if (ob_truthy(ob_binop("==", e, ob_null(), ob_eq))) {
        return ob_null();
    }
    OboeValue k = ob_index_get(e, ob_interpolate(1, ob_string("kind")));
    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_ASSIGN")), ob_eq))) {
        OboeValue t = ob_index_get(e, ob_interpolate(1, ob_string("target")));
        if (ob_truthy(ob_bool(ob_truthy(ob_bool(ob_truthy(ob_binop("==", ob_index_get(t, ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("EXPR_FIELD")), ob_eq)) && ob_truthy(ob_binop("==", ob_index_get(ob_index_get(t, ob_interpolate(1, ob_string("obj"))), ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("EXPR_IDENT")), ob_eq)))) && ob_truthy(ob_binop("==", ob_index_get(ob_index_get(t, ob_interpolate(1, ob_string("obj"))), ob_interpolate(1, ob_string("ident"))), ob_interpolate(1, ob_string("this")), ob_eq))))) {
            (void)(codegen__note_field(c, ob_index_get(t, ob_interpolate(1, ob_string("name"))), codegen__field_type_hint(ob_index_get(e, ob_interpolate(1, ob_string("value"))))));
        }
        (void)(codegen__scan_expr_for_fields(c, ob_index_get(e, ob_interpolate(1, ob_string("target")))));
        (void)(codegen__scan_expr_for_fields(c, ob_index_get(e, ob_interpolate(1, ob_string("value")))));
    }
    else {
        if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_BINARY")), ob_eq))) {
            (void)(codegen__scan_expr_for_fields(c, ob_index_get(e, ob_interpolate(1, ob_string("l")))));
            (void)(codegen__scan_expr_for_fields(c, ob_index_get(e, ob_interpolate(1, ob_string("r")))));
        }
        else {
            if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_UNARY")), ob_eq))) {
                (void)(codegen__scan_expr_for_fields(c, ob_index_get(e, ob_interpolate(1, ob_string("operand")))));
            }
            else {
                if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_IS")), ob_eq))) {
                    (void)(codegen__scan_expr_for_fields(c, ob_index_get(e, ob_interpolate(1, ob_string("value")))));
                }
                else {
                    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_INDEX")), ob_eq))) {
                        (void)(codegen__scan_expr_for_fields(c, ob_index_get(e, ob_interpolate(1, ob_string("arr")))));
                        (void)(codegen__scan_expr_for_fields(c, ob_index_get(e, ob_interpolate(1, ob_string("idx")))));
                    }
                    else {
                        if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_FIELD")), ob_eq)) || ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_SAFE_FIELD")), ob_eq))))) {
                            (void)(codegen__scan_expr_for_fields(c, ob_index_get(e, ob_interpolate(1, ob_string("obj")))));
                        }
                        else {
                            if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_TERNARY")), ob_eq))) {
                                (void)(codegen__scan_expr_for_fields(c, ob_index_get(e, ob_interpolate(1, ob_string("cond")))));
                                (void)(codegen__scan_expr_for_fields(c, ob_index_get(e, ob_interpolate(1, ob_string("then_e")))));
                                (void)(codegen__scan_expr_for_fields(c, ob_index_get(e, ob_interpolate(1, ob_string("else_e")))));
                            }
                            else {
                                if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_CALL")), ob_eq))) {
                                    (void)(codegen__scan_expr_for_fields(c, ob_index_get(e, ob_interpolate(1, ob_string("callee")))));
                                    { OboeValue __it = ob_index_get(e, ob_interpolate(1, ob_string("args"))); int64_t __n = ob_iter_len(__it);
                                    for (int64_t __i = 0; __i < __n; __i++) {
                                        OboeValue a = ob_iter_value(__it, __i);
                                        (void)(codegen__scan_expr_for_fields(c, a));
                                    } }
                                }
                                else {
                                    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_ARRAY")), ob_eq))) {
                                        { OboeValue __it = ob_index_get(e, ob_interpolate(1, ob_string("items"))); int64_t __n = ob_iter_len(__it);
                                        for (int64_t __i = 0; __i < __n; __i++) {
                                            OboeValue item = ob_iter_value(__it, __i);
                                            (void)(codegen__scan_expr_for_fields(c, item));
                                        } }
                                    }
                                    else {
                                        if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_DICT")), ob_eq))) {
                                            OboeValue i = ob_int(0LL);
                                            while (ob_truthy(ob_binop("<", i, ob_m_len(ob_index_get(e, ob_interpolate(1, ob_string("keys")))), ob_lt))) {
                                                (void)(codegen__scan_expr_for_fields(c, ob_index_get(ob_index_get(e, ob_interpolate(1, ob_string("keys"))), i)));
                                                (void)(codegen__scan_expr_for_fields(c, ob_index_get(ob_index_get(e, ob_interpolate(1, ob_string("values"))), i)));
                                                (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
                                            }
                                        }
                                        else {
                                            if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("EXPR_STRING")), ob_eq))) {
                                                { OboeValue __it = ob_index_get(e, ob_interpolate(1, ob_string("str_parts"))); int64_t __n = ob_iter_len(__it);
                                                for (int64_t __i = 0; __i < __n; __i++) {
                                                    OboeValue p = ob_iter_value(__it, __i);
                                                    if (ob_truthy(ob_index_get(p, ob_interpolate(1, ob_string("is_expr"))))) {
                                                        (void)(codegen__scan_expr_for_fields(c, ob_index_get(p, ob_interpolate(1, ob_string("expr")))));
                                                    }
                                                } }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return ob_null();
}

OboeValue codegen__scan_stmt_list_for_fields(OboeValue c, OboeValue body) {
    { OboeValue __it = body; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue s = ob_iter_value(__it, __i);
        OboeValue k = ob_index_get(s, ob_interpolate(1, ob_string("kind")));
        if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("STMT_LET")), ob_eq))) {
            (void)(codegen__scan_expr_for_fields(c, ob_index_get(s, ob_interpolate(1, ob_string("init")))));
        }
        else {
            if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("STMT_EXPR")), ob_eq))) {
                (void)(codegen__scan_expr_for_fields(c, ob_index_get(s, ob_interpolate(1, ob_string("expr")))));
            }
            else {
                if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("STMT_RETURN")), ob_eq))) {
                    (void)(codegen__scan_expr_for_fields(c, ob_index_get(s, ob_interpolate(1, ob_string("value")))));
                }
                else {
                    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("STMT_IF")), ob_eq))) {
                        (void)(codegen__scan_expr_for_fields(c, ob_index_get(s, ob_interpolate(1, ob_string("cond")))));
                        (void)(codegen__scan_stmt_list_for_fields(c, ob_index_get(s, ob_interpolate(1, ob_string("then_body")))));
                        (void)(codegen__scan_stmt_list_for_fields(c, ob_index_get(s, ob_interpolate(1, ob_string("else_body")))));
                    }
                    else {
                        if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("STMT_WHILE")), ob_eq))) {
                            (void)(codegen__scan_expr_for_fields(c, ob_index_get(s, ob_interpolate(1, ob_string("cond")))));
                            (void)(codegen__scan_stmt_list_for_fields(c, ob_index_get(s, ob_interpolate(1, ob_string("body")))));
                        }
                        else {
                            if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("STMT_FOR")), ob_eq))) {
                                (void)(codegen__scan_expr_for_fields(c, ob_index_get(s, ob_interpolate(1, ob_string("range_a")))));
                                (void)(codegen__scan_expr_for_fields(c, ob_index_get(s, ob_interpolate(1, ob_string("range_b")))));
                                (void)(codegen__scan_expr_for_fields(c, ob_index_get(s, ob_interpolate(1, ob_string("iterable")))));
                                (void)(codegen__scan_stmt_list_for_fields(c, ob_index_get(s, ob_interpolate(1, ob_string("body")))));
                            }
                            else {
                                if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("STMT_SWITCH")), ob_eq))) {
                                    (void)(codegen__scan_expr_for_fields(c, ob_index_get(s, ob_interpolate(1, ob_string("subject")))));
                                    { OboeValue __it = ob_index_get(s, ob_interpolate(1, ob_string("cases"))); int64_t __n = ob_iter_len(__it);
                                    for (int64_t __i = 0; __i < __n; __i++) {
                                        OboeValue cc = ob_iter_value(__it, __i);
                                        (void)(codegen__scan_stmt_list_for_fields(c, ob_index_get(cc, ob_interpolate(1, ob_string("body")))));
                                    } }
                                }
                                else {
                                    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("STMT_TRY")), ob_eq))) {
                                        (void)(codegen__scan_stmt_list_for_fields(c, ob_index_get(s, ob_interpolate(1, ob_string("body")))));
                                        { OboeValue __it = ob_index_get(s, ob_interpolate(1, ob_string("catches"))); int64_t __n = ob_iter_len(__it);
                                        for (int64_t __i = 0; __i < __n; __i++) {
                                            OboeValue cc = ob_iter_value(__it, __i);
                                            (void)(codegen__scan_stmt_list_for_fields(c, ob_index_get(cc, ob_interpolate(1, ob_string("body")))));
                                        } }
                                        (void)(codegen__scan_stmt_list_for_fields(c, ob_index_get(s, ob_interpolate(1, ob_string("finally_body")))));
                                    }
                                    else {
                                        if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("STMT_THROW")), ob_eq))) {
                                            (void)(codegen__scan_expr_for_fields(c, ob_index_get(s, ob_interpolate(1, ob_string("value")))));
                                        }
                                        else {
                                            if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("STMT_BLOCK")), ob_eq))) {
                                                (void)(codegen__scan_stmt_list_for_fields(c, ob_index_get(s, ob_interpolate(1, ob_string("body")))));
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    } }
    return ob_null();
}

OboeValue codegen__infer_instance_fields(OboeValue c) {
    { OboeValue __it = codegen__INFERRED; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue seen = ob_iter_value(__it, __i);
        if (ob_truthy(ob_binop("==", seen, c, ob_eq))) {
            return ob_null();
        }
    } }
    (void)(ob_arr_push(codegen__INFERRED, c));
    OboeValue parent = (ob_truthy(ob_binop("!=", ob_index_get(c, ob_interpolate(1, ob_string("parent_name"))), ob_null(), ob_neq)) ? (codegen__find_class(ob_index_get(c, ob_interpolate(1, ob_string("parent_name"))))) : (ob_null()));
    if (ob_truthy(ob_binop("!=", parent, ob_null(), ob_neq))) {
        (void)(codegen__infer_instance_fields(parent));
    }
    { OboeValue __it = ob_index_get(c, ob_interpolate(1, ob_string("methods"))); int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue m = ob_iter_value(__it, __i);
        (void)((codegen__SCAN_METHOD = m));
        (void)(codegen__scan_stmt_list_for_fields(c, ob_index_get(m, ob_interpolate(1, ob_string("body")))));
    } }
    (void)((codegen__SCAN_METHOD = ob_null()));
    return ob_null();
}

OboeValue codegen__add_class(OboeValue c, OboeValue unit_prefix) {
    (void)(ob_index_set(c, ob_interpolate(1, ob_string("unit_prefix")), unit_prefix));
    (void)(ob_arr_push(codegen__CLASSES, c));
    (void)(ob_arr_push(codegen__CLASS_EMITTED, ob_bool(false)));
    { OboeValue __it = ob_index_get(c, ob_interpolate(1, ob_string("methods"))); int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue m = ob_iter_value(__it, __i);
        if (ob_truthy(ob_binop("==", ob_index_get(m, ob_interpolate(1, ob_string("op_symbol"))), ob_null(), ob_eq))) {
            continue;
        }
        (void)(ob_arr_push(codegen__CLASS_OPS, ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cls"))), c); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("symbol"))), ob_index_get(m, ob_interpolate(1, ob_string("op_symbol")))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_binop("+", ob_binop("+", ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("__opov_")), ob_add), ob_str(ob_m_len(codegen__CLASS_OPS)), ob_add)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("decl"))), m); __d; })));
    } }
    return ob_null();
}

OboeValue codegen__collect_classes(OboeValue decls, OboeValue unit_prefix) {
    { OboeValue __it = decls; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue d = ob_iter_value(__it, __i);
        if (ob_truthy(ob_binop("==", ob_index_get(d, ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("DECL_CLASS")), ob_eq))) {
            (void)(codegen__add_class(ob_index_get(d, ob_interpolate(1, ob_string("klass"))), unit_prefix));
        }
    } }
    return ob_null();
}

OboeValue codegen__read_whole_file(OboeValue path) {
    { OboeExceptionFrame __frame_0; __frame_0.prev = ob_exc_stack; ob_exc_stack = &__frame_0;
    bool __rethrow_0 = false;
    if (setjmp(__frame_0.buf) == 0) {
        { OboeValue __ret = ob_std_os_read_file(path); ob_exc_stack = __frame_0.prev; return __ret; }
        ob_exc_stack = __frame_0.prev;
    } else {
        if (ob_exception_matches("os.FileNotFoundError")) {
            OboeValue e = ob_current_exception;
            { OboeValue __ret = ob_null(); ob_exc_stack = __frame_0.prev; return __ret; }
        }
        else { __rethrow_0 = true; }
    }
    if (__rethrow_0) ob_throw(ob_current_exception_type, ob_current_exception);
    }
    return ob_null();
}

OboeValue codegen__codegen_set_source_dir(OboeValue dir) {
    (void)((codegen__SOURCE_DIR = dir));
    return ob_null();
}

OboeValue codegen__find_unit(OboeValue module) {
    OboeValue i = ob_int(0LL);
    while (ob_truthy(ob_binop("<", i, ob_m_len(codegen__UNITS), ob_lt))) {
        if (ob_truthy(ob_binop("==", ob_index_get(ob_index_get(codegen__UNITS, i), ob_interpolate(1, ob_string("module"))), module, ob_eq))) {
            return i;
        }
        (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
    }
    return ob_neg(ob_int(1LL));
    return ob_null();
}

OboeValue codegen__codegen_set_target_os(OboeValue target) {
    (void)((codegen__TARGET_OS = target));
    return ob_null();
}

OboeValue codegen__is_json_ws(OboeValue c) {
    return ob_bool(ob_truthy(ob_bool(ob_truthy(ob_bool(ob_truthy(ob_binop("==", c, ob_int(32LL), ob_eq)) || ob_truthy(ob_binop("==", c, ob_int(9LL), ob_eq)))) || ob_truthy(ob_binop("==", c, ob_int(10LL), ob_eq)))) || ob_truthy(ob_binop("==", c, ob_int(13LL), ob_eq)));
    return ob_null();
}

OboeValue codegen__json_skip_ws(OboeValue s, OboeValue i) {
    OboeValue n = ob_m_len(s);
    while (ob_truthy(ob_bool(ob_truthy(ob_binop("<", i, n, ob_lt)) && ob_truthy(codegen__is_json_ws(codegen__byte_of(s, i)))))) {
        (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
    }
    return i;
    return ob_null();
}

OboeValue codegen__json_string_field(OboeValue json, OboeValue field) {
    OboeValue n = ob_m_len(json);
    OboeValue start = codegen__json_skip_ws(json, ob_int(0LL));
    if (ob_truthy(ob_binop("==", codegen__byte_of(json, start), ob_int(123LL), ob_eq))) {
        (void)((start = ob_binop("+", start, ob_int(1LL), ob_add)));
    }
    OboeValue in_string = ob_bool(false);
    OboeValue escaped = ob_bool(false);
    OboeValue key_start = ob_neg(ob_int(1LL));
    OboeValue p = start;
    while (ob_truthy(ob_binop("<", p, n, ob_lt))) {
        OboeValue c = codegen__byte_of(json, p);
        if (ob_truthy(in_string)) {
            if (ob_truthy(escaped)) {
                (void)((escaped = ob_bool(false)));
            }
            else {
                if (ob_truthy(ob_binop("==", c, ob_int(92LL), ob_eq))) {
                    (void)((escaped = ob_bool(true)));
                }
                else {
                    if (ob_truthy(ob_binop("==", c, ob_int(34LL), ob_eq))) {
                        (void)((in_string = ob_bool(false)));
                        if (ob_truthy(ob_binop(">=", key_start, ob_int(0LL), ob_gte))) {
                            OboeValue after = codegen__json_skip_ws(json, ob_binop("+", p, ob_int(1LL), ob_add));
                            if (ob_truthy(ob_binop("==", codegen__byte_of(json, after), ob_int(58LL), ob_eq))) {
                                OboeValue key = ob_str_substr(json, key_start, ob_binop("-", p, key_start, ob_sub));
                                if (ob_truthy(ob_binop("==", key, field, ob_eq))) {
                                    return codegen__json_read_string(json, codegen__json_skip_ws(json, ob_binop("+", after, ob_int(1LL), ob_add)));
                                }
                            }
                        }
                        (void)((key_start = ob_neg(ob_int(1LL))));
                    }
                }
            }
        }
        else {
            if (ob_truthy(ob_binop("==", c, ob_int(34LL), ob_eq))) {
                (void)((in_string = ob_bool(true)));
                (void)((key_start = ob_binop("+", p, ob_int(1LL), ob_add)));
            }
            else {
                if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", c, ob_int(47LL), ob_eq)) && ob_truthy(ob_binop("==", codegen__byte_of(json, ob_binop("+", p, ob_int(1LL), ob_add)), ob_int(47LL), ob_eq))))) {
                    while (ob_truthy(ob_bool(ob_truthy(ob_binop("!=", codegen__byte_of(json, ob_binop("+", p, ob_int(1LL), ob_add)), ob_neg(ob_int(1LL)), ob_neq)) && ob_truthy(ob_binop("!=", codegen__byte_of(json, ob_binop("+", p, ob_int(1LL), ob_add)), ob_int(10LL), ob_neq))))) {
                        (void)((p = ob_binop("+", p, ob_int(1LL), ob_add)));
                    }
                }
            }
        }
        (void)((p = ob_binop("+", p, ob_int(1LL), ob_add)));
    }
    return ob_null();
    return ob_null();
}

OboeValue codegen__json_read_string(OboeValue s, OboeValue p) {
    if (ob_truthy(ob_binop("!=", codegen__byte_of(s, p), ob_int(34LL), ob_neq))) {
        return ob_null();
    }
    (void)((p = ob_binop("+", p, ob_int(1LL), ob_add)));
    OboeValue n = ob_m_len(s);
    OboeValue q = p;
    while (ob_truthy(ob_bool(ob_truthy(ob_binop("<", q, n, ob_lt)) && ob_truthy(ob_binop("!=", codegen__byte_of(s, q), ob_int(34LL), ob_neq))))) {
        (void)((q = ob_binop("+", q, ob_int(1LL), ob_add)));
    }
    if (ob_truthy(ob_binop(">=", q, n, ob_gte))) {
        return ob_null();
    }
    return ob_str_substr(s, p, ob_binop("-", q, p, ob_sub));
    return ob_null();
}

OboeValue codegen__folder_entry_path(OboeValue folder) {
    OboeValue pj = codegen__find_project_json_in(folder);
    OboeValue json = (ob_truthy(ob_binop("!=", pj, ob_null(), ob_neq)) ? (codegen__read_whole_file(pj)) : (ob_null()));
    if (ob_truthy(ob_binop("!=", json, ob_null(), ob_neq))) {
        OboeValue entry = codegen__json_string_field(json, ob_interpolate(1, ob_string("entry")));
        if (ob_truthy(ob_binop("!=", entry, ob_null(), ob_neq))) {
            OboeValue path = ob_binop("+", ob_binop("+", folder, ob_interpolate(1, ob_string("/")), ob_add), entry, ob_add);
            return (ob_truthy(codegen__file_exists(path)) ? (path) : (ob_null()));
        }
    }
    OboeValue main_path = ob_binop("+", folder, ob_interpolate(1, ob_string("/main.oboe")), ob_add);
    return (ob_truthy(codegen__file_exists(main_path)) ? (main_path) : (ob_null()));
    return ob_null();
}

OboeValue codegen__folder_project_name(OboeValue folder) {
    OboeValue pj = codegen__find_project_json_in(folder);
    OboeValue json = (ob_truthy(ob_binop("!=", pj, ob_null(), ob_neq)) ? (codegen__read_whole_file(pj)) : (ob_null()));
    if (ob_truthy(ob_binop("==", json, ob_null(), ob_eq))) {
        return ob_null();
    }
    return codegen__json_string_field(json, ob_interpolate(1, ob_string("name")));
    return ob_null();
}

OboeValue codegen__resolve_folder_module(OboeValue dir, OboeValue module) {
    OboeValue entries = ({ OboeValue __a = ob_array_new(); __a; });
    { OboeExceptionFrame __frame_0; __frame_0.prev = ob_exc_stack; ob_exc_stack = &__frame_0;
    bool __rethrow_0 = false;
    if (setjmp(__frame_0.buf) == 0) {
        (void)((entries = ob_std_os_listdir(dir)));
        ob_exc_stack = __frame_0.prev;
    } else {
        if (ob_exception_matches("os.FileNotFoundError")) {
            OboeValue e = ob_current_exception;
            (void)((entries = ({ OboeValue __a = ob_array_new(); __a; })));
        }
        else { __rethrow_0 = true; }
    }
    if (__rethrow_0) ob_throw(ob_current_exception_type, ob_current_exception);
    }
    { OboeValue __it = entries; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue name = ob_iter_value(__it, __i);
        if (ob_truthy(ob_binop("==", ob_str_substr(name, ob_int(0LL), ob_int(1LL)), ob_interpolate(1, ob_string(".")), ob_eq))) {
            continue;
        }
        OboeValue sub = ob_binop("+", ob_binop("+", dir, ob_interpolate(1, ob_string("/")), ob_add), name, ob_add);
        if (ob_truthy(ob_not(ob_std_os_is_dir(sub)))) {
            continue;
        }
        OboeValue pname = codegen__folder_project_name(sub);
        if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", pname, ob_null(), ob_eq)) || ob_truthy(ob_binop("!=", pname, module, ob_neq))))) {
            continue;
        }
        return codegen__folder_entry_path(sub);
    } }
    OboeValue direct = ob_binop("+", ob_binop("+", dir, ob_interpolate(1, ob_string("/")), ob_add), module, ob_add);
    if (ob_truthy(ob_std_os_is_dir(direct))) {
        return codegen__folder_entry_path(direct);
    }
    return ob_null();
    return ob_null();
}

OboeValue codegen__codegen_set_library_root(OboeValue main_path) {
    OboeValue abs = ob_coalesce(ob_std_os_realpath(main_path), main_path);
    OboeValue root = codegen__walk_to_project_root(codegen__path_dirname(abs));
    (void)((codegen__LIB_ROOT = ob_binop("+", root, ob_interpolate(1, ob_string("/.oboe/libraries")), ob_add)));
    return ob_null();
}

OboeValue codegen__resolve_module_path(OboeValue dir, OboeValue module) {
    OboeValue dirs = ({ OboeValue __a = ob_array_new(); ob_array_push(__a, dir); ob_array_push(__a, ob_binop("+", dir, ob_interpolate(1, ob_string("/.oboe/libraries")), ob_add)); __a; });
    OboeValue suffixed = ob_binop("+", ob_binop("+", module, ob_interpolate(1, ob_string(".")), ob_add), codegen__TARGET_OS, ob_add);
    OboeValue names = ({ OboeValue __a = ob_array_new(); ob_array_push(__a, suffixed); ob_array_push(__a, module); __a; });
    { OboeValue __it = dirs; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue root = ob_iter_value(__it, __i);
        { OboeValue __it = names; int64_t __n = ob_iter_len(__it);
        for (int64_t __i = 0; __i < __n; __i++) {
            OboeValue name = ob_iter_value(__it, __i);
            OboeValue path = ob_binop("+", ob_binop("+", ob_binop("+", root, ob_interpolate(1, ob_string("/")), ob_add), name, ob_add), ob_interpolate(1, ob_string(".oboe")), ob_add);
            if (ob_truthy(codegen__file_exists(path))) {
                return path;
            }
        } }
        OboeValue folder = codegen__resolve_folder_module(root, module);
        if (ob_truthy(ob_binop("!=", folder, ob_null(), ob_neq))) {
            return folder;
        }
    } }
    if (ob_truthy(ob_binop("!=", codegen__LIB_ROOT, ob_string(""), ob_neq))) {
        { OboeValue __it = names; int64_t __n = ob_iter_len(__it);
        for (int64_t __i = 0; __i < __n; __i++) {
            OboeValue name = ob_iter_value(__it, __i);
            OboeValue path = ob_binop("+", ob_binop("+", ob_binop("+", codegen__LIB_ROOT, ob_interpolate(1, ob_string("/")), ob_add), name, ob_add), ob_interpolate(1, ob_string(".oboe")), ob_add);
            if (ob_truthy(codegen__file_exists(path))) {
                return path;
            }
        } }
        OboeValue folder = codegen__resolve_folder_module(codegen__LIB_ROOT, module);
        if (ob_truthy(ob_binop("!=", folder, ob_null(), ob_neq))) {
            return folder;
        }
    }
    return ob_null();
    return ob_null();
}

OboeValue codegen__strip_for_scan(OboeValue src) {
    OboeValue out = ob_str_split(src, ob_string(""));
    OboeValue len = ob_m_len(out);
    OboeValue i = ob_int(0LL);
    while (ob_truthy(ob_binop("<", i, len, ob_lt))) {
        if (ob_truthy(ob_bool(ob_truthy(ob_bool(ob_truthy(ob_binop("==", ob_index_get(out, i), ob_interpolate(1, ob_string("/")), ob_eq)) && ob_truthy(ob_binop("<", ob_binop("+", i, ob_int(1LL), ob_add), len, ob_lt)))) && ob_truthy(ob_binop("==", ob_index_get(out, ob_binop("+", i, ob_int(1LL), ob_add)), ob_interpolate(1, ob_string("/")), ob_eq))))) {
            while (ob_truthy(ob_bool(ob_truthy(ob_binop("<", i, len, ob_lt)) && ob_truthy(ob_binop("!=", ob_index_get(out, i), ob_interpolate(1, ob_string("\n")), ob_neq))))) {
                (void)(ob_index_set(out, i, ob_interpolate(1, ob_string(" "))));
                (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
            }
        }
        else {
            if (ob_truthy(ob_binop("==", ob_index_get(out, i), ob_interpolate(1, ob_string("\"")), ob_eq))) {
                (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
                while (ob_truthy(ob_bool(ob_truthy(ob_binop("<", i, len, ob_lt)) && ob_truthy(ob_binop("!=", ob_index_get(out, i), ob_interpolate(1, ob_string("\"")), ob_neq))))) {
                    if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", ob_index_get(out, i), ob_interpolate(1, ob_string("\\")), ob_eq)) && ob_truthy(ob_binop("<", ob_binop("+", i, ob_int(1LL), ob_add), len, ob_lt))))) {
                        (void)(ob_index_set(out, ob_binop("+", i, ob_int(1LL), ob_add), ob_interpolate(1, ob_string(" "))));
                    }
                    if (ob_truthy(ob_binop("!=", ob_index_get(out, i), ob_interpolate(1, ob_string("\n")), ob_neq))) {
                        (void)(ob_index_set(out, i, ob_interpolate(1, ob_string(" "))));
                    }
                    (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
                }
            }
        }
        (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
    }
    return ob_arr_join(out, ob_string(""));
    return ob_null();
}

OboeValue codegen__is_ident_char(OboeValue c) {
    return ob_bool(ob_truthy(ob_bool(ob_truthy(ob_bool(ob_truthy(ob_bool(ob_truthy(ob_binop(">=", c, ob_int(97LL), ob_gte)) && ob_truthy(ob_binop("<=", c, ob_int(122LL), ob_lte)))) || ob_truthy(ob_bool(ob_truthy(ob_binop(">=", c, ob_int(65LL), ob_gte)) && ob_truthy(ob_binop("<=", c, ob_int(90LL), ob_lte)))))) || ob_truthy(ob_bool(ob_truthy(ob_binop(">=", c, ob_int(48LL), ob_gte)) && ob_truthy(ob_binop("<=", c, ob_int(57LL), ob_lte)))))) || ob_truthy(ob_binop("==", c, ob_int(95LL), ob_eq)));
    return ob_null();
}

OboeValue codegen__scan_imports_textual(OboeValue src, OboeValue dir) {
    OboeValue s = codegen__strip_for_scan(src);
    OboeValue len = ob_m_len(s);
    OboeValue i = ob_int(0LL);
    while (ob_truthy(ob_binop("<", ob_binop("+", i, ob_int(6LL), ob_add), len, ob_lt))) {
        if (ob_truthy(ob_bool(ob_truthy(ob_bool(ob_truthy(ob_binop("!=", ob_str_substr(s, i, ob_int(6LL)), ob_interpolate(1, ob_string("import")), ob_neq)) || ob_truthy(ob_bool(ob_truthy(ob_binop(">", i, ob_int(0LL), ob_gt)) && ob_truthy(codegen__is_ident_char(codegen__byte_of(s, ob_binop("-", i, ob_int(1LL), ob_sub)))))))) || ob_truthy(codegen__is_ident_char(codegen__byte_of(s, ob_binop("+", i, ob_int(6LL), ob_add))))))) {
            (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
            continue;
        }
        OboeValue start = ob_binop("+", i, ob_int(6LL), ob_add);
        OboeValue end = start;
        while (ob_truthy(ob_bool(ob_truthy(ob_binop("<", end, len, ob_lt)) && ob_truthy(ob_binop("!=", codegen__byte_of(s, end), ob_int(10LL), ob_neq))))) {
            (void)((end = ob_binop("+", end, ob_int(1LL), ob_add)));
        }
        OboeValue take = ob_binop("-", end, start, ob_sub);
        if (ob_truthy(ob_binop(">", take, ob_int(511LL), ob_gt))) {
            (void)((take = ob_int(511LL)));
        }
        OboeValue line = ob_str_substr(s, start, take);
        OboeValue fpos = ob_m_index_of(line, ob_interpolate(1, ob_string(" from ")));
        OboeValue word = (ob_truthy(ob_binop(">=", fpos, ob_int(0LL), ob_gte)) ? (ob_str_substr(line, ob_binop("+", fpos, ob_int(6LL), ob_add), ob_binop("-", ob_binop("-", ob_m_len(line), fpos, ob_sub), ob_int(6LL), ob_sub))) : (line));
        OboeValue w = ob_int(0LL);
        while (ob_truthy(ob_bool(ob_truthy(ob_binop("==", codegen__byte_of(word, w), ob_int(32LL), ob_eq)) || ob_truthy(ob_binop("==", codegen__byte_of(word, w), ob_int(9LL), ob_eq))))) {
            (void)((w = ob_binop("+", w, ob_int(1LL), ob_add)));
        }
        OboeValue k = ob_int(0LL);
        while (ob_truthy(ob_bool(ob_truthy(codegen__is_ident_char(codegen__byte_of(word, ob_binop("+", w, k, ob_add)))) && ob_truthy(ob_binop("<", k, ob_int(255LL), ob_lt))))) {
            (void)((k = ob_binop("+", k, ob_int(1LL), ob_add)));
        }
        OboeValue name = ob_str_substr(word, w, k);
        if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", k, ob_int(0LL), ob_eq)) || ob_truthy(ob_binop(">=", codegen__find_unit(name), ob_int(0LL), ob_gte))))) {
            (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
            continue;
        }
        OboeValue path = codegen__resolve_module_path(dir, name);
        if (ob_truthy(ob_binop("==", path, ob_null(), ob_eq))) {
            (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
            continue;
        }
        OboeValue msrc = codegen__read_whole_file(path);
        if (ob_truthy(ob_binop("!=", msrc, ob_null(), ob_neq))) {
            (void)(codegen__load_unit_textual(name, msrc, path, codegen__path_dirname(path)));
        }
        (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
    }
    return ob_null();
}

OboeValue codegen__load_unit_textual(OboeValue module, OboeValue src, OboeValue path, OboeValue dir) {
    (void)(ob_arr_push(codegen__UNITS, ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("module"))), module); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("prefix"))), (ob_truthy(ob_binop("!=", module, ob_null(), ob_neq)) ? (ob_binop("+", module, ob_interpolate(1, ob_string("__")), ob_add)) : (ob_string("")))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("path"))), path); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("dir"))), dir); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("src"))), src); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("prog"))), ob_null()); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("referenced"))), ob_bool(false)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("builtin"))), ob_bool(false)); __d; })));
    (void)(codegen__scan_imports_textual(src, dir));
    return ob_null();
}

OboeValue codegen__unit_file_for_prefix(OboeValue prefix) {
    { OboeValue __it = codegen__UNITS; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue u = ob_iter_value(__it, __i);
        if (ob_truthy(ob_binop("==", ob_index_get(u, ob_interpolate(1, ob_string("prefix"))), prefix, ob_eq))) {
            return ob_index_get(u, ob_interpolate(1, ob_string("path")));
        }
    } }
    return ob_null();
    return ob_null();
}

OboeValue codegen__module_is_builtin(OboeValue module) {
    OboeValue ui = codegen__find_unit(module);
    return ob_bool(ob_truthy(ob_binop(">=", ui, ob_int(0LL), ob_gte)) && ob_truthy(ob_index_get(ob_index_get(codegen__UNITS, ui), ob_interpolate(1, ob_string("builtin")))));
    return ob_null();
}

OboeValue codegen__parse_unit(OboeValue ui) {
    OboeValue u = ob_index_get(codegen__UNITS, ui);
    if (ob_truthy(ob_binop("!=", ob_index_get(u, ob_interpolate(1, ob_string("prog"))), ob_null(), ob_neq))) {
        return ob_null();
    }
    (void)(ob_index_set(u, ob_interpolate(1, ob_string("prog")), parser__parse_program(lexer__lex_all(ob_index_get(u, ob_interpolate(1, ob_string("src")))), ob_index_get(u, ob_interpolate(1, ob_string("path"))))));
    return ob_null();
}

OboeValue codegen__ensure_unit(OboeValue module, OboeValue from_dir) {
    OboeValue ui = codegen__find_unit(module);
    if (ob_truthy(ob_binop("<", ui, ob_int(0LL), ob_lt))) {
        OboeValue path = codegen__resolve_module_path(from_dir, module);
        if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", path, ob_null(), ob_eq)) && ob_truthy(ob_binop("!=", codegen__std_module_members(module), ob_null(), ob_neq))))) {
            (void)(ob_arr_push(codegen__UNITS, ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("module"))), module); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("prefix"))), ob_binop("+", module, ob_interpolate(1, ob_string("__")), ob_add)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("path"))), ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("<builtin ")), module, ob_add), ob_interpolate(1, ob_string(">")), ob_add)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("dir"))), from_dir); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("src"))), ob_string("")); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("prog"))), ob_null()); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("referenced"))), ob_bool(false)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("builtin"))), ob_bool(true)); __d; })));
            (void)((ui = ob_binop("-", ob_m_len(codegen__UNITS), ob_int(1LL), ob_sub)));
            (void)(codegen__parse_unit(ui));
            return ui;
        }
        if (ob_truthy(ob_binop("==", path, ob_null(), ob_eq))) {
            (void)(codegen__flush_out());
            (void)(diag__fail(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("cannot find module '")), module, ob_add), ob_interpolate(1, ob_string("' (looked in ")), ob_add), from_dir, ob_add), ob_interpolate(1, ob_string(" and ")), ob_add), from_dir, ob_add), ob_interpolate(1, ob_string("/.oboe/libraries)")), ob_add)));
        }
        OboeValue src = codegen__read_whole_file(path);
        OboeValue before = ob_m_len(codegen__UNITS);
        (void)(codegen__load_unit_textual(module, src, path, codegen__path_dirname(path)));
        OboeValue i = before;
        while (ob_truthy(ob_binop("<", i, ob_m_len(codegen__UNITS), ob_lt))) {
            (void)(lexer__prescan_ops(ob_index_get(ob_index_get(codegen__UNITS, i), ob_interpolate(1, ob_string("src")))));
            (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
        }
        (void)((ui = codegen__find_unit(module)));
    }
    (void)(codegen__parse_unit(ui));
    return ui;
    return ob_null();
}

OboeValue codegen__resolve_imports(OboeValue ui) {
    { OboeValue __it = ob_index_get(ob_index_get(codegen__UNITS, ui), ob_interpolate(1, ob_string("prog"))); int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue d = ob_iter_value(__it, __i);
        if (ob_truthy(ob_binop("!=", ob_index_get(d, ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("DECL_IMPORT")), ob_neq))) {
            continue;
        }
        OboeValue owner = ob_index_get(ob_index_get(codegen__UNITS, ui), ob_interpolate(1, ob_string("prefix")));
        if (ob_truthy(ob_binop(">", ob_m_len(ob_index_get(d, ob_interpolate(1, ob_string("members")))), ob_int(0LL), ob_gt))) {
            { OboeValue __it = ob_index_get(d, ob_interpolate(1, ob_string("members"))); int64_t __n = ob_iter_len(__it);
            for (int64_t __i = 0; __i < __n; __i++) {
                OboeValue m = ob_iter_value(__it, __i);
                (void)(ob_arr_push(codegen__IMPORT_DIRECTS, ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("local_name"))), m); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("module"))), ob_index_get(d, ob_interpolate(1, ob_string("module")))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("owner"))), owner); __d; })));
            } }
        }
        else {
            (void)(ob_arr_push(codegen__IMPORT_ALIASES, ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("local_name"))), ob_coalesce(ob_index_get(d, ob_interpolate(1, ob_string("alias"))), ob_index_get(d, ob_interpolate(1, ob_string("module"))))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("module"))), ob_index_get(d, ob_interpolate(1, ob_string("module")))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("owner"))), owner); __d; })));
        }
        OboeValue dep = codegen__ensure_unit(ob_index_get(d, ob_interpolate(1, ob_string("module"))), ob_index_get(ob_index_get(codegen__UNITS, ui), ob_interpolate(1, ob_string("dir"))));
        if (ob_truthy(ob_not(ob_index_get(ob_index_get(codegen__UNITS, dep), ob_interpolate(1, ob_string("referenced")))))) {
            (void)(ob_index_set(ob_index_get(codegen__UNITS, dep), ob_interpolate(1, ob_string("referenced")), ob_bool(true)));
            (void)(codegen__resolve_imports(dep));
        }
    } }
    return ob_null();
}

OboeValue codegen__register_event_decl(OboeValue ev, OboeValue prefix) {
    OboeValue fields = ({ OboeValue __a = ob_array_new(); __a; });
    { OboeValue __it = ob_index_get(ev, ob_interpolate(1, ob_string("params"))); int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue p = ob_iter_value(__it, __i);
        (void)(ob_arr_push(fields, ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("name"))), ob_index_get(p, ob_interpolate(1, ob_string("name")))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("type_name"))), ob_index_get(p, ob_interpolate(1, ob_string("type_name")))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_static"))), ob_bool(false)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_private"))), ob_bool(false)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_const"))), ob_bool(false)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("init"))), ob_null()); __d; })));
    } }
    OboeValue c = ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("name"))), ob_index_get(ev, ob_interpolate(1, ob_string("name")))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("parent_name"))), ob_null()); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("fields"))), fields); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("methods"))), ({ OboeValue __a = ob_array_new(); __a; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("line"))), ob_int(0LL)); __d; });
    (void)(codegen__add_class(c, ob_string("")));
    (void)(ob_arr_push(codegen__EVENTS, ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("decl"))), ev); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cls"))), c); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("prefix"))), prefix); __d; })));
    return ob_null();
}

OboeValue codegen__collect_extras(OboeValue decls, OboeValue prefix) {
    { OboeValue __it = decls; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue d = ob_iter_value(__it, __i);
        OboeValue k = ob_index_get(d, ob_interpolate(1, ob_string("kind")));
        if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("DECL_OPERATOR")), ob_eq))) {
            OboeValue f = ob_index_get(d, ob_interpolate(1, ob_string("func")));
            if (ob_truthy(ob_binop("!=", ob_m_len(ob_index_get(f, ob_interpolate(1, ob_string("params")))), ob_int(2LL), ob_neq))) {
                (void)(codegen__codegen_error(ob_index_get(f, ob_interpolate(1, ob_string("line"))), ob_interpolate(1, ob_string("a top-level operator must take exactly two parameters"))));
            }
            if (ob_truthy(ob_binop("!=", codegen__find_user_op(ob_index_get(f, ob_interpolate(1, ob_string("op_symbol")))), ob_null(), ob_neq))) {
                (void)(codegen__codegen_error(ob_index_get(f, ob_interpolate(1, ob_string("line"))), ob_interpolate(1, ob_string("operator is already defined"))));
            }
            (void)(ob_arr_push(codegen__USER_OPS, ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("symbol"))), ob_index_get(f, ob_interpolate(1, ob_string("op_symbol")))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_binop("+", ob_interpolate(1, ob_string("__oboe_userop_")), ob_str(ob_m_len(codegen__USER_OPS)), ob_add)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("decl"))), f); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("prefix"))), prefix); __d; })));
        }
        else {
            if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("DECL_EVENT")), ob_eq))) {
                if (ob_truthy(ob_bool(ob_truthy(ob_binop("!=", codegen__find_event(ob_index_get(d, ob_interpolate(1, ob_string("name")))), ob_null(), ob_neq)) || ob_truthy(ob_binop("!=", codegen__find_class(ob_index_get(d, ob_interpolate(1, ob_string("name")))), ob_null(), ob_neq))))) {
                    (void)(codegen__codegen_error(ob_index_get(d, ob_interpolate(1, ob_string("line"))), ob_interpolate(1, ob_string("an event or class with this name already exists"))));
                }
                (void)(codegen__register_event_decl(d, prefix));
            }
            else {
                if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("DECL_ON")), ob_eq))) {
                    (void)(ob_arr_push(codegen__HANDLERS, ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("decl"))), d); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("__on_")), ob_index_get(d, ob_interpolate(1, ob_string("event_name"))), ob_add), ob_interpolate(1, ob_string("_")), ob_add), ob_str(ob_m_len(codegen__HANDLERS)), ob_add)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("prefix"))), prefix); __d; })));
                }
                else {
                    if (ob_truthy(ob_binop("==", k, ob_interpolate(1, ob_string("DECL_CIMPORT")), ob_eq))) {
                        if (ob_truthy(ob_binop("==", codegen__find_ffi(ob_index_get(d, ob_interpolate(1, ob_string("name")))), ob_null(), ob_eq))) {
                            (void)(ob_arr_push(codegen__FFI, ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("name"))), ob_index_get(d, ob_interpolate(1, ob_string("name")))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("lib"))), ob_index_get(d, ob_interpolate(1, ob_string("lib")))); __d; })));
                        }
                    }
                }
            }
        }
    } }
    return ob_null();
}

OboeValue codegen__handler_event_prefix(OboeValue h) {
    OboeValue d = ob_index_get(h, ob_interpolate(1, ob_string("decl")));
    { OboeValue __it = codegen__IMPORT_ALIASES; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue ia = ob_iter_value(__it, __i);
        if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", ob_index_get(ia, ob_interpolate(1, ob_string("local_name"))), ob_index_get(d, ob_interpolate(1, ob_string("event_module"))), ob_eq)) && ob_truthy(ob_binop("==", ob_index_get(ia, ob_interpolate(1, ob_string("owner"))), ob_index_get(h, ob_interpolate(1, ob_string("prefix"))), ob_eq))))) {
            return ob_binop("+", ob_index_get(ia, ob_interpolate(1, ob_string("module"))), ob_interpolate(1, ob_string("__")), ob_add);
        }
    } }
    (void)(codegen__codegen_error(ob_index_get(d, ob_interpolate(1, ob_string("line"))), ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("'")), ob_index_get(d, ob_interpolate(1, ob_string("event_module"))), ob_add), ob_interpolate(1, ob_string("' is not a module imported here")), ob_add)));
    return ob_null();
}

OboeValue codegen__finalize_events() {
    OboeValue saved = codegen__CURRENT_FILE;
    { OboeValue __it = codegen__HANDLERS; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue h = ob_iter_value(__it, __i);
        OboeValue ev = ob_index_get(h, ob_interpolate(1, ob_string("decl")));
        (void)((codegen__CURRENT_FILE = codegen__unit_file_for_prefix(ob_index_get(h, ob_interpolate(1, ob_string("prefix"))))));
        OboeValue got = codegen__find_event(ob_index_get(ev, ob_interpolate(1, ob_string("event_name"))));
        if (ob_truthy(ob_binop("!=", ob_index_get(ev, ob_interpolate(1, ob_string("event_module"))), ob_null(), ob_neq))) {
            OboeValue want = codegen__handler_event_prefix(h);
            if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", got, ob_null(), ob_eq)) || ob_truthy(ob_binop("!=", ob_index_get(got, ob_interpolate(1, ob_string("prefix"))), want, ob_neq))))) {
                (void)(codegen__codegen_error(ob_index_get(ev, ob_interpolate(1, ob_string("line"))), ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("module '")), ob_index_get(ev, ob_interpolate(1, ob_string("event_module"))), ob_add), ob_interpolate(1, ob_string("' declares no such event")), ob_add)));
            }
            continue;
        }
        if (ob_truthy(ob_binop("!=", got, ob_null(), ob_neq))) {
            continue;
        }
        if (ob_truthy(ob_binop("==", ob_index_get(ev, ob_interpolate(1, ob_string("event_name"))), ob_interpolate(1, ob_string("KeyboardInterruptEvent")), ob_eq))) {
            (void)(codegen__register_event_decl(({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("KeyboardInterruptEvent"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("params"))), ({ OboeValue __a = ob_array_new(); __a; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("line"))), ob_int(0LL)); __d; }), ob_string("")));
        }
        else {
            (void)(codegen__codegen_error(ob_index_get(ev, ob_interpolate(1, ob_string("line"))), ob_interpolate(1, ob_string("'on' handler references an undeclared event"))));
        }
    } }
    (void)((codegen__CURRENT_FILE = saved));
    return ob_null();
}

OboeValue codegen__has_kbint_handlers() {
    { OboeValue __it = codegen__HANDLERS; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue h = ob_iter_value(__it, __i);
        if (ob_truthy(ob_binop("==", ob_index_get(ob_index_get(h, ob_interpolate(1, ob_string("decl"))), ob_interpolate(1, ob_string("event_name"))), ob_interpolate(1, ob_string("KeyboardInterruptEvent")), ob_eq))) {
            return ob_bool(true);
        }
    } }
    return ob_bool(false);
    return ob_null();
}

OboeValue codegen__emit_event_params(OboeValue params) {
    OboeValue first = ob_bool(true);
    { OboeValue __it = params; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue p = ob_iter_value(__it, __i);
        if (ob_truthy(ob_not(first))) {
            (void)(codegen__emit(ob_interpolate(1, ob_string(", "))));
        }
        (void)(codegen__emit(ob_binop("+", ob_interpolate(1, ob_string("OboeValue ")), ob_index_get(p, ob_interpolate(1, ob_string("name"))), ob_add)));
        (void)((first = ob_bool(false)));
    } }
    if (ob_truthy(first)) {
        (void)(codegen__emit(ob_interpolate(1, ob_string("void"))));
    }
    return ob_null();
}

OboeValue codegen__emit_extras_predecls() {
    { OboeValue __it = codegen__USER_OPS; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue u = ob_iter_value(__it, __i);
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("static OboeValue ")), ob_index_get(u, ob_interpolate(1, ob_string("cfunc"))), ob_add), ob_interpolate(1, ob_string("(OboeValue, OboeValue);\n")), ob_add)));
    } }
    { OboeValue __it = codegen__HANDLERS; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue h = ob_iter_value(__it, __i);
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("static OboeValue ")), ob_index_get(h, ob_interpolate(1, ob_string("cfunc"))), ob_add), ob_interpolate(1, ob_string("(OboeValue __ev);\n")), ob_add)));
    } }
    { OboeValue __it = codegen__EVENTS; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue ev = ob_iter_value(__it, __i);
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("OboeValue ")), ob_index_get(ob_index_get(ev, ob_interpolate(1, ob_string("decl"))), ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("__fire(")), ob_add)));
        (void)(codegen__emit_event_params(ob_index_get(ob_index_get(ev, ob_interpolate(1, ob_string("decl"))), ob_interpolate(1, ob_string("params")))));
        (void)(codegen__emit(ob_interpolate(1, ob_string(");\n"))));
    } }
    { OboeValue __it = codegen__FFI; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue f = ob_iter_value(__it, __i);
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("static void *__ffi_")), ob_index_get(f, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string(";\n")), ob_add)));
    } }
    return ob_null();
}

OboeValue codegen__emit_extras_defs() {
    { OboeValue __it = codegen__USER_OPS; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue u = ob_iter_value(__it, __i);
        OboeValue f = ob_index_get(u, ob_interpolate(1, ob_string("decl")));
        (void)((codegen__CURRENT_FILE = codegen__unit_file_for_prefix(ob_index_get(u, ob_interpolate(1, ob_string("prefix"))))));
        OboeValue a = ob_index_get(ob_index_get(f, ob_interpolate(1, ob_string("params"))), ob_int(0LL));
        OboeValue b = ob_index_get(ob_index_get(f, ob_interpolate(1, ob_string("params"))), ob_int(1LL));
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("static OboeValue ")), ob_index_get(u, ob_interpolate(1, ob_string("cfunc"))), ob_add), ob_interpolate(1, ob_string("(OboeValue ")), ob_add), ob_index_get(a, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string(", OboeValue ")), ob_add), ob_index_get(b, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string(") {\n")), ob_add)));
        (void)((codegen__CURRENT_PREFIX = ob_index_get(u, ob_interpolate(1, ob_string("prefix")))));
        (void)(codegen__push_scope());
        (void)(codegen__define_var(ob_index_get(a, ob_interpolate(1, ob_string("name"))), (ob_truthy(ob_bool(ob_truthy(ob_binop("!=", ob_index_get(a, ob_interpolate(1, ob_string("type_name"))), ob_null(), ob_neq)) && ob_truthy(ob_binop("!=", codegen__find_class(ob_index_get(a, ob_interpolate(1, ob_string("type_name")))), ob_null(), ob_neq)))) ? (ob_index_get(a, ob_interpolate(1, ob_string("type_name")))) : (ob_null()))));
        (void)(codegen__define_var(ob_index_get(b, ob_interpolate(1, ob_string("name"))), (ob_truthy(ob_bool(ob_truthy(ob_binop("!=", ob_index_get(b, ob_interpolate(1, ob_string("type_name"))), ob_null(), ob_neq)) && ob_truthy(ob_binop("!=", codegen__find_class(ob_index_get(b, ob_interpolate(1, ob_string("type_name")))), ob_null(), ob_neq)))) ? (ob_index_get(b, ob_interpolate(1, ob_string("type_name")))) : (ob_null()))));
        (void)(codegen__gen_stmt_list(ob_index_get(f, ob_interpolate(1, ob_string("body"))), ob_int(4LL)));
        (void)(codegen__pop_scope());
        (void)(codegen__emit(ob_interpolate(1, ob_string("    return ob_null();\n"))));
        (void)(codegen__emit(ob_interpolate(1, ob_string("}\n\n"))));
    } }
    { OboeValue __it = codegen__HANDLERS; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue h = ob_iter_value(__it, __i);
        OboeValue d = ob_index_get(h, ob_interpolate(1, ob_string("decl")));
        (void)((codegen__CURRENT_FILE = codegen__unit_file_for_prefix(ob_index_get(h, ob_interpolate(1, ob_string("prefix"))))));
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("static OboeValue ")), ob_index_get(h, ob_interpolate(1, ob_string("cfunc"))), ob_add), ob_interpolate(1, ob_string("(OboeValue __ev) {\n")), ob_add)));
        (void)(codegen__emit(ob_interpolate(1, ob_string("    (void)__ev;\n"))));
        (void)((codegen__CURRENT_PREFIX = ob_index_get(h, ob_interpolate(1, ob_string("prefix")))));
        (void)(codegen__push_scope());
        if (ob_truthy(ob_binop("!=", ob_index_get(d, ob_interpolate(1, ob_string("var_name"))), ob_null(), ob_neq))) {
            (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("    OboeValue ")), ob_index_get(d, ob_interpolate(1, ob_string("var_name"))), ob_add), ob_interpolate(1, ob_string(" = __ev;\n")), ob_add)));
            (void)(codegen__define_var(ob_index_get(d, ob_interpolate(1, ob_string("var_name"))), ob_index_get(d, ob_interpolate(1, ob_string("event_name")))));
        }
        (void)(codegen__gen_stmt_list(ob_index_get(d, ob_interpolate(1, ob_string("body"))), ob_int(4LL)));
        (void)(codegen__pop_scope());
        (void)(codegen__emit(ob_interpolate(1, ob_string("    return ob_null();\n"))));
        (void)(codegen__emit(ob_interpolate(1, ob_string("}\n\n"))));
    } }
    (void)((codegen__CURRENT_PREFIX = ob_string("")));
    { OboeValue __it = codegen__EVENTS; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue e = ob_iter_value(__it, __i);
        OboeValue ev = ob_index_get(e, ob_interpolate(1, ob_string("decl")));
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("OboeValue ")), ob_index_get(ev, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("__fire(")), ob_add)));
        (void)(codegen__emit_event_params(ob_index_get(ev, ob_interpolate(1, ob_string("params")))));
        (void)(codegen__emit(ob_interpolate(1, ob_string(") {\n"))));
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("    ")), ob_index_get(ev, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string(" *obj = calloc(1, sizeof(")), ob_add), ob_index_get(ev, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("));\n")), ob_add)));
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("    ((OboeObject*)obj)->cls = &")), ob_index_get(ev, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("__classinfo;\n")), ob_add)));
        { OboeValue __it = ob_index_get(ev, ob_interpolate(1, ob_string("params"))); int64_t __n = ob_iter_len(__it);
        for (int64_t __i = 0; __i < __n; __i++) {
            OboeValue p = ob_iter_value(__it, __i);
            (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("    obj->")), ob_index_get(p, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string(" = ")), ob_add), ob_index_get(p, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string(";\n")), ob_add)));
        } }
        (void)(codegen__emit(ob_interpolate(1, ob_string("    OboeValue __ev = ob_object_wrap(obj);\n"))));
        (void)(codegen__emit(ob_interpolate(1, ob_string("    (void)__ev;\n"))));
        { OboeValue __it = codegen__HANDLERS; int64_t __n = ob_iter_len(__it);
        for (int64_t __i = 0; __i < __n; __i++) {
            OboeValue h = ob_iter_value(__it, __i);
            if (ob_truthy(ob_binop("==", ob_index_get(ob_index_get(h, ob_interpolate(1, ob_string("decl"))), ob_interpolate(1, ob_string("event_name"))), ob_index_get(ev, ob_interpolate(1, ob_string("name"))), ob_eq))) {
                (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("    ")), ob_index_get(h, ob_interpolate(1, ob_string("cfunc"))), ob_add), ob_interpolate(1, ob_string("(__ev);\n")), ob_add)));
            }
        } }
        (void)(codegen__emit(ob_interpolate(1, ob_string("    return ob_null();\n"))));
        (void)(codegen__emit(ob_interpolate(1, ob_string("}\n\n"))));
    } }
    if (ob_truthy(codegen__has_kbint_handlers())) {
        (void)(codegen__emit(ob_interpolate(1, ob_string("static void __oboe_fire_kbint(void) { KeyboardInterruptEvent__fire(); }\n\n"))));
    }
    return ob_null();
}

OboeValue codegen__codegen_compile(OboeValue main_path) {
    OboeValue main_src = codegen__read_whole_file(main_path);
    if (ob_truthy(ob_binop("==", main_src, ob_null(), ob_eq))) {
        (void)(diag__fail(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("cannot read '")), main_path, ob_add), ob_interpolate(1, ob_string("'")), ob_add)));
    }
    (void)(codegen__codegen_set_library_root(main_path));
    (void)(codegen__load_unit_textual(ob_null(), main_src, main_path, ob_coalesce(codegen__SOURCE_DIR, ob_interpolate(1, ob_string(".")))));
    OboeValue i = ob_int(0LL);
    while (ob_truthy(ob_binop("<", i, ob_m_len(codegen__UNITS), ob_lt))) {
        (void)(lexer__prescan_ops(ob_index_get(ob_index_get(codegen__UNITS, i), ob_interpolate(1, ob_string("src")))));
        (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
    }
    (void)((i = ob_int(0LL)));
    while (ob_truthy(ob_binop("<", i, ob_m_len(codegen__UNITS), ob_lt))) {
        (void)(codegen__parse_unit(i));
        (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
    }
    (void)(ob_index_set(ob_index_get(codegen__UNITS, ob_int(0LL)), ob_interpolate(1, ob_string("referenced")), ob_bool(true)));
    (void)(codegen__resolve_imports(ob_int(0LL)));
    (void)((i = ob_int(0LL)));
    while (ob_truthy(ob_binop("<", i, ob_m_len(codegen__UNITS), ob_lt))) {
        if (ob_truthy(ob_index_get(ob_index_get(codegen__UNITS, i), ob_interpolate(1, ob_string("referenced"))))) {
            (void)((codegen__CURRENT_FILE = ob_index_get(ob_index_get(codegen__UNITS, i), ob_interpolate(1, ob_string("path")))));
            (void)(codegen__collect_classes(ob_index_get(ob_index_get(codegen__UNITS, i), ob_interpolate(1, ob_string("prog"))), ob_index_get(ob_index_get(codegen__UNITS, i), ob_interpolate(1, ob_string("prefix")))));
            (void)(codegen__register_funcs(ob_index_get(ob_index_get(codegen__UNITS, i), ob_interpolate(1, ob_string("prog"))), ob_index_get(ob_index_get(codegen__UNITS, i), ob_interpolate(1, ob_string("prefix")))));
            (void)(codegen__collect_extras(ob_index_get(ob_index_get(codegen__UNITS, i), ob_interpolate(1, ob_string("prog"))), ob_index_get(ob_index_get(codegen__UNITS, i), ob_interpolate(1, ob_string("prefix")))));
        }
        (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
    }
    (void)(codegen__finalize_events());
    (void)((i = ob_int(0LL)));
    while (ob_truthy(ob_binop("<", i, ob_m_len(codegen__CLASSES), ob_lt))) {
        (void)(codegen__infer_instance_fields(ob_index_get(codegen__CLASSES, i)));
        (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
    }
    (void)(codegen__emit(ob_interpolate(1, ob_string("#include \"oboe_runtime.h\"\n#include <stdlib.h>\n#include <stdio.h>\n\n"))));
    (void)(codegen__emit_script_path_builtins(main_path));
    (void)((i = ob_int(0LL)));
    while (ob_truthy(ob_binop("<", i, ob_m_len(codegen__CLASSES), ob_lt))) {
        (void)(codegen__emit_class_struct(ob_index_get(codegen__CLASSES, i), i));
        (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
    }
    OboeValue ui = ob_int(0LL);
    while (ob_truthy(ob_binop("<", ui, ob_m_len(codegen__UNITS), ob_lt))) {
        if (ob_truthy(ob_index_get(ob_index_get(codegen__UNITS, ui), ob_interpolate(1, ob_string("referenced"))))) {
            OboeValue pfx = (ob_truthy(ob_binop("!=", ob_index_get(ob_index_get(codegen__UNITS, ui), ob_interpolate(1, ob_string("prefix"))), ob_string(""), ob_neq)) ? (ob_index_get(ob_index_get(codegen__UNITS, ui), ob_interpolate(1, ob_string("prefix")))) : (ob_null()));
            { OboeValue __it = ob_index_get(ob_index_get(codegen__UNITS, ui), ob_interpolate(1, ob_string("prog"))); int64_t __n = ob_iter_len(__it);
            for (int64_t __i = 0; __i < __n; __i++) {
                OboeValue d = ob_iter_value(__it, __i);
                if (ob_truthy(ob_binop("==", ob_index_get(d, ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("DECL_CLASS")), ob_eq))) {
                    (void)(codegen__emit_class_predecls(ob_index_get(d, ob_interpolate(1, ob_string("klass")))));
                }
                else {
                    if (ob_truthy(ob_binop("==", ob_index_get(d, ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("DECL_FUNC")), ob_eq))) {
                        (void)(codegen__emit_func_prototype(pfx, ob_null(), ob_index_get(d, ob_interpolate(1, ob_string("func")))));
                    }
                }
            } }
        }
        (void)((ui = ob_binop("+", ui, ob_int(1LL), ob_add)));
    }
    (void)(codegen__emit_extras_predecls());
    OboeValue prog = ob_index_get(ob_index_get(codegen__UNITS, ob_int(0LL)), ob_interpolate(1, ob_string("prog")));
    (void)((ui = ob_int(0LL)));
    while (ob_truthy(ob_binop("<", ui, ob_m_len(codegen__UNITS), ob_lt))) {
        if (ob_truthy(ob_index_get(ob_index_get(codegen__UNITS, ui), ob_interpolate(1, ob_string("referenced"))))) {
            { OboeValue __it = ob_index_get(ob_index_get(codegen__UNITS, ui), ob_interpolate(1, ob_string("prog"))); int64_t __n = ob_iter_len(__it);
            for (int64_t __i = 0; __i < __n; __i++) {
                OboeValue d = ob_iter_value(__it, __i);
                if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", ob_index_get(d, ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("DECL_STMT")), ob_eq)) && ob_truthy(ob_binop("==", ob_index_get(ob_index_get(d, ob_interpolate(1, ob_string("stmt"))), ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("STMT_LET")), ob_eq))))) {
                    (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("static OboeValue ")), ob_index_get(ob_index_get(codegen__UNITS, ui), ob_interpolate(1, ob_string("prefix"))), ob_add), ob_index_get(ob_index_get(d, ob_interpolate(1, ob_string("stmt"))), ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string(";\n")), ob_add)));
                }
            } }
        }
        (void)((ui = ob_binop("+", ui, ob_int(1LL), ob_add)));
    }
    (void)(codegen__emit(ob_interpolate(1, ob_string("\n"))));
    OboeValue has_main = ob_bool(false);
    { OboeValue __it = prog; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue d = ob_iter_value(__it, __i);
        if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", ob_index_get(d, ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("DECL_FUNC")), ob_eq)) && ob_truthy(ob_binop("==", ob_index_get(ob_index_get(d, ob_interpolate(1, ob_string("func"))), ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("main")), ob_eq))))) {
            (void)((has_main = ob_bool(true)));
        }
    } }
    (void)((ui = ob_binop("-", ob_m_len(codegen__UNITS), ob_int(1LL), ob_sub)));
    while (ob_truthy(ob_binop(">=", ui, ob_int(0LL), ob_gte))) {
        if (ob_truthy(ob_not(ob_index_get(ob_index_get(codegen__UNITS, ui), ob_interpolate(1, ob_string("referenced")))))) {
            (void)((ui = ob_binop("-", ui, ob_int(1LL), ob_sub)));
            continue;
        }
        OboeValue pfx = (ob_truthy(ob_binop("!=", ob_index_get(ob_index_get(codegen__UNITS, ui), ob_interpolate(1, ob_string("prefix"))), ob_string(""), ob_neq)) ? (ob_index_get(ob_index_get(codegen__UNITS, ui), ob_interpolate(1, ob_string("prefix")))) : (ob_null()));
        (void)((codegen__CURRENT_PREFIX = ob_index_get(ob_index_get(codegen__UNITS, ui), ob_interpolate(1, ob_string("prefix")))));
        (void)((codegen__CURRENT_FILE = ob_index_get(ob_index_get(codegen__UNITS, ui), ob_interpolate(1, ob_string("path")))));
        (void)(codegen__push_scope());
        { OboeValue __it = ob_index_get(ob_index_get(codegen__UNITS, ui), ob_interpolate(1, ob_string("prog"))); int64_t __n = ob_iter_len(__it);
        for (int64_t __i = 0; __i < __n; __i++) {
            OboeValue d = ob_iter_value(__it, __i);
            if (ob_truthy(ob_bool(ob_truthy(ob_binop("!=", ob_index_get(d, ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("DECL_STMT")), ob_neq)) || ob_truthy(ob_binop("!=", ob_index_get(ob_index_get(d, ob_interpolate(1, ob_string("stmt"))), ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("STMT_LET")), ob_neq))))) {
                continue;
            }
            OboeValue s = ob_index_get(d, ob_interpolate(1, ob_string("stmt")));
            OboeValue class_type = ob_null();
            if (ob_truthy(ob_bool(ob_truthy(ob_binop("!=", ob_index_get(s, ob_interpolate(1, ob_string("type_name"))), ob_null(), ob_neq)) && ob_truthy(ob_binop("!=", codegen__find_class(ob_index_get(s, ob_interpolate(1, ob_string("type_name")))), ob_null(), ob_neq))))) {
                (void)((class_type = ob_index_get(s, ob_interpolate(1, ob_string("type_name")))));
            }
            else {
                if (ob_truthy(ob_binop("!=", ob_index_get(s, ob_interpolate(1, ob_string("init"))), ob_null(), ob_neq))) {
                    (void)((class_type = codegen__infer_class(ob_index_get(s, ob_interpolate(1, ob_string("init"))))));
                }
            }
            (void)(codegen__define_var_c(ob_index_get(s, ob_interpolate(1, ob_string("name"))), class_type, ob_binop("+", ob_index_get(ob_index_get(codegen__UNITS, ui), ob_interpolate(1, ob_string("prefix"))), ob_index_get(s, ob_interpolate(1, ob_string("name"))), ob_add)));
        } }
        { OboeValue __it = ob_index_get(ob_index_get(codegen__UNITS, ui), ob_interpolate(1, ob_string("prog"))); int64_t __n = ob_iter_len(__it);
        for (int64_t __i = 0; __i < __n; __i++) {
            OboeValue d = ob_iter_value(__it, __i);
            if (ob_truthy(ob_binop("==", ob_index_get(d, ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("DECL_CLASS")), ob_eq))) {
                (void)(codegen__gen_class(ob_index_get(d, ob_interpolate(1, ob_string("klass")))));
            }
            else {
                if (ob_truthy(ob_binop("==", ob_index_get(d, ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("DECL_FUNC")), ob_eq))) {
                    (void)(codegen__gen_func_def(pfx, ob_null(), ob_index_get(d, ob_interpolate(1, ob_string("func")))));
                }
            }
        } }
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("static void __oboe_toplevel_")), ob_str(ui), ob_add), ob_interpolate(1, ob_string("(void) {\n")), ob_add)));
        { OboeValue __it = ob_index_get(ob_index_get(codegen__UNITS, ui), ob_interpolate(1, ob_string("prog"))); int64_t __n = ob_iter_len(__it);
        for (int64_t __i = 0; __i < __n; __i++) {
            OboeValue d = ob_iter_value(__it, __i);
            if (ob_truthy(ob_binop("!=", ob_index_get(d, ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("DECL_STMT")), ob_neq))) {
                continue;
            }
            OboeValue s = ob_index_get(d, ob_interpolate(1, ob_string("stmt")));
            if (ob_truthy(ob_binop("==", ob_index_get(s, ob_interpolate(1, ob_string("kind"))), ob_interpolate(1, ob_string("STMT_LET")), ob_eq))) {
                OboeValue init = (ob_truthy(ob_binop("!=", ob_index_get(s, ob_interpolate(1, ob_string("init"))), ob_null(), ob_neq)) ? (codegen__gen_expr(ob_index_get(s, ob_interpolate(1, ob_string("init"))))) : (ob_interpolate(1, ob_string("ob_null()"))));
                (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("    ")), ob_index_get(ob_index_get(codegen__UNITS, ui), ob_interpolate(1, ob_string("prefix"))), ob_add), ob_index_get(s, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string(" = ")), ob_add), init, ob_add), ob_interpolate(1, ob_string(";\n")), ob_add)));
            }
            else {
                (void)(codegen__gen_stmt(s, ob_int(4LL)));
            }
        } }
        (void)(codegen__emit(ob_interpolate(1, ob_string("}\n\n"))));
        (void)(codegen__pop_scope());
        (void)((ui = ob_binop("-", ui, ob_int(1LL), ob_sub)));
    }
    (void)((codegen__CURRENT_PREFIX = ob_string("")));
    (void)(codegen__emit_extras_defs());
    (void)(codegen__emit(ob_interpolate(1, ob_string("static void __oboe_static_init(void) {\n"))));
    { OboeValue __it = codegen__CLASS_OPS; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue o = ob_iter_value(__it, __i);
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("    ob_register_operator(&")), ob_index_get(ob_index_get(o, ob_interpolate(1, ob_string("cls"))), ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("__classinfo, \"")), ob_add), ob_index_get(o, ob_interpolate(1, ob_string("symbol"))), ob_add), ob_interpolate(1, ob_string("\", ")), ob_add), ob_index_get(o, ob_interpolate(1, ob_string("cfunc"))), ob_add), ob_interpolate(1, ob_string(");\n")), ob_add)));
    } }
    { OboeValue __it = codegen__FFI; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue f = ob_iter_value(__it, __i);
        (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("    __ffi_")), ob_index_get(f, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string(" = ob_ffi_sym(\"")), ob_add), ob_index_get(f, ob_interpolate(1, ob_string("lib"))), ob_add), ob_interpolate(1, ob_string("\", \"")), ob_add), ob_index_get(f, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("\");\n")), ob_add)));
    } }
    { OboeValue __it = codegen__CLASSES; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue c = ob_iter_value(__it, __i);
        OboeValue saved_class = codegen__CURRENT_CLASS;
        (void)((codegen__CURRENT_CLASS = ob_index_get(c, ob_interpolate(1, ob_string("name")))));
        (void)((codegen__CURRENT_PREFIX = ob_coalesce(ob_index_get(c, ob_interpolate(1, ob_string("unit_prefix"))), ob_string(""))));
        (void)((codegen__CURRENT_FILE = codegen__unit_file_for_prefix(codegen__CURRENT_PREFIX)));
        { OboeValue __it = ob_index_get(c, ob_interpolate(1, ob_string("fields"))); int64_t __n = ob_iter_len(__it);
        for (int64_t __i = 0; __i < __n; __i++) {
            OboeValue fd = ob_iter_value(__it, __i);
            if (ob_truthy(ob_not(ob_index_get(fd, ob_interpolate(1, ob_string("is_static")))))) {
                continue;
            }
            OboeValue v = (ob_truthy(ob_binop("!=", ob_index_get(fd, ob_interpolate(1, ob_string("init"))), ob_null(), ob_neq)) ? (codegen__gen_expr(ob_index_get(fd, ob_interpolate(1, ob_string("init"))))) : (ob_interpolate(1, ob_string("ob_null()"))));
            (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("    ")), ob_index_get(c, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string("__")), ob_add), ob_index_get(fd, ob_interpolate(1, ob_string("name"))), ob_add), ob_interpolate(1, ob_string(" = ")), ob_add), v, ob_add), ob_interpolate(1, ob_string(";\n")), ob_add)));
        } }
        (void)((codegen__CURRENT_CLASS = saved_class));
    } }
    (void)((codegen__CURRENT_PREFIX = ob_string("")));
    (void)(codegen__emit(ob_interpolate(1, ob_string("}\n\n"))));
    (void)(codegen__emit(ob_interpolate(1, ob_string("int main(int argc, char **argv) {\n"))));
    (void)(codegen__emit(ob_interpolate(1, ob_string("    __oboe_static_init();\n"))));
    if (ob_truthy(codegen__has_kbint_handlers())) {
        (void)(codegen__emit(ob_interpolate(1, ob_string("    ob_install_sigint(__oboe_fire_kbint);\n"))));
    }
    (void)((ui = ob_binop("-", ob_m_len(codegen__UNITS), ob_int(1LL), ob_sub)));
    while (ob_truthy(ob_binop(">=", ui, ob_int(0LL), ob_gte))) {
        if (ob_truthy(ob_index_get(ob_index_get(codegen__UNITS, ui), ob_interpolate(1, ob_string("referenced"))))) {
            (void)(codegen__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("    __oboe_toplevel_")), ob_str(ui), ob_add), ob_interpolate(1, ob_string("();\n")), ob_add)));
        }
        (void)((ui = ob_binop("-", ui, ob_int(1LL), ob_sub)));
    }
    if (ob_truthy(has_main)) {
        (void)(codegen__emit(ob_interpolate(1, ob_string("    oboe_user_main(ob_args_from_argv(argc, argv));\n"))));
    }
    (void)(codegen__emit(ob_interpolate(1, ob_string("    return 0;\n"))));
    (void)(codegen__emit(ob_interpolate(1, ob_string("}\n"))));
    return ob_arr_join(codegen__OUT, ob_string(""));
    return ob_null();
}

static void __oboe_toplevel_5(void) {
    codegen__CLASSES = ({ OboeValue __a = ob_array_new(); __a; });
    codegen__CLASS_EMITTED = ({ OboeValue __a = ob_array_new(); __a; });
    codegen__SOURCE_DIR = ob_null();
    codegen__IMPORT_ALIASES = ({ OboeValue __a = ob_array_new(); __a; });
    codegen__IMPORT_DIRECTS = ({ OboeValue __a = ob_array_new(); __a; });
    codegen__USER_OPS = ({ OboeValue __a = ob_array_new(); __a; });
    codegen__CLASS_OPS = ({ OboeValue __a = ob_array_new(); __a; });
    codegen__EVENTS = ({ OboeValue __a = ob_array_new(); __a; });
    codegen__HANDLERS = ({ OboeValue __a = ob_array_new(); __a; });
    codegen__FFI = ({ OboeValue __a = ob_array_new(); __a; });
    codegen__KNOWN_FUNCS = ({ OboeValue __a = ob_array_new(); __a; });
    codegen__TRY_DEPTH = ob_int(0LL);
    codegen__LOOP_TRY_DEPTH = ob_neg(ob_int(1LL));
    codegen__STD_MATH = ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("abs"))), ob_int(1LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("min"))), ob_int(2LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("max"))), ob_int(2LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("pow"))), ob_int(2LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("sqrt"))), ob_int(1LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("floor"))), ob_int(1LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("ceil"))), ob_int(1LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("round"))), ob_int(1LL)); __d; });
    codegen__STD_RANDOM = ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("seed"))), ob_int(1LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("randint"))), ob_int(2LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("choice"))), ob_int(1LL)); __d; });
    codegen__STD_OS = ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("run"))), ob_int(1LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("spawn"))), ob_int(1LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("read_file"))), ob_int(1LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("write_file"))), ob_int(2LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("append_file"))), ob_int(2LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("exists"))), ob_int(1LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("remove"))), ob_int(1LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("getenv"))), ob_int(1LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("exit"))), ob_int(1LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_dir"))), ob_int(1LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("mkdir"))), ob_int(1LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("listdir"))), ob_int(1LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("realpath"))), ob_int(1LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("script_file"))), ob_int(0LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("script_dir"))), ob_int(0LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("project_root"))), ob_int(0LL)); __d; });
    codegen__OUT = ({ OboeValue __a = ob_array_new(); __a; });
    codegen__OUT_PATH = ob_null();
    codegen__CURRENT_FILE = ob_null();
    codegen__CURRENT_PREFIX = ob_string("");
    codegen__SCOPES = ({ OboeValue __a = ob_array_new(); __a; });
    codegen__CURRENT_CLASS = ob_null();
    codegen__NUMERIC_TYPES = ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("int"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("width"))), ob_int(0LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_unsigned"))), ob_bool(false)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_float"))), ob_bool(false)); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("int8"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("width"))), ob_int(8LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_unsigned"))), ob_bool(false)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_float"))), ob_bool(false)); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("int16"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("width"))), ob_int(16LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_unsigned"))), ob_bool(false)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_float"))), ob_bool(false)); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("int32"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("width"))), ob_int(32LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_unsigned"))), ob_bool(false)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_float"))), ob_bool(false)); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("int64"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("width"))), ob_int(64LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_unsigned"))), ob_bool(false)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_float"))), ob_bool(false)); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("uint"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("width"))), ob_int(64LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_unsigned"))), ob_bool(true)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_float"))), ob_bool(false)); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("uint8"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("width"))), ob_int(8LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_unsigned"))), ob_bool(true)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_float"))), ob_bool(false)); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("uint16"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("width"))), ob_int(16LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_unsigned"))), ob_bool(true)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_float"))), ob_bool(false)); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("uint32"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("width"))), ob_int(32LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_unsigned"))), ob_bool(true)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_float"))), ob_bool(false)); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("uint64"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("width"))), ob_int(64LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_unsigned"))), ob_bool(true)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_float"))), ob_bool(false)); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("float"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("width"))), ob_int(64LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_unsigned"))), ob_bool(false)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_float"))), ob_bool(true)); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("float32"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("width"))), ob_int(32LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_unsigned"))), ob_bool(false)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_float"))), ob_bool(true)); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("float64"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("width"))), ob_int(64LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_unsigned"))), ob_bool(false)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_float"))), ob_bool(true)); __d; })); __d; });
    codegen__BUILTIN_METHODS = ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("str"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("arity"))), ob_int(0LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_interpolate(1, ob_string("ob_m_str"))); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("len"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("arity"))), ob_int(0LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_interpolate(1, ob_string("ob_m_len"))); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("contains"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("arity"))), ob_int(1LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_interpolate(1, ob_string("ob_m_contains"))); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("index_of"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("arity"))), ob_int(1LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_interpolate(1, ob_string("ob_m_index_of"))); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("reverse"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("arity"))), ob_int(0LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_interpolate(1, ob_string("ob_m_reverse"))); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("slice"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("arity"))), ob_int(2LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_interpolate(1, ob_string("ob_m_slice"))); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("upper"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("arity"))), ob_int(0LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_interpolate(1, ob_string("ob_str_upper"))); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("lower"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("arity"))), ob_int(0LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_interpolate(1, ob_string("ob_str_lower"))); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("trim"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("arity"))), ob_int(0LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_interpolate(1, ob_string("ob_str_trim"))); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("split"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("arity"))), ob_int(1LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_interpolate(1, ob_string("ob_str_split"))); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("starts_with"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("arity"))), ob_int(1LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_interpolate(1, ob_string("ob_str_starts_with"))); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("ends_with"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("arity"))), ob_int(1LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_interpolate(1, ob_string("ob_str_ends_with"))); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("replace"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("arity"))), ob_int(2LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_interpolate(1, ob_string("ob_str_replace"))); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("substr"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("arity"))), ob_int(2LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_interpolate(1, ob_string("ob_str_substr"))); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("repeat"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("arity"))), ob_int(1LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_interpolate(1, ob_string("ob_str_repeat"))); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("to_int"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("arity"))), ob_int(0LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_interpolate(1, ob_string("ob_str_to_int"))); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("to_float"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("arity"))), ob_int(0LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_interpolate(1, ob_string("ob_str_to_float"))); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("push"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("arity"))), ob_int(1LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_interpolate(1, ob_string("ob_arr_push"))); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("pop"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("arity"))), ob_int(0LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_interpolate(1, ob_string("ob_arr_pop"))); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("insert"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("arity"))), ob_int(2LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_interpolate(1, ob_string("ob_arr_insert"))); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("remove_at"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("arity"))), ob_int(1LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_interpolate(1, ob_string("ob_arr_remove_at"))); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("join"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("arity"))), ob_int(1LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_interpolate(1, ob_string("ob_arr_join"))); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("keys"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("arity"))), ob_int(0LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_interpolate(1, ob_string("ob_dict_keys"))); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("values"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("arity"))), ob_int(0LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_interpolate(1, ob_string("ob_dict_values"))); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("has"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("arity"))), ob_int(1LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_interpolate(1, ob_string("ob_dict_has_m"))); __d; })); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("remove"))), ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("arity"))), ob_int(1LL)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cfunc"))), ob_interpolate(1, ob_string("ob_dict_remove"))); __d; })); __d; });
    codegen__BINOP_FALLBACKS = ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("+"))), ob_interpolate(1, ob_string("ob_add"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("-"))), ob_interpolate(1, ob_string("ob_sub"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("*"))), ob_interpolate(1, ob_string("ob_mul"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("/"))), ob_interpolate(1, ob_string("ob_div"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("%"))), ob_interpolate(1, ob_string("ob_mod"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("=="))), ob_interpolate(1, ob_string("ob_eq"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("!="))), ob_interpolate(1, ob_string("ob_neq"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("<"))), ob_interpolate(1, ob_string("ob_lt"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("<="))), ob_interpolate(1, ob_string("ob_lte"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string(">"))), ob_interpolate(1, ob_string("ob_gt"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string(">="))), ob_interpolate(1, ob_string("ob_gte"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("&"))), ob_interpolate(1, ob_string("ob_band"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("|"))), ob_interpolate(1, ob_string("ob_bor"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("^"))), ob_interpolate(1, ob_string("ob_bxor"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("<<"))), ob_interpolate(1, ob_string("ob_shl"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string(">>"))), ob_interpolate(1, ob_string("ob_shr"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("x"))), ob_interpolate(1, ob_string("ob_repeat"))); __d; });
    codegen__PRINT_FUNCS = ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("print"))), ob_interpolate(1, ob_string("ob_print"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("write"))), ob_interpolate(1, ob_string("ob_write"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("eprint"))), ob_interpolate(1, ob_string("ob_eprint"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("ewrite"))), ob_interpolate(1, ob_string("ob_ewrite"))); __d; });
    codegen__SCAN_METHOD = ob_null();
    codegen__INFERRED = ({ OboeValue __a = ob_array_new(); __a; });
    codegen__UNITS = ({ OboeValue __a = ob_array_new(); __a; });
    codegen__TARGET_OS = hostos__HOST_OS;
    codegen__LIB_ROOT = ob_string("");
}

OboeValue dump__oct3(OboeValue n) {
    return ob_binop("+", ob_binop("+", ob_str(ob_binop("/", n, ob_int(64LL), ob_div)), ob_str(ob_binop("%", ob_binop("/", n, ob_int(8LL), ob_div), ob_int(8LL), ob_mod)), ob_add), ob_str(ob_binop("%", n, ob_int(8LL), ob_mod)), ob_add);
    return ob_null();
}

OboeValue dump__dump_escape(OboeValue s) {
    OboeValue out = ({ OboeValue __a = ob_array_new(); __a; });
    OboeValue n = ob_m_len(s);
    OboeValue i = ob_int(0LL);
    while (ob_truthy(ob_binop("<", i, n, ob_lt))) {
        OboeValue c = ob_str_substr(s, i, ob_int(1LL));
        OboeValue v = ob_ord(c);
        if (ob_truthy(ob_binop("==", c, ob_interpolate(1, ob_string("\\")), ob_eq))) {
            (void)(ob_arr_push(out, ob_interpolate(1, ob_string("\\\\"))));
        }
        else {
            if (ob_truthy(ob_binop("==", v, ob_int(10LL), ob_eq))) {
                (void)(ob_arr_push(out, ob_interpolate(1, ob_string("\\n"))));
            }
            else {
                if (ob_truthy(ob_binop("==", v, ob_int(9LL), ob_eq))) {
                    (void)(ob_arr_push(out, ob_interpolate(1, ob_string("\\t"))));
                }
                else {
                    if (ob_truthy(ob_binop("==", v, ob_int(13LL), ob_eq))) {
                        (void)(ob_arr_push(out, ob_interpolate(1, ob_string("\\r"))));
                    }
                    else {
                        if (ob_truthy(ob_binop("==", c, ob_interpolate(1, ob_string("\"")), ob_eq))) {
                            (void)(ob_arr_push(out, ob_interpolate(1, ob_string("\\\""))));
                        }
                        else {
                            if (ob_truthy(ob_bool(ob_truthy(ob_binop("<", v, ob_int(32LL), ob_lt)) || ob_truthy(ob_binop("==", v, ob_int(127LL), ob_eq))))) {
                                (void)(ob_arr_push(out, ob_binop("+", ob_interpolate(1, ob_string("\\")), dump__oct3(v), ob_add)));
                            }
                            else {
                                (void)(ob_arr_push(out, c));
                            }
                        }
                    }
                }
            }
        }
        (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
    }
    return ob_arr_join(out, ob_string(""));
    return ob_null();
}

OboeValue dump__dump_tokens(OboeValue toks) {
    OboeValue out = ({ OboeValue __a = ob_array_new(); __a; });
    { OboeValue __it = toks; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue t = ob_iter_value(__it, __i);
        (void)(ob_arr_push(out, ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_index_get(t, ob_interpolate(1, ob_string("type"))), ob_interpolate(1, ob_string(" ")), ob_add), ob_str(ob_index_get(t, ob_interpolate(1, ob_string("line")))), ob_add), ob_interpolate(1, ob_string(" ")), ob_add), dump__dump_escape(ob_index_get(t, ob_interpolate(1, ob_string("text")))), ob_add), ob_interpolate(1, ob_string("\n")), ob_add)));
    } }
    return ob_arr_join(out, ob_string(""));
    return ob_null();
}

OboeValue dump__emit(OboeValue s) {
    (void)(ob_arr_push(dump__OUT, s));
    return ob_null();
}

OboeValue dump__ind(OboeValue depth) {
    (void)(dump__emit(ob_str_repeat(ob_interpolate(1, ob_string("  ")), depth)));
    return ob_null();
}

OboeValue dump__b01(OboeValue flag) {
    return (ob_truthy(flag) ? (ob_interpolate(1, ob_string("1"))) : (ob_interpolate(1, ob_string("0"))));
    return ob_null();
}

OboeValue dump__field_str(OboeValue key, OboeValue val) {
    (void)(dump__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string(" ")), key, ob_add), ob_interpolate(1, ob_string("=")), ob_add)));
    if (ob_truthy(ob_binop("==", val, ob_null(), ob_eq))) {
        (void)(dump__emit(ob_interpolate(1, ob_string("-"))));
    }
    else {
        (void)(dump__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("\"")), dump__dump_escape(val), ob_add), ob_interpolate(1, ob_string("\"")), ob_add)));
    }
    return ob_null();
}

OboeValue dump__field_double(OboeValue key, OboeValue value) {
    (void)(dump__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string(" ")), key, ob_add), ob_interpolate(1, ob_string("=")), ob_add), ob_str(value), ob_add)));
    return ob_null();
}

OboeValue dump__dump_body(OboeValue label, OboeValue body, OboeValue depth) {
    (void)(dump__ind(depth));
    (void)(dump__emit(ob_binop("+", label, ob_interpolate(1, ob_string("\n")), ob_add)));
    { OboeValue __it = body; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue s = ob_iter_value(__it, __i);
        (void)(dump__dump_stmt(s, ob_binop("+", depth, ob_int(1LL), ob_add)));
    } }
    return ob_null();
}

OboeValue dump__dump_opt(OboeValue label, OboeValue e, OboeValue depth) {
    if (ob_truthy(ob_binop("==", e, ob_null(), ob_eq))) {
        return ob_null();
    }
    (void)(dump__ind(depth));
    (void)(dump__emit(ob_binop("+", label, ob_interpolate(1, ob_string("\n")), ob_add)));
    (void)(dump__dump_expr(e, ob_binop("+", depth, ob_int(1LL), ob_add)));
    return ob_null();
}

OboeValue dump__dump_params(OboeValue params, OboeValue depth) {
    (void)(dump__ind(depth));
    (void)(dump__emit(ob_interpolate(1, ob_string("params\n"))));
    { OboeValue __it = params; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue p = ob_iter_value(__it, __i);
        (void)(dump__ind(ob_binop("+", depth, ob_int(1LL), ob_add)));
        (void)(dump__emit(ob_interpolate(1, ob_string("param"))));
        (void)(dump__field_str(ob_interpolate(1, ob_string("name")), ob_index_get(p, ob_interpolate(1, ob_string("name")))));
        (void)(dump__field_str(ob_interpolate(1, ob_string("type")), ob_index_get(p, ob_interpolate(1, ob_string("type_name")))));
        (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
        (void)(dump__dump_opt(ob_interpolate(1, ob_string("default")), ob_index_get(p, ob_interpolate(1, ob_string("default_value"))), ob_binop("+", depth, ob_int(2LL), ob_add)));
    } }
    return ob_null();
}

OboeValue dump__dump_expr(OboeValue e, OboeValue depth) {
    (void)(dump__ind(depth));
    OboeValue kind = ob_index_get(e, ob_interpolate(1, ob_string("kind")));
    (void)(dump__emit(ob_binop("+", ob_binop("+", kind, ob_interpolate(1, ob_string(" line=")), ob_add), ob_str(ob_index_get(e, ob_interpolate(1, ob_string("line")))), ob_add)));
    if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("EXPR_INT")), ob_eq))) {
        (void)(dump__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string(" value=")), ob_str(ob_index_get(e, ob_interpolate(1, ob_string("int_val")))), ob_add), ob_interpolate(1, ob_string("\n")), ob_add)));
    }
    else {
        if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("EXPR_FLOAT")), ob_eq))) {
            (void)(dump__field_double(ob_interpolate(1, ob_string("value")), ob_index_get(e, ob_interpolate(1, ob_string("float_val")))));
            (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
        }
        else {
            if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("EXPR_BOOL")), ob_eq))) {
                (void)(dump__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string(" value=")), dump__b01(ob_index_get(e, ob_interpolate(1, ob_string("bool_val")))), ob_add), ob_interpolate(1, ob_string("\n")), ob_add)));
            }
            else {
                if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("EXPR_NULL")), ob_eq))) {
                    (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
                }
                else {
                    if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("EXPR_STRING")), ob_eq))) {
                        (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
                        { OboeValue __it = ob_index_get(e, ob_interpolate(1, ob_string("str_parts"))); int64_t __n = ob_iter_len(__it);
                        for (int64_t __i = 0; __i < __n; __i++) {
                            OboeValue part = ob_iter_value(__it, __i);
                            (void)(dump__ind(ob_binop("+", depth, ob_int(1LL), ob_add)));
                            if (ob_truthy(ob_index_get(part, ob_interpolate(1, ob_string("is_expr"))))) {
                                (void)(dump__emit(ob_interpolate(1, ob_string("part expr\n"))));
                                (void)(dump__dump_expr(ob_index_get(part, ob_interpolate(1, ob_string("expr"))), ob_binop("+", depth, ob_int(2LL), ob_add)));
                            }
                            else {
                                (void)(dump__emit(ob_interpolate(1, ob_string("part lit"))));
                                (void)(dump__field_str(ob_interpolate(1, ob_string("text")), ob_index_get(part, ob_interpolate(1, ob_string("literal")))));
                                (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
                            }
                        } }
                    }
                    else {
                        if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("EXPR_IDENT")), ob_eq))) {
                            (void)(dump__field_str(ob_interpolate(1, ob_string("name")), ob_index_get(e, ob_interpolate(1, ob_string("ident")))));
                            (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
                        }
                        else {
                            if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("EXPR_ARRAY")), ob_eq))) {
                                (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
                                { OboeValue __it = ob_index_get(e, ob_interpolate(1, ob_string("items"))); int64_t __n = ob_iter_len(__it);
                                for (int64_t __i = 0; __i < __n; __i++) {
                                    OboeValue item = ob_iter_value(__it, __i);
                                    (void)(dump__dump_expr(item, ob_binop("+", depth, ob_int(1LL), ob_add)));
                                } }
                            }
                            else {
                                if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("EXPR_DICT")), ob_eq))) {
                                    (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
                                    OboeValue keys = ob_index_get(e, ob_interpolate(1, ob_string("keys")));
                                    OboeValue values = ob_index_get(e, ob_interpolate(1, ob_string("values")));
                                    OboeValue i = ob_int(0LL);
                                    while (ob_truthy(ob_binop("<", i, ob_m_len(keys), ob_lt))) {
                                        (void)(dump__ind(ob_binop("+", depth, ob_int(1LL), ob_add)));
                                        (void)(dump__emit(ob_interpolate(1, ob_string("entry\n"))));
                                        (void)(dump__dump_expr(ob_index_get(keys, i), ob_binop("+", depth, ob_int(2LL), ob_add)));
                                        (void)(dump__dump_expr(ob_index_get(values, i), ob_binop("+", depth, ob_int(2LL), ob_add)));
                                        (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
                                    }
                                }
                                else {
                                    if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("EXPR_BINARY")), ob_eq))) {
                                        (void)(dump__field_str(ob_interpolate(1, ob_string("op")), ob_index_get(e, ob_interpolate(1, ob_string("op")))));
                                        (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
                                        (void)(dump__dump_expr(ob_index_get(e, ob_interpolate(1, ob_string("l"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
                                        (void)(dump__dump_expr(ob_index_get(e, ob_interpolate(1, ob_string("r"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
                                    }
                                    else {
                                        if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("EXPR_UNARY")), ob_eq))) {
                                            (void)(dump__field_str(ob_interpolate(1, ob_string("op")), ob_index_get(e, ob_interpolate(1, ob_string("op")))));
                                            (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
                                            (void)(dump__dump_expr(ob_index_get(e, ob_interpolate(1, ob_string("operand"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
                                        }
                                        else {
                                            if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("EXPR_CALL")), ob_eq))) {
                                                (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
                                                (void)(dump__ind(ob_binop("+", depth, ob_int(1LL), ob_add)));
                                                (void)(dump__emit(ob_interpolate(1, ob_string("callee\n"))));
                                                (void)(dump__dump_expr(ob_index_get(e, ob_interpolate(1, ob_string("callee"))), ob_binop("+", depth, ob_int(2LL), ob_add)));
                                                (void)(dump__ind(ob_binop("+", depth, ob_int(1LL), ob_add)));
                                                (void)(dump__emit(ob_interpolate(1, ob_string("args\n"))));
                                                OboeValue args = ob_index_get(e, ob_interpolate(1, ob_string("args")));
                                                OboeValue names = ob_index_get(e, ob_interpolate(1, ob_string("arg_names")));
                                                OboeValue i = ob_int(0LL);
                                                while (ob_truthy(ob_binop("<", i, ob_m_len(args), ob_lt))) {
                                                    (void)(dump__ind(ob_binop("+", depth, ob_int(2LL), ob_add)));
                                                    (void)(dump__emit(ob_interpolate(1, ob_string("arg"))));
                                                    (void)(dump__field_str(ob_interpolate(1, ob_string("name")), ob_index_get(names, i)));
                                                    (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
                                                    (void)(dump__dump_expr(ob_index_get(args, i), ob_binop("+", depth, ob_int(3LL), ob_add)));
                                                    (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
                                                }
                                            }
                                            else {
                                                if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("EXPR_FIELD")), ob_eq)) || ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("EXPR_SAFE_FIELD")), ob_eq))))) {
                                                    (void)(dump__field_str(ob_interpolate(1, ob_string("name")), ob_index_get(e, ob_interpolate(1, ob_string("name")))));
                                                    (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
                                                    (void)(dump__dump_expr(ob_index_get(e, ob_interpolate(1, ob_string("obj"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
                                                }
                                                else {
                                                    if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("EXPR_INDEX")), ob_eq))) {
                                                        (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
                                                        (void)(dump__dump_expr(ob_index_get(e, ob_interpolate(1, ob_string("arr"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
                                                        (void)(dump__dump_expr(ob_index_get(e, ob_interpolate(1, ob_string("idx"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
                                                    }
                                                    else {
                                                        if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("EXPR_IS")), ob_eq))) {
                                                            (void)(dump__field_str(ob_interpolate(1, ob_string("type")), ob_index_get(e, ob_interpolate(1, ob_string("type_name")))));
                                                            (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
                                                            (void)(dump__dump_expr(ob_index_get(e, ob_interpolate(1, ob_string("value"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
                                                        }
                                                        else {
                                                            if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("EXPR_ASSIGN")), ob_eq))) {
                                                                (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
                                                                (void)(dump__dump_expr(ob_index_get(e, ob_interpolate(1, ob_string("target"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
                                                                (void)(dump__dump_expr(ob_index_get(e, ob_interpolate(1, ob_string("value"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
                                                            }
                                                            else {
                                                                if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("EXPR_TERNARY")), ob_eq))) {
                                                                    (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
                                                                    (void)(dump__dump_expr(ob_index_get(e, ob_interpolate(1, ob_string("cond"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
                                                                    (void)(dump__dump_expr(ob_index_get(e, ob_interpolate(1, ob_string("then_e"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
                                                                    (void)(dump__dump_expr(ob_index_get(e, ob_interpolate(1, ob_string("else_e"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return ob_null();
}

OboeValue dump__dump_stmt(OboeValue s, OboeValue depth) {
    (void)(dump__ind(depth));
    OboeValue kind = ob_index_get(s, ob_interpolate(1, ob_string("kind")));
    (void)(dump__emit(ob_binop("+", ob_binop("+", kind, ob_interpolate(1, ob_string(" line=")), ob_add), ob_str(ob_index_get(s, ob_interpolate(1, ob_string("line")))), ob_add)));
    if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("STMT_LET")), ob_eq))) {
        (void)(dump__field_str(ob_interpolate(1, ob_string("name")), ob_index_get(s, ob_interpolate(1, ob_string("name")))));
        (void)(dump__field_str(ob_interpolate(1, ob_string("type")), ob_index_get(s, ob_interpolate(1, ob_string("type_name")))));
        (void)(dump__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string(" const=")), dump__b01(ob_index_get(s, ob_interpolate(1, ob_string("is_const")))), ob_add), ob_interpolate(1, ob_string("\n")), ob_add)));
        (void)(dump__dump_opt(ob_interpolate(1, ob_string("init")), ob_index_get(s, ob_interpolate(1, ob_string("init"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
    }
    else {
        if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("STMT_EXPR")), ob_eq))) {
            (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
            (void)(dump__dump_opt(ob_interpolate(1, ob_string("expr")), ob_index_get(s, ob_interpolate(1, ob_string("expr"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
        }
        else {
            if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("STMT_RETURN")), ob_eq))) {
                (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
                (void)(dump__dump_opt(ob_interpolate(1, ob_string("value")), ob_index_get(s, ob_interpolate(1, ob_string("value"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
            }
            else {
                if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("STMT_IF")), ob_eq))) {
                    (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
                    (void)(dump__dump_opt(ob_interpolate(1, ob_string("cond")), ob_index_get(s, ob_interpolate(1, ob_string("cond"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
                    (void)(dump__dump_body(ob_interpolate(1, ob_string("then")), ob_index_get(s, ob_interpolate(1, ob_string("then_body"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
                    (void)(dump__dump_body(ob_interpolate(1, ob_string("else")), ob_index_get(s, ob_interpolate(1, ob_string("else_body"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
                }
                else {
                    if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("STMT_WHILE")), ob_eq))) {
                        (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
                        (void)(dump__dump_opt(ob_interpolate(1, ob_string("cond")), ob_index_get(s, ob_interpolate(1, ob_string("cond"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
                        (void)(dump__dump_body(ob_interpolate(1, ob_string("body")), ob_index_get(s, ob_interpolate(1, ob_string("body"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
                    }
                    else {
                        if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("STMT_FOR")), ob_eq))) {
                            (void)(dump__field_str(ob_interpolate(1, ob_string("var")), ob_index_get(s, ob_interpolate(1, ob_string("var_name")))));
                            (void)(dump__field_str(ob_interpolate(1, ob_string("var2")), ob_index_get(s, ob_interpolate(1, ob_string("var2_name")))));
                            (void)(dump__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string(" iter=")), ob_index_get(s, ob_interpolate(1, ob_string("kind_iter"))), ob_add), ob_interpolate(1, ob_string("\n")), ob_add)));
                            (void)(dump__dump_opt(ob_interpolate(1, ob_string("range_a")), ob_index_get(s, ob_interpolate(1, ob_string("range_a"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
                            (void)(dump__dump_opt(ob_interpolate(1, ob_string("range_b")), ob_index_get(s, ob_interpolate(1, ob_string("range_b"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
                            (void)(dump__dump_opt(ob_interpolate(1, ob_string("iterable")), ob_index_get(s, ob_interpolate(1, ob_string("iterable"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
                            (void)(dump__dump_body(ob_interpolate(1, ob_string("body")), ob_index_get(s, ob_interpolate(1, ob_string("body"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
                        }
                        else {
                            if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("STMT_SWITCH")), ob_eq))) {
                                (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
                                (void)(dump__dump_opt(ob_interpolate(1, ob_string("subject")), ob_index_get(s, ob_interpolate(1, ob_string("subject"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
                                (void)(dump__ind(ob_binop("+", depth, ob_int(1LL), ob_add)));
                                (void)(dump__emit(ob_interpolate(1, ob_string("cases\n"))));
                                { OboeValue __it = ob_index_get(s, ob_interpolate(1, ob_string("cases"))); int64_t __n = ob_iter_len(__it);
                                for (int64_t __i = 0; __i < __n; __i++) {
                                    OboeValue c = ob_iter_value(__it, __i);
                                    (void)(dump__ind(ob_binop("+", depth, ob_int(2LL), ob_add)));
                                    (void)(dump__emit(ob_interpolate(1, ob_string("case\n"))));
                                    (void)(dump__dump_opt(ob_interpolate(1, ob_string("value")), ob_index_get(c, ob_interpolate(1, ob_string("value"))), ob_binop("+", depth, ob_int(3LL), ob_add)));
                                    (void)(dump__dump_body(ob_interpolate(1, ob_string("body")), ob_index_get(c, ob_interpolate(1, ob_string("body"))), ob_binop("+", depth, ob_int(3LL), ob_add)));
                                } }
                            }
                            else {
                                if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("STMT_TRY")), ob_eq))) {
                                    (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
                                    (void)(dump__dump_body(ob_interpolate(1, ob_string("body")), ob_index_get(s, ob_interpolate(1, ob_string("body"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
                                    (void)(dump__ind(ob_binop("+", depth, ob_int(1LL), ob_add)));
                                    (void)(dump__emit(ob_interpolate(1, ob_string("catches\n"))));
                                    { OboeValue __it = ob_index_get(s, ob_interpolate(1, ob_string("catches"))); int64_t __n = ob_iter_len(__it);
                                    for (int64_t __i = 0; __i < __n; __i++) {
                                        OboeValue c = ob_iter_value(__it, __i);
                                        (void)(dump__ind(ob_binop("+", depth, ob_int(2LL), ob_add)));
                                        (void)(dump__emit(ob_interpolate(1, ob_string("catch"))));
                                        (void)(dump__field_str(ob_interpolate(1, ob_string("type")), ob_index_get(c, ob_interpolate(1, ob_string("type_name")))));
                                        (void)(dump__field_str(ob_interpolate(1, ob_string("var")), ob_index_get(c, ob_interpolate(1, ob_string("var_name")))));
                                        (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
                                        (void)(dump__dump_body(ob_interpolate(1, ob_string("body")), ob_index_get(c, ob_interpolate(1, ob_string("body"))), ob_binop("+", depth, ob_int(3LL), ob_add)));
                                    } }
                                    (void)(dump__dump_body(ob_interpolate(1, ob_string("finally")), ob_index_get(s, ob_interpolate(1, ob_string("finally_body"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
                                }
                                else {
                                    if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("STMT_THROW")), ob_eq))) {
                                        (void)(dump__field_str(ob_interpolate(1, ob_string("type")), ob_index_get(s, ob_interpolate(1, ob_string("type_name")))));
                                        (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
                                        (void)(dump__dump_opt(ob_interpolate(1, ob_string("value")), ob_index_get(s, ob_interpolate(1, ob_string("value"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
                                    }
                                    else {
                                        if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("STMT_BLOCK")), ob_eq))) {
                                            (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
                                            (void)(dump__dump_body(ob_interpolate(1, ob_string("body")), ob_index_get(s, ob_interpolate(1, ob_string("body"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
                                        }
                                        else {
                                            (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return ob_null();
}

OboeValue dump__dump_func(OboeValue label, OboeValue f, OboeValue depth) {
    (void)(dump__ind(depth));
    (void)(dump__emit(label));
    (void)(dump__field_str(ob_interpolate(1, ob_string("name")), ob_index_get(f, ob_interpolate(1, ob_string("name")))));
    (void)(dump__emit(ob_binop("+", ob_interpolate(1, ob_string(" line=")), ob_str(ob_index_get(f, ob_interpolate(1, ob_string("line")))), ob_add)));
    (void)(dump__field_str(ob_interpolate(1, ob_string("ret")), ob_index_get(f, ob_interpolate(1, ob_string("return_type")))));
    (void)(dump__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string(" static=")), dump__b01(ob_index_get(f, ob_interpolate(1, ob_string("is_static")))), ob_add), ob_interpolate(1, ob_string(" private=")), ob_add), dump__b01(ob_index_get(f, ob_interpolate(1, ob_string("is_private")))), ob_add)));
    (void)(dump__field_str(ob_interpolate(1, ob_string("op")), ob_index_get(f, ob_interpolate(1, ob_string("op_symbol")))));
    (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
    (void)(dump__dump_params(ob_index_get(f, ob_interpolate(1, ob_string("params"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
    (void)(dump__dump_body(ob_interpolate(1, ob_string("body")), ob_index_get(f, ob_interpolate(1, ob_string("body"))), ob_binop("+", depth, ob_int(1LL), ob_add)));
    return ob_null();
}

OboeValue dump__dump_class(OboeValue c, OboeValue depth) {
    (void)(dump__ind(depth));
    (void)(dump__emit(ob_interpolate(1, ob_string("DECL_CLASS"))));
    (void)(dump__field_str(ob_interpolate(1, ob_string("name")), ob_index_get(c, ob_interpolate(1, ob_string("name")))));
    (void)(dump__field_str(ob_interpolate(1, ob_string("parent")), ob_index_get(c, ob_interpolate(1, ob_string("parent_name")))));
    (void)(dump__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string(" line=")), ob_str(ob_index_get(c, ob_interpolate(1, ob_string("line")))), ob_add), ob_interpolate(1, ob_string("\n")), ob_add)));
    (void)(dump__ind(ob_binop("+", depth, ob_int(1LL), ob_add)));
    (void)(dump__emit(ob_interpolate(1, ob_string("fields\n"))));
    { OboeValue __it = ob_index_get(c, ob_interpolate(1, ob_string("fields"))); int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue f = ob_iter_value(__it, __i);
        (void)(dump__ind(ob_binop("+", depth, ob_int(2LL), ob_add)));
        (void)(dump__emit(ob_interpolate(1, ob_string("field"))));
        (void)(dump__field_str(ob_interpolate(1, ob_string("name")), ob_index_get(f, ob_interpolate(1, ob_string("name")))));
        (void)(dump__field_str(ob_interpolate(1, ob_string("type")), ob_index_get(f, ob_interpolate(1, ob_string("type_name")))));
        (void)(dump__emit(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string(" static=")), dump__b01(ob_index_get(f, ob_interpolate(1, ob_string("is_static")))), ob_add), ob_interpolate(1, ob_string(" private=")), ob_add), dump__b01(ob_index_get(f, ob_interpolate(1, ob_string("is_private")))), ob_add), ob_interpolate(1, ob_string(" const=")), ob_add), dump__b01(ob_index_get(f, ob_interpolate(1, ob_string("is_const")))), ob_add), ob_interpolate(1, ob_string("\n")), ob_add)));
        (void)(dump__dump_opt(ob_interpolate(1, ob_string("init")), ob_index_get(f, ob_interpolate(1, ob_string("init"))), ob_binop("+", depth, ob_int(3LL), ob_add)));
    } }
    (void)(dump__ind(ob_binop("+", depth, ob_int(1LL), ob_add)));
    (void)(dump__emit(ob_interpolate(1, ob_string("methods\n"))));
    { OboeValue __it = ob_index_get(c, ob_interpolate(1, ob_string("methods"))); int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue m = ob_iter_value(__it, __i);
        (void)(dump__dump_func(ob_interpolate(1, ob_string("method")), m, ob_binop("+", depth, ob_int(2LL), ob_add)));
    } }
    return ob_null();
}

OboeValue dump__dump_ast(OboeValue decls) {
    (void)((dump__OUT = ({ OboeValue __a = ob_array_new(); __a; })));
    (void)(dump__emit(ob_interpolate(1, ob_string("program\n"))));
    { OboeValue __it = decls; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue d = ob_iter_value(__it, __i);
        OboeValue kind = ob_index_get(d, ob_interpolate(1, ob_string("kind")));
        if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("DECL_FUNC")), ob_eq)) || ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("DECL_OPERATOR")), ob_eq))))) {
            (void)(dump__dump_func(kind, ob_index_get(d, ob_interpolate(1, ob_string("func"))), ob_int(1LL)));
        }
        else {
            if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("DECL_CLASS")), ob_eq))) {
                (void)(dump__dump_class(ob_index_get(d, ob_interpolate(1, ob_string("klass"))), ob_int(1LL)));
            }
            else {
                if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("DECL_IMPORT")), ob_eq))) {
                    (void)(dump__ind(ob_int(1LL)));
                    (void)(dump__emit(ob_interpolate(1, ob_string("DECL_IMPORT"))));
                    (void)(dump__field_str(ob_interpolate(1, ob_string("module")), ob_index_get(d, ob_interpolate(1, ob_string("module")))));
                    (void)(dump__field_str(ob_interpolate(1, ob_string("alias")), ob_index_get(d, ob_interpolate(1, ob_string("alias")))));
                    (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
                    (void)(dump__ind(ob_int(2LL)));
                    (void)(dump__emit(ob_interpolate(1, ob_string("members\n"))));
                    { OboeValue __it = ob_index_get(d, ob_interpolate(1, ob_string("members"))); int64_t __n = ob_iter_len(__it);
                    for (int64_t __i = 0; __i < __n; __i++) {
                        OboeValue m = ob_iter_value(__it, __i);
                        (void)(dump__ind(ob_int(3LL)));
                        (void)(dump__emit(ob_interpolate(1, ob_string("member"))));
                        (void)(dump__field_str(ob_interpolate(1, ob_string("name")), m));
                        (void)(dump__emit(ob_interpolate(1, ob_string("\n"))));
                    } }
                }
                else {
                    if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("DECL_STMT")), ob_eq))) {
                        (void)(dump__ind(ob_int(1LL)));
                        (void)(dump__emit(ob_interpolate(1, ob_string("DECL_STMT\n"))));
                        (void)(dump__dump_stmt(ob_index_get(d, ob_interpolate(1, ob_string("stmt"))), ob_int(2LL)));
                    }
                    else {
                        if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("DECL_EVENT")), ob_eq))) {
                            (void)(dump__ind(ob_int(1LL)));
                            (void)(dump__emit(ob_interpolate(1, ob_string("DECL_EVENT"))));
                            (void)(dump__field_str(ob_interpolate(1, ob_string("name")), ob_index_get(d, ob_interpolate(1, ob_string("name")))));
                            (void)(dump__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string(" line=")), ob_str(ob_index_get(d, ob_interpolate(1, ob_string("line")))), ob_add), ob_interpolate(1, ob_string("\n")), ob_add)));
                            (void)(dump__dump_params(ob_index_get(d, ob_interpolate(1, ob_string("params"))), ob_int(2LL)));
                        }
                        else {
                            if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("DECL_ON")), ob_eq))) {
                                (void)(dump__ind(ob_int(1LL)));
                                (void)(dump__emit(ob_interpolate(1, ob_string("DECL_ON"))));
                                (void)(dump__field_str(ob_interpolate(1, ob_string("module")), ob_index_get(d, ob_interpolate(1, ob_string("event_module")))));
                                (void)(dump__field_str(ob_interpolate(1, ob_string("event")), ob_index_get(d, ob_interpolate(1, ob_string("event_name")))));
                                (void)(dump__field_str(ob_interpolate(1, ob_string("var")), ob_index_get(d, ob_interpolate(1, ob_string("var_name")))));
                                (void)(dump__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string(" line=")), ob_str(ob_index_get(d, ob_interpolate(1, ob_string("line")))), ob_add), ob_interpolate(1, ob_string("\n")), ob_add)));
                                (void)(dump__dump_body(ob_interpolate(1, ob_string("body")), ob_index_get(d, ob_interpolate(1, ob_string("body"))), ob_int(2LL)));
                            }
                            else {
                                if (ob_truthy(ob_binop("==", kind, ob_interpolate(1, ob_string("DECL_CIMPORT")), ob_eq))) {
                                    (void)(dump__ind(ob_int(1LL)));
                                    (void)(dump__emit(ob_interpolate(1, ob_string("DECL_CIMPORT"))));
                                    (void)(dump__field_str(ob_interpolate(1, ob_string("name")), ob_index_get(d, ob_interpolate(1, ob_string("name")))));
                                    (void)(dump__field_str(ob_interpolate(1, ob_string("lib")), ob_index_get(d, ob_interpolate(1, ob_string("lib")))));
                                    (void)(dump__emit(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string(" line=")), ob_str(ob_index_get(d, ob_interpolate(1, ob_string("line")))), ob_add), ob_interpolate(1, ob_string("\n")), ob_add)));
                                }
                            }
                        }
                    }
                }
            }
        }
    } }
    return ob_arr_join(dump__OUT, ob_string(""));
    return ob_null();
}

static void __oboe_toplevel_4(void) {
    dump__OUT = ({ OboeValue __a = ob_array_new(); __a; });
}

void Parser__init_0(Parser *this, OboeValue toks, OboeValue filename) {
    (void)((((Parser*)(this))->toks = toks));
    (void)((((Parser*)(this))->count = ob_m_len(toks)));
    (void)((((Parser*)(this))->pos = ob_int(0LL)));
    (void)((((Parser*)(this))->filename = filename));
}

OboeValue Parser__new_0(OboeValue toks, OboeValue filename) {
    Parser *obj = calloc(1, sizeof(Parser));
    ((OboeObject*)obj)->cls = &Parser__classinfo;
    Parser__init_0(obj, toks, filename);
    return ob_object_wrap(obj);
}

OboeValue parser__peek(OboeValue p) {
    return ob_index_get(((Parser*)((p).as.obj))->toks, ((Parser*)((p).as.obj))->pos);
    return ob_null();
}

OboeValue parser__peek_at(OboeValue p, OboeValue off) {
    OboeValue i = ob_binop("+", ((Parser*)((p).as.obj))->pos, off, ob_add);
    if (ob_truthy(ob_binop(">=", i, ((Parser*)((p).as.obj))->count, ob_gte))) {
        (void)((i = ob_binop("-", ((Parser*)((p).as.obj))->count, ob_int(1LL), ob_sub)));
    }
    return ob_index_get(((Parser*)((p).as.obj))->toks, i);
    return ob_null();
}

OboeValue parser__advance(OboeValue p) {
    OboeValue t = ob_index_get(((Parser*)((p).as.obj))->toks, ((Parser*)((p).as.obj))->pos);
    if (ob_truthy(ob_binop("<", ((Parser*)((p).as.obj))->pos, ob_binop("-", ((Parser*)((p).as.obj))->count, ob_int(1LL), ob_sub), ob_lt))) {
        (void)((((Parser*)((p).as.obj))->pos = ob_binop("+", ((Parser*)((p).as.obj))->pos, ob_int(1LL), ob_add)));
    }
    return t;
    return ob_null();
}

OboeValue parser__check(OboeValue p, OboeValue t) {
    return ob_binop("==", ob_index_get(ob_index_get(((Parser*)((p).as.obj))->toks, ((Parser*)((p).as.obj))->pos), ob_interpolate(1, ob_string("type"))), t, ob_eq);
    return ob_null();
}

OboeValue parser__match(OboeValue p, OboeValue t) {
    if (ob_truthy(parser__check(p, t))) {
        (void)(parser__advance(p));
        return ob_bool(true);
    }
    return ob_bool(false);
    return ob_null();
}

OboeValue parser__fail(OboeValue p, OboeValue msg) {
    OboeValue t = parser__peek(p);
    (void)(diag__die(ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ob_binop("+", ((Parser*)((p).as.obj))->filename, ob_interpolate(1, ob_string(":")), ob_add), ob_str(ob_index_get(t, ob_interpolate(1, ob_string("line")))), ob_add), ob_interpolate(1, ob_string(": parse error: ")), ob_add), msg, ob_add), ob_interpolate(1, ob_string(" (got '")), ob_add), ob_index_get(t, ob_interpolate(1, ob_string("text"))), ob_add), ob_interpolate(1, ob_string("')")), ob_add)));
    return ob_null();
}

OboeValue parser__expect(OboeValue p, OboeValue t, OboeValue what) {
    if (ob_truthy(ob_not(parser__check(p, t)))) {
        (void)(parser__fail(p, what));
    }
    return parser__advance(p);
    return ob_null();
}

OboeValue parser__new_expr(OboeValue kind, OboeValue line) {
    return ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("kind"))), kind); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("line"))), line); __d; });
    return ob_null();
}

OboeValue parser__new_stmt(OboeValue kind, OboeValue line) {
    return ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("kind"))), kind); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("line"))), line); __d; });
    return ob_null();
}

OboeValue parser__parse_string_literal_parts(OboeValue p, OboeValue raw) {
    OboeValue parts = ({ OboeValue __a = ob_array_new(); __a; });
    OboeValue len = ob_m_len(raw);
    OboeValue i = ob_int(0LL);
    OboeValue litstart = ob_int(0LL);
    while (ob_truthy(ob_binop("<", i, len, ob_lt))) {
        if (ob_truthy(ob_bool(ob_truthy(ob_bool(ob_truthy(ob_binop("==", ob_str_substr(raw, i, ob_int(1LL)), ob_interpolate(1, ob_string("$")), ob_eq)) && ob_truthy(ob_binop("<", ob_binop("+", i, ob_int(1LL), ob_add), len, ob_lt)))) && ob_truthy(ob_binop("==", ob_str_substr(raw, ob_binop("+", i, ob_int(1LL), ob_add), ob_int(1LL)), ob_interpolate(1, ob_string("{")), ob_eq))))) {
            if (ob_truthy(ob_binop(">", i, litstart, ob_gt))) {
                (void)(ob_arr_push(parts, ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_expr"))), ob_bool(false)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("literal"))), ob_str_substr(raw, litstart, ob_binop("-", i, litstart, ob_sub))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("expr"))), ob_null()); __d; })));
            }
            OboeValue start = ob_binop("+", i, ob_int(2LL), ob_add);
            OboeValue depth = ob_int(1LL);
            OboeValue j = start;
            while (ob_truthy(ob_bool(ob_truthy(ob_binop("<", j, len, ob_lt)) && ob_truthy(ob_binop(">", depth, ob_int(0LL), ob_gt))))) {
                OboeValue cj = ob_str_substr(raw, j, ob_int(1LL));
                if (ob_truthy(ob_binop("==", cj, ob_interpolate(1, ob_string("{")), ob_eq))) {
                    (void)((depth = ob_binop("+", depth, ob_int(1LL), ob_add)));
                }
                else {
                    if (ob_truthy(ob_binop("==", cj, ob_interpolate(1, ob_string("}")), ob_eq))) {
                        (void)((depth = ob_binop("-", depth, ob_int(1LL), ob_sub)));
                    }
                }
                if (ob_truthy(ob_binop(">", depth, ob_int(0LL), ob_gt))) {
                    (void)((j = ob_binop("+", j, ob_int(1LL), ob_add)));
                }
            }
            OboeValue sub = ob_str_substr(raw, start, ob_binop("-", j, start, ob_sub));
            OboeValue sp = Parser__new_0(lexer__lex_all(sub), ((Parser*)((p).as.obj))->filename);
            (void)(ob_arr_push(parts, ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_expr"))), ob_bool(true)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("literal"))), ob_null()); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("expr"))), parser__parse_expression(sp)); __d; })));
            (void)((i = ob_binop("+", j, ob_int(1LL), ob_add)));
            (void)((litstart = i));
        }
        else {
            (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
        }
    }
    if (ob_truthy(ob_binop(">", i, litstart, ob_gt))) {
        (void)(ob_arr_push(parts, ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_expr"))), ob_bool(false)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("literal"))), ob_str_substr(raw, litstart, ob_binop("-", i, litstart, ob_sub))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("expr"))), ob_null()); __d; })));
    }
    return parts;
    return ob_null();
}

OboeValue parser__parse_arg_list(OboeValue p, OboeValue close, OboeValue want_names) {
    OboeValue items = ({ OboeValue __a = ob_array_new(); __a; });
    OboeValue names = ({ OboeValue __a = ob_array_new(); __a; });
    if (ob_truthy(ob_not(parser__check(p, close)))) {
        OboeValue more = ob_bool(true);
        while (ob_truthy(more)) {
            OboeValue name = ob_null();
            if (ob_truthy(ob_bool(ob_truthy(ob_bool(ob_truthy(want_names) && ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_IDENT")))))) && ob_truthy(ob_binop("==", ob_index_get(parser__peek_at(p, ob_int(1LL)), ob_interpolate(1, ob_string("type"))), ob_interpolate(1, ob_string("T_ASSIGN")), ob_eq))))) {
                (void)((name = ob_index_get(parser__advance(p), ob_interpolate(1, ob_string("text")))));
                (void)(parser__advance(p));
            }
            (void)(ob_arr_push(names, name));
            (void)(ob_arr_push(items, parser__parse_expression(p)));
            (void)((more = parser__match(p, ob_interpolate(1, ob_string("T_COMMA")))));
        }
    }
    (void)(parser__expect(p, close, ob_interpolate(1, ob_string("expected closing delimiter"))));
    return ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("items"))), items); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("names"))), names); __d; });
    return ob_null();
}

OboeValue parser__parse_primary(OboeValue p) {
    OboeValue t = parser__peek(p);
    OboeValue line = ob_index_get(t, ob_interpolate(1, ob_string("line")));
    if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_INT"))))) {
        OboeValue e = parser__new_expr(ob_interpolate(1, ob_string("EXPR_INT")), line);
        (void)(ob_index_set(e, ob_interpolate(1, ob_string("int_val")), ob_str_to_int(ob_index_get(t, ob_interpolate(1, ob_string("text"))))));
        return e;
    }
    if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_FLOAT"))))) {
        OboeValue e = parser__new_expr(ob_interpolate(1, ob_string("EXPR_FLOAT")), line);
        (void)(ob_index_set(e, ob_interpolate(1, ob_string("float_val")), ob_str_to_float(ob_index_get(t, ob_interpolate(1, ob_string("text"))))));
        return e;
    }
    if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_TRUE"))))) {
        OboeValue e = parser__new_expr(ob_interpolate(1, ob_string("EXPR_BOOL")), line);
        (void)(ob_index_set(e, ob_interpolate(1, ob_string("bool_val")), ob_bool(true)));
        return e;
    }
    if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_FALSE"))))) {
        OboeValue e = parser__new_expr(ob_interpolate(1, ob_string("EXPR_BOOL")), line);
        (void)(ob_index_set(e, ob_interpolate(1, ob_string("bool_val")), ob_bool(false)));
        return e;
    }
    if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_NULL"))))) {
        return parser__new_expr(ob_interpolate(1, ob_string("EXPR_NULL")), line);
    }
    if (ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_STRING"))))) {
        OboeValue tok = parser__advance(p);
        OboeValue e = parser__new_expr(ob_interpolate(1, ob_string("EXPR_STRING")), line);
        (void)(ob_index_set(e, ob_interpolate(1, ob_string("str_parts")), parser__parse_string_literal_parts(p, ob_index_get(tok, ob_interpolate(1, ob_string("text"))))));
        return e;
    }
    if (ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_IDENT"))))) {
        OboeValue tok = parser__advance(p);
        OboeValue e = parser__new_expr(ob_interpolate(1, ob_string("EXPR_IDENT")), line);
        (void)(ob_index_set(e, ob_interpolate(1, ob_string("ident")), ob_index_get(tok, ob_interpolate(1, ob_string("text")))));
        return e;
    }
    if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_LPAREN"))))) {
        OboeValue e = parser__parse_expression(p);
        (void)(parser__expect(p, ob_interpolate(1, ob_string("T_RPAREN")), ob_interpolate(1, ob_string("expected ')'"))));
        return e;
    }
    if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_LBRACKET"))))) {
        OboeValue got = parser__parse_arg_list(p, ob_interpolate(1, ob_string("T_RBRACKET")), ob_bool(false));
        OboeValue e = parser__new_expr(ob_interpolate(1, ob_string("EXPR_ARRAY")), line);
        (void)(ob_index_set(e, ob_interpolate(1, ob_string("items")), ob_index_get(got, ob_interpolate(1, ob_string("items")))));
        return e;
    }
    if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_LBRACE"))))) {
        OboeValue keys = ({ OboeValue __a = ob_array_new(); __a; });
        OboeValue values = ({ OboeValue __a = ob_array_new(); __a; });
        if (ob_truthy(ob_not(parser__check(p, ob_interpolate(1, ob_string("T_RBRACE")))))) {
            OboeValue more = ob_bool(true);
            while (ob_truthy(more)) {
                (void)(ob_arr_push(keys, parser__parse_expression(p)));
                (void)(parser__expect(p, ob_interpolate(1, ob_string("T_COLON")), ob_interpolate(1, ob_string("expected ':' in dict literal"))));
                (void)(ob_arr_push(values, parser__parse_expression(p)));
                (void)((more = parser__match(p, ob_interpolate(1, ob_string("T_COMMA")))));
            }
        }
        (void)(parser__expect(p, ob_interpolate(1, ob_string("T_RBRACE")), ob_interpolate(1, ob_string("expected '}'"))));
        OboeValue e = parser__new_expr(ob_interpolate(1, ob_string("EXPR_DICT")), line);
        (void)(ob_index_set(e, ob_interpolate(1, ob_string("keys")), keys));
        (void)(ob_index_set(e, ob_interpolate(1, ob_string("values")), values));
        return e;
    }
    (void)(parser__fail(p, ob_interpolate(1, ob_string("expected expression"))));
    return ob_null();
    return ob_null();
}

OboeValue parser__parse_postfix(OboeValue p) {
    OboeValue e = parser__parse_primary(p);
    while (ob_truthy(ob_bool(true))) {
        OboeValue line = ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line")));
        if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_DOT"))))) {
            OboeValue name = parser__expect(p, ob_interpolate(1, ob_string("T_IDENT")), ob_interpolate(1, ob_string("expected member name after '.'")));
            OboeValue f = parser__new_expr(ob_interpolate(1, ob_string("EXPR_FIELD")), line);
            (void)(ob_index_set(f, ob_interpolate(1, ob_string("obj")), e));
            (void)(ob_index_set(f, ob_interpolate(1, ob_string("name")), ob_index_get(name, ob_interpolate(1, ob_string("text")))));
            (void)((e = f));
        }
        else {
            if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_QDOT"))))) {
                OboeValue name = parser__expect(p, ob_interpolate(1, ob_string("T_IDENT")), ob_interpolate(1, ob_string("expected member name after '?.'")));
                OboeValue f = parser__new_expr(ob_interpolate(1, ob_string("EXPR_SAFE_FIELD")), line);
                (void)(ob_index_set(f, ob_interpolate(1, ob_string("obj")), e));
                (void)(ob_index_set(f, ob_interpolate(1, ob_string("name")), ob_index_get(name, ob_interpolate(1, ob_string("text")))));
                (void)((e = f));
            }
            else {
                if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_LPAREN"))))) {
                    OboeValue got = parser__parse_arg_list(p, ob_interpolate(1, ob_string("T_RPAREN")), ob_bool(true));
                    OboeValue c = parser__new_expr(ob_interpolate(1, ob_string("EXPR_CALL")), line);
                    (void)(ob_index_set(c, ob_interpolate(1, ob_string("callee")), e));
                    (void)(ob_index_set(c, ob_interpolate(1, ob_string("args")), ob_index_get(got, ob_interpolate(1, ob_string("items")))));
                    (void)(ob_index_set(c, ob_interpolate(1, ob_string("arg_names")), ob_index_get(got, ob_interpolate(1, ob_string("names")))));
                    (void)((e = c));
                }
                else {
                    if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_LBRACKET"))))) {
                        OboeValue idx = parser__parse_expression(p);
                        (void)(parser__expect(p, ob_interpolate(1, ob_string("T_RBRACKET")), ob_interpolate(1, ob_string("expected ']'"))));
                        OboeValue ix = parser__new_expr(ob_interpolate(1, ob_string("EXPR_INDEX")), line);
                        (void)(ob_index_set(ix, ob_interpolate(1, ob_string("arr")), e));
                        (void)(ob_index_set(ix, ob_interpolate(1, ob_string("idx")), idx));
                        (void)((e = ix));
                    }
                    else {
                        break;
                    }
                }
            }
        }
    }
    return e;
    return ob_null();
}

OboeValue parser__parse_unary(OboeValue p) {
    OboeValue line = ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line")));
    OboeValue op = ob_string("");
    if (ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_NOT"))))) {
        (void)((op = ob_interpolate(1, ob_string("!"))));
    }
    else {
        if (ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_MINUS"))))) {
            (void)((op = ob_interpolate(1, ob_string("-"))));
        }
        else {
            if (ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_TILDE"))))) {
                (void)((op = ob_interpolate(1, ob_string("~"))));
            }
        }
    }
    if (ob_truthy(ob_binop("==", op, ob_string(""), ob_eq))) {
        return parser__parse_postfix(p);
    }
    (void)(parser__advance(p));
    OboeValue e = parser__new_expr(ob_interpolate(1, ob_string("EXPR_UNARY")), line);
    (void)(ob_index_set(e, ob_interpolate(1, ob_string("op")), op));
    (void)(ob_index_set(e, ob_interpolate(1, ob_string("operand")), parser__parse_unary(p)));
    return e;
    return ob_null();
}

OboeValue parser__binary_op_here(OboeValue p, OboeValue types, OboeValue ops) {
    OboeValue i = ob_int(0LL);
    while (ob_truthy(ob_binop("<", i, ob_m_len(types), ob_lt))) {
        if (ob_truthy(parser__check(p, ob_index_get(types, i)))) {
            return ob_index_get(ops, i);
        }
        (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
    }
    return ob_string("");
    return ob_null();
}

OboeValue parser__parse_term(OboeValue p) {
    OboeValue e = parser__parse_unary(p);
    while (ob_truthy(ob_bool(true))) {
        OboeValue line = ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line")));
        OboeValue op = parser__binary_op_here(p, ({ OboeValue __a = ob_array_new(); ob_array_push(__a, ob_interpolate(1, ob_string("T_STAR"))); ob_array_push(__a, ob_interpolate(1, ob_string("T_SLASH"))); ob_array_push(__a, ob_interpolate(1, ob_string("T_PERCENT"))); __a; }), ({ OboeValue __a = ob_array_new(); ob_array_push(__a, ob_interpolate(1, ob_string("*"))); ob_array_push(__a, ob_interpolate(1, ob_string("/"))); ob_array_push(__a, ob_interpolate(1, ob_string("%"))); __a; }));
        if (ob_truthy(ob_bool(ob_truthy(ob_bool(ob_truthy(ob_binop("==", op, ob_string(""), ob_eq)) && ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_IDENT")))))) && ob_truthy(ob_binop("==", ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("text"))), ob_interpolate(1, ob_string("x")), ob_eq))))) {
            (void)((op = ob_interpolate(1, ob_string("x"))));
        }
        if (ob_truthy(ob_binop("==", op, ob_string(""), ob_eq))) {
            break;
        }
        (void)(parser__advance(p));
        OboeValue b = parser__new_expr(ob_interpolate(1, ob_string("EXPR_BINARY")), line);
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("op")), op));
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("l")), e));
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("r")), parser__parse_unary(p)));
        (void)((e = b));
    }
    return e;
    return ob_null();
}

OboeValue parser__parse_additive(OboeValue p) {
    OboeValue e = parser__parse_term(p);
    while (ob_truthy(ob_bool(true))) {
        OboeValue line = ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line")));
        OboeValue op = parser__binary_op_here(p, ({ OboeValue __a = ob_array_new(); ob_array_push(__a, ob_interpolate(1, ob_string("T_PLUS"))); ob_array_push(__a, ob_interpolate(1, ob_string("T_MINUS"))); __a; }), ({ OboeValue __a = ob_array_new(); ob_array_push(__a, ob_interpolate(1, ob_string("+"))); ob_array_push(__a, ob_interpolate(1, ob_string("-"))); __a; }));
        if (ob_truthy(ob_binop("==", op, ob_string(""), ob_eq))) {
            break;
        }
        (void)(parser__advance(p));
        OboeValue b = parser__new_expr(ob_interpolate(1, ob_string("EXPR_BINARY")), line);
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("op")), op));
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("l")), e));
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("r")), parser__parse_term(p)));
        (void)((e = b));
    }
    return e;
    return ob_null();
}

OboeValue parser__parse_shift(OboeValue p) {
    OboeValue e = parser__parse_additive(p);
    while (ob_truthy(ob_bool(true))) {
        OboeValue line = ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line")));
        OboeValue op = parser__binary_op_here(p, ({ OboeValue __a = ob_array_new(); ob_array_push(__a, ob_interpolate(1, ob_string("T_SHL"))); ob_array_push(__a, ob_interpolate(1, ob_string("T_SHR"))); __a; }), ({ OboeValue __a = ob_array_new(); ob_array_push(__a, ob_interpolate(1, ob_string("<<"))); ob_array_push(__a, ob_interpolate(1, ob_string(">>"))); __a; }));
        if (ob_truthy(ob_binop("==", op, ob_string(""), ob_eq))) {
            break;
        }
        (void)(parser__advance(p));
        OboeValue b = parser__new_expr(ob_interpolate(1, ob_string("EXPR_BINARY")), line);
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("op")), op));
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("l")), e));
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("r")), parser__parse_additive(p)));
        (void)((e = b));
    }
    return e;
    return ob_null();
}

OboeValue parser__parse_is(OboeValue p) {
    OboeValue e = parser__parse_shift(p);
    if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_IS"))))) {
        OboeValue line = ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line")));
        OboeValue name = parser__expect(p, ob_interpolate(1, ob_string("T_IDENT")), ob_interpolate(1, ob_string("expected type name after 'is'")));
        OboeValue ie = parser__new_expr(ob_interpolate(1, ob_string("EXPR_IS")), line);
        (void)(ob_index_set(ie, ob_interpolate(1, ob_string("value")), e));
        (void)(ob_index_set(ie, ob_interpolate(1, ob_string("type_name")), ob_index_get(name, ob_interpolate(1, ob_string("text")))));
        (void)((e = ie));
    }
    return e;
    return ob_null();
}

OboeValue parser__parse_comparison(OboeValue p) {
    OboeValue e = parser__parse_is(p);
    while (ob_truthy(ob_bool(true))) {
        OboeValue line = ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line")));
        OboeValue op = parser__binary_op_here(p, ({ OboeValue __a = ob_array_new(); ob_array_push(__a, ob_interpolate(1, ob_string("T_LT"))); ob_array_push(__a, ob_interpolate(1, ob_string("T_LTE"))); ob_array_push(__a, ob_interpolate(1, ob_string("T_GT"))); ob_array_push(__a, ob_interpolate(1, ob_string("T_GTE"))); __a; }), ({ OboeValue __a = ob_array_new(); ob_array_push(__a, ob_interpolate(1, ob_string("<"))); ob_array_push(__a, ob_interpolate(1, ob_string("<="))); ob_array_push(__a, ob_interpolate(1, ob_string(">"))); ob_array_push(__a, ob_interpolate(1, ob_string(">="))); __a; }));
        if (ob_truthy(ob_binop("==", op, ob_string(""), ob_eq))) {
            break;
        }
        (void)(parser__advance(p));
        OboeValue b = parser__new_expr(ob_interpolate(1, ob_string("EXPR_BINARY")), line);
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("op")), op));
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("l")), e));
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("r")), parser__parse_is(p)));
        (void)((e = b));
    }
    return e;
    return ob_null();
}

OboeValue parser__parse_equality(OboeValue p) {
    OboeValue e = parser__parse_comparison(p);
    while (ob_truthy(ob_bool(true))) {
        OboeValue line = ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line")));
        OboeValue op = parser__binary_op_here(p, ({ OboeValue __a = ob_array_new(); ob_array_push(__a, ob_interpolate(1, ob_string("T_EQ"))); ob_array_push(__a, ob_interpolate(1, ob_string("T_NEQ"))); __a; }), ({ OboeValue __a = ob_array_new(); ob_array_push(__a, ob_interpolate(1, ob_string("=="))); ob_array_push(__a, ob_interpolate(1, ob_string("!="))); __a; }));
        if (ob_truthy(ob_binop("==", op, ob_string(""), ob_eq))) {
            break;
        }
        (void)(parser__advance(p));
        OboeValue b = parser__new_expr(ob_interpolate(1, ob_string("EXPR_BINARY")), line);
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("op")), op));
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("l")), e));
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("r")), parser__parse_comparison(p)));
        (void)((e = b));
    }
    return e;
    return ob_null();
}

OboeValue parser__parse_bitand(OboeValue p) {
    OboeValue e = parser__parse_equality(p);
    while (ob_truthy(ob_bool(true))) {
        OboeValue line = ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line")));
        OboeValue op = parser__binary_op_here(p, ({ OboeValue __a = ob_array_new(); ob_array_push(__a, ob_interpolate(1, ob_string("T_AMP"))); __a; }), ({ OboeValue __a = ob_array_new(); ob_array_push(__a, ob_interpolate(1, ob_string("&"))); __a; }));
        if (ob_truthy(ob_binop("==", op, ob_string(""), ob_eq))) {
            break;
        }
        (void)(parser__advance(p));
        OboeValue b = parser__new_expr(ob_interpolate(1, ob_string("EXPR_BINARY")), line);
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("op")), op));
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("l")), e));
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("r")), parser__parse_equality(p)));
        (void)((e = b));
    }
    return e;
    return ob_null();
}

OboeValue parser__parse_bitxor(OboeValue p) {
    OboeValue e = parser__parse_bitand(p);
    while (ob_truthy(ob_bool(true))) {
        OboeValue line = ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line")));
        OboeValue op = parser__binary_op_here(p, ({ OboeValue __a = ob_array_new(); ob_array_push(__a, ob_interpolate(1, ob_string("T_CARET"))); __a; }), ({ OboeValue __a = ob_array_new(); ob_array_push(__a, ob_interpolate(1, ob_string("^"))); __a; }));
        if (ob_truthy(ob_binop("==", op, ob_string(""), ob_eq))) {
            break;
        }
        (void)(parser__advance(p));
        OboeValue b = parser__new_expr(ob_interpolate(1, ob_string("EXPR_BINARY")), line);
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("op")), op));
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("l")), e));
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("r")), parser__parse_bitand(p)));
        (void)((e = b));
    }
    return e;
    return ob_null();
}

OboeValue parser__parse_bitor(OboeValue p) {
    OboeValue e = parser__parse_bitxor(p);
    while (ob_truthy(ob_bool(true))) {
        OboeValue line = ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line")));
        OboeValue op = parser__binary_op_here(p, ({ OboeValue __a = ob_array_new(); ob_array_push(__a, ob_interpolate(1, ob_string("T_PIPE"))); __a; }), ({ OboeValue __a = ob_array_new(); ob_array_push(__a, ob_interpolate(1, ob_string("|"))); __a; }));
        if (ob_truthy(ob_binop("==", op, ob_string(""), ob_eq))) {
            break;
        }
        (void)(parser__advance(p));
        OboeValue b = parser__new_expr(ob_interpolate(1, ob_string("EXPR_BINARY")), line);
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("op")), op));
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("l")), e));
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("r")), parser__parse_bitxor(p)));
        (void)((e = b));
    }
    return e;
    return ob_null();
}

OboeValue parser__parse_custom_op(OboeValue p) {
    OboeValue e = parser__parse_bitor(p);
    while (ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_CUSTOMOP"))))) {
        OboeValue line = ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line")));
        OboeValue op = parser__advance(p);
        OboeValue b = parser__new_expr(ob_interpolate(1, ob_string("EXPR_BINARY")), line);
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("op")), ob_index_get(op, ob_interpolate(1, ob_string("text")))));
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("l")), e));
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("r")), parser__parse_bitor(p)));
        (void)((e = b));
    }
    return e;
    return ob_null();
}

OboeValue parser__parse_and(OboeValue p) {
    OboeValue e = parser__parse_custom_op(p);
    while (ob_truthy(ob_bool(ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_ANDAND")))) || ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_AND"))))))) {
        OboeValue line = ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line")));
        (void)(parser__advance(p));
        OboeValue b = parser__new_expr(ob_interpolate(1, ob_string("EXPR_BINARY")), line);
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("op")), ob_interpolate(1, ob_string("&&"))));
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("l")), e));
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("r")), parser__parse_custom_op(p)));
        (void)((e = b));
    }
    return e;
    return ob_null();
}

OboeValue parser__parse_or(OboeValue p) {
    OboeValue e = parser__parse_and(p);
    while (ob_truthy(ob_bool(ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_OROR")))) || ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_OR"))))))) {
        OboeValue line = ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line")));
        (void)(parser__advance(p));
        OboeValue b = parser__new_expr(ob_interpolate(1, ob_string("EXPR_BINARY")), line);
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("op")), ob_interpolate(1, ob_string("||"))));
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("l")), e));
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("r")), parser__parse_and(p)));
        (void)((e = b));
    }
    return e;
    return ob_null();
}

OboeValue parser__parse_nullish(OboeValue p) {
    OboeValue e = parser__parse_or(p);
    while (ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_QQ"))))) {
        OboeValue line = ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line")));
        (void)(parser__advance(p));
        OboeValue b = parser__new_expr(ob_interpolate(1, ob_string("EXPR_BINARY")), line);
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("op")), ob_interpolate(1, ob_string("??"))));
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("l")), e));
        (void)(ob_index_set(b, ob_interpolate(1, ob_string("r")), parser__parse_or(p)));
        (void)((e = b));
    }
    return e;
    return ob_null();
}

OboeValue parser__parse_ternary(OboeValue p) {
    OboeValue e = parser__parse_nullish(p);
    if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_QUESTION"))))) {
        OboeValue line = ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line")));
        OboeValue then_e = parser__parse_ternary(p);
        (void)(parser__expect(p, ob_interpolate(1, ob_string("T_COLON")), ob_interpolate(1, ob_string("expected ':' in ternary expression"))));
        OboeValue t = parser__new_expr(ob_interpolate(1, ob_string("EXPR_TERNARY")), line);
        (void)(ob_index_set(t, ob_interpolate(1, ob_string("cond")), e));
        (void)(ob_index_set(t, ob_interpolate(1, ob_string("then_e")), then_e));
        (void)(ob_index_set(t, ob_interpolate(1, ob_string("else_e")), parser__parse_ternary(p)));
        return t;
    }
    return e;
    return ob_null();
}

OboeValue parser__parse_assignment(OboeValue p) {
    OboeValue e = parser__parse_ternary(p);
    if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_ASSIGN"))))) {
        OboeValue line = ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line")));
        OboeValue a = parser__new_expr(ob_interpolate(1, ob_string("EXPR_ASSIGN")), line);
        (void)(ob_index_set(a, ob_interpolate(1, ob_string("target")), e));
        (void)(ob_index_set(a, ob_interpolate(1, ob_string("value")), parser__parse_assignment(p)));
        return a;
    }
    return e;
    return ob_null();
}

OboeValue parser__parse_expression(OboeValue p) {
    return parser__parse_assignment(p);
    return ob_null();
}

OboeValue parser__parse_typed_name(OboeValue p) {
    OboeValue first = parser__expect(p, ob_interpolate(1, ob_string("T_IDENT")), ob_interpolate(1, ob_string("expected name")));
    if (ob_truthy(ob_bool(ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_IDENT")))) && ob_truthy(ob_binop("==", ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line"))), ob_index_get(first, ob_interpolate(1, ob_string("line"))), ob_eq))))) {
        OboeValue second = parser__advance(p);
        return ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("type"))), ob_index_get(first, ob_interpolate(1, ob_string("text")))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("name"))), ob_index_get(second, ob_interpolate(1, ob_string("text")))); __d; });
    }
    return ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("type"))), ob_null()); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("name"))), ob_index_get(first, ob_interpolate(1, ob_string("text")))); __d; });
    return ob_null();
}

OboeValue parser__parse_params(OboeValue p) {
    (void)(parser__expect(p, ob_interpolate(1, ob_string("T_LPAREN")), ob_interpolate(1, ob_string("expected '('"))));
    OboeValue params = ({ OboeValue __a = ob_array_new(); __a; });
    OboeValue seen_default = ob_bool(false);
    if (ob_truthy(ob_not(parser__check(p, ob_interpolate(1, ob_string("T_RPAREN")))))) {
        OboeValue more = ob_bool(true);
        while (ob_truthy(more)) {
            (void)(parser__match(p, ob_interpolate(1, ob_string("T_LET"))));
            OboeValue tn = parser__parse_typed_name(p);
            OboeValue param = ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("type_name"))), ob_index_get(tn, ob_interpolate(1, ob_string("type")))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("name"))), ob_index_get(tn, ob_interpolate(1, ob_string("name")))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("default_value"))), ob_null()); __d; });
            if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_ASSIGN"))))) {
                (void)(ob_index_set(param, ob_interpolate(1, ob_string("default_value")), parser__parse_expression(p)));
                (void)((seen_default = ob_bool(true)));
            }
            else {
                if (ob_truthy(ob_bool(ob_truthy(seen_default) && ob_truthy(ob_binop("!=", ob_index_get(tn, ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("this")), ob_neq))))) {
                    (void)(parser__fail(p, ob_interpolate(1, ob_string("a parameter without a default cannot follow one with a default"))));
                }
            }
            (void)(ob_arr_push(params, param));
            (void)((more = parser__match(p, ob_interpolate(1, ob_string("T_COMMA")))));
        }
    }
    (void)(parser__expect(p, ob_interpolate(1, ob_string("T_RPAREN")), ob_interpolate(1, ob_string("expected ')'"))));
    return params;
    return ob_null();
}

OboeValue parser__parse_let(OboeValue p, OboeValue is_const) {
    OboeValue line = ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line")));
    (void)(parser__match(p, ob_interpolate(1, ob_string("T_LET"))));
    OboeValue tn = parser__parse_typed_name(p);
    OboeValue init = ob_null();
    if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_ASSIGN"))))) {
        (void)((init = parser__parse_expression(p)));
    }
    (void)(parser__match(p, ob_interpolate(1, ob_string("T_SEMI"))));
    OboeValue s = parser__new_stmt(ob_interpolate(1, ob_string("STMT_LET")), line);
    (void)(ob_index_set(s, ob_interpolate(1, ob_string("name")), ob_index_get(tn, ob_interpolate(1, ob_string("name")))));
    (void)(ob_index_set(s, ob_interpolate(1, ob_string("type_name")), ob_index_get(tn, ob_interpolate(1, ob_string("type")))));
    (void)(ob_index_set(s, ob_interpolate(1, ob_string("is_const")), is_const));
    (void)(ob_index_set(s, ob_interpolate(1, ob_string("init")), init));
    return s;
    return ob_null();
}

OboeValue parser__parse_if(OboeValue p) {
    OboeValue line = ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line")));
    (void)(parser__expect(p, ob_interpolate(1, ob_string("T_LPAREN")), ob_interpolate(1, ob_string("expected '(' after if"))));
    OboeValue cond = parser__parse_expression(p);
    (void)(parser__expect(p, ob_interpolate(1, ob_string("T_RPAREN")), ob_interpolate(1, ob_string("expected ')'"))));
    OboeValue then_body = parser__parse_block(p);
    OboeValue else_body = ({ OboeValue __a = ob_array_new(); __a; });
    if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_ELSE"))))) {
        if (ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_IF"))))) {
            (void)(parser__advance(p));
            (void)((else_body = ({ OboeValue __a = ob_array_new(); ob_array_push(__a, parser__parse_if(p)); __a; })));
        }
        else {
            (void)((else_body = parser__parse_block(p)));
        }
    }
    OboeValue s = parser__new_stmt(ob_interpolate(1, ob_string("STMT_IF")), line);
    (void)(ob_index_set(s, ob_interpolate(1, ob_string("cond")), cond));
    (void)(ob_index_set(s, ob_interpolate(1, ob_string("then_body")), then_body));
    (void)(ob_index_set(s, ob_interpolate(1, ob_string("else_body")), else_body));
    return s;
    return ob_null();
}

OboeValue parser__parse_while(OboeValue p) {
    OboeValue line = ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line")));
    (void)(parser__expect(p, ob_interpolate(1, ob_string("T_LPAREN")), ob_interpolate(1, ob_string("expected '(' after while"))));
    OboeValue cond = parser__parse_expression(p);
    (void)(parser__expect(p, ob_interpolate(1, ob_string("T_RPAREN")), ob_interpolate(1, ob_string("expected ')'"))));
    OboeValue s = parser__new_stmt(ob_interpolate(1, ob_string("STMT_WHILE")), line);
    (void)(ob_index_set(s, ob_interpolate(1, ob_string("cond")), cond));
    (void)(ob_index_set(s, ob_interpolate(1, ob_string("body")), parser__parse_block(p)));
    return s;
    return ob_null();
}

OboeValue parser__looks_like_call_to(OboeValue p, OboeValue name) {
    return ob_bool(ob_truthy(ob_bool(ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_IDENT")))) && ob_truthy(ob_binop("==", ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("text"))), name, ob_eq)))) && ob_truthy(ob_binop("==", ob_index_get(parser__peek_at(p, ob_int(1LL)), ob_interpolate(1, ob_string("type"))), ob_interpolate(1, ob_string("T_LPAREN")), ob_eq)));
    return ob_null();
}

OboeValue parser__parse_for(OboeValue p) {
    OboeValue line = ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line")));
    (void)(parser__expect(p, ob_interpolate(1, ob_string("T_LPAREN")), ob_interpolate(1, ob_string("expected '(' after for"))));
    OboeValue loop_var = parser__expect(p, ob_interpolate(1, ob_string("T_IDENT")), ob_interpolate(1, ob_string("expected loop variable")));
    OboeValue s = parser__new_stmt(ob_interpolate(1, ob_string("STMT_FOR")), line);
    (void)(ob_index_set(s, ob_interpolate(1, ob_string("var_name")), ob_index_get(loop_var, ob_interpolate(1, ob_string("text")))));
    (void)(ob_index_set(s, ob_interpolate(1, ob_string("var2_name")), ob_null()));
    (void)(ob_index_set(s, ob_interpolate(1, ob_string("range_a")), ob_null()));
    (void)(ob_index_set(s, ob_interpolate(1, ob_string("range_b")), ob_null()));
    (void)(ob_index_set(s, ob_interpolate(1, ob_string("iterable")), ob_null()));
    if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_COMMA"))))) {
        OboeValue var2 = parser__expect(p, ob_interpolate(1, ob_string("T_IDENT")), ob_interpolate(1, ob_string("expected second loop variable")));
        (void)(ob_index_set(s, ob_interpolate(1, ob_string("var2_name")), ob_index_get(var2, ob_interpolate(1, ob_string("text")))));
    }
    (void)(parser__expect(p, ob_interpolate(1, ob_string("T_IN")), ob_interpolate(1, ob_string("expected 'in' in for loop"))));
    if (ob_truthy(parser__looks_like_call_to(p, ob_interpolate(1, ob_string("range"))))) {
        (void)(parser__advance(p));
        (void)(parser__expect(p, ob_interpolate(1, ob_string("T_LPAREN")), ob_interpolate(1, ob_string("expected '(' after range"))));
        (void)(ob_index_set(s, ob_interpolate(1, ob_string("kind_iter")), ob_interpolate(1, ob_string("FOR_RANGE"))));
        (void)(ob_index_set(s, ob_interpolate(1, ob_string("range_a")), parser__parse_expression(p)));
        (void)(parser__expect(p, ob_interpolate(1, ob_string("T_COMMA")), ob_interpolate(1, ob_string("expected ',' in range()"))));
        (void)(ob_index_set(s, ob_interpolate(1, ob_string("range_b")), parser__parse_expression(p)));
        (void)(parser__expect(p, ob_interpolate(1, ob_string("T_RPAREN")), ob_interpolate(1, ob_string("expected ')'"))));
    }
    else {
        if (ob_truthy(ob_bool(ob_truthy(parser__looks_like_call_to(p, ob_interpolate(1, ob_string("pairs")))) || ob_truthy(parser__looks_like_call_to(p, ob_interpolate(1, ob_string("ipairs"))))))) {
            OboeValue indexed = ob_binop("==", ob_str_substr(ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("text"))), ob_int(0LL), ob_int(1LL)), ob_interpolate(1, ob_string("i")), ob_eq);
            (void)(parser__advance(p));
            (void)(parser__expect(p, ob_interpolate(1, ob_string("T_LPAREN")), ob_interpolate(1, ob_string("expected '('"))));
            (void)(ob_index_set(s, ob_interpolate(1, ob_string("kind_iter")), (ob_truthy(indexed) ? (ob_interpolate(1, ob_string("FOR_IPAIRS"))) : (ob_interpolate(1, ob_string("FOR_PAIRS"))))));
            (void)(ob_index_set(s, ob_interpolate(1, ob_string("iterable")), parser__parse_expression(p)));
            (void)(parser__expect(p, ob_interpolate(1, ob_string("T_RPAREN")), ob_interpolate(1, ob_string("expected ')'"))));
        }
        else {
            (void)(ob_index_set(s, ob_interpolate(1, ob_string("kind_iter")), ob_interpolate(1, ob_string("FOR_ITER"))));
            (void)(ob_index_set(s, ob_interpolate(1, ob_string("iterable")), parser__parse_expression(p)));
        }
    }
    OboeValue is_pairs = ob_bool(ob_truthy(ob_binop("==", ob_index_get(s, ob_interpolate(1, ob_string("kind_iter"))), ob_interpolate(1, ob_string("FOR_PAIRS")), ob_eq)) || ob_truthy(ob_binop("==", ob_index_get(s, ob_interpolate(1, ob_string("kind_iter"))), ob_interpolate(1, ob_string("FOR_IPAIRS")), ob_eq)));
    if (ob_truthy(ob_bool(ob_truthy(is_pairs) && ob_truthy(ob_binop("==", ob_index_get(s, ob_interpolate(1, ob_string("var2_name"))), ob_null(), ob_eq))))) {
        (void)(parser__fail(p, ob_interpolate(1, ob_string("pairs()/ipairs() need two loop variables, as in `for (k, v in pairs(d))`"))));
    }
    if (ob_truthy(ob_bool(ob_truthy(ob_not(is_pairs)) && ob_truthy(ob_binop("!=", ob_index_get(s, ob_interpolate(1, ob_string("var2_name"))), ob_null(), ob_neq))))) {
        (void)(parser__fail(p, ob_interpolate(1, ob_string("two loop variables require pairs() or ipairs()"))));
    }
    (void)(parser__expect(p, ob_interpolate(1, ob_string("T_RPAREN")), ob_interpolate(1, ob_string("expected ')'"))));
    (void)(ob_index_set(s, ob_interpolate(1, ob_string("body")), parser__parse_block(p)));
    return s;
    return ob_null();
}

OboeValue parser__parse_switch(OboeValue p) {
    OboeValue line = ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line")));
    OboeValue subject = parser__parse_expression(p);
    (void)(parser__expect(p, ob_interpolate(1, ob_string("T_LBRACE")), ob_interpolate(1, ob_string("expected '{' after switch subject"))));
    OboeValue cases = ({ OboeValue __a = ob_array_new(); __a; });
    while (ob_truthy(ob_bool(ob_truthy(ob_not(parser__check(p, ob_interpolate(1, ob_string("T_RBRACE"))))) && ob_truthy(ob_not(parser__check(p, ob_interpolate(1, ob_string("T_EOF")))))))) {
        (void)(parser__expect(p, ob_interpolate(1, ob_string("T_CASE")), ob_interpolate(1, ob_string("expected 'case'"))));
        OboeValue val = parser__parse_expression(p);
        (void)(ob_arr_push(cases, ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("value"))), val); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("body"))), parser__parse_block(p)); __d; })));
    }
    (void)(parser__expect(p, ob_interpolate(1, ob_string("T_RBRACE")), ob_interpolate(1, ob_string("expected '}' to close switch"))));
    OboeValue s = parser__new_stmt(ob_interpolate(1, ob_string("STMT_SWITCH")), line);
    (void)(ob_index_set(s, ob_interpolate(1, ob_string("subject")), subject));
    (void)(ob_index_set(s, ob_interpolate(1, ob_string("cases")), cases));
    return s;
    return ob_null();
}

OboeValue parser__parse_try(OboeValue p) {
    OboeValue line = ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line")));
    OboeValue body = parser__parse_block(p);
    OboeValue catches = ({ OboeValue __a = ob_array_new(); __a; });
    while (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_CATCH"))))) {
        (void)(parser__expect(p, ob_interpolate(1, ob_string("T_LPAREN")), ob_interpolate(1, ob_string("expected '(' after catch"))));
        OboeValue type_name = parser__expect(p, ob_interpolate(1, ob_string("T_IDENT")), ob_interpolate(1, ob_string("expected exception type")));
        OboeValue full_type = ob_index_get(type_name, ob_interpolate(1, ob_string("text")));
        while (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_DOT"))))) {
            OboeValue more = parser__expect(p, ob_interpolate(1, ob_string("T_IDENT")), ob_interpolate(1, ob_string("expected identifier after '.'")));
            (void)((full_type = ob_binop("+", ob_binop("+", full_type, ob_interpolate(1, ob_string(".")), ob_add), ob_index_get(more, ob_interpolate(1, ob_string("text"))), ob_add)));
        }
        OboeValue var_name = parser__expect(p, ob_interpolate(1, ob_string("T_IDENT")), ob_interpolate(1, ob_string("expected exception variable name")));
        (void)(parser__expect(p, ob_interpolate(1, ob_string("T_RPAREN")), ob_interpolate(1, ob_string("expected ')'"))));
        (void)(ob_arr_push(catches, ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("type_name"))), full_type); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("var_name"))), ob_index_get(var_name, ob_interpolate(1, ob_string("text")))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("body"))), parser__parse_block(p)); __d; })));
    }
    OboeValue finally_body = ({ OboeValue __a = ob_array_new(); __a; });
    if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_FINALLY"))))) {
        (void)((finally_body = parser__parse_block(p)));
    }
    OboeValue s = parser__new_stmt(ob_interpolate(1, ob_string("STMT_TRY")), line);
    (void)(ob_index_set(s, ob_interpolate(1, ob_string("body")), body));
    (void)(ob_index_set(s, ob_interpolate(1, ob_string("catches")), catches));
    (void)(ob_index_set(s, ob_interpolate(1, ob_string("finally_body")), finally_body));
    return s;
    return ob_null();
}

OboeValue parser__parse_throw(OboeValue p) {
    OboeValue line = ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line")));
    OboeValue type_name = parser__expect(p, ob_interpolate(1, ob_string("T_IDENT")), ob_interpolate(1, ob_string("expected exception type after throw")));
    OboeValue full_type = ob_index_get(type_name, ob_interpolate(1, ob_string("text")));
    while (ob_truthy(ob_bool(ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_DOT")))) && ob_truthy(ob_binop("==", ob_index_get(parser__peek_at(p, ob_int(1LL)), ob_interpolate(1, ob_string("type"))), ob_interpolate(1, ob_string("T_IDENT")), ob_eq))))) {
        (void)(parser__advance(p));
        (void)((full_type = ob_binop("+", ob_binop("+", full_type, ob_interpolate(1, ob_string(".")), ob_add), ob_index_get(parser__advance(p), ob_interpolate(1, ob_string("text"))), ob_add)));
    }
    OboeValue value = ob_null();
    if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_LPAREN"))))) {
        if (ob_truthy(ob_not(parser__check(p, ob_interpolate(1, ob_string("T_RPAREN")))))) {
            (void)((value = parser__parse_expression(p)));
        }
        (void)(parser__expect(p, ob_interpolate(1, ob_string("T_RPAREN")), ob_interpolate(1, ob_string("expected ')'"))));
    }
    (void)(parser__match(p, ob_interpolate(1, ob_string("T_SEMI"))));
    OboeValue s = parser__new_stmt(ob_interpolate(1, ob_string("STMT_THROW")), line);
    (void)(ob_index_set(s, ob_interpolate(1, ob_string("type_name")), full_type));
    (void)(ob_index_set(s, ob_interpolate(1, ob_string("value")), value));
    return s;
    return ob_null();
}

OboeValue parser__parse_statement(OboeValue p) {
    OboeValue line = ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line")));
    if (ob_truthy(ob_bool(ob_truthy(ob_bool(ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_LET")))) && ob_truthy(ob_binop("==", ob_index_get(parser__peek_at(p, ob_int(1LL)), ob_interpolate(1, ob_string("type"))), ob_interpolate(1, ob_string("T_IDENT")), ob_eq)))) && ob_truthy(ob_binop("==", ob_index_get(parser__peek_at(p, ob_int(2LL)), ob_interpolate(1, ob_string("type"))), ob_interpolate(1, ob_string("T_DOT")), ob_eq))))) {
        (void)(parser__advance(p));
        OboeValue e = parser__parse_expression(p);
        (void)(parser__match(p, ob_interpolate(1, ob_string("T_SEMI"))));
        OboeValue s = parser__new_stmt(ob_interpolate(1, ob_string("STMT_EXPR")), line);
        (void)(ob_index_set(s, ob_interpolate(1, ob_string("expr")), e));
        return s;
    }
    if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_LET"))))) {
        return parser__parse_let(p, ob_bool(false));
    }
    if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_CONST"))))) {
        return parser__parse_let(p, ob_bool(true));
    }
    if (ob_truthy(ob_bool(ob_truthy(ob_bool(ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_IDENT")))) && ob_truthy(ob_binop("==", ob_index_get(parser__peek_at(p, ob_int(1LL)), ob_interpolate(1, ob_string("type"))), ob_interpolate(1, ob_string("T_IDENT")), ob_eq)))) && ob_truthy(ob_binop("==", ob_index_get(parser__peek_at(p, ob_int(2LL)), ob_interpolate(1, ob_string("type"))), ob_interpolate(1, ob_string("T_ASSIGN")), ob_eq))))) {
        return parser__parse_let(p, ob_bool(false));
    }
    if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_RETURN"))))) {
        OboeValue value = ob_null();
        if (ob_truthy(ob_bool(ob_truthy(ob_not(parser__check(p, ob_interpolate(1, ob_string("T_RBRACE"))))) && ob_truthy(ob_not(parser__check(p, ob_interpolate(1, ob_string("T_SEMI")))))))) {
            (void)((value = parser__parse_expression(p)));
        }
        (void)(parser__match(p, ob_interpolate(1, ob_string("T_SEMI"))));
        OboeValue s = parser__new_stmt(ob_interpolate(1, ob_string("STMT_RETURN")), line);
        (void)(ob_index_set(s, ob_interpolate(1, ob_string("value")), value));
        return s;
    }
    if (ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_IF"))))) {
        (void)(parser__advance(p));
        return parser__parse_if(p);
    }
    if (ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_WHILE"))))) {
        (void)(parser__advance(p));
        return parser__parse_while(p);
    }
    if (ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_FOR"))))) {
        (void)(parser__advance(p));
        return parser__parse_for(p);
    }
    if (ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_SWITCH"))))) {
        (void)(parser__advance(p));
        return parser__parse_switch(p);
    }
    if (ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_TRY"))))) {
        (void)(parser__advance(p));
        return parser__parse_try(p);
    }
    if (ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_THROW"))))) {
        (void)(parser__advance(p));
        return parser__parse_throw(p);
    }
    if (ob_truthy(ob_bool(ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_BREAK")))) || ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_CONTINUE"))))))) {
        OboeValue k = (ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_BREAK")))) ? (ob_interpolate(1, ob_string("STMT_BREAK"))) : (ob_interpolate(1, ob_string("STMT_CONTINUE"))));
        (void)(parser__advance(p));
        (void)(parser__match(p, ob_interpolate(1, ob_string("T_SEMI"))));
        return parser__new_stmt(k, line);
    }
    if (ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_LBRACE"))))) {
        OboeValue s = parser__new_stmt(ob_interpolate(1, ob_string("STMT_BLOCK")), line);
        (void)(ob_index_set(s, ob_interpolate(1, ob_string("body")), parser__parse_block(p)));
        return s;
    }
    OboeValue e = parser__parse_expression(p);
    (void)(parser__match(p, ob_interpolate(1, ob_string("T_SEMI"))));
    OboeValue s = parser__new_stmt(ob_interpolate(1, ob_string("STMT_EXPR")), line);
    (void)(ob_index_set(s, ob_interpolate(1, ob_string("expr")), e));
    return s;
    return ob_null();
}

OboeValue parser__parse_block(OboeValue p) {
    (void)(parser__expect(p, ob_interpolate(1, ob_string("T_LBRACE")), ob_interpolate(1, ob_string("expected '{'"))));
    OboeValue items = ({ OboeValue __a = ob_array_new(); __a; });
    while (ob_truthy(ob_bool(ob_truthy(ob_not(parser__check(p, ob_interpolate(1, ob_string("T_RBRACE"))))) && ob_truthy(ob_not(parser__check(p, ob_interpolate(1, ob_string("T_EOF")))))))) {
        (void)(ob_arr_push(items, parser__parse_statement(p)));
    }
    (void)(parser__expect(p, ob_interpolate(1, ob_string("T_RBRACE")), ob_interpolate(1, ob_string("expected '}'"))));
    return items;
    return ob_null();
}

OboeValue parser__parse_func(OboeValue p, OboeValue is_static, OboeValue is_private) {
    OboeValue line = ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line")));
    (void)(parser__expect(p, ob_interpolate(1, ob_string("T_FUNC")), ob_interpolate(1, ob_string("expected 'func'"))));
    OboeValue name = parser__expect(p, ob_interpolate(1, ob_string("T_IDENT")), ob_interpolate(1, ob_string("expected function name")));
    OboeValue params = parser__parse_params(p);
    return ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("name"))), ob_index_get(name, ob_interpolate(1, ob_string("text")))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("return_type"))), ob_null()); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("params"))), params); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_static"))), is_static); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_private"))), is_private); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("op_symbol"))), ob_null()); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("body"))), parser__parse_block(p)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("line"))), line); __d; });
    return ob_null();
}

OboeValue parser__parse_operator_decl(OboeValue p) {
    OboeValue line = ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line")));
    OboeValue sym = parser__advance(p);
    if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", ob_index_get(sym, ob_interpolate(1, ob_string("text"))), ob_string(""), ob_eq)) || ob_truthy(ob_binop("==", ob_index_get(sym, ob_interpolate(1, ob_string("type"))), ob_interpolate(1, ob_string("T_LPAREN")), ob_eq))))) {
        (void)(parser__fail(p, ob_interpolate(1, ob_string("expected an operator symbol after 'operator'"))));
    }
    OboeValue params = parser__parse_params(p);
    return ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("name"))), ob_interpolate(1, ob_string("operator"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("return_type"))), ob_null()); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("params"))), params); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_static"))), ob_bool(false)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_private"))), ob_bool(false)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("op_symbol"))), ob_index_get(sym, ob_interpolate(1, ob_string("text")))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("body"))), parser__parse_block(p)); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("line"))), line); __d; });
    return ob_null();
}

OboeValue parser__parse_class(OboeValue p) {
    OboeValue line = ob_index_get(parser__peek(p), ob_interpolate(1, ob_string("line")));
    OboeValue name = parser__expect(p, ob_interpolate(1, ob_string("T_IDENT")), ob_interpolate(1, ob_string("expected class name")));
    OboeValue parent_name = ob_null();
    if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_EXTENDS"))))) {
        OboeValue pn = parser__expect(p, ob_interpolate(1, ob_string("T_IDENT")), ob_interpolate(1, ob_string("expected parent class name")));
        (void)((parent_name = ob_index_get(pn, ob_interpolate(1, ob_string("text")))));
    }
    (void)(parser__expect(p, ob_interpolate(1, ob_string("T_LBRACE")), ob_interpolate(1, ob_string("expected '{' to start class body"))));
    OboeValue fields = ({ OboeValue __a = ob_array_new(); __a; });
    OboeValue methods = ({ OboeValue __a = ob_array_new(); __a; });
    while (ob_truthy(ob_bool(ob_truthy(ob_not(parser__check(p, ob_interpolate(1, ob_string("T_RBRACE"))))) && ob_truthy(ob_not(parser__check(p, ob_interpolate(1, ob_string("T_EOF")))))))) {
        OboeValue is_static = ob_bool(false);
        OboeValue is_private = ob_bool(false);
        while (ob_truthy(ob_bool(ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_STATIC")))) || ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_PRIVATE"))))))) {
            if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_STATIC"))))) {
                (void)((is_static = ob_bool(true)));
            }
            else {
                if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_PRIVATE"))))) {
                    (void)((is_private = ob_bool(true)));
                }
            }
        }
        OboeValue mret = ob_null();
        if (ob_truthy(ob_bool(ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_IDENT")))) && ob_truthy(ob_binop("==", ob_index_get(parser__peek_at(p, ob_int(1LL)), ob_interpolate(1, ob_string("type"))), ob_interpolate(1, ob_string("T_FUNC")), ob_eq))))) {
            (void)((mret = ob_index_get(parser__advance(p), ob_interpolate(1, ob_string("text")))));
        }
        if (ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_FUNC"))))) {
            OboeValue m = parser__parse_func(p, is_static, is_private);
            (void)(ob_index_set(m, ob_interpolate(1, ob_string("return_type")), mret));
            (void)(ob_arr_push(methods, m));
        }
        else {
            if (ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_OPERATOR"))))) {
                (void)(parser__advance(p));
                (void)(ob_arr_push(methods, parser__parse_operator_decl(p)));
            }
            else {
                if (ob_truthy(ob_bool(ob_truthy(ob_bool(ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_CONST")))) || ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_LET")))))) || ob_truthy(ob_bool(ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_IDENT")))) && ob_truthy(ob_binop("==", ob_index_get(parser__peek_at(p, ob_int(1LL)), ob_interpolate(1, ob_string("type"))), ob_interpolate(1, ob_string("T_IDENT")), ob_eq))))))) {
                    OboeValue is_const = parser__match(p, ob_interpolate(1, ob_string("T_CONST")));
                    (void)(parser__match(p, ob_interpolate(1, ob_string("T_LET"))));
                    OboeValue tn = parser__parse_typed_name(p);
                    OboeValue init = ob_null();
                    if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_ASSIGN"))))) {
                        (void)((init = parser__parse_expression(p)));
                    }
                    (void)(parser__match(p, ob_interpolate(1, ob_string("T_SEMI"))));
                    (void)(ob_arr_push(fields, ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("type_name"))), ob_index_get(tn, ob_interpolate(1, ob_string("type")))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("name"))), ob_index_get(tn, ob_interpolate(1, ob_string("name")))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_static"))), is_static); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_private"))), is_private); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is_const"))), is_const); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("init"))), init); __d; })));
                }
                else {
                    (void)(parser__fail(p, ob_interpolate(1, ob_string("expected field or method declaration in class body"))));
                }
            }
        }
    }
    (void)(parser__expect(p, ob_interpolate(1, ob_string("T_RBRACE")), ob_interpolate(1, ob_string("expected '}' to close class"))));
    return ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("name"))), ob_index_get(name, ob_interpolate(1, ob_string("text")))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("parent_name"))), parent_name); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("fields"))), fields); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("methods"))), methods); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("line"))), line); __d; });
    return ob_null();
}

OboeValue parser__parse_import(OboeValue p) {
    OboeValue is_member_list = ob_bool(false);
    if (ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_IDENT"))))) {
        OboeValue k = ob_int(1LL);
        while (ob_truthy(ob_bool(ob_truthy(ob_binop("==", ob_index_get(parser__peek_at(p, k), ob_interpolate(1, ob_string("type"))), ob_interpolate(1, ob_string("T_COMMA")), ob_eq)) && ob_truthy(ob_binop("==", ob_index_get(parser__peek_at(p, ob_binop("+", k, ob_int(1LL), ob_add)), ob_interpolate(1, ob_string("type"))), ob_interpolate(1, ob_string("T_IDENT")), ob_eq))))) {
            (void)((k = ob_binop("+", k, ob_int(2LL), ob_add)));
        }
        (void)((is_member_list = ob_binop("==", ob_index_get(parser__peek_at(p, k), ob_interpolate(1, ob_string("type"))), ob_interpolate(1, ob_string("T_FROM")), ob_eq)));
    }
    OboeValue imp = ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("module"))), ob_null()); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("alias"))), ob_null()); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("members"))), ({ OboeValue __a = ob_array_new(); __a; })); __d; });
    if (ob_truthy(is_member_list)) {
        OboeValue more = ob_bool(true);
        while (ob_truthy(more)) {
            OboeValue m = parser__expect(p, ob_interpolate(1, ob_string("T_IDENT")), ob_interpolate(1, ob_string("expected imported member name")));
            (void)(ob_arr_push(ob_index_get(imp, ob_interpolate(1, ob_string("members"))), ob_index_get(m, ob_interpolate(1, ob_string("text")))));
            (void)((more = parser__match(p, ob_interpolate(1, ob_string("T_COMMA")))));
        }
        (void)(parser__expect(p, ob_interpolate(1, ob_string("T_FROM")), ob_interpolate(1, ob_string("expected 'from'"))));
        (void)(ob_index_set(imp, ob_interpolate(1, ob_string("module")), ob_index_get(parser__expect(p, ob_interpolate(1, ob_string("T_IDENT")), ob_interpolate(1, ob_string("expected module name"))), ob_interpolate(1, ob_string("text")))));
    }
    else {
        (void)(ob_index_set(imp, ob_interpolate(1, ob_string("module")), ob_index_get(parser__expect(p, ob_interpolate(1, ob_string("T_IDENT")), ob_interpolate(1, ob_string("expected module name"))), ob_interpolate(1, ob_string("text")))));
        if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_AS"))))) {
            OboeValue alias = parser__expect(p, ob_interpolate(1, ob_string("T_IDENT")), ob_interpolate(1, ob_string("expected alias after 'as'")));
            (void)(ob_index_set(imp, ob_interpolate(1, ob_string("alias")), ob_index_get(alias, ob_interpolate(1, ob_string("text")))));
        }
    }
    (void)(parser__match(p, ob_interpolate(1, ob_string("T_SEMI"))));
    return imp;
    return ob_null();
}

OboeValue parser__parse_program(OboeValue tokens, OboeValue filename) {
    OboeValue p = Parser__new_0(tokens, filename);
    OboeValue decls = ({ OboeValue __a = ob_array_new(); __a; });
    while (ob_truthy(ob_not(parser__check(p, ob_interpolate(1, ob_string("T_EOF")))))) {
        OboeValue d = ({ OboeValue __d = ob_dict_new(); __d; });
        if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_IMPORT"))))) {
            (void)((d = parser__parse_import(p)));
            (void)(ob_index_set(d, ob_interpolate(1, ob_string("kind")), ob_interpolate(1, ob_string("DECL_IMPORT"))));
        }
        else {
            if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_CIMPORT"))))) {
                OboeValue name = parser__expect(p, ob_interpolate(1, ob_string("T_IDENT")), ob_interpolate(1, ob_string("expected C symbol name after cimport")));
                (void)(ob_index_set(d, ob_interpolate(1, ob_string("kind")), ob_interpolate(1, ob_string("DECL_CIMPORT"))));
                (void)(ob_index_set(d, ob_interpolate(1, ob_string("name")), ob_index_get(name, ob_interpolate(1, ob_string("text")))));
                (void)(ob_index_set(d, ob_interpolate(1, ob_string("line")), ob_index_get(name, ob_interpolate(1, ob_string("line")))));
                (void)(parser__expect(p, ob_interpolate(1, ob_string("T_FROM")), ob_interpolate(1, ob_string("expected 'from' in cimport"))));
                OboeValue lib = parser__expect(p, ob_interpolate(1, ob_string("T_STRING")), ob_interpolate(1, ob_string("expected a library path string in cimport")));
                (void)(ob_index_set(d, ob_interpolate(1, ob_string("lib")), ob_index_get(lib, ob_interpolate(1, ob_string("text")))));
                (void)(parser__match(p, ob_interpolate(1, ob_string("T_SEMI"))));
            }
            else {
                if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_OPERATOR"))))) {
                    (void)(ob_index_set(d, ob_interpolate(1, ob_string("kind")), ob_interpolate(1, ob_string("DECL_OPERATOR"))));
                    (void)(ob_index_set(d, ob_interpolate(1, ob_string("func")), parser__parse_operator_decl(p)));
                }
                else {
                    if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_EVENT"))))) {
                        OboeValue name = parser__expect(p, ob_interpolate(1, ob_string("T_IDENT")), ob_interpolate(1, ob_string("expected event name")));
                        (void)(ob_index_set(d, ob_interpolate(1, ob_string("kind")), ob_interpolate(1, ob_string("DECL_EVENT"))));
                        (void)(ob_index_set(d, ob_interpolate(1, ob_string("name")), ob_index_get(name, ob_interpolate(1, ob_string("text")))));
                        (void)(ob_index_set(d, ob_interpolate(1, ob_string("line")), ob_index_get(name, ob_interpolate(1, ob_string("line")))));
                        (void)(parser__expect(p, ob_interpolate(1, ob_string("T_ASSIGN")), ob_interpolate(1, ob_string("expected '=' in event declaration"))));
                        (void)(parser__expect(p, ob_interpolate(1, ob_string("T_EVENT")), ob_interpolate(1, ob_string("expected event(...) constructor"))));
                        (void)(ob_index_set(d, ob_interpolate(1, ob_string("params")), parser__parse_params(p)));
                        (void)(parser__match(p, ob_interpolate(1, ob_string("T_SEMI"))));
                    }
                    else {
                        if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_ON"))))) {
                            OboeValue ev = parser__expect(p, ob_interpolate(1, ob_string("T_IDENT")), ob_interpolate(1, ob_string("expected event name after 'on'")));
                            (void)(ob_index_set(d, ob_interpolate(1, ob_string("kind")), ob_interpolate(1, ob_string("DECL_ON"))));
                            (void)(ob_index_set(d, ob_interpolate(1, ob_string("event_module")), ob_null()));
                            if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_DOT"))))) {
                                (void)(ob_index_set(d, ob_interpolate(1, ob_string("event_module")), ob_index_get(ev, ob_interpolate(1, ob_string("text")))));
                                (void)((ev = parser__expect(p, ob_interpolate(1, ob_string("T_IDENT")), ob_interpolate(1, ob_string("expected event name after '.'")))));
                            }
                            (void)(ob_index_set(d, ob_interpolate(1, ob_string("event_name")), ob_index_get(ev, ob_interpolate(1, ob_string("text")))));
                            (void)(ob_index_set(d, ob_interpolate(1, ob_string("line")), ob_index_get(ev, ob_interpolate(1, ob_string("line")))));
                            (void)(ob_index_set(d, ob_interpolate(1, ob_string("var_name")), ob_null()));
                            if (ob_truthy(parser__match(p, ob_interpolate(1, ob_string("T_AS"))))) {
                                OboeValue v = parser__expect(p, ob_interpolate(1, ob_string("T_IDENT")), ob_interpolate(1, ob_string("expected variable name after 'as'")));
                                (void)(ob_index_set(d, ob_interpolate(1, ob_string("var_name")), ob_index_get(v, ob_interpolate(1, ob_string("text")))));
                            }
                            (void)(ob_index_set(d, ob_interpolate(1, ob_string("body")), parser__parse_block(p)));
                        }
                        else {
                            if (ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_CLASS"))))) {
                                (void)(parser__advance(p));
                                (void)(ob_index_set(d, ob_interpolate(1, ob_string("kind")), ob_interpolate(1, ob_string("DECL_CLASS"))));
                                (void)(ob_index_set(d, ob_interpolate(1, ob_string("klass")), parser__parse_class(p)));
                            }
                            else {
                                if (ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_FUNC"))))) {
                                    (void)(ob_index_set(d, ob_interpolate(1, ob_string("kind")), ob_interpolate(1, ob_string("DECL_FUNC"))));
                                    (void)(ob_index_set(d, ob_interpolate(1, ob_string("func")), parser__parse_func(p, ob_bool(false), ob_bool(false))));
                                }
                                else {
                                    if (ob_truthy(ob_bool(ob_truthy(parser__check(p, ob_interpolate(1, ob_string("T_IDENT")))) && ob_truthy(ob_binop("==", ob_index_get(parser__peek_at(p, ob_int(1LL)), ob_interpolate(1, ob_string("type"))), ob_interpolate(1, ob_string("T_FUNC")), ob_eq))))) {
                                        OboeValue ret = parser__advance(p);
                                        (void)(ob_index_set(d, ob_interpolate(1, ob_string("kind")), ob_interpolate(1, ob_string("DECL_FUNC"))));
                                        (void)(ob_index_set(d, ob_interpolate(1, ob_string("func")), parser__parse_func(p, ob_bool(false), ob_bool(false))));
                                        (void)(ob_index_set(ob_index_get(d, ob_interpolate(1, ob_string("func"))), ob_interpolate(1, ob_string("return_type")), ob_index_get(ret, ob_interpolate(1, ob_string("text")))));
                                    }
                                    else {
                                        (void)(ob_index_set(d, ob_interpolate(1, ob_string("kind")), ob_interpolate(1, ob_string("DECL_STMT"))));
                                        (void)(ob_index_set(d, ob_interpolate(1, ob_string("stmt")), parser__parse_statement(p)));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        (void)(ob_arr_push(decls, d));
    }
    return decls;
    return ob_null();
}

static void __oboe_toplevel_3(void) {
}

OboeValue diag__fail(OboeValue msg) {
    (void)((ob_eprint(ob_binop("+", ob_interpolate(1, ob_string("oboe: ")), msg, ob_add)), ob_null()));
    (void)(ob_std_os_exit(ob_int(1LL)));
    return ob_null();
}

OboeValue diag__die(OboeValue msg) {
    (void)((ob_eprint(msg), ob_null()));
    (void)(ob_std_os_exit(ob_int(1LL)));
    return ob_null();
}

static void __oboe_toplevel_2(void) {
}

OboeValue lexer__byte_at(OboeValue i) {
    if (ob_truthy(ob_bool(ob_truthy(ob_binop("<", i, ob_int(0LL), ob_lt)) || ob_truthy(ob_binop(">=", i, lexer__SLEN, ob_gte))))) {
        return ob_neg(ob_int(1LL));
    }
    return ob_ord(ob_str_substr(lexer__SRC, i, ob_int(1LL)));
    return ob_null();
}

OboeValue lexer__ch(OboeValue i) {
    if (ob_truthy(ob_bool(ob_truthy(ob_binop("<", i, ob_int(0LL), ob_lt)) || ob_truthy(ob_binop(">=", i, lexer__SLEN, ob_gte))))) {
        return ob_string("");
    }
    return ob_str_substr(lexer__SRC, i, ob_int(1LL));
    return ob_null();
}

OboeValue lexer__is_digit(OboeValue c) {
    return ob_bool(ob_truthy(ob_binop(">=", c, ob_int(48LL), ob_gte)) && ob_truthy(ob_binop("<=", c, ob_int(57LL), ob_lte)));
    return ob_null();
}

OboeValue lexer__is_alpha(OboeValue c) {
    return ob_bool(ob_truthy(ob_bool(ob_truthy(ob_binop(">=", c, ob_int(97LL), ob_gte)) && ob_truthy(ob_binop("<=", c, ob_int(122LL), ob_lte)))) || ob_truthy(ob_bool(ob_truthy(ob_binop(">=", c, ob_int(65LL), ob_gte)) && ob_truthy(ob_binop("<=", c, ob_int(90LL), ob_lte)))));
    return ob_null();
}

OboeValue lexer__is_alnum(OboeValue c) {
    return ob_bool(ob_truthy(lexer__is_alpha(c)) || ob_truthy(lexer__is_digit(c)));
    return ob_null();
}

OboeValue lexer__is_space(OboeValue c) {
    return ob_bool(ob_truthy(ob_binop("==", c, ob_int(32LL), ob_eq)) || ob_truthy(ob_bool(ob_truthy(ob_binop(">=", c, ob_int(9LL), ob_gte)) && ob_truthy(ob_binop("<=", c, ob_int(13LL), ob_lte)))));
    return ob_null();
}

OboeValue lexer__is_ident_start(OboeValue c) {
    return ob_bool(ob_truthy(lexer__is_alpha(c)) || ob_truthy(ob_binop("==", c, ob_int(95LL), ob_eq)));
    return ob_null();
}

OboeValue lexer__is_ident_char(OboeValue c) {
    return ob_bool(ob_truthy(lexer__is_alnum(c)) || ob_truthy(ob_binop("==", c, ob_int(95LL), ob_eq)));
    return ob_null();
}

OboeValue lexer__is_op_char(OboeValue c) {
    if (ob_truthy(ob_binop("<", c, ob_int(0LL), ob_lt))) {
        return ob_bool(false);
    }
    return ob_m_contains(lexer__OP_CHARS, ob_chr(c));
    return ob_null();
}

OboeValue lexer__register_custom_op(OboeValue sym) {
    if (ob_truthy(ob_m_contains(lexer__BUILTIN_OPS, sym))) {
        return ob_null();
    }
    if (ob_truthy(ob_m_contains(lexer__CUSTOM_OPS, sym))) {
        return ob_null();
    }
    if (ob_truthy(ob_binop(">=", ob_m_len(lexer__CUSTOM_OPS), ob_int(64LL), ob_gte))) {
        (void)(diag__fail(ob_interpolate(1, ob_string("too many custom operators"))));
    }
    (void)(ob_arr_push(lexer__CUSTOM_OPS, sym));
    return ob_null();
}

OboeValue lexer__prescan_ops(OboeValue src) {
    (void)((lexer__SRC = src));
    (void)((lexer__SLEN = ob_m_len(src)));
    OboeValue i = ob_int(0LL);
    while (ob_truthy(ob_binop("<", ob_binop("+", i, ob_int(8LL), ob_add), lexer__SLEN, ob_lt))) {
        if (ob_truthy(ob_bool(ob_truthy(ob_binop("!=", lexer__ch(i), ob_interpolate(1, ob_string("o")), ob_neq)) || ob_truthy(ob_binop("!=", ob_str_substr(lexer__SRC, i, ob_int(8LL)), ob_interpolate(1, ob_string("operator")), ob_neq))))) {
            (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
            continue;
        }
        if (ob_truthy(ob_bool(ob_truthy(ob_binop(">", i, ob_int(0LL), ob_gt)) && ob_truthy(lexer__is_ident_char(lexer__byte_at(ob_binop("-", i, ob_int(1LL), ob_sub))))))) {
            (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
            continue;
        }
        OboeValue p = ob_binop("+", i, ob_int(8LL), ob_add);
        if (ob_truthy(ob_bool(ob_truthy(ob_binop("<", p, lexer__SLEN, ob_lt)) && ob_truthy(lexer__is_ident_char(lexer__byte_at(p)))))) {
            (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
            continue;
        }
        while (ob_truthy(ob_bool(ob_truthy(ob_binop("<", p, lexer__SLEN, ob_lt)) && ob_truthy(lexer__is_space(lexer__byte_at(p)))))) {
            (void)((p = ob_binop("+", p, ob_int(1LL), ob_add)));
        }
        OboeValue start = p;
        while (ob_truthy(ob_bool(ob_truthy(ob_binop("<", p, lexer__SLEN, ob_lt)) && ob_truthy(lexer__is_op_char(lexer__byte_at(p)))))) {
            (void)((p = ob_binop("+", p, ob_int(1LL), ob_add)));
        }
        if (ob_truthy(ob_binop(">", p, start, ob_gt))) {
            (void)(lexer__register_custom_op(ob_str_substr(lexer__SRC, start, ob_binop("-", p, start, ob_sub))));
        }
        (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
    }
    return ob_null();
}

OboeValue lexer__match_custom_op(OboeValue pos) {
    OboeValue best = ob_string("");
    OboeValue best_len = ob_int(0LL);
    { OboeValue __it = lexer__CUSTOM_OPS; int64_t __n = ob_iter_len(__it);
    for (int64_t __i = 0; __i < __n; __i++) {
        OboeValue op = ob_iter_value(__it, __i);
        OboeValue n = ob_m_len(op);
        if (ob_truthy(ob_bool(ob_truthy(ob_bool(ob_truthy(ob_binop(">", n, best_len, ob_gt)) && ob_truthy(ob_binop("<=", ob_binop("+", pos, n, ob_add), lexer__SLEN, ob_lte)))) && ob_truthy(ob_binop("==", ob_str_substr(lexer__SRC, pos, n), op, ob_eq))))) {
            (void)((best = op));
            (void)((best_len = n));
        }
    } }
    return best;
    return ob_null();
}

OboeValue lexer__tok(OboeValue type, OboeValue text, OboeValue line) {
    return ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("type"))), type); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("text"))), text); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("line"))), line); __d; });
    return ob_null();
}

OboeValue lexer__read_string_body() {
    OboeValue parts = ({ OboeValue __a = ob_array_new(); __a; });
    while (ob_truthy(ob_bool(ob_truthy(ob_binop("<", lexer__POS, lexer__SLEN, ob_lt)) && ob_truthy(ob_binop("!=", lexer__ch(lexer__POS), ob_interpolate(1, ob_string("\"")), ob_neq))))) {
        OboeValue c = lexer__ch(lexer__POS);
        if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", c, ob_interpolate(1, ob_string("\\")), ob_eq)) && ob_truthy(ob_binop("<", ob_binop("+", lexer__POS, ob_int(1LL), ob_add), lexer__SLEN, ob_lt))))) {
            OboeValue e = lexer__ch(ob_binop("+", lexer__POS, ob_int(1LL), ob_add));
            OboeValue decoded = e;
            if (ob_truthy(ob_binop("==", e, ob_interpolate(1, ob_string("n")), ob_eq))) {
                (void)((decoded = ob_interpolate(1, ob_string("\n"))));
            }
            else {
                if (ob_truthy(ob_binop("==", e, ob_interpolate(1, ob_string("t")), ob_eq))) {
                    (void)((decoded = ob_interpolate(1, ob_string("\t"))));
                }
                else {
                    if (ob_truthy(ob_binop("==", e, ob_interpolate(1, ob_string("r")), ob_eq))) {
                        (void)((decoded = ob_interpolate(1, ob_string("\r"))));
                    }
                }
            }
            (void)(ob_arr_push(parts, decoded));
            (void)((lexer__POS = ob_binop("+", lexer__POS, ob_int(2LL), ob_add)));
            continue;
        }
        if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", c, ob_interpolate(1, ob_string("$")), ob_eq)) && ob_truthy(ob_binop("==", lexer__ch(ob_binop("+", lexer__POS, ob_int(1LL), ob_add)), ob_interpolate(1, ob_string("{")), ob_eq))))) {
            OboeValue start = lexer__POS;
            OboeValue p = ob_binop("+", lexer__POS, ob_int(2LL), ob_add);
            OboeValue depth = ob_int(1LL);
            while (ob_truthy(ob_bool(ob_truthy(ob_binop("<", p, lexer__SLEN, ob_lt)) && ob_truthy(ob_binop(">", depth, ob_int(0LL), ob_gt))))) {
                OboeValue d = lexer__ch(p);
                if (ob_truthy(ob_binop("==", d, ob_interpolate(1, ob_string("{")), ob_eq))) {
                    (void)((depth = ob_binop("+", depth, ob_int(1LL), ob_add)));
                }
                else {
                    if (ob_truthy(ob_binop("==", d, ob_interpolate(1, ob_string("}")), ob_eq))) {
                        (void)((depth = ob_binop("-", depth, ob_int(1LL), ob_sub)));
                    }
                }
                if (ob_truthy(ob_binop(">", depth, ob_int(0LL), ob_gt))) {
                    (void)((p = ob_binop("+", p, ob_int(1LL), ob_add)));
                }
            }
            (void)(ob_arr_push(parts, ob_str_substr(lexer__SRC, start, ob_binop("+", ob_binop("-", p, start, ob_sub), ob_int(1LL), ob_add))));
            (void)((lexer__POS = ob_binop("+", p, ob_int(1LL), ob_add)));
            continue;
        }
        if (ob_truthy(ob_binop("==", c, ob_interpolate(1, ob_string("\n")), ob_eq))) {
            (void)((lexer__LINE = ob_binop("+", lexer__LINE, ob_int(1LL), ob_add)));
        }
        (void)(ob_arr_push(parts, c));
        (void)((lexer__POS = ob_binop("+", lexer__POS, ob_int(1LL), ob_add)));
    }
    return ob_arr_join(parts, ob_string(""));
    return ob_null();
}

OboeValue lexer__lex_all(OboeValue src) {
    (void)(lexer__prescan_ops(src));
    (void)((lexer__SRC = src));
    (void)((lexer__SLEN = ob_m_len(src)));
    (void)((lexer__POS = ob_int(0LL)));
    (void)((lexer__LINE = ob_int(1LL)));
    OboeValue toks = ({ OboeValue __a = ob_array_new(); __a; });
    while (ob_truthy(ob_binop("<", lexer__POS, lexer__SLEN, ob_lt))) {
        OboeValue c = lexer__ch(lexer__POS);
        OboeValue cv = lexer__byte_at(lexer__POS);
        if (ob_truthy(ob_binop("==", c, ob_interpolate(1, ob_string("\n")), ob_eq))) {
            (void)((lexer__LINE = ob_binop("+", lexer__LINE, ob_int(1LL), ob_add)));
            (void)((lexer__POS = ob_binop("+", lexer__POS, ob_int(1LL), ob_add)));
            continue;
        }
        if (ob_truthy(lexer__is_space(cv))) {
            (void)((lexer__POS = ob_binop("+", lexer__POS, ob_int(1LL), ob_add)));
            continue;
        }
        if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", c, ob_interpolate(1, ob_string("/")), ob_eq)) && ob_truthy(ob_binop("==", lexer__ch(ob_binop("+", lexer__POS, ob_int(1LL), ob_add)), ob_interpolate(1, ob_string("/")), ob_eq))))) {
            while (ob_truthy(ob_bool(ob_truthy(ob_binop("<", lexer__POS, lexer__SLEN, ob_lt)) && ob_truthy(ob_binop("!=", lexer__ch(lexer__POS), ob_interpolate(1, ob_string("\n")), ob_neq))))) {
                (void)((lexer__POS = ob_binop("+", lexer__POS, ob_int(1LL), ob_add)));
            }
            continue;
        }
        if (ob_truthy(lexer__is_op_char(cv))) {
            OboeValue cop = lexer__match_custom_op(lexer__POS);
            if (ob_truthy(ob_binop("!=", cop, ob_string(""), ob_neq))) {
                (void)(ob_arr_push(toks, lexer__tok(ob_interpolate(1, ob_string("T_CUSTOMOP")), cop, lexer__LINE)));
                (void)((lexer__POS = ob_binop("+", lexer__POS, ob_m_len(cop), ob_add)));
                continue;
            }
        }
        if (ob_truthy(ob_binop("==", c, ob_interpolate(1, ob_string("\"")), ob_eq))) {
            (void)((lexer__POS = ob_binop("+", lexer__POS, ob_int(1LL), ob_add)));
            OboeValue body = lexer__read_string_body();
            (void)((lexer__POS = ob_binop("+", lexer__POS, ob_int(1LL), ob_add)));
            (void)(ob_arr_push(toks, lexer__tok(ob_interpolate(1, ob_string("T_STRING")), body, lexer__LINE)));
            continue;
        }
        if (ob_truthy(lexer__is_digit(cv))) {
            OboeValue start = lexer__POS;
            while (ob_truthy(ob_bool(ob_truthy(ob_binop("<", lexer__POS, lexer__SLEN, ob_lt)) && ob_truthy(lexer__is_digit(lexer__byte_at(lexer__POS)))))) {
                (void)((lexer__POS = ob_binop("+", lexer__POS, ob_int(1LL), ob_add)));
            }
            OboeValue is_float = ob_bool(false);
            if (ob_truthy(ob_bool(ob_truthy(ob_bool(ob_truthy(ob_binop("<", ob_binop("+", lexer__POS, ob_int(1LL), ob_add), lexer__SLEN, ob_lt)) && ob_truthy(ob_binop("==", lexer__ch(lexer__POS), ob_interpolate(1, ob_string(".")), ob_eq)))) && ob_truthy(lexer__is_digit(lexer__byte_at(ob_binop("+", lexer__POS, ob_int(1LL), ob_add))))))) {
                (void)((is_float = ob_bool(true)));
                (void)((lexer__POS = ob_binop("+", lexer__POS, ob_int(1LL), ob_add)));
                while (ob_truthy(ob_bool(ob_truthy(ob_binop("<", lexer__POS, lexer__SLEN, ob_lt)) && ob_truthy(lexer__is_digit(lexer__byte_at(lexer__POS)))))) {
                    (void)((lexer__POS = ob_binop("+", lexer__POS, ob_int(1LL), ob_add)));
                }
            }
            OboeValue numstr = ob_str_substr(lexer__SRC, start, ob_binop("-", lexer__POS, start, ob_sub));
            OboeValue nkind = (ob_truthy(is_float) ? (ob_interpolate(1, ob_string("T_FLOAT"))) : (ob_interpolate(1, ob_string("T_INT"))));
            (void)(ob_arr_push(toks, lexer__tok(nkind, numstr, lexer__LINE)));
            continue;
        }
        if (ob_truthy(lexer__is_ident_start(cv))) {
            OboeValue istart = lexer__POS;
            while (ob_truthy(ob_bool(ob_truthy(ob_binop("<", lexer__POS, lexer__SLEN, ob_lt)) && ob_truthy(lexer__is_ident_char(lexer__byte_at(lexer__POS)))))) {
                (void)((lexer__POS = ob_binop("+", lexer__POS, ob_int(1LL), ob_add)));
            }
            OboeValue ident = ob_str_substr(lexer__SRC, istart, ob_binop("-", lexer__POS, istart, ob_sub));
            OboeValue kind = ob_interpolate(1, ob_string("T_IDENT"));
            if (ob_truthy(ob_dict_has_m(lexer__KEYWORDS, ident))) {
                (void)((kind = ob_index_get(lexer__KEYWORDS, ident)));
            }
            (void)(ob_arr_push(toks, lexer__tok(kind, ident, lexer__LINE)));
            continue;
        }
        OboeValue two = ob_str_substr(lexer__SRC, lexer__POS, ob_int(2LL));
        if (ob_truthy(ob_dict_has_m(lexer__OPS2, two))) {
            (void)(ob_arr_push(toks, lexer__tok(ob_index_get(lexer__OPS2, two), two, lexer__LINE)));
            (void)((lexer__POS = ob_binop("+", lexer__POS, ob_int(2LL), ob_add)));
            continue;
        }
        if (ob_truthy(ob_dict_has_m(lexer__OPS1, c))) {
            (void)(ob_arr_push(toks, lexer__tok(ob_index_get(lexer__OPS1, c), c, lexer__LINE)));
            (void)((lexer__POS = ob_binop("+", lexer__POS, ob_int(1LL), ob_add)));
            continue;
        }
        (void)(diag__fail(ob_binop("+", ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("unexpected character '")), c, ob_add), ob_interpolate(1, ob_string("' at line ")), ob_add), ob_str(lexer__LINE), ob_add)));
    }
    (void)(ob_arr_push(toks, lexer__tok(ob_interpolate(1, ob_string("T_EOF")), ob_string(""), lexer__LINE)));
    return toks;
    return ob_null();
}

static void __oboe_toplevel_1(void) {
    lexer__SRC = ob_string("");
    lexer__SLEN = ob_int(0LL);
    lexer__POS = ob_int(0LL);
    lexer__LINE = ob_int(1LL);
    lexer__OP_CHARS = ob_interpolate(1, ob_string("+-*/%<>=!&|^~?@#$:."));
    lexer__KEYWORDS = ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("let"))), ob_interpolate(1, ob_string("T_LET"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("var"))), ob_interpolate(1, ob_string("T_LET"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("const"))), ob_interpolate(1, ob_string("T_CONST"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("func"))), ob_interpolate(1, ob_string("T_FUNC"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("return"))), ob_interpolate(1, ob_string("T_RETURN"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("class"))), ob_interpolate(1, ob_string("T_CLASS"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("static"))), ob_interpolate(1, ob_string("T_STATIC"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("private"))), ob_interpolate(1, ob_string("T_PRIVATE"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("if"))), ob_interpolate(1, ob_string("T_IF"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("else"))), ob_interpolate(1, ob_string("T_ELSE"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("while"))), ob_interpolate(1, ob_string("T_WHILE"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("for"))), ob_interpolate(1, ob_string("T_FOR"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("in"))), ob_interpolate(1, ob_string("T_IN"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("break"))), ob_interpolate(1, ob_string("T_BREAK"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("continue"))), ob_interpolate(1, ob_string("T_CONTINUE"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("switch"))), ob_interpolate(1, ob_string("T_SWITCH"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("case"))), ob_interpolate(1, ob_string("T_CASE"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("try"))), ob_interpolate(1, ob_string("T_TRY"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("catch"))), ob_interpolate(1, ob_string("T_CATCH"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("finally"))), ob_interpolate(1, ob_string("T_FINALLY"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("throw"))), ob_interpolate(1, ob_string("T_THROW"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("import"))), ob_interpolate(1, ob_string("T_IMPORT"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("as"))), ob_interpolate(1, ob_string("T_AS"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("from"))), ob_interpolate(1, ob_string("T_FROM"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("true"))), ob_interpolate(1, ob_string("T_TRUE"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("false"))), ob_interpolate(1, ob_string("T_FALSE"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("null"))), ob_interpolate(1, ob_string("T_NULL"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("and"))), ob_interpolate(1, ob_string("T_AND"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("or"))), ob_interpolate(1, ob_string("T_OR"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("is"))), ob_interpolate(1, ob_string("T_IS"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("extends"))), ob_interpolate(1, ob_string("T_EXTENDS"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("event"))), ob_interpolate(1, ob_string("T_EVENT"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("on"))), ob_interpolate(1, ob_string("T_ON"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("operator"))), ob_interpolate(1, ob_string("T_OPERATOR"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("cimport"))), ob_interpolate(1, ob_string("T_CIMPORT"))); __d; });
    lexer__OPS2 = ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("->"))), ob_interpolate(1, ob_string("T_ARROW"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("=="))), ob_interpolate(1, ob_string("T_EQ"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("!="))), ob_interpolate(1, ob_string("T_NEQ"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("<="))), ob_interpolate(1, ob_string("T_LTE"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("<<"))), ob_interpolate(1, ob_string("T_SHL"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string(">="))), ob_interpolate(1, ob_string("T_GTE"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string(">>"))), ob_interpolate(1, ob_string("T_SHR"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("&&"))), ob_interpolate(1, ob_string("T_ANDAND"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("||"))), ob_interpolate(1, ob_string("T_OROR"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("??"))), ob_interpolate(1, ob_string("T_QQ"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("?."))), ob_interpolate(1, ob_string("T_QDOT"))); __d; });
    lexer__OPS1 = ({ OboeValue __d = ob_dict_new(); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("{"))), ob_interpolate(1, ob_string("T_LBRACE"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("}"))), ob_interpolate(1, ob_string("T_RBRACE"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("("))), ob_interpolate(1, ob_string("T_LPAREN"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string(")"))), ob_interpolate(1, ob_string("T_RPAREN"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("["))), ob_interpolate(1, ob_string("T_LBRACKET"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("]"))), ob_interpolate(1, ob_string("T_RBRACKET"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string(","))), ob_interpolate(1, ob_string("T_COMMA"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("."))), ob_interpolate(1, ob_string("T_DOT"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string(":"))), ob_interpolate(1, ob_string("T_COLON"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string(";"))), ob_interpolate(1, ob_string("T_SEMI"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("+"))), ob_interpolate(1, ob_string("T_PLUS"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("-"))), ob_interpolate(1, ob_string("T_MINUS"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("*"))), ob_interpolate(1, ob_string("T_STAR"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("/"))), ob_interpolate(1, ob_string("T_SLASH"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("%"))), ob_interpolate(1, ob_string("T_PERCENT"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("="))), ob_interpolate(1, ob_string("T_ASSIGN"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("<"))), ob_interpolate(1, ob_string("T_LT"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string(">"))), ob_interpolate(1, ob_string("T_GT"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("!"))), ob_interpolate(1, ob_string("T_NOT"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("&"))), ob_interpolate(1, ob_string("T_AMP"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("|"))), ob_interpolate(1, ob_string("T_PIPE"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("^"))), ob_interpolate(1, ob_string("T_CARET"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("~"))), ob_interpolate(1, ob_string("T_TILDE"))); ob_dict_set(__d, ob_to_cstr(ob_interpolate(1, ob_string("?"))), ob_interpolate(1, ob_string("T_QUESTION"))); __d; });
    lexer__CUSTOM_OPS = ({ OboeValue __a = ob_array_new(); __a; });
    lexer__BUILTIN_OPS = ({ OboeValue __a = ob_array_new(); ob_array_push(__a, ob_interpolate(1, ob_string("+"))); ob_array_push(__a, ob_interpolate(1, ob_string("-"))); ob_array_push(__a, ob_interpolate(1, ob_string("*"))); ob_array_push(__a, ob_interpolate(1, ob_string("/"))); ob_array_push(__a, ob_interpolate(1, ob_string("%"))); ob_array_push(__a, ob_interpolate(1, ob_string("="))); ob_array_push(__a, ob_interpolate(1, ob_string("=="))); ob_array_push(__a, ob_interpolate(1, ob_string("!="))); ob_array_push(__a, ob_interpolate(1, ob_string("<"))); ob_array_push(__a, ob_interpolate(1, ob_string("<="))); ob_array_push(__a, ob_interpolate(1, ob_string(">"))); ob_array_push(__a, ob_interpolate(1, ob_string(">="))); ob_array_push(__a, ob_interpolate(1, ob_string("&&"))); ob_array_push(__a, ob_interpolate(1, ob_string("||"))); ob_array_push(__a, ob_interpolate(1, ob_string("??"))); ob_array_push(__a, ob_interpolate(1, ob_string("?."))); ob_array_push(__a, ob_interpolate(1, ob_string("!"))); ob_array_push(__a, ob_interpolate(1, ob_string("?"))); ob_array_push(__a, ob_interpolate(1, ob_string("."))); ob_array_push(__a, ob_interpolate(1, ob_string("->"))); ob_array_push(__a, ob_interpolate(1, ob_string(":"))); ob_array_push(__a, ob_interpolate(1, ob_string("&"))); ob_array_push(__a, ob_interpolate(1, ob_string("|"))); ob_array_push(__a, ob_interpolate(1, ob_string("^"))); ob_array_push(__a, ob_interpolate(1, ob_string("~"))); ob_array_push(__a, ob_interpolate(1, ob_string("<<"))); ob_array_push(__a, ob_interpolate(1, ob_string(">>"))); __a; });
}

OboeValue usage() {
    (void)((ob_eprint(ob_interpolate(1, ob_string("usage: oboec <file> -o <out.c> [--target-os <os>]"))), ob_null()));
    (void)((ob_eprint(ob_interpolate(1, ob_string("       oboec --dump-tokens|--dump-ast|--emit-c <file>"))), ob_null()));
    (void)(ob_std_os_exit(ob_int(2LL)));
    return ob_null();
}

OboeValue read_source(OboeValue path) {
    { OboeExceptionFrame __frame_0; __frame_0.prev = ob_exc_stack; ob_exc_stack = &__frame_0;
    bool __rethrow_0 = false;
    if (setjmp(__frame_0.buf) == 0) {
        { OboeValue __ret = ob_std_os_read_file(path); ob_exc_stack = __frame_0.prev; return __ret; }
        ob_exc_stack = __frame_0.prev;
    } else {
        if (ob_exception_matches("os.FileNotFoundError")) {
            OboeValue e = ob_current_exception;
            (void)(diag__fail(ob_binop("+", ob_binop("+", ob_interpolate(1, ob_string("cannot read '")), path, ob_add), ob_interpolate(1, ob_string("'")), ob_add)));
        }
        else { __rethrow_0 = true; }
    }
    if (__rethrow_0) ob_throw(ob_current_exception_type, ob_current_exception);
    }
    return ob_null();
}

OboeValue oboe_user_main(OboeValue args) {
    OboeValue mode = ob_interpolate(1, ob_string("c"));
    OboeValue input = ob_null();
    OboeValue output = ob_null();
    OboeValue target_os = ob_null();
    OboeValue i = ob_int(1LL);
    while (ob_truthy(ob_binop("<", i, ob_m_len(args), ob_lt))) {
        OboeValue a = ob_index_get(args, i);
        if (ob_truthy(ob_binop("==", a, ob_interpolate(1, ob_string("--dump-tokens")), ob_eq))) {
            (void)((mode = ob_interpolate(1, ob_string("tokens"))));
            (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
            continue;
        }
        if (ob_truthy(ob_binop("==", a, ob_interpolate(1, ob_string("--dump-ast")), ob_eq))) {
            (void)((mode = ob_interpolate(1, ob_string("ast"))));
            (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
            continue;
        }
        if (ob_truthy(ob_binop("==", a, ob_interpolate(1, ob_string("--emit-c")), ob_eq))) {
            (void)((mode = ob_interpolate(1, ob_string("c"))));
            (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
            continue;
        }
        if (ob_truthy(ob_bool(ob_truthy(ob_binop("==", a, ob_interpolate(1, ob_string("-o")), ob_eq)) || ob_truthy(ob_binop("==", a, ob_interpolate(1, ob_string("--output")), ob_eq))))) {
            if (ob_truthy(ob_binop(">=", ob_binop("+", i, ob_int(1LL), ob_add), ob_m_len(args), ob_gte))) {
                (void)(usage());
            }
            (void)((output = ob_index_get(args, ob_binop("+", i, ob_int(1LL), ob_add))));
            (void)((i = ob_binop("+", i, ob_int(2LL), ob_add)));
            continue;
        }
        if (ob_truthy(ob_binop("==", a, ob_interpolate(1, ob_string("--target-os")), ob_eq))) {
            if (ob_truthy(ob_binop(">=", ob_binop("+", i, ob_int(1LL), ob_add), ob_m_len(args), ob_gte))) {
                (void)(usage());
            }
            (void)((target_os = ob_index_get(args, ob_binop("+", i, ob_int(1LL), ob_add))));
            (void)((i = ob_binop("+", i, ob_int(2LL), ob_add)));
            continue;
        }
        if (ob_truthy(ob_str_starts_with(a, ob_interpolate(1, ob_string("-"))))) {
            (void)(usage());
        }
        (void)((input = a));
        (void)((i = ob_binop("+", i, ob_int(1LL), ob_add)));
    }
    if (ob_truthy(ob_binop("==", input, ob_null(), ob_eq))) {
        (void)(usage());
    }
    if (ob_truthy(ob_bool(ob_truthy(ob_binop("!=", output, ob_null(), ob_neq)) && ob_truthy(ob_binop("!=", mode, ob_interpolate(1, ob_string("c")), ob_neq))))) {
        (void)(usage());
    }
    if (ob_truthy(ob_binop("==", mode, ob_interpolate(1, ob_string("c")), ob_eq))) {
        if (ob_truthy(ob_binop("!=", target_os, ob_null(), ob_neq))) {
            (void)(codegen__codegen_set_target_os(target_os));
        }
        (void)(codegen__codegen_set_source_dir(codegen__path_dirname(input)));
        (void)(codegen__codegen_set_output_path(output));
        OboeValue c = codegen__codegen_compile(input);
        if (ob_truthy(ob_binop("==", output, ob_null(), ob_eq))) {
            (void)((ob_write(c), ob_null()));
        }
        else {
            (void)(ob_std_os_write_file(output, c));
        }
        return ob_null();
    }
    OboeValue toks = lexer__lex_all(read_source(input));
    if (ob_truthy(ob_binop("==", mode, ob_interpolate(1, ob_string("tokens")), ob_eq))) {
        (void)((ob_write(dump__dump_tokens(toks)), ob_null()));
    }
    else {
        (void)((ob_write(dump__dump_ast(parser__parse_program(toks, input))), ob_null()));
    }
    return ob_null();
}

static void __oboe_toplevel_0(void) {
}

static void __oboe_static_init(void) {
}

int main(int argc, char **argv) {
    __oboe_static_init();
    __oboe_toplevel_7();
    __oboe_toplevel_6();
    __oboe_toplevel_5();
    __oboe_toplevel_4();
    __oboe_toplevel_3();
    __oboe_toplevel_2();
    __oboe_toplevel_1();
    __oboe_toplevel_0();
    oboe_user_main(ob_args_from_argv(argc, argv));
    return 0;
}
