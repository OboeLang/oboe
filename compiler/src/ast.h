#ifndef OBOE_AST_H
#define OBOE_AST_H

#include <stdbool.h>

typedef struct Expr Expr;
typedef struct Stmt Stmt;
typedef struct Type Type;
typedef struct Param Param;
typedef struct FuncDecl FuncDecl;
typedef struct ClassDecl ClassDecl;
typedef struct FieldDecl FieldDecl;
typedef struct Decl Decl;
typedef struct Program Program;
typedef struct StringPart StringPart;
typedef struct CatchClause CatchClause;
typedef struct CaseClause CaseClause;

/* ---- types ---- */
struct Type {
    char *name; /* "int", "bool", "string", "array", "dict", class name, or NULL if unspecified */
};

/* ---- expressions ---- */
typedef enum {
    EXPR_INT, EXPR_BOOL, EXPR_NULL, EXPR_STRING, EXPR_IDENT,
    EXPR_ARRAY, EXPR_DICT, EXPR_BINARY, EXPR_UNARY, EXPR_CALL,
    EXPR_FIELD, EXPR_SAFE_FIELD, EXPR_INDEX, EXPR_IS, EXPR_ASSIGN
} ExprKind;

struct StringPart {
    bool is_expr;
    char *literal;   /* when !is_expr */
    Expr *expr;      /* when is_expr */
    StringPart *next;
};

struct Expr {
    ExprKind kind;
    int line;
    union {
        long long int_val;
        bool bool_val;
        char *ident;         /* EXPR_IDENT */
        StringPart *str_parts; /* EXPR_STRING */
        struct { Expr **items; int count; } array_lit;
        struct { Expr **keys; Expr **values; int count; } dict_lit; /* keys are literal strings */
        struct { char *op; Expr *l, *r; } binary;
        struct { char *op; Expr *operand; } unary;
        struct { Expr *callee; Expr **args; int arg_count; } call;
        struct { Expr *obj; char *name; } field;       /* EXPR_FIELD / EXPR_SAFE_FIELD */
        struct { Expr *arr; Expr *idx; } index;
        struct { Expr *value; char *type_name; } is_check;
        struct { Expr *target; Expr *value; } assign;
    } as;
};

/* ---- statements ---- */
typedef enum {
    STMT_LET, STMT_EXPR, STMT_RETURN, STMT_IF, STMT_WHILE, STMT_FOR,
    STMT_SWITCH, STMT_TRY, STMT_THROW, STMT_BLOCK
} StmtKind;

struct CatchClause {
    char *type_name;
    char *var_name;
    Stmt **body;
    int body_count;
    CatchClause *next;
};

struct CaseClause {
    Expr *value; /* NULL means default */
    Stmt **body;
    int body_count;
    CaseClause *next;
};

struct Stmt {
    StmtKind kind;
    int line;
    union {
        struct { char *name; char *type_name; bool is_const; Expr *init; } let;
        struct { Expr *expr; } expr_stmt;
        struct { Expr *value; } ret;
        struct { Expr *cond; Stmt **then_body; int then_count; Stmt **else_body; int else_count; } if_stmt;
        struct { Expr *cond; Stmt **body; int body_count; } while_stmt;
        struct {
            char *var_name;
            bool is_range; Expr *range_a, *range_b;
            Expr *iterable; /* used when !is_range */
            Stmt **body; int body_count;
        } for_stmt;
        struct { Expr *subject; CaseClause *cases; } switch_stmt;
        struct {
            Stmt **body; int body_count;
            CatchClause *catches;
            Stmt **finally_body; int finally_count;
        } try_stmt;
        struct { char *type_name; Expr *value; } throw_stmt;
        struct { Stmt **body; int body_count; } block;
    } as;
};

/* ---- declarations ---- */
struct Param {
    char *type_name; /* may be NULL */
    char *name;
    Param *next;
};

struct FuncDecl {
    char *name;
    char *return_type; /* may be NULL */
    Param *params;
    bool is_static;
    bool is_private;
    Stmt **body;
    int body_count;
    int line;
};

struct FieldDecl {
    char *type_name;
    char *name;
    bool is_static;
    bool is_private;
    bool is_const;
    Expr *init; /* may be NULL */
    FieldDecl *next;
};

struct ClassDecl {
    char *name;
    char *parent_name; /* may be NULL */
    FieldDecl *fields;
    FuncDecl **methods;
    int method_count;
    int line;
};

typedef enum { DECL_FUNC, DECL_CLASS, DECL_IMPORT, DECL_STMT } DeclKind;

typedef struct {
    char *module;      /* module name imported */
    char *alias;       /* `as` alias, may be NULL */
    char **members;    /* `from` member list, may be NULL */
    int member_count;
} ImportDecl;

struct Decl {
    DeclKind kind;
    union {
        FuncDecl *func;
        ClassDecl *klass;
        ImportDecl import;
        Stmt *stmt;
    } as;
    Decl *next;
};

struct Program {
    Decl *decls;
};

#endif
