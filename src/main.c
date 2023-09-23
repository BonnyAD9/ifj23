#include <stdio.h>

#include "utils.h"
#include "str.h"

int main(void) {
    StringBuffer sb = sb_new();

    bool res = true;

    res &= sb_push(&sb, 'h');
    res &= sb_push(&sb, 'e');
    res &= sb_push(&sb, 'l');
    res &= sb_push(&sb, 'l');
    res &= sb_push(&sb, 'o');

    DPRINTF("%d: %s", res, sb_get(&sb).str);

    sb_clear(&sb);

    DPRINTF("cleared: %s", sb_get(&sb).str);

    sb_free(&sb);

    TODO("ifj project");
}
