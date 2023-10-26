#include "test.h"

char* test_code;

/*===========================================================================*/
TEST(Line_position, "Token position on the LINE")

    LineOrCol line_or_column = LINE;
    test_code = "let\n"
                "a=a\n"
                "??\n"
                "0";
    int correct_position[] = {1, 2, 2, 2, 3, 4};
    code_insert(test_code);
    check_position(line_or_column, correct_position);

ENDTEST
/*===========================================================================*/
TEST(Column_position, "Token position on the COLUMN")

    LineOrCol line_or_column = COLUMN;
    test_code = "let a=a\n"
                "??0";
    int correct_position[] = {1, 5, 6, 7, 1, 4};
    code_insert(test_code);
    check_position(line_or_column, correct_position);

ENDTEST
/*===========================================================================*/
TEST(Comment_test_1, "Testing - //")

    test_code = "// Program 2: Vypocet faktorialu (rekurzivne)\n"
                "// Hlavni telo programu";
    TokenData correct_tokens[] = {{}};
    run_test(correct_tokens, test_code);

ENDTEST
/*===========================================================================*/
TEST(Comment_test_2, "Testing - /**/")

    test_code = "/* Program 3: Prace s retezci a vestavenymi funkcemi */";
    TokenData correct_tokens[] = {{}};
    run_test(correct_tokens, test_code);

ENDTEST
/*===========================================================================*/
TEST(Comment_test_3, "Testing - // and /**/")

    test_code = "// Program 1: Vypocet faktorialu (iterativne)\n"
                "/* Hlavni telo programu */";
    TokenData correct_tokens[] = {{}};
    run_test(correct_tokens, test_code);

ENDTEST
/*===========================================================================*/
TEST(Comment_test_4, "Testing - Tokens with //")

    test_code = "return ret // Vracení se zpět";
    TokenData correct_tokens[] = { 
        {.enum_type = STRING, .cur = T_RETURN, .str = "return"},
        {.enum_type = STRING, .cur = T_IDENT, .str = "ret"},
        };
    run_test(correct_tokens, test_code);

ENDTEST
/*===========================================================================*/
TEST(Comment_test_5, "Testing - Tokens with /**/")

    test_code = "return /* Vracení se zpět */ ret";
    TokenData correct_tokens[] = { 
        {.enum_type = STRING, .cur = T_RETURN, .str = "return"},
        {.enum_type = STRING, .cur = T_IDENT, .str = "ret"},
        };
    run_test(correct_tokens, test_code);

ENDTEST
/*===========================================================================*/
TEST(Comment_test_6, "Testing - Tokens with nested comments")

    test_code = "let res = /* vnejsi/*ve Swift warning*/ vnejsi*/";
    TokenData correct_tokens[] = { 
        {.enum_type = STRING, .cur = T_DECL, .str = "let"},
        {.enum_type = STRING, .cur = T_IDENT, .str = "res"},
        {.enum_type = ENUM_VALUE, .cur = '='}
        };
    run_test(correct_tokens, test_code);

ENDTEST 
/*===========================================================================*/
TEST(String_write_1, "write(\"Zadejte cislo pro vypocet faktorialu:\\n\")")

    test_code = "write(\"Zadejte cislo pro vypocet faktorialu:\\n\")";
    TokenData correct_tokens[] = {
        {.enum_type = STRING, .cur = T_IDENT, .str = "write"},
        {.enum_type = ENUM_VALUE, .cur = '('},
        {.enum_type = STRING, .cur = T_SLIT, .str = "Zadejte cislo pro vypocet faktorialu:\n"},
        {.enum_type = ENUM_VALUE, .cur = ')'},
        };
    run_test(correct_tokens, test_code);

ENDTEST
/*===========================================================================*/
TEST(String_write_2, "write(\"Ahoj\\n\\\"Sve'te \\\\\\u{22}\")")

    test_code = "write(\"Ahoj\\n\\\"Sve'te \\\\\\u{22}\")";
    TokenData correct_tokens[] = {
        {.enum_type = STRING, .cur = T_IDENT, .str = "write"},
        {.enum_type = ENUM_VALUE, .cur = '('},
        {.enum_type = STRING, .cur = T_SLIT, .str = "Ahoj\n\"Sve'te \\\""},
        {.enum_type = ENUM_VALUE, .cur = ')'},
        };
    run_test(correct_tokens, test_code);

