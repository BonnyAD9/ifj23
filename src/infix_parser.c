#include "infix_parser.h"

#include <stdint.h>

#include "semantics.h"

Token tok_next(Parser *par);

enum StackItemType {
    SI_STOP,
    SI_TERM,
    SI_NTERM,
    SI_CALL_PARAMS,
};

enum PrecedenceAction {
    PA_SHIFT, // <
    PA_PUSH,  // =
    PA_FOLD,  // >
    PA_END,   // .
    PA_ERR,   // !
    PA_TOP,   // x
    PA_STOP,  // o
    PA_CALL,  // c
};

enum FoldResult {
    NO_MATCH,
    MATCH_ERR,
    MATCH_OK,
};

struct StackItem {
    enum StackItemType type;
    FilePos pos;
    union {
        FullToken term;
        AstExpr *nterm;
        Vec call_params;
    };
};

struct ExpansionStack {
    Vec stack;
    Token last_term;
    bool is_top_term;
    bool is_stop_term;
};

static struct ExpansionStack es_new(void);
/// <
static bool es_shift(struct ExpansionStack *stack, Parser *lex);
/// =
static bool es_push(struct ExpansionStack *stack, Parser *lex);
/// >
static bool es_fold(struct ExpansionStack *stack);
static enum FoldResult es_fold_value(struct ExpansionStack *stack);
static enum FoldResult es_fold_bracket(struct ExpansionStack *stack);
static enum FoldResult es_fold_call(struct ExpansionStack *stack);
static enum FoldResult es_fold_binary(struct ExpansionStack *stack);
static enum FoldResult es_fold_prefix(struct ExpansionStack *stack);
static enum FoldResult es_fold_postfix(struct ExpansionStack *stack);
/// Folds, parses function call parameters and folds again
static bool es_call(struct ExpansionStack *stack, Parser *par);
static size_t es_top_index(struct ExpansionStack *stack, enum StackItemType type);

// implemented in parser.c
bool parse_func_params(Parser *par, Vec *res);
/// folds while there is more than single non terminal, returns the last
/// non terminal
static AstExpr *es_finish(struct ExpansionStack *stack);
static void es_free(struct ExpansionStack *stack);
static void si_free(struct StackItem *si);
/// the precedence table
static enum PrecedenceAction prec_table(Token stack, Token input);
static bool is_infix_token(Token t);
static bool is_value_token(Token t);


AstExpr *parse_infix(Parser *par) {
    struct ExpansionStack stack = es_new();

    // the first can be anything different than PA_NONE
    enum PrecedenceAction action;
    while ((action = prec_table(stack.last_term, par->cur)) != PA_END) {
        switch (action) {
        case PA_SHIFT:
            if (!es_shift(&stack, par)) {
                es_free(&stack);
                return NULL;
            }
            break;
        case PA_PUSH:
            if (!es_push(&stack, par)) {
                es_free(&stack);
                return NULL;
            }
            es_push(&stack, par);
            break;
        case PA_FOLD:
            if (!es_fold(&stack)) {
                es_free(&stack);
                return NULL;
            }
            break;
        case PA_ERR:
            es_free(&stack);
            return NULL;
        case PA_TOP:
            if (stack.is_top_term) {
                if (!es_shift(&stack, par)) {
                    es_free(&stack);
                    return NULL;
                }
                break;
            }
            if (!es_fold(&stack)) {
                es_free(&stack);
                return NULL;
            }
            break;
        case PA_STOP:
            if (stack.is_stop_term) {
                if (!es_fold(&stack)) {
                    es_free(&stack);
                    return NULL;
                    break;
                }
            }
            if (!es_shift(&stack, par)) {
                es_free(&stack);
                return NULL;
            }
            break;
        case PA_CALL:
            if (!es_call(&stack, par)) {
                return NULL;
            }
            break;
        default:
            break;
        }
    }

    AstExpr *res = es_finish(&stack);
    es_free(&stack);
    return res;
}

