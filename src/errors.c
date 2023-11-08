#include "errors.h"

static int first_err_code;
static int last_err_code;

void set_err_code(int err_code) {
    last_err_code = err_code;
    if (err_code == 0) {
        first_err_code = err_code;
    }
}

int get_first_err_code() {
    return first_err_code;
}

int get_last_err_code() {
    return last_err_code;
}
