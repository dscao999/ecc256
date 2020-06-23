#ifndef LOGLOG_DSCAO__
#define LOGLOG_DSCAO__
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <syslog.h>

#define ENOMEM	12
#define ENOSPACE	112

#if defined(__GNUC__)
#define likely(x) __builtin_expect(x, 1)
#define unlikely(x) __builtin_expect(x, 0)
#define DLLExport
typedef unsigned long ulong64;
typedef long long64;
#elif defined(_MSC_VER)
#define likely(x) (x)
#define unlikely(x)	(x)
#define DLLExport(retype) __declspec(dllexport) retype __cdecl
typedef unsigned long long ulong64;
typedef long long long64;
#endif

static inline unsigned int swap32(unsigned int x)
{
	union {
		unsigned int v;
		unsigned char b[4];
	} u;
	u.v = x;
	return (u.b[0] << 24)|(u.b[1] << 16)|(u.b[2] << 8)|u.b[3];
}

#ifdef LOGLOG
#define loginfo(level, fmt, ap)			\
	do {					\
		vsyslog(level, fmt, ap);	\
	} while (0)				
#else
#define loginfo(level, fmt, ap)			\
	do {					\
		vfprintf(stderr, fmt, ap);	\
	} while (0)
static inline void log_flush(void)
{
	fflush(stderr);
}
#endif

static const char *nomem __attribute__((used)) = "Out of Memory!\n";

static inline void vlogmsg(int level, const char *fmt, va_list ap)
{
	loginfo(level, fmt, ap);
}

static inline void logmsg(int level, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	loginfo(level, fmt, ap);
	va_end(ap);
}

static inline void *check_pointer(void *ptr)
{
	if (!ptr)
		logmsg(LOG_CRIT, nomem);
	return ptr;
}

static inline ulong64 time_elapsed(const struct timespec *tm0,
		const struct timespec *tm1)
{
	ulong64 tv_sec;
	long64	tv_nsec;

	tv_sec = tm1->tv_sec - tm0->tv_sec;
	tv_nsec = tm1->tv_nsec - tm0->tv_nsec;
	if (tv_nsec < 0) {
		tv_sec--;
		tv_nsec += 1000000000ul;
	}
	return tv_sec * 1000 + (tv_nsec / 1000000);
}

static inline void time_advance(struct timespec *tm, long64 micro_sec)
{
	long64 sec, rem_micro;

	sec = micro_sec / 1000000;
	rem_micro = micro_sec % 1000000;
	tm->tv_nsec += (rem_micro * 1000);
	tm->tv_sec += sec;
	if (tm->tv_nsec >= 1000000000ul) {
		tm->tv_nsec -= 1000000000ul;
		tm->tv_sec += 1;
	}
}

static inline int align8(int len)
{
	return (((len - 1) >> 3) + 1) << 3;
}

#endif  /* LOGLOG_DSCAO__ */