// +------------+-----------------+-------------+---------------+
// | PRECEDENCE | OPERATOR        | DESCRIPTION | ASOCIATIVITY  |
// +------------+-----------------+-------------+---------------+
// | 1          | () !            | postfix     | left to right |
// +------------+-----------------+-------------+               |
// | 2          | - +             | prefix      |               |
// +------------+-----------------+-------------+               |
// | 3          | * /             | mul/div     |               |
// +------------+-----------------+-------------+               |
// | 4          | + -             | add/sub     |               |
// +------------+-----------------+-------------+               |
// | 5          | == != < > <= >= | comparison  |               |
// +------------+-----------------+-------------+---------------+
// | 6          | ??              | nil check   | right to left |
// +------------+-----------------+-------------+               |
// | 7          | =               | assignment  |               |
// +------------+-----------------+-------------+---------------+
//
// < PA_SHIFT Shift
// = PA_PUSH  Push
// > PA_FOLD  Fold
// . PA_END   Successful end
// ! PA_ERR   Error
// x PA_TOP   Shift if terminal is top, fold if non-terminal is top
// o PA_STOP  Fold if terminal after stop, shift if non-terminal after stop
// c PA_CALL  Parse function call
//
//     | p!  *   /   +   -   ==  !=  <   >   <=  >=  ??  =   (   )   t   $
// ----+-------------------------------------------------------------------
//  p! | >   >   >   >   >   >   >   >   >   >   >   >   !   c   >   .   .
//  *  | !   >   >   x   x   >   >   >   >   >   >   >   !   <   >   <   .
//  /  | !   >   >   x   x   >   >   >   >   >   >   >   !   <   >   <   .
//  +  | !   o   o   >   >   >   >   >   >   >   >   >   !   <   >   <   .
//  -  | !   o   o   >   >   >   >   >   >   >   >   >   !   <   >   <   .
//  == | !   <   <   <   <   >   >   >   >   >   >   >   !   <   >   <   .
//  != | !   <   <   <   <   >   >   >   >   >   >   >   !   <   >   <   .
//  <  | !   <   <   <   <   >   >   >   >   >   >   >   !   <   >   <   .
//  >  | !   <   <   <   <   >   >   >   >   >   >   >   !   <   >   <   .
//  <= | !   <   <   <   <   >   >   >   >   >   >   >   !   <   >   <   .
//  >= | !   <   <   <   <   >   >   >   >   >   >   >   !   <   >   <   .
//  ?? | !   <   <   <   <   <   <   <   <   <   <   <   !   <   >   <   .
//  =  | !   <   <   <   <   <   <   <   <   <   <   <   !   <   !   <   .
//  (  | !   <   <   <   <   <   <   <   <   <   <   <   !   <   =   <   !
//  )  | =   >   >   >   >   >   >   >   >   >   >   >   !   c   >   .   .
//  t  | >   >   >   >   >   >   >   >   >   >   >   >   >   c   >   .   .
//  $  | <   <   <   <   <   <   <   <   <   <   <   <   <   !   !   <   .
static enum PrecedenceAction prec_table(Token stack, Token input) {
    switch ((int)stack) {
    case '!':
        switch ((int)input) {
        case '=':
            return PA_ERR;
        case '(':
            return PA_CALL;
        }
        if (is_value_token(input) || !is_infix_token(input)) {
            return PA_END;
        }
        return PA_FOLD;
    case '*':
    case '/':
        switch ((int)input) {
        case '!':
        case '=':
            return PA_ERR;
        case '+':
        case '-':
            return PA_TOP;
        case '(':
            return PA_SHIFT;
        }
        if (is_value_token(input)) {
            return PA_SHIFT;
        }
        if (!is_infix_token(input)) {
            return PA_END;
        }
        return PA_FOLD;
    case '+':
    case '-':
        switch ((int)input) {
        case '!':
        case '=':
            return PA_ERR;
        case '*':
        case '/':
            return PA_STOP;
        case '(':
            return PA_SHIFT;
        }
        if (is_value_token(input)) {
            return PA_SHIFT;
        }
        if (!is_infix_token(input)) {
            return PA_END;
        }
        return PA_FOLD;
    case T_DOUBLE_QUES:
        switch ((int)input) {
        case '!':
        case '=':
            return PA_ERR;
        case ')':
            return PA_FOLD;
        }
        if (!is_infix_token(input)) {
            return PA_END;
        }
        return PA_SHIFT;
    case '=':
        switch ((int)input) {
        case '!':
        case '=':
        case ')':
            return PA_ERR;
        }
        if (!is_infix_token(input)) {
            return PA_END;
        }
        return PA_SHIFT;
    case '(':
        switch ((int)input) {
        case '!':
        case '=':
            return PA_ERR;
        case ')':
            return PA_PUSH;
        }
        if (!is_infix_token(input)) {
            return PA_ERR;
        }
        return PA_SHIFT;
    case ')':
        switch ((int)input) {
        case '!':
            return PA_PUSH;
        case '=':
            return PA_ERR;
        }
        if (is_value_token(input) || !is_infix_token(input)) {
            return PA_END;
        }
        return PA_FOLD;
    }
    if (is_value_token(stack)) {
        switch ((int)input) {
        case '(':
            return PA_CALL;
        }
        if (is_value_token(input) || !is_infix_token(input)) {
            return PA_END;
        }
        return PA_FOLD;
    }
    if (!is_infix_token(stack)) {
        if (input == '(' || input == ')') {
            return PA_ERR;
        }
        if (!is_infix_token(input)) {
            return PA_END;
        }
        return PA_SHIFT;
    }
    switch ((int)input) {
    case '!':
    case '=':
        return PA_ERR;
    case '*':
    case '/':
    case '+':
    case '-':
    case '(':
        return PA_SHIFT;
    }
    if (is_value_token(input)) {
        return PA_SHIFT;
    }
    if (!is_infix_token(input)) {
        return PA_END;
    }
    return PA_FOLD;
}

