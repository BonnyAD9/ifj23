#include <stddef.h>  // size_t
#include <stdbool.h> // bool

/// Creates string from literal.
/// WORKS ONLY FOR LITERALS
#define STR(literal) ((String) { .str = literal, .len = sizeof(literal) - 1 })
/// Invalid string
#define NUL_STR ((String) { .str = NULL, .len = 0 })

typedef struct {
    /// C string ended with the NUL character
    char *str;
    /// Length of the string (position of the NUL character)
    size_t len;
} String;

typedef struct {
    /// Current data of the string buffer, terminated with the NUL character
    char *str;
    /// maximum length in the buffer
    size_t alloc;
    /// the current length in the buffer not including the NUL character
    size_t len;
} StringBuffer;

/// Checks whether the two strings are the same
bool str_eq(const String a, const String b);

/// Checks whether String is equal to const char
bool str_eq_const_str(const String a, const char *string);

/// Copies the data of the string to newly allocated string.
/// Returns invalid string when fails.
String str_clone(const String s);

/// Frees the given string
void str_free(String *s);

/// Creates new string buffer
StringBuffer sb_new();

/// Frees the string buffer
void sb_free(StringBuffer *sb);

/// Appends to the string buffer
/// Returns false when fails.
bool sb_push(StringBuffer *sb, char c);

/// Clears the data in the string buffer
void sb_clear(StringBuffer *sb);

String sb_get(const StringBuffer *sb);
