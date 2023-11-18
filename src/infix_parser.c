#include "infix_parser.h"

enum StackItemType {
    SI_STOP,
    SI_TERM,
    SI_NTERM,
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

struct StackItem {
    enum StackItemType type;
    union {
        FullToken term;
        AstExpr nterm;
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
/// Folds, parses function call parameters and folds again
static bool es_call(struct ExpansionStack *stack, Parser *par);
/// folds while there is more than single non terminal, returns the last
/// non terminal
static AstExpr *es_finish(struct ExpansionStack *stack);
static void es_free(struct ExpansionStack *stack);
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
//  t  | =   >   >   >   >   >   >   >   >   >   >   >   >   c   >   .   .
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
        case '!':
            return PA_PUSH;
        case '(':
            return PA_CALL;
        }
        if (is_value_token(stack) || !is_infix_token(stack)) {
            return PA_END;
        }
        return PA_FOLD;
    }
    if (!is_infix_token(stack)) {
        if (input == '(' || input == ')') {
            return PA_ERR;
        }
        if (!is_infix_token(stack)) {
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
        .stack = VEC_NEW(struct StackItem),
        .last_term = T_EOF,
        .is_stop_term = true,
        .is_stop_term = true,
    };
}

static bool es_shift(struct ExpansionStack *stack, Parser *par) {
    vec_reserve(&stack->stack, stack->stack.len + 2);

    size_t i = stack->stack.len - 1;
    while (VEC_AT(&stack->stack, struct StackItem, i).type != SI_TERM) {
        --i;
    }

    VEC_INSERT(&stack->stack, struct StackItem, i + 1, ((struct StackItem) {
        .type = SI_STOP,
    }));

    return es_push(stack, par);
}

static bool es_push(struct ExpansionStack *stack, Parser *par) {
    struct StackItem item = {
        .type = SI_TERM,
        .term = {
            .type = par->cur,
        },
    };

    switch (par->cur) {
    case T_IDENT: {
        SymItem *itm = sym_find(par->table, par->lex->str);
        if (!itm) {
            return false;
        }
        item.term.ident = itm;
        return true;
    }
    case T_LITERAL:
        switch (par->lex->subtype) {
        case DT_INT:
            item.term.int_v = par->lex->i_num;
            return true;
        case DT_DOUBLE:
            item.term.double_v = par->lex->d_num;
            return true;
        case DT_STRING:
            item.term.str = str_clone(par->lex->str);
            return true;
        }
    }

    return true;
}

// 3 2  1
// E op   -> E (unary operator)
// ( E  ) -> E
//   op E -> E (unary operator)
// E op E -> E (binary operator)
static bool es_fold(struct ExpansionStack *stack) {
    struct StackItem top = VEC_POP(&stack->stack, struct StackItem);
    if (top.type == SI_STOP) {
        return false;
    }
    struct StackItem top2 = VEC_POP(&stack->stack, struct StackItem);
    AstExpr *res = NULL;

    if (top.type == SI_TERM) {
        if (top2.type != SI_NTERM) {
            return false;
        }
        struct StackItem top3 = VEC_POP(&stack->stack, struct StackItem);

        if (top.term.type != ')') {
            if (top3.type != SI_STOP) {
                return false;
            }
            res = sem_unary(top2.nterm, top.term);
        } else if (top3.type != SI_TERM || top3.term.type != '(') {
            return false;
        } else {
            struct StackItem top4 = VEC_POP(&stack->stack, strucz StackItem);
            if (top4 != SI_STOP) {
                return false;
            }
            res = top2;
        }
    }
}

static bool es_call(struct ExpansionStack *stack, Parser *par);

static AstExpr *es_finish(struct ExpansionStack *stack);

static void es_free(struct ExpansionStack *stack);

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