ENDTEST
/*===========================================================================*/
TEST(String_write_3, "write(str1, \"\\n\", str2, \"\\n\")")

    test_code = "write(str1, \"\\n\", str2, \"\\n\")";
    TokenData correct_tokens[] = {
        {.enum_type = STRING, .cur = T_IDENT, .str = "write"},
        {.enum_type = ENUM_VALUE, .cur = '('},
        {.enum_type = STRING, .cur = T_IDENT, .str = "str1"},
        {.enum_type = ENUM_VALUE, .cur = ','},
        {.enum_type = STRING, .cur = T_SLIT, .str = "\n"},
        {.enum_type = ENUM_VALUE, .cur = ','},
        {.enum_type = STRING, .cur = T_IDENT, .str = "str2"},
        {.enum_type = ENUM_VALUE, .cur = ','},
        {.enum_type = STRING, .cur = T_SLIT, .str = "\n"},
        {.enum_type = ENUM_VALUE, .cur = ')'},
        };
    run_test(correct_tokens, test_code);
    
ENDTEST
/*===========================================================================*/
TEST(String_write_4, "write(\"Pozice retezce \\\"text\\\" v str2: \", i, \"\\n\")")

    test_code = "write(\"Pozice retezce \\\"text\\\" v str2: \", i, \"\\n\")";
    TokenData correct_tokens[] = {
        {.enum_type = STRING, .cur = T_IDENT, .str = "write"},
        {.enum_type = ENUM_VALUE, .cur = '('},
        {.enum_type = STRING, .cur = T_SLIT, .str = "Pozice retezce \"text\" v str2: "},
        {.enum_type = ENUM_VALUE, .cur = ','},
        {.enum_type = STRING, .cur = T_IDENT, .str = "i"},
        {.enum_type = ENUM_VALUE, .cur = ','},
        {.enum_type = STRING, .cur = T_SLIT, .str = "\n"},
        {.enum_type = ENUM_VALUE, .cur = ')'},
        };
    run_test(correct_tokens, test_code);
    
ENDTEST
/*===========================================================================*/
TEST(Tripple_quotes, "Testing tripple quote string")

    test_code = "let tq = \"\"\"\n"
    "   triple quote test\n"
    "   \"\"\"";
    TokenData correct_tokens[] = {
        {.enum_type = STRING, .cur = T_DECL, .str = "let"},
        {.enum_type = STRING, .cur = T_IDENT, .str = "tq"},
        {.enum_type = ENUM_VALUE, .cur = '='},
        {.enum_type = STRING, .cur = T_SLIT, .str = "triple quote test"},
        };
    run_test(correct_tokens, test_code);
    
ENDTEST 
/*===========================================================================*/
TEST(Conditions_1, "if let a {")

    test_code = "if let a {";
    TokenData correct_tokens[] = {
        {.enum_type = STRING, .cur = T_IF, .str = "if"},
        {.enum_type = STRING, .cur = T_DECL, .str = "let"},
        {.enum_type = STRING, .cur = T_IDENT, .str = "a"},
        {.enum_type = ENUM_VALUE, .cur = '{'}
        };
    run_test(correct_tokens, test_code);
    
ENDTEST
/*===========================================================================*/
TEST(Conditions_2, "if (a <= 0.0)")

    test_code = "if (a <= 0.0)";
    TokenData correct_tokens[] = {
        {.enum_type = STRING, .cur = T_IF, .str = "if"},
        {.enum_type = ENUM_VALUE, .cur = '('},
        {.enum_type = STRING, .cur = T_IDENT, .str = "a"},
        {.enum_type = ENUM_VALUE, .cur = T_LESS_OR_EQUAL},
        {.enum_type = NUMBER, .cur = T_DLIT, .d_num = 0.0},
        {.enum_type = ENUM_VALUE, .cur = ')'}
        };
    run_test(correct_tokens, test_code);
    
