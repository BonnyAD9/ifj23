#include "test.h"

#define LEX_TEST_COUNT ((long int) (sizeof(lex_tests) / sizeof(void *)))

char* test_code;

TEST(Line_position, "Token position on the LINE")

    test_code = "let\n"
                "a=a\n"
                "??\n"
                "0";
    int correct_position[] = {1, 2, 2, 2, 3, 4};
    code_insert(test_code);
    check_position("line", correct_position);

ENDTEST

TEST(Column_position, "Token position on the COLUMN")

    test_code = "let a=a\n"
                "??0";
    int correct_position[] = {1, 5, 6, 7, 1, 4};
    code_insert(test_code);
    check_position("column", correct_position);

ENDTEST

TEST(Comment_test_1, "Testing - //")

    test_code = "// Program 2: Vypocet faktorialu (rekurzivne)\n"
                "// Hlavni telo programu";
    char *correct_tokens[] = {""};
    run_test(correct_tokens, test_code);

ENDTEST

TEST(Comment_test_2, "Testing - /**/")

    test_code = "/* Program 3: Prace s retezci a vestavenymi funkcemi */";
    char *correct_tokens[] = {""};
    run_test(correct_tokens, test_code);

ENDTEST

TEST(Comment_test_3, "Testing - // and /**/")

    test_code = "// Program 1: Vypocet faktorialu (iterativne)\n"
                "/* Hlavni telo programu */";
    char *correct_tokens[] = {""};
    run_test(correct_tokens, test_code);

ENDTEST

TEST(Comment_test_4, "Testing - Tokens with //")

    test_code = "return ret // Vracení se zpět";
    char *correct_tokens[] = {"return", "ret"};
    run_test(correct_tokens, test_code);

ENDTEST

TEST(Comment_test_5, "Testing - Tokens with /**/")

    test_code = "return ret /* Vracení se zpět */";
    char *correct_tokens[] = {"return", "ret"};
    run_test(correct_tokens, test_code);

ENDTEST

TEST(String_write_1, "write(\"Zadejte cislo pro vypocet faktorialu:\\n\")")

    test_code = "write(\"Zadejte cislo pro vypocet faktorialu:\\n\")";
    char *correct_tokens[] = {"write", "(", 
                              "Zadejte cislo pro vypocet faktorialu:\n", ")"};
    run_test(correct_tokens, test_code);

ENDTEST

TEST(String_write_2, "write(\"Ahoj\\n\\\"Sve'te \\\\\\u{22}\")")

    test_code = "write(\"Ahoj\\n\\\"Sve'te \\\\\\u{22}\")";
    char *correct_tokens[] = {"write", "(", "Ahoj\n\"Sve'te \\\"", ")"};
    run_test(correct_tokens, test_code);

ENDTEST

TEST(String_write_3, "write(str1, \"\\n\", str2, \"\\n\")")

    test_code = "write(str1, \"\\n\", str2, \"\\n\")";
    char *correct_tokens[] = {"write", "(", "str1", ",", "\n", ",", "str2", 
                              ",", "\n", ")"};
    run_test(correct_tokens, test_code);
    
ENDTEST

TEST(String_write_4, "write(\"Pozice retezce \\\"text\\\" v str2: \", i, \"\\n\")")

    test_code = "write(\"Pozice retezce \\\"text\\\" v str2: \", i, \"\\n\")";
    char *correct_tokens[] = {"write", "(", "Pozice retezce \"text\" v str2: ",
                              ",", "i", ",", "\n", ")"};
    run_test(correct_tokens, test_code);
    
ENDTEST

TEST(Conditions_1, "if let a {")

    test_code = "if let a {";
    char *correct_tokens[] = {"if", "let", "a", "{"};
    run_test(correct_tokens, test_code);
    
ENDTEST

TEST(Conditions_2, "if (a < 0)")

    test_code = "if (a < 0)";
    char *correct_tokens[] = {"if", "(", "a", "<", "0", ")"};
    run_test(correct_tokens, test_code);
    
ENDTEST

TEST(Conditions_3, "else {")

    test_code = "else {";
    char *correct_tokens[] = {"else", "{", "=", "x", "+", "y"};
    run_test(correct_tokens, test_code);
    
