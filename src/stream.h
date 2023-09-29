#ifndef STREAM_H_INCLUDED
#define STREAM_H_INCLUDED

#include <stdio.h>   // FILE, EOF
#include <stdbool.h> // bool

/// An invalid stream
#define NUL_STREAM ((Stream) { .cur = EOF })

typedef struct {
    // starting from 1
    size_t line;
    // starting from 1
    size_t column;
} FilePos;

typedef struct {
    FILE *inner;
    int cur;
    size_t last_column;
    /// Internal tracking of position, may be different from real.
    /// Use stream_get_pos to get the correct position.
    FilePos pos;
} Stream;

/// Creates new stream from FILE, doesn't take ownership of file, you
/// need to close it yourself.
Stream stream_from_file(FILE *f);

/// Reads single char from stream
int stream_get(Stream *s);

/// Peeks at the next char in the stream
int stream_peak(const Stream *s);

/// Checks whether the stream is sinvalid
bool stream_is_invalid(const Stream *s);

/// Gets the position of the last char returned by stream_get
FilePos stream_get_pos(const Stream *s);

#endif // STREAM_H_INCLUDED