ENDTEST
/*===========================================================================*/
TEST(Conditions_3, "if (a == 1.5)")

    test_code = "if (a == 1.5)";
    TokenData correct_tokens[] = {
        {.enum_type = STRING, .cur = T_IF, .str = "if"},
        {.enum_type = ENUM_VALUE, .cur = '('},
        {.enum_type = STRING, .cur = T_IDENT, .str = "a"},
        {.enum_type = ENUM_VALUE, .cur = T_EQUALS},
        {.enum_type = NUMBER, .cur = T_DLIT, .d_num = 1.5},
        {.enum_type = ENUM_VALUE, .cur = ')'}
        };
    run_test(correct_tokens, test_code);
    
ENDTEST
/*===========================================================================*/
TEST(Conditions_4, "else {")

    test_code = "else {";
    TokenData correct_tokens[] = {
        {.enum_type = STRING, .cur = T_ELSE, .str = "else"},
        {.enum_type = ENUM_VALUE, .cur = '{'}
        };
    run_test(correct_tokens, test_code);
    
ENDTEST
/*===========================================================================*/
TEST(Return_1, "return result")

    test_code = "return result";
    TokenData correct_tokens[] = { 
        {.enum_type = STRING, .cur = T_RETURN, .str = "return"},
        {.enum_type = STRING, .cur = T_IDENT, .str = "result"}
        };
    run_test(correct_tokens, test_code);
    
ENDTEST
/*===========================================================================*/
TEST(Return_2, "return x + \" \" + y")

    test_code = "return x + \" \" + y";
    TokenData correct_tokens[] = { 
        {.enum_type = STRING, .cur = T_RETURN, .str = "return"},
        {.enum_type = STRING, .cur = T_IDENT, .str = "x"},
        {.enum_type = ENUM_VALUE, .cur = '+'},
        {.enum_type = STRING, .cur = T_SLIT, .str = " "},
        {.enum_type = ENUM_VALUE, .cur = '+'},
        {.enum_type = STRING, .cur = T_IDENT, .str = "y"}
        };
    run_test(correct_tokens, test_code);
    
ENDTEST
/*===========================================================================*/
TEST(Cycles_1, "while (a >= 0) {")

    test_code = "while (a >= 0) {";
    TokenData correct_tokens[] = { 
        {.enum_type = STRING, .cur = T_WHILE, .str = "while"},
        {.enum_type = ENUM_VALUE, .cur = '('},
        {.enum_type = STRING, .cur = T_IDENT, .str = "a"},
        {.enum_type = ENUM_VALUE, .cur = T_GREATER_OR_EQUAL},
        {.enum_type = NUMBER, .cur = T_ILIT, .i_num = 0},
        {.enum_type = ENUM_VALUE, .cur = ')'},
        {.enum_type = ENUM_VALUE, .cur = '{'}
        };
    run_test(correct_tokens, test_code);
    
ENDTEST
/*===========================================================================*/
TEST(Cycles_2, "while (str1 != \"abcdefgh\") {")

    test_code = "while (str1 != \"abcdefgh\") {";
     TokenData correct_tokens[] = { 
        {.enum_type = STRING, .cur = T_WHILE, .str = "while"},
        {.enum_type = ENUM_VALUE, .cur = '('},
        {.enum_type = STRING, .cur = T_IDENT, .str = "str1"},
        {.enum_type = ENUM_VALUE, .cur = T_DIFFERS},
        {.enum_type = STRING, .cur = T_SLIT, .str = "abcdefgh"},
        {.enum_type = ENUM_VALUE, .cur = ')'},
        {.enum_type = ENUM_VALUE, .cur = '{'}
        };
    run_test(correct_tokens, test_code);
    