ENDTEST

TEST(Return_1, "return result")

    test_code = "return result";
    char *correct_tokens[] = {"return", "result"};
    run_test(correct_tokens, test_code);
    
ENDTEST

TEST(Return_2, "return x + \" \" + y")

    test_code = "return x + \" \" + y";
    char *correct_tokens[] = {"return", "x", "+", " ", "+", "y"};
    run_test(correct_tokens, test_code);
    
ENDTEST

TEST(Cycles_1, "while (a > 0) {")

    test_code = "while (a > 0) {";
    char *correct_tokens[] = {"while", "(", "a", ">", "0", ")", "{"};
    run_test(correct_tokens, test_code);
    
ENDTEST

TEST(Cycles_2, "while (str1 != \"abcdefgh\") {")

    test_code = "while (str1 != \"abcdefgh\") {";
    char *correct_tokens[] = {"while", "(", "str1", "!=", "abcdefgh",
                              ")", "{"};
    run_test(correct_tokens, test_code);
    
ENDTEST

TEST(Functions_1, "func bar(with param : String) -> String {")

    test_code = "func bar(with param : String) -> String {";
    char *correct_tokens[] = {"func", "bar", "(", "with", "param", ":",
                              "String", ")", "->", "String", "{"};
    run_test(correct_tokens, test_code);
    
ENDTEST

TEST(Functions_2, "func foo(_ par : String) -> String {")

    test_code = "func foo(_ par : String) -> String {";
    char *correct_tokens[] = {"func", "foo", "(", "_", "par", ":",
                              "String", ")", "->", "String", "{"};
    run_test(correct_tokens, test_code);
    
ENDTEST

TEST(Functions_3, "func concat(_ x : Double, with y : Double) -> Double {")

    test_code = "func concat(_ x : Double, with y : Double) -> Double {";
    char *correct_tokens[] = {"func", "concat", "(", "_", "x", ":", "Double", 
                              ",", "with", "y", ":", "Double", ")", "->", 
                              "Double", "{"};
    run_test(correct_tokens, test_code);
    
ENDTEST

TEST(Functions_4, "func decrement(of n: Int, by m: Int) -> Int {")

    test_code = "func decrement(of n: Int, by m: Int) -> Int {";
    char *correct_tokens[] = {"func", "decrement", "(", "of", "n", ":", "Int",
                              ",", "by", "m", ":", "Int", ")", "->", "Int", "{"};
    run_test(correct_tokens, test_code);
    
ENDTEST

TEST(Function_calls_1, "bar(with: \"ahoj\")")

    test_code = "bar(with: \"ahoj\")";
    char *correct_tokens[] = {"bar", "(", "with", ":", "ahoj", ")"};
    run_test(correct_tokens, test_code);
    
ENDTEST

TEST(Function_calls_2, "str1 = readString() ?? \"\"")

    test_code = "str1 = readString() ?? \"\"";
    char *correct_tokens[] = {"str1", "=", "readString", "(", ")", "??", ""};
    run_test(correct_tokens, test_code);
    
ENDTEST

TEST(Function_calls_3, "let decremented_n = decrement(of: n, by: 1)")

    test_code = "let decremented_n = decrement(of: n, by: 1)";
    char *correct_tokens[] = {"let", "decremented_n", "=", "decrement", "(",
                              "of", ":", "n", ",", "by", ":", "1", ")"};
    run_test(correct_tokens, test_code);
    
ENDTEST

TEST(Function_calls_4, "var a = Int2Double(a)")

    test_code = "var a = Int2Double(a)";
    char *correct_tokens[] = {"var", "a", "=", "Int2Double", "(", "a", ")"};
    run_test(correct_tokens, test_code);
    
ENDTEST

TEST(Function_calls_5, "ct = concat(a, with: \"svete\")")

    test_code = "ct = concat(a, with: \"svete\")";
    char *correct_tokens[] = {"ct", "=", "concat", "(", "a", ",", "with",
                              ":", "svete", ")"};
    run_test(correct_tokens, test_code);
    
ENDTEST

