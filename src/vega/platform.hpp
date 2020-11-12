#pragma once

#ifdef _MSC_VER

// TODO

#elif defined(__clang__)

// TODO

#elif defined(__GNUC__)

#define BEGIN_DISABLE_WARNINGS \
_Pragma("GCC diagnostic push") \
_Pragma("GCC diagnostic ignored \"-Wsign-conversion\"") /* conversion to ‘T’ from ‘U’ may change the sign of the result */ \
_Pragma("GCC diagnostic ignored \"-Wconversion\"") /* conversion from ‘T’ to ‘U’ may change value */ \
_Pragma("GCC diagnostic ignored \"-Wvolatile\"") /* compound assignment with ‘volatile’-qualified left operand is deprecated */

#define END_DISABLE_WARNINGS _Pragma("GCC diagnostic pop")

#endif