ENDTEST
/*===========================================================================*/
TEST(Functions_1, "func bar(with param : String) -> String {")

    test_code = "func bar(with param : String) -> String {";
    TokenData correct_tokens[] = {
        {.enum_type = STRING, .cur = T_FUNC, .str = "func"},
        {.enum_type = STRING, .cur = T_IDENT, .str = "bar"},
        {.enum_type = ENUM_VALUE, .cur = '('},
        {.enum_type = STRING, .cur = T_IDENT, .str = "with"},
        {.enum_type = STRING, .cur = T_IDENT, .str = "param"},
        {.enum_type = ENUM_VALUE, .cur = ':'},
        {.enum_type = STRING, .cur = T_TYPE, .str = "String"},
        {.enum_type = ENUM_VALUE, .cur = ')'},
        {.enum_type = ENUM_VALUE, .cur = T_RETURNS},
        {.enum_type = STRING, .cur = T_TYPE, .str = "String"},
        {.enum_type = ENUM_VALUE, .cur = '{'}
        };
    run_test(correct_tokens, test_code);
    
ENDTEST
/*===========================================================================*/
TEST(Functions_2, "func concat(_ x : Double, with y : Double) -> Double {")

    test_code = "func concat(_ x : Double, with y : Double) -> Double {";
    TokenData correct_tokens[] = {
        {.enum_type = STRING, .cur = T_FUNC, .str = "func"},
        {.enum_type = STRING, .cur = T_IDENT, .str = "concat"},
        {.enum_type = ENUM_VALUE, .cur = '('},
        {.enum_type = ENUM_VALUE, .cur = '_'},
        {.enum_type = STRING, .cur = T_IDENT, .str = "x"},
        {.enum_type = ENUM_VALUE, .cur = ':'},
        {.enum_type = STRING, .cur = T_TYPE, .str = "Double"},
        {.enum_type = ENUM_VALUE, .cur = ','},
        {.enum_type = STRING, .cur = T_IDENT, .str = "with"},
        {.enum_type = STRING, .cur = T_IDENT, .str = "y"},
        {.enum_type = ENUM_VALUE, .cur = ':'},
        {.enum_type = STRING, .cur = T_TYPE, .str = "Double"},
        {.enum_type = ENUM_VALUE, .cur = ')'},
        {.enum_type = ENUM_VALUE, .cur = T_RETURNS},
        {.enum_type = STRING, .cur = T_TYPE, .str = "Double"},
        {.enum_type = ENUM_VALUE, .cur = '{'}
        };
    run_test(correct_tokens, test_code);
    
ENDTEST
/*===========================================================================*/
TEST(Function_calls_1, "bar(with: \"ahoj\")")

    test_code = "bar(with: \"ahoj\")";
    TokenData correct_tokens[] = {
        {.enum_type = STRING, .cur = T_IDENT, .str = "bar"},
        {.enum_type = ENUM_VALUE, .cur = '('},
        {.enum_type = STRING, .cur = T_IDENT, .str = "with"},
        {.enum_type = ENUM_VALUE, .cur = ':'},
        {.enum_type = STRING, .cur = T_SLIT, .str = "ahoj"},
        {.enum_type = ENUM_VALUE, .cur = ')'},
        };
    run_test(correct_tokens, test_code);
    
ENDTEST
/*===========================================================================*/
TEST(Function_calls_2, "str1 = readString() ?? \"\"")

    test_code = "str1 = readString() ?? \"\"";
    TokenData correct_tokens[] = {
        {.enum_type = STRING, .cur = T_IDENT, .str = "str1"},
        {.enum_type = ENUM_VALUE, .cur = '='},
        {.enum_type = STRING, .cur = T_IDENT, .str = "readString"},
        {.enum_type = ENUM_VALUE, .cur = '('},
        {.enum_type = ENUM_VALUE, .cur = ')'},
        {.enum_type = ENUM_VALUE, .cur = T_DOUBLE_QUES},
        {.enum_type = STRING, .cur = T_SLIT, .str = ""},
        };
    run_test(correct_tokens, test_code);
    
