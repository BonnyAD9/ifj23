/**
 * IFJ23
 *
 * xstigl00 Jakub Antonín Štigler
 */

#ifndef ENUMS_H_INCLUDED
#define ENUMS_H_INCLUDED

typedef enum {
    // unknown type
    DT_NONE    = 0x0,

    // mask for nillable
    DT_NIL     = 0x1,

    // the basic types
    DT_INT     = 0x2,
    DT_DOUBLE  = 0x4,
    DT_STRING  = 0x8,

    // Any non nillable type
    DT_ANY     = 0xE,
    // Any nillable type
    DT_ANY_NIL = 0xF,

    // no type
    DT_VOID    = 0x10,

    // type mask
    DT_TYPE_M  = 0x1E,

    // names for some exact types

    DT_INT_NIL    = DT_INT    | DT_NIL,
    DT_DOUBLE_NIL = DT_DOUBLE | DT_NIL,
    DT_STRING_NIL = DT_STRING | DT_NIL,
} DataType;

#endif // ENUMS_H_INCLUDED