static struct ExpansionStack es_new(void) {
    Vec v = VEC_NEW(struct StackItem);
    VEC_PUSH(&v, struct StackItem, ((struct StackItem) {
        .type = SI_TERM,
        .term = {
            .type = T_EOF,
        },
    }));

    return (struct ExpansionStack) {
        .stack = v,
        .last_term = T_EOF,
        .is_stop_term = true,
    };
}

static bool es_shift(struct ExpansionStack *stack, Parser *par) {
    vec_reserve(&stack->stack, stack->stack.len + 2);

    size_t i = es_top_index(stack, SI_TERM);

    VEC_INSERT(&stack->stack, struct StackItem, i + 1, ((struct StackItem) {
        .type = SI_STOP,
    }));

    return es_push(stack, par);
}

static bool es_push(struct ExpansionStack *stack, Parser *par) {
    struct StackItem item = {
        .type = SI_TERM,
        .pos = par->lex->token_start,
        .term = {
            .type = par->cur,
            .subtype = par->lex->datatype,
        },
    };

    stack->last_term = par->cur;
    stack->is_top_term = true;

    switch (par->cur) {
    case T_IDENT: {
        SymItem *itm = sym_find(par->table, par->lex->str);
        if (!itm) {
            return false;
        }
        item.term.ident = itm;
        break;
    }
    case T_LITERAL:
        switch (par->lex->datatype) {
        case DT_INT:
            item.term.int_v = par->lex->i_num;
            break;
        case DT_DOUBLE:
            item.term.double_v = par->lex->d_num;
            break;
        case DT_STRING:
            item.term.str = str_clone(par->lex->str);
            break;
        }
        break;
    default:
        break;
    }

    VEC_PUSH(&stack->stack, struct StackItem, item);

    tok_next(par);

    return true;
}

static bool es_fold(struct ExpansionStack *stack) {
    enum FoldResult (*folds[])(struct ExpansionStack *stack) = {
        es_fold_value,
        es_fold_bracket,
        es_fold_call,
        es_fold_binary,
        es_fold_prefix,
        es_fold_postfix,
    };
    const size_t fold_len = sizeof(folds) / sizeof(*folds);

    enum FoldResult res;
    for (size_t i = 0; i < fold_len; ++i) {
        if ((res = folds[i](stack)) != NO_MATCH) {
            break;
        }
    }

    stack->is_top_term = false;

    size_t t_idx = es_top_index(stack, SI_TERM);
    stack->last_term = VEC_AT(
        &stack->stack,
        struct StackItem,
        t_idx
    ).term.type;
    size_t t_s = es_top_index(stack, SI_STOP);
    if (t_s >= stack->stack.len - 1) {
        stack->is_stop_term = false;
    } else {
        stack->is_stop_term = VEC_AT(
            &stack->stack,
            struct StackItem,
            t_s + 1
        ).type == SI_TERM;
    }

    return res == MATCH_OK;
}