ENDTEST
/*===========================================================================*/
TEST(Function_calls_3, "let decremented_n = decrement(of: n, by: 1)")

    test_code = "let decremented_n = decrement(of: n, by: 1)";
    TokenData correct_tokens[] = {
        {.enum_type = STRING, .cur = T_DECL, .str = "let"},
        {.enum_type = STRING, .cur = T_IDENT, .str = "decremented_n"},
        {.enum_type = ENUM_VALUE, .cur = '='},
        {.enum_type = STRING, .cur = T_IDENT, .str = "decrement"},
        {.enum_type = ENUM_VALUE, .cur = '('},
        {.enum_type = STRING, .cur = T_IDENT, .str = "of"},
        {.enum_type = ENUM_VALUE, .cur = ':'},
        {.enum_type = STRING, .cur = T_IDENT, .str = "n"},
        {.enum_type = ENUM_VALUE, .cur = ','},
        {.enum_type = STRING, .cur = T_IDENT, .str = "by"},
        {.enum_type = ENUM_VALUE, .cur = ':'},
        {.enum_type = NUMBER, .cur = T_ILIT, .i_num = 1},
        {.enum_type = ENUM_VALUE, .cur = ')'},
        };
    run_test(correct_tokens, test_code);
    
ENDTEST
/*===========================================================================*/
TEST(Function_calls_4, "var a = Int2Double(a)")

    test_code = "var a = Int2Double(a)";
    TokenData correct_tokens[] = {
        {.enum_type = STRING, .cur = T_DECL, .str = "var"},
        {.enum_type = STRING, .cur = T_IDENT, .str = "a"},
        {.enum_type = ENUM_VALUE, .cur = '='},
        {.enum_type = STRING, .cur = T_IDENT, .str = "Int2Double"},
        {.enum_type = ENUM_VALUE, .cur = '('},
        {.enum_type = STRING, .cur = T_IDENT, .str = "a"},
        {.enum_type = ENUM_VALUE, .cur = ')'},
        };
    run_test(correct_tokens, test_code);
    
ENDTEST
/*===========================================================================*/
TEST(Function_calls_5, "ct = concat(a, with: \"svete\")")

    test_code = "ct = concat(a, with: \"svete\")";
    TokenData correct_tokens[] = {
        {.enum_type = STRING, .cur = T_IDENT, .str = "ct"},
        {.enum_type = ENUM_VALUE, .cur = '='},
        {.enum_type = STRING, .cur = T_IDENT, .str = "concat"},
        {.enum_type = ENUM_VALUE, .cur = '('},
        {.enum_type = STRING, .cur = T_IDENT, .str = "a"},
        {.enum_type = ENUM_VALUE, .cur = ','},
        {.enum_type = STRING, .cur = T_IDENT, .str = "with"},
        {.enum_type = ENUM_VALUE, .cur = ':'},
        {.enum_type = STRING, .cur = T_SLIT, .str = "svete"},
        {.enum_type = ENUM_VALUE, .cur = ')'},
        };
    run_test(correct_tokens, test_code);
    
ENDTEST
/*===========================================================================*/
TEST(Variables_var_1, "var str1 = \"Toto je nejaky text\"")

    test_code = "var str1 = \"Toto je nejaky text\"";
    TokenData correct_tokens[] = {
        {.enum_type = STRING, .cur = T_DECL, .str = "var"},
        {.enum_type = STRING, .cur = T_IDENT, .str = "str1"},
        {.enum_type = ENUM_VALUE, .cur = '='},
        {.enum_type = STRING, .cur = T_SLIT, .str = "Toto je nejaky text"}
        };
    run_test(correct_tokens, test_code);
    
ENDTEST
/*===========================================================================*/
TEST(Variables_var_2, "var vysl : Double = 1")

    test_code = "var vysl : Double = 1";
    TokenData correct_tokens[] = {
        {.enum_type = STRING, .cur = T_DECL, .str = "var"},
        {.enum_type = STRING, .cur = T_IDENT, .str = "vysl"},
        {.enum_type = ENUM_VALUE, .cur = ':'},
        {.enum_type = STRING, .cur = T_TYPE, .str = "Double"},
        {.enum_type = ENUM_VALUE, .cur = '='},
        {.enum_type = NUMBER, .cur = T_ILIT, .i_num = 1},
        };
    run_test(correct_tokens, test_code);
    
