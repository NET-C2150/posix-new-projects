/*
 * <syslog.h> wrapper functions.
 *
 * Authors:
 *   Jonathan Pryor (jonpryor@vt.edu)
 *
 * Copyright (C) 2005 Jonathan Pryor
 */

#include <stdarg.h>
#include <syslog.h>
#include <errno.h>

#include "map.hh"
#include "mph.hh"
#include <glib.h>

int
Mono_Posix_Syscall_openlog (void* ident, int option, int facility)
{
	openlog ((const char*) ident, option, facility);
	return 0;
}

int
Mono_Posix_Syscall_closelog (void)
{
	closelog ();
	return 0;
}

#ifdef __GNUC__
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wformat-security"
#endif
int
Mono_Posix_Syscall_syslog (int priority, const char* message)
{
	syslog (priority, message);
	return 0;
}

#ifdef __GNUC__
	#pragma GCC diagnostic pop
#endif

/* vararg version of syslog(3). */
int32_t
Mono_Posix_Syscall_syslog2 (int priority, const char *format, ...);

int32_t
Mono_Posix_Syscall_syslog2 (int priority, const char *format, ...)
{
	va_list ap;

#if HAVE_VSYSLOG
	va_start (ap, format);
	vsyslog (priority, format, ap);
	va_end (ap);
#else
	/* some OSes like AIX lack vsyslog; simulate with vsprintf */
	char message[256];

	va_start (ap, format);
	vsnprintf(message, 256, format, ap);
	va_end(ap);

	syslog(priority, message);
#endif
	return 0;
}

/*
 * vim: noexpandtab
 */