#ifndef MISCS_DSCAO__
#define MISCS_DSCAO__

#if defined(__GNUC__)
#define likely(x) __builtin_expect(x, 1)
#define unlikely(x) __builtin_expect(x, 0)
#define __declspec(dllexport)
#define __cdecl
typedef unsigned long ulong64;
typedef long long64;

#elif defined(_MSC_VER)
#define __attribute__(x)
#define likely(x) (x)
#define unlikely(x)	(x)
#define DLLExport(retype) __declspec(dllexport) retype __cdecl
typedef unsigned long long ulong64;
typedef long long long64;

#endif

#if defined(_WIN64)
#define CLOCK_REALTIME	0
#define CLOCK_REALTIME_COARSE 1
void clock_gettime(int clock, struct timespec* tm);
#endif
#endif  /* MISCS_DSCAO__ */
