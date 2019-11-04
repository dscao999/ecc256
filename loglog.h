#ifndef LOGLOG_DSCAO__
#define LOGLOG_DSCAO__
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <syslog.h>

#define NOMEM	6
#define likely(x) __builtin_expect(x, 1)
#define unlikely(x) __builtin_expect(x, 0)

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

static inline void *check_pointer(void *ptr, int level, const char *fmt, ...)
{
	va_list ap;

	if (!ptr) {
		va_start(ap, fmt);
		vlogmsg(level, fmt, ap);
		va_end(ap);
	}
	return ptr;
}
#endif  /* LOGLOG_DSCAO__ */
