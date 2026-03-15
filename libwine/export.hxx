#pragma once

#if defined(LIBWINE_STATIC)
#  define LIBWINE_SYMEXPORT
#elif defined(LIBWINE_STATIC_BUILD)
#  define LIBWINE_SYMEXPORT
#elif defined(LIBWINE_SHARED)
#  ifdef _WIN32
#    define LIBWINE_SYMEXPORT __declspec (dllimport)
#  else
#    define LIBWINE_SYMEXPORT
#  endif
#elif defined(LIBWINE_SHARED_BUILD)
#  ifdef _WIN32
#    define LIBWINE_SYMEXPORT __declspec (dllexport)
#  else
#    define LIBWINE_SYMEXPORT
#  endif
#else
// If none of the above macros are defined, then we assume we are being used by
// some third-party build system that cannot/doesn't signal the library type
// being linked.
//
#  define LIBWINE_SYMEXPORT
#endif
