#pragma once

#ifdef _MSC_VER

#define BEGIN_DISABLE_WARNINGS

#define END_DISABLE_WARNINGS

#elif defined(__clang__)

// TODO

#elif defined(__GNUC__)

#define BEGIN_DISABLE_WARNINGS \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wsign-conversion\"") /* conversion may change the sign of result */ \
    _Pragma("GCC diagnostic ignored \"-Wconversion\"")      /* conversion may change the value of result */ \
    _Pragma("GCC diagnostic ignored \"-Wvolatile\"")        /* assignment with volatile left operand is deprecated */

#define END_DISABLE_WARNINGS _Pragma("GCC diagnostic pop")

#endif
