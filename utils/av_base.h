#ifndef AV_BASE_H_
#define AV_BASE_H_

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#else

#if defined(_UNICODE) || defined(UNICODE) || defined(__UNICODE__)
#ifdef _UNICODE
#undef _UNICODE
#endif
#ifdef UNICODE
#undef UNICODE
#endif
#ifdef __UNICODE__
#undef __UNICODE__
#endif
#endif

#define TEXT(quote) quote

#endif // _WIN32

#endif