ENDTEST
/*===========================================================================*/
TEST(Variables_var_3, "var result : Int?")

    test_code = "var result : Int?";
    TokenData correct_tokens[] = {
        {.enum_type = STRING, .cur = T_DECL, .str = "var"},
        {.enum_type = STRING, .cur = T_IDENT, .str = "result"},
        {.enum_type = ENUM_VALUE, .cur = ':'},
        {.enum_type = STRING, .cur = T_TYPE, .str = "Int"},
        {.enum_type = ENUM_VALUE, .cur = '?'},
        };
    run_test(correct_tokens, test_code);
    
ENDTEST
/*===========================================================================*/
TEST(Variables_var_4, "var : ct String")

    test_code = "var : ct String";
    TokenData correct_tokens[] = {
        {.enum_type = STRING, .cur = T_DECL, .str = "var"},
        {.enum_type = ENUM_VALUE, .cur = ':'},
        {.enum_type = STRING, .cur = T_IDENT, .str = "ct"},
        {.enum_type = STRING, .cur = T_TYPE, .str = "String"}
        };
    run_test(correct_tokens, test_code);
    
ENDTEST
/*===========================================================================*/
TEST(Variables_var_5, "var myVariable: Int? = nil")

    test_code = "var myVariable: Int? = nil";
    TokenData correct_tokens[] = {
        {.enum_type = STRING, .cur = T_DECL, .str = "var"},
        {.enum_type = STRING, .cur = T_IDENT, .str = "myVariable"},
        {.enum_type = ENUM_VALUE, .cur = ':'},
        {.enum_type = STRING, .cur = T_TYPE, .str = "Int"},
        {.enum_type = ENUM_VALUE, .cur = '?'},
        {.enum_type = ENUM_VALUE, .cur = '='},
        {.enum_type = STRING, .cur = T_NIL, .str = "nil"},
        };
    run_test(correct_tokens, test_code);
    
ENDTEST
/*===========================================================================*/
TEST(Variables_let1, "let str2 = str1 + \", ktery jeste trochu obohatime\"")

    test_code = "let str2 = str1 + \", ktery jeste trochu obohatime\"";
    TokenData correct_tokens[] = {
        {.enum_type = STRING, .cur = T_DECL, .str = "let"},
        {.enum_type = STRING, .cur = T_IDENT, .str = "str2"},
        {.enum_type = ENUM_VALUE, .cur = '='},
        {.enum_type = STRING, .cur = T_IDENT, .str = "str1"},
        {.enum_type = ENUM_VALUE, .cur = '+'},
        {.enum_type = STRING, .cur = T_SLIT, .str = ", ktery jeste trochu obohatime"}
        };
    run_test(correct_tokens, test_code);
    
ENDTEST
/*===========================================================================*/
TEST(Variables_let2, "let r : String = foo(param)")

    test_code = "let r : String = foo(param)";
    TokenData correct_tokens[] = {
        {.enum_type = STRING, .cur = T_DECL, .str = "let"},
        {.enum_type = STRING, .cur = T_IDENT, .str = "r"},
        {.enum_type = ENUM_VALUE, .cur = ':'},
        {.enum_type = STRING, .cur = T_TYPE, .str = "String"},
        {.enum_type = ENUM_VALUE, .cur = '='},
        {.enum_type = STRING, .cur = T_IDENT, .str = "foo"},
        {.enum_type = ENUM_VALUE, .cur = '('},
        {.enum_type = STRING, .cur = T_IDENT, .str = "param"},
        {.enum_type = ENUM_VALUE, .cur = ')'},
        };
    run_test(correct_tokens, test_code);
    
