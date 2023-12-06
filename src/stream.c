#include "stream.h" // Stream, FILE, NUL_STREAM, fgetc, EOF

#include <errno.h>

#include "errors.h"
#include "utils.h"

Stream stream_from_file(FILE *f, const char *filename) {
    if (!f) {
        return NUL_STREAM;
    }
    return (Stream) {
        .inner = f,
        .cur = fgetc(f),
        .last_column = 0,
        .filename = filename,
        .pos = { .line = 1, .column = 0 }
    };
}

int stream_get(Stream *s) {
    if (s->cur == '\n') {
        ++s->pos.line;
        s->last_column = s->pos.column;
        s->pos.column = 0;

        int ret = s->cur;
        s->cur = fgetc(s->inner);
        return ret;
    }

    if (s->cur == EOF) {
        return EOF;
    }

    ++s->pos.column;

    int ret = s->cur;
    s->cur = fgetc(s->inner);
    if (errno) {
        EPRINTF("error: Failed to read from file.\n");
        set_err_code(ERR_OTHER);
        s->cur = STREAM_ERR;
    }
    return ret;
}

int stream_peak(const Stream *s) {
    return s->cur;
}

bool stream_is_invalid(const Stream *s) {
    return s->inner == NULL;
}

FilePos stream_get_pos(const Stream *s) {
    FilePos pos = s->pos;

    if (s->cur == EOF) {
        ++pos.column;
    } else if (pos.column == 0) {
        pos.column = s->last_column + 1;
        --pos.line;
    }

    return pos;
}

FilePos pos_min(FilePos a, FilePos b) {
    return a.line < b.line || ((a.line == b.line) && a.column < b.column)
        ? a
        : b;
}
