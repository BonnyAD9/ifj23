#ifndef SEMANTICS_H_INCLUDED
#define SEMANTICS_H_INCLUDED

#include "stdbool.h"
#include "string.h"
#include "ast.h"
#include "errors.h"
#include "vec.h"
#include "utils.h"

bool check_statement(AstStmt *stmt);

typedef struct {
    bool in_func;
    bool in_main;
    bool in_while;
    bool in_if;
    bool return_used;
    AstFuncType func_type;
} Context;

                    // INT NOT_NIL DOUBLE NOT_NIL STRING NOT_NIL NIL NOT_NIL
// INT              |
// INT_NOT_NIL      |
// DOUBLE           |
// DOUBLE_NOT_NIL   |
// STRING           |
// STRING_NOT_NIL   |
// NIL              |
// NOT_NIL          |

bool compat_array[6][8][8] = {
    // *********************** '*', '-', '/' ***********************
    // INT   NOT_NIL  DOUBLE NOT_NIL  STRING NOT_NIL   NIL   NOT_NIL
   {{   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // INT
    {   0   ,   1   ,   0   ,   1   ,   0   ,   0   ,   0   ,   0   }, // INT_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // DOUBLE
    {   0   ,   1   ,   0   ,   1   ,   0   ,   0   ,   0   ,   0   }, // DOUBLE_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // STRING
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // STRING_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }},// NIL_NOT_NIL

    // **************************** '+' ****************************
    // INT   NOT_NIL  DOUBLE NOT_NIL  STRING NOT_NIL   NIL   NOT_NIL
   {{   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // INT
    {   0   ,   1   ,   0   ,   1   ,   0   ,   0   ,   0   ,   0   }, // INT_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // DOUBLE
    {   0   ,   1   ,   0   ,   1   ,   0   ,   0   ,   0   ,   0   }, // DOUBLE_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // STRING
    {   0   ,   0   ,   0   ,   0   ,   0   ,   1   ,   0   ,   0   }, // STRING_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }},// NIL_NOT_NIL

    // ******************* '==', '!=', '<=', '=>' ******************
    // INT   NOT_NIL  DOUBLE NOT_NIL  STRING NOT_NIL   NIL   NOT_NIL
   {{   1   ,   0   ,   0   ,   0   ,   0   ,   0   ,   1   ,   0   }, // INT
    {   0   ,   1   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // INT_NOT_NIL
    {   0   ,   0   ,   1   ,   0   ,   0   ,   0   ,   1   ,   0   }, // DOUBLE
    {   0   ,   1   ,   0   ,   1   ,   0   ,   0   ,   0   ,   0   }, // DOUBLE_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   1   ,   0   ,   1   ,   0   }, // STRING
    {   0   ,   0   ,   0   ,   0   ,   0   ,   1   ,   0   ,   0   }, // STRING_NOT_NIL
    {   1   ,   0   ,   1   ,   0   ,   1   ,   0   ,   1   ,   0   }, // NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   1   }},// NIL_NOT_NIL

    // *************************** '??' ****************************
    // INT   NOT_NIL  DOUBLE NOT_NIL  STRING NOT_NIL   NIL   NOT_NIL
   {{   0   ,   1   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // INT
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // INT_NOT_NIL
    {   0   ,   0   ,   0   ,   1   ,   0   ,   0   ,   0   ,   0   }, // DOUBLE
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // DOUBLE_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   1   ,   0   ,   0   }, // STRING
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // STRING_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   1   }, // NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }},// NIL_NOT_NIL

    // **************************** '=' ****************************
    // INT   NOT_NIL  DOUBLE NOT_NIL  STRING NOT_NIL   NIL   NOT_NIL
   {{   1   ,   1   ,   0   ,   0   ,   0   ,   0   ,   1   ,   1   }, // INT
    {   0   ,   1   ,   0   ,   0   ,   0   ,   0   ,   0   ,   1   }, // INT_NOT_NIL
    {   0   ,   0   ,   1   ,   1   ,   0   ,   0   ,   1   ,   1   }, // DOUBLE
    {   0   ,   0   ,   0   ,   1   ,   0   ,   0   ,   0   ,   1   }, // DOUBLE_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   1   ,   1   ,   1   ,   1   }, // STRING
    {   0   ,   0   ,   0   ,   0   ,   0   ,   1   ,   0   ,   1   }, // STRING_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }},// NIL_NOT_NIL

    // ************************* '<', '>' **************************
    // INT   NOT_NIL  DOUBLE NOT_NIL  STRING NOT_NIL   NIL   NOT_NIL
   {{   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // INT
    {   0   ,   1   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // INT_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // DOUBLE
    {   0   ,   0   ,   0   ,   1   ,   0   ,   0   ,   0   ,   0   }, // DOUBLE_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // STRING
    {   0   ,   0   ,   0   ,   0   ,   0   ,   1   ,   0   ,   0   }, // STRING_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }, // NIL
    {   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   }} // NIL_NOT_NIL
};

/* UNKNOWN.........0
   INT.............1
   INT_NOT_NIL.....2
   DOUBLE..........3
   DOUBLE_NOT_NIL..4
   STRING..........5
   STRING_NOT_NIL..6
   NIL.............7
   NIL_NOT_NIL.....8 */

int final_type_arr[8][8] = {
    // INT   NOT_NIL  DOUBLE NOT_NIL  STRING NOT_NIL   NIL   NOT_NIL
    {   1   ,   1   ,   3   ,   3   ,   0   ,   0   ,   1   ,   1   }, // INT
    {   1   ,   2   ,   3   ,   4   ,   0   ,   0   ,   1   ,   2   }, // INT_NOT_NIL
    {   3   ,   3   ,   3   ,   3   ,   0   ,   0   ,   3   ,   3   }, // DOUBLE
    {   3   ,   4   ,   3   ,   4   ,   0   ,   0   ,   3   ,   4   }, // DOUBLE_NOT_NIL
    {   0   ,   0   ,   0   ,   0   ,   5   ,   5   ,   4   ,   5   }, // STRING
    {   0   ,   0   ,   0   ,   0   ,   5   ,   6   ,   5   ,   6   }, // STRING_NOT_NIL
    {   1   ,   1   ,   3   ,   3   ,   4   ,   5   ,   7   ,   0   }, // NIL
    {   1   ,   2   ,   3   ,   4   ,   5   ,   6   ,   0   ,   8   }// NIL_NOT_NIL
};

#endif // SEMANTICS_H_INCLUDED