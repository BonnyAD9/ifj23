#ifndef DEBUG_TOOLS_INCLUDED
#define DEBUG_TOOLS_INCLUDED

#ifdef NDEBUG

#define DEBUG_FILE

#else // NDEBUG

#ifndef DEBUG_FILE
#define DEBUG_FILE "test/test7.swift"
#endif // DEBUG_FILE

#endif // NDEBUG

#endif // DEBUG_TOOLS_INCLUDED