TEST(Variables_var_1, "var str1 = \"Toto je nejaky text v programu jazyka IFJ23\"")

    test_code = "var str1 = \"Toto je nejaky text v programu jazyka IFJ23\"";
    char *correct_tokens[] = {"var", "str1", "=",
                              "Toto je nejaky text v programu jazyka IFJ23"};
    run_test(correct_tokens, test_code);
    
ENDTEST

TEST(Variables_var_2, "var vysl : Double = 1")

    test_code = "var vysl : Double = 1";
    char *correct_tokens[] = {"var", "vysl", ":", "Double", "=", "1"};
    run_test(correct_tokens, test_code);
    
ENDTEST

TEST(Variables_var_3, "var result : Int?")

    test_code = "var result : Int?";
    char *correct_tokens[] = {"var", "result", ":", "Int", "?"};
    run_test(correct_tokens, test_code);
    
ENDTEST

TEST(Variables_var_4, "var ct : String")

    test_code = "var ct : String";
    char *correct_tokens[] = {"var", "ct", ":", "String"};
    run_test(correct_tokens, test_code);
    
ENDTEST

TEST(Variables_let1, "let str2 = str1 + \", ktery jeste trochu obohatime\"")

    test_code = "let str2 = str1 + \", ktery jeste trochu obohatime\"";
    char *correct_tokens[] = {"let", "str2", "=", "str1", "+",
                              ", ktery jeste trochu obohatime"};
    run_test(correct_tokens, test_code);
    
ENDTEST

TEST(Variables_let2, "let r : String = foo(param)")

    test_code = "let r : String = foo(param)";
    char *correct_tokens[] = {"let", "r", ":", "String", "=", "foo", "(",
                              "param", ")"};
    run_test(correct_tokens, test_code);
    
ENDTEST

TEST(Variables_let3, "let ret = bar(with: par)")

    test_code = "let ret = bar(with: par)";
    char *correct_tokens[] = {"let", "ret", "=", "bar", "(", "with", ":",
                              "par", ")"};
    run_test(correct_tokens, test_code);
    
ENDTEST

TEST(Variables_let4, "let x = x + y")
    test_code = "let x = x + y";
    char *correct_tokens[] = {"let", "x", "=", "x", "+", "y"};
    run_test(correct_tokens, test_code);
ENDTEST

TEST(Variables_let5, "let a : Int? = readInt()")

    test_code = "let a : Int? = readInt()";
    char *correct_tokens[] = {"let", "a", ":", "Int", "?", "=", "readInt",
                              "(", ")"};
    run_test(correct_tokens, test_code);
    
ENDTEST

void (*lex_tests[])(void) = {
    Line_position,
    Column_position,
    Comment_test_1,
    Comment_test_2,
    Comment_test_3,
    Comment_test_4,
    Comment_test_5,
    String_write_1,
    String_write_2,
    String_write_3,
    String_write_4,
    Conditions_1,
    Conditions_2,
    Conditions_3,
    Return_1,
    Return_2,
    Cycles_1,
    Cycles_2,
    Functions_1,
    Functions_2,
    Functions_3,
    Functions_4,
    Function_calls_1,
    Function_calls_2,
    Function_calls_3,
    Function_calls_4,
    Function_calls_5,
    Variables_var_1,
    Variables_var_2,
    Variables_var_3,
    Variables_var_4,
    Variables_let1,
    Variables_let2,
    Variables_let3,
    Variables_let4,
    Variables_let5,
};

int main (void) {
    printf("\033[0;33m");
    printf("============================================================================\n");
    printf("                           Starting lexer tests\n");
    printf("============================================================================\n\n");
    printf ("\033[0m");

    for (int lex_test_id = 0; lex_test_id < LEX_TEST_COUNT; lex_test_id++) {
		lex_tests[lex_test_id]();
	}

    if (remove(TEST_FILE)) {
        EPRINTF("Unable to remove %s.\n", TEST_FILE);
    }

    printf("\033[0;33m");
    printf("============================================================================\n");
    printf("                             Ending lexer tests\n");
    printf("============================================================================\n");
    printf ("\033[0m");
    return 0;
}