static enum FoldResult es_fold_value(struct ExpansionStack *stack) {
    if (stack->stack.len < 3) {
        return NO_MATCH;
    }

    struct StackItem *si = &VEC_AT(&stack->stack, struct StackItem, 0);
    size_t sl = stack->stack.len;

    struct StackItem s0 = si[sl - 2];
    struct StackItem s1 = si[sl - 1];

    if (s0.type != SI_STOP || s1.type != SI_TERM
        || !is_value_token(s1.term.type)
    ) {
        return false;
    }

    AstExpr *res = NULL;
    vec_pop_range(&stack->stack, 2);

    res = s1.term.type == T_IDENT
        ? sem_variable(s1.pos, s1.term.ident)
        : sem_literal(s1.pos, s1.term);

    if (!res) {
        return MATCH_ERR;
    }

    VEC_PUSH(&stack->stack, struct StackItem, ((struct StackItem) {
        .type = SI_NTERM,
        .nterm = res,
    }));

    return MATCH_OK;
}

static enum FoldResult es_fold_bracket(struct ExpansionStack *stack) {
    if (stack->stack.len < 5) {
        return NO_MATCH;
    }

    struct StackItem *si = &VEC_AT(&stack->stack, struct StackItem, 0);
    size_t sl = stack->stack.len;

    struct StackItem s0 = si[sl - 4];
    struct StackItem s1 = si[sl - 3];
    struct StackItem s2 = si[sl - 2];
    struct StackItem s3 = si[sl - 1];

    if (s1.type != SI_TERM || s1.term.type != '(' || s2.type != SI_NTERM
        || s3.type != SI_TERM || s3.term.type != ')' || s0.type != SI_STOP
    ) {
        return NO_MATCH;
    }

    vec_pop_range(&stack->stack, 4);
    VEC_PUSH(&stack->stack, struct StackItem, s2);

    return MATCH_OK;
}

static enum FoldResult es_fold_call(struct ExpansionStack *stack) {
    if (stack->stack.len < 4) {
        return NO_MATCH;
    }

    struct StackItem *si = &VEC_AT(&stack->stack, struct StackItem, 0);
    size_t sl = stack->stack.len;

    struct StackItem s0 = si[sl - 3];
    struct StackItem s1 = si[sl - 2];
    struct StackItem s2 = si[sl - 1];

    if (s1.type != SI_NTERM || s2.type != SI_CALL_PARAMS
        || s0.type != SI_STOP
    ) {
        return NO_MATCH;
    }

    vec_pop_range(&stack->stack, 3);

    AstExpr *res = sem_call(s2.pos, s1.nterm, s2.call_params);

    if (!res) {
        return MATCH_ERR;
    }

    VEC_PUSH(&stack->stack, struct StackItem, ((struct StackItem) {
        .type = SI_NTERM,
        .nterm = res,
    }));

    return MATCH_OK;
}

static enum FoldResult es_fold_binary(struct ExpansionStack *stack) {
    if (stack->stack.len < 5) {
        return NO_MATCH;
    }

    struct StackItem *si = &VEC_AT(&stack->stack, struct StackItem, 0);
    size_t sl = stack->stack.len;

    struct StackItem s0 = si[sl - 4];
    struct StackItem s1 = si[sl - 3];
    struct StackItem s2 = si[sl - 2];
    struct StackItem s3 = si[sl - 1];

    if (s1.type != SI_NTERM || s2.type != SI_TERM || s3.type != SI_NTERM
        || s0.type != SI_STOP
    ) {
        return NO_MATCH;
    }

    vec_pop_range(&stack->stack, 4);

    AstExpr *res = sem_binary(s2.pos, s1.nterm, s2.term.type, s3.nterm);

    if (!res) {
        return MATCH_ERR;
    }

    VEC_PUSH(&stack->stack, struct StackItem, ((struct StackItem) {
        .type = SI_NTERM,
        .nterm = res,
    }));

    return MATCH_OK;
}

static enum FoldResult es_fold_prefix(struct ExpansionStack *stack) {
    if (stack->stack.len < 4) {
        return NO_MATCH;
    }

