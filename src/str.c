#include "str.h" // String, bool, false, true

#include <string.h> // memcmp, memcpy
#include <stdlib.h> // free, malloc, NULL, realloc

#include "errors.h" // ERR_OTHER

bool str_eq(const String a, const String b) {
    if (a.len != b.len) {
        return false;
    }

    if (a.str == b.str) {
        return true;
    }

    if (!a.str || !b.str) {
        return false;
    }

    return memcmp(a.str, b.str, a.len) == 0;
}

String str_clone(const String s) {
    char *str = malloc(sizeof(*str) * (s.len + 1));
    if (!str) {
        exit(ERR_OTHER);
    }

    memcpy(str, s.str, s.len + 1);

    return (String) { .str = str, .len = s.len };
}

void str_free(String *s) {
    if (!s->str) {
        return;
    }

    free(s->str);
    s->str = NULL;
    s->len = 0;
}

StringBuffer sb_new() {
    return (StringBuffer) { .str = NULL, .alloc = 0, .len = 0 };
}

void sb_free(StringBuffer *sb) {
    if (!sb->str) {
        return;
    }

    free(sb->str);
    sb->str = NULL;
    sb->len = 0;
    sb->alloc = 0;
}

bool sb_push(StringBuffer *sb, char c) {
    if (sb->len >= sb->alloc) {
        size_t new_size = sb->len * 2;

        if (new_size == 0) {
            new_size = 1;
        }

        char *new_str = realloc(sb->str, sizeof(*new_str) * (new_size + 1));
        if (!new_str) {
            return false;
        }
        sb->alloc = new_size;
        sb->str = new_str;
    }

    sb->str[sb->len] = c;
    sb->str[++sb->len] = 0;

    return true;
}

void sb_clear(StringBuffer *sb) {
    sb->len = 0;
    sb->str[0] = 0;
}

String sb_get(const StringBuffer *sb) {
    return (String) { .str = sb->str, .len = sb->len };
}
