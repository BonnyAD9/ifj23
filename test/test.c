#include "test.h"
#include "test_utils.h"

#define LEX_TEST_COUNT ((long int) (sizeof(lex_tests) / sizeof(void *)))

char* test_code;

TEST(Line_location, "Token location on the LINE")
    test_code = "let \n"
                "a=a\n"
                "??\n"
                "0";
    code_insert(test_code);
    print_location(1);
ENDTEST

TEST(Column_location, "Token location on the COLUMN")
    test_code = "let a=a\n"
                "??0";
    code_insert(test_code);
    print_location(0);
ENDTEST

TEST(Comment_test1, "Testing - //")
    test_code = "// Program 2: Vypocet faktorialu (rekurzivne)\n"
                "// Hlavni telo programu";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Comment_test2, "Testing - /**/")
    test_code = "/* Program 3: Prace s retezci a vestavenymi funkcemi */";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Comment_test3, "Testing - // and /**/")
    test_code = "// Program 1: Vypocet faktorialu (iterativne)\n"
                "/* Hlavni telo programu */";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Comment_test4, "Testing - Tokens with //")
    test_code = "return ret // Vracení se zpět";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Comment_test5, "Testing - Tokens with /**/")
    test_code = "return ret /* Vracení se zpět */";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(String_write1, "Basic string print using write()")
    test_code = "write(\"Zadejte cislo pro vypocet faktorialu:\\n\")";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(String_write2, "Advanced string print using write()")
    test_code = "write(\"Ahoj\\n\\\"Sve'te \\\\\\u{22}\")";
    // write("Ahoj\n\"Sve'te \\\u{22}")
    code_insert(test_code);
    print_token();
ENDTEST

TEST(String_write3, "Advanced string print with variable using write()")
    test_code = "write(str1, \"\\n\", str2, \"\\n\")";
    // write(str1, "\n", str2, "\n")
    code_insert(test_code);
    print_token();
ENDTEST

TEST(String_write4, "Advanced string print with variable using write()")
    test_code = "write(\"Pozice retezce \\\"text\\\" v str2: \", i, \"\\n\")";
    // write("Pozice retezce \"text\" v str2: ", i, "\n")
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Conditions1, "If condition test")
    test_code = "if let a {";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Conditions2, "If condition test")
    test_code = "if (a < 0)";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Conditions3, "If condition test")
    test_code = "else {";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Return1, "Basic return test")
    test_code = "return result";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Return2, "Advanced return test")
    test_code = "return x + \" \" + y";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Cycles1, "Basic cycle test")
    test_code = "while (a > 0) {";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Cycles2, "Advanced cycle test")
    test_code = "while (str1 != \"abcdefgh\") {";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Functions1, "Basic function test")
    test_code = "func bar(with param : String) -> String {";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Functions2, "Basic function test")
    test_code = "func foo(_ par : String) -> String {";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Functions3, "Basic combined function test")
    test_code = "func concat(_ x : Double, with y : Double) -> Double {";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Functions4, "Basic combined function test")
    test_code = "func decrement(of n: Int, by m: Int) -> Int {";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Function_calls1, "Basic function call test")
    test_code = "bar(with: \"ahoj\")";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Function_calls2, "Basic function call test")
    test_code = "str1 = readString() ?? \"\"";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Function_calls3, "Basic function call test")
    test_code = "let decremented_n = decrement(of: n, by: 1)";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Function_calls4, "Basic function call test")
    test_code = "var a = Int2Double(a)";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Function_calls5, "Basic function call test")
    test_code = "ct = concat(a, with: \"svete\")";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Variables_var1, "Basic variables [var] test")
    test_code = "var str1 = \"Toto je nejaky text v programu jazyka IFJ23\"";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Variables_var2, "Basic variables [var] test")
    test_code = "var vysl : Double = 1";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Variables_var3, "Basic variables [var] test")
    test_code = "var result : Int?";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Variables_var4, "Basic variables [var] test")
    test_code = "var ct : String";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Variables_let1, "Basic variables [let] test")
    test_code = "let str2 = str1 + \", ktery jeste trochu obohatime\"";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Variables_let2, "Basic variables [let] test")
    test_code = "let r : String = foo(param)";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Variables_let3, "Basic variables [let] test")
    test_code = "let ret = bar(with: par)";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Variables_let4, "Basic variables [let] test")
    test_code = "let x = x + y";
    code_insert(test_code);
    print_token();
ENDTEST

TEST(Variables_let5, "Basic variables [let] test")
    test_code = "let a : Int? = readInt()";
    code_insert(test_code);
    print_token();
ENDTEST

void (*lex_tests[])(void) = {
    Line_location,
    Column_location,
    Comment_test1,
    Comment_test2,
    Comment_test3,
    Comment_test4,
    Comment_test5,
    String_write1,
    String_write2,
    String_write3,
    String_write4,
    Conditions1,
    Conditions2,
    Conditions3,
    Return1,
    Return2,
    Cycles1,
    Cycles2,
    Functions1,
    Functions2,
    Functions3,
    Functions4,
    Function_calls1,
    Function_calls2,
    Variables_var1,
    Variables_var2,
    Variables_var3,
    Variables_var4,
    Variables_let1,
    Variables_let2,
    Variables_let3,
    Variables_let4,
    Variables_let5,
};

int main (void) {
    printf("===========================================================\n");
    printf("                 Starting lexer testing\n");
    printf("===========================================================\n\n");

    for (int lex_test_id = 0; lex_test_id < LEX_TEST_COUNT; lex_test_id++) {
		lex_tests[lex_test_id]();
	}

    if (remove(TEST_FILE)) {
        EPRINTF("Unable to remove %s.\n", TEST_FILE);
    }

    printf("===========================================================\n");
    printf("                 Ending lexer testing\n");
    printf("===========================================================\n");
    return 0;
}