    struct StackItem *si = &VEC_AT(&stack->stack, struct StackItem, 0);
    size_t sl = stack->stack.len;

    struct StackItem s0 = si[sl - 3];
    struct StackItem s1 = si[sl - 2];
    struct StackItem s2 = si[sl - 1];

    if (s1.type != SI_TERM || s2.type != SI_NTERM || s0.type != SI_STOP) {
        return NO_MATCH;
    }

    vec_pop_range(&stack->stack, 3);

    AstExpr *res = sem_unary(s1.pos, s2.nterm, s1.term.type);

    if (!res) {
        return MATCH_ERR;
    }

    VEC_PUSH(&stack->stack, struct StackItem, ((struct StackItem) {
        .type = SI_NTERM,
        .nterm = res,
    }));

    return MATCH_OK;
}

static enum FoldResult es_fold_postfix(struct ExpansionStack *stack) {
    if (stack->stack.len < 4) {
        return NO_MATCH;
    }

    struct StackItem *si = &VEC_AT(&stack->stack, struct StackItem, 0);
    size_t sl = stack->stack.len;

    struct StackItem s0 = si[sl - 3];
    struct StackItem s1 = si[sl - 2];
    struct StackItem s2 = si[sl - 1];

    if (s1.type != SI_NTERM || s2.type != SI_TERM || s0.type != SI_STOP) {
        return NO_MATCH;
    }

    vec_pop_range(&stack->stack, 3);

    AstExpr *res = sem_unary(s2.pos, s1.nterm, s2.term.type);

    if (!res) {
        return MATCH_ERR;
    }

    VEC_PUSH(&stack->stack, struct StackItem, ((struct StackItem) {
        .type = SI_NTERM,
        .nterm = res,
    }));

    return MATCH_OK;
}

static bool es_call(struct ExpansionStack *stack, Parser *par) {
    if (!es_fold(stack)) {
        return false;
    }

    VEC_INSERT(
        &stack->stack,
        struct StackItem,
        stack->stack.len - 2,
        ((struct StackItem) {
            .type = SI_STOP,
        })
    );

    Vec params = VEC_NEW(AstFuncCallParam);

    if (!parse_func_params(par, &params)) {
        return false;
    }

    VEC_INSERT(
        &stack->stack,
        struct StackItem,
        stack->stack.len - 2,
        ((struct StackItem) {
            .type = SI_CALL_PARAMS,
            .call_params = params,
        })
    );

    return es_fold(stack);
}

static AstExpr *es_finish(struct ExpansionStack *stack) {
    while (stack->stack.len > 2) {
        if (!es_fold(stack)) {
            return NULL;
        }
    }
    struct StackItem last = VEC_POP(&stack->stack, struct StackItem);
    if (last.type != SI_NTERM) {
        return NULL;
    }
    return last.nterm;
}

static void es_free(struct ExpansionStack *stack) {
    vec_free_with(&stack->stack, (FreeFun)si_free);
}

static void si_free(struct StackItem *si) {
    switch (si->type) {
    case SI_STOP:
        break;
    case SI_TERM:
        if (si->term.type == T_LITERAL && si->term.subtype == DT_STRING) {
            str_free(&si->term.str);
        }
        break;
    case SI_NTERM:
        ast_free_expr(&si->nterm);
        break;
    case SI_CALL_PARAMS:
        vec_free_with(&si->call_params, (FreeFun)ast_free_func_call_param);
        break;
    }
}

static bool is_infix_token(Token t) {
    return t == '!' || t == '*' || t == '/' || t == '+' || t == '-'
        || t == T_EQUALS || t == T_DIFFERS || t == '<' || t == '>'
        || t == T_LESS_OR_EQUAL || t == T_GREATER_OR_EQUAL
        || t == T_DOUBLE_QUES || t == '=' || t == '(' || t == ')'
        || is_value_token(t);
}

static bool is_value_token(Token t) {
    return t == T_IDENT || t == T_LITERAL;
}

static size_t es_top_index(struct ExpansionStack *stack, enum StackItemType type) {
    size_t i = stack->stack.len - 1;
    while (VEC_AT(&stack->stack, struct StackItem, i).type != type) {
        if (i == 0) {
            return SIZE_MAX;
        }
        --i;
    }
    return i;
}