ENDTEST
/*===========================================================================*/
TEST(Variables_let3, "let ret = bar(with: par)")

    test_code = "let ret = bar(with: par)";
    TokenData correct_tokens[] = {
        {.enum_type = STRING, .cur = T_DECL, .str = "let"},
        {.enum_type = STRING, .cur = T_IDENT, .str = "ret"},
        {.enum_type = ENUM_VALUE, .cur = '='},
        {.enum_type = STRING, .cur = T_IDENT, .str = "bar"},
        {.enum_type = ENUM_VALUE, .cur = '('},
        {.enum_type = STRING, .cur = T_IDENT, .str = "with"},
        {.enum_type = ENUM_VALUE, .cur = ':'},
        {.enum_type = STRING, .cur = T_IDENT, .str = "par"},
        {.enum_type = ENUM_VALUE, .cur = ')'},
        };
    run_test(correct_tokens, test_code);
    
ENDTEST
/*===========================================================================*/
TEST(Variables_let4, "let x = x + y")

    test_code = "let x = x + y";
    TokenData correct_tokens[] = {
        {.enum_type = STRING, .cur = T_DECL, .str = "let"},
        {.enum_type = STRING, .cur = T_IDENT, .str = "x"},
        {.enum_type = ENUM_VALUE, .cur = '='},
        {.enum_type = STRING, .cur = T_IDENT, .str = "x"},
        {.enum_type = ENUM_VALUE, .cur = '+'},
        {.enum_type = STRING, .cur = T_IDENT, .str = "y"},
        };
    run_test(correct_tokens, test_code);

ENDTEST
/*===========================================================================*/
TEST(Variables_let5, "let a : Int? = readInt()")

    test_code = "let a : Int? = readInt()";
    TokenData correct_tokens[] = {
        {.enum_type = STRING, .cur = T_DECL, .str = "let"},
        {.enum_type = STRING, .cur = T_IDENT, .str = "a"},
        {.enum_type = ENUM_VALUE, .cur = ':'},
        {.enum_type = STRING, .cur = T_TYPE, .str = "Int"},
        {.enum_type = ENUM_VALUE, .cur = '?'},
        {.enum_type = ENUM_VALUE, .cur = '='},
        {.enum_type = STRING, .cur = T_IDENT, .str = "readInt"},
        {.enum_type = ENUM_VALUE, .cur = '('},
        {.enum_type = ENUM_VALUE, .cur = ')'},
        };
    run_test(correct_tokens, test_code);
    
ENDTEST
/*===========================================================================*/

// Jednotlivé testy
void (*lex_tests[])(void) = {
    Line_position,
    Column_position,
    Comment_test_1,
    Comment_test_2,
    Comment_test_3,
    Comment_test_4,
    Comment_test_5,
    Comment_test_6,
    Tripple_quotes,
    String_write_1,
    String_write_2,
    String_write_3,
    String_write_4,
    Conditions_1,
    Conditions_2,
    Conditions_3,
    Conditions_4,
    Return_1,
    Return_2,
    Cycles_1,
    Cycles_2,
    Functions_1,
    Functions_2,
    Function_calls_1,
    Function_calls_2,
    Function_calls_3,
    Function_calls_4,
    Function_calls_5,
    Variables_var_1,
    Variables_var_2,
    Variables_var_3,
    Variables_var_4,
    Variables_var_5,
    Variables_let1,
    Variables_let2,
    Variables_let3,
    Variables_let4,
    Variables_let5,
};

int main (void) {
    printf(C_YELLOW(
    "============================================================================\n"
    "                             Starting lexer tests\n"
    "============================================================================\n"));

    // Vypíše všechny testy, které se nacházejí v void (*lex_tests[])(void)
    for (int lex_test_id = 0; lex_test_id < LEX_TEST_COUNT; lex_test_id++) {
		lex_tests[lex_test_id]();
	}

    // Smazání testovacího souboru 
    if (remove(TEST_FILE)) {
        EPRINTF("Unable to remove %s.\n", TEST_FILE);
        return EXIT_FAILURE;
    }

    printf(C_YELLOW(
    "============================================================================\n"
    "                             Ending lexer tests\n"
    "============================================================================\n"));

    return EXIT_SUCCESS;
}