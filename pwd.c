/*
 * <pwd.h> wrapper functions.
 *
 * Authors:
 *   Jonathan Pryor (jonpryor@vt.edu)
 *
 * Copyright (C) 2004-2005 Jonathan Pryor
 */

#include <pwd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "map.h"
#include "mph.h"

G_BEGIN_DECLS

static const size_t
passwd_offsets[] = {
	offsetof (struct passwd, pw_name),
	offsetof (struct passwd, pw_passwd),
	offsetof (struct passwd, pw_gecos),
	offsetof (struct passwd, pw_dir),
	offsetof (struct passwd, pw_shell)
};

static const size_t
mph_passwd_offsets[] = {
	offsetof (struct Mono_Posix_Syscall__Passwd, pw_name),
	offsetof (struct Mono_Posix_Syscall__Passwd, pw_passwd),
	offsetof (struct Mono_Posix_Syscall__Passwd, pw_gecos),
	offsetof (struct Mono_Posix_Syscall__Passwd, pw_dir),
	offsetof (struct Mono_Posix_Syscall__Passwd, pw_shell)
};

/*
 * Copy the native `passwd' structure to it's managed representation.
 *
 * To minimize separate mallocs, all the strings are allocated within the same
 * memory block (stored in _pw_buf_).
 */
static int
copy_passwd (struct Mono_Posix_Syscall__Passwd *to, struct passwd *from)
{
	char *buf;
	buf = _mph_copy_structure_strings (to, mph_passwd_offsets,
			from, passwd_offsets, sizeof(passwd_offsets)/sizeof(passwd_offsets[0]));

	to->pw_uid    = from->pw_uid;
	to->pw_gid    = from->pw_gid;

	to->_pw_buf_ = buf;
	if (buf == NULL) {
		return -1;
	}

	return 0;
}

gint32
Mono_Posix_Syscall_getpwnam (const char *name, struct Mono_Posix_Syscall__Passwd *pwbuf)
{
	struct passwd *pw;

	if (pwbuf == NULL) {
		errno = EFAULT;
		return -1;
	}

	errno = 0;
	pw = getpwnam (name);
	if (pw == NULL)
		return -1;

	if (copy_passwd (pwbuf, pw) == -1) {
		errno = ENOMEM;
		return -1;
	}
	return 0;
}

gint32
Mono_Posix_Syscall_getpwuid (mph_uid_t uid, struct Mono_Posix_Syscall__Passwd *pwbuf)
{
	struct passwd *pw;

	if (pwbuf == NULL) {
		errno = EFAULT;
		return -1;
	}

	errno = 0;
	pw = getpwuid (uid);
	if (pw == NULL) {
		return -1;
	}

	if (copy_passwd (pwbuf, pw) == -1) {
		errno = ENOMEM;
		return -1;
	}
	return 0;
}

#ifdef HAVE_GETPWNAM_R
gint32
Mono_Posix_Syscall_getpwnam_r (const char *name, 
	struct Mono_Posix_Syscall__Passwd *pwbuf,
	void **pwbufp)
{
	char *buf, *buf2;
	size_t buflen;
	int r;
	struct passwd _pwbuf;

	if (pwbuf == NULL) {
		errno = EFAULT;
		return -1;
	}

	buf = buf2 = NULL;
	buflen = 2;

	do {
		buf2 = realloc (buf, buflen *= 2);
		if (buf2 == NULL) {
			free (buf);
			errno = ENOMEM;
			return -1;
		}
		buf = buf2;
		errno = 0;
	} while ((r = getpwnam_r (name, &_pwbuf, buf, buflen, (struct passwd**) pwbufp)) && 
			recheck_range (r));

	if (r == 0 && !(*pwbufp))
		/* On solaris, this function returns 0 even if the entry was not found */
		r = errno = ENOENT;

	if (r == 0 && copy_passwd (pwbuf, &_pwbuf) == -1)
		r = errno = ENOMEM;
	free (buf);

	return r;
}
#endif /* ndef HAVE_GETPWNAM_R */

#ifdef HAVE_GETPWUID_R
gint32
Mono_Posix_Syscall_getpwuid_r (mph_uid_t uid,
	struct Mono_Posix_Syscall__Passwd *pwbuf,
	void **pwbufp)
{
	char *buf, *buf2;
	size_t buflen;
	int r;
	struct passwd _pwbuf;

	if (pwbuf == NULL) {
		errno = EFAULT;
		return -1;
	}

	buf = buf2 = NULL;
	buflen = 2;

	do {
		buf2 = realloc (buf, buflen *= 2);
		if (buf2 == NULL) {
			free (buf);
			errno = ENOMEM;
			return -1;
		}
		buf = buf2;
		errno = 0;
	} while ((r = getpwuid_r (uid, &_pwbuf, buf, buflen, (struct passwd**) pwbufp)) && 
			recheck_range (r));

	if (r == 0 && copy_passwd (pwbuf, &_pwbuf) == -1)
		r = errno = ENOMEM;
	free (buf);

	return r;
}
#endif /* ndef HAVE_GETPWUID_R */

gint32
Mono_Posix_Syscall_getpwent (struct Mono_Posix_Syscall__Passwd *pwbuf)
{
	struct passwd *pw;

	if (pwbuf == NULL) {
		errno = EFAULT;
		return -1;
	}

	errno = 0;
	pw = getpwent ();
	if (pw == NULL)
		return -1;

	if (copy_passwd (pwbuf, pw) == -1) {
		errno = ENOMEM;
		return -1;
	}
	return 0;
}

#ifdef HAVE_FGETPWENT
gint32
Mono_Posix_Syscall_fgetpwent (void *stream, struct Mono_Posix_Syscall__Passwd *pwbuf)
{
	struct passwd *pw;

	if (pwbuf == NULL) {
		errno = EFAULT;
		return -1;
	}

	errno = 0;
	pw = fgetpwent ((FILE*) stream);
	if (pw == NULL)
		return -1;

	if (copy_passwd (pwbuf, pw) == -1) {
		errno = ENOMEM;
		return -1;
	}
	return 0;
}
#endif /* ndef HAVE_FGETPWENT */

int
Mono_Posix_Syscall_setpwent (void)
{
	errno = 0;
	setpwent ();
	return errno == 0 ? 0 : -1;
}

int
Mono_Posix_Syscall_endpwent (void)
{
	errno = 0;
	endpwent ();
	return errno == 0 ? 0 : -1;
}

G_END_DECLS

/*
 * vim: noexpandtab
 */
