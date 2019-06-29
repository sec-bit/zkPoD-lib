#ifndef __MSVC_HACK_H__
#define __MSVC_HACK_H__

#if defined(_MSC_VER)
#if defined(_WIN64)
typedef __int64 ssize_t;
#else
typedef _W64 __int32 ssize_t;
#endif // _WIN64
#endif // _MSC_VER

#if defined(_MSC_VER)
#define __noinline__ __declspec(noinline)
#else
#define __noinline__ __attribute__((noinline))	
#endif // _MSC_VER

#ifdef _MSC_VER
#include <intrin.h>
#if defined(_WIN64)
#define builtin_clz(x) __lzcnt(x)
#define builtin_clzl(x) __lzcnt64(x)
#else // defined(_WIN64)
#define buildin_clz(x) __lzcnt(x)
#define buildin_clzl(x) __lzcnt(x)
#endif // defined(_WIN64)
#endif // _MSC_VER

#ifdef __GNUC__
#define builtin_clz(x) __builtin_clz(x)
#define builtin_clzl(x) __builtin_clzl(x)
#endif // __GNUC__

#endif // __MSVC_HACK_H__