/*
 * <dirent.h> wrapper functions.
 *
 * Authors:
 *   Jonathan Pryor (jonpryor@vt.edu)
 *
 * Copyright (C) 2004-2005 Jonathan Pryor
 */

#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "map.h"
#include "mph.h"

G_BEGIN_DECLS

gint32
Mono_Posix_Syscall_seekdir (void *dir, mph_off_t offset)
{
	mph_return_if_off_t_overflow (offset);

	errno = 0;

	seekdir ((DIR*) dir, (off_t) offset);

	return errno != 0;
}

mph_off_t
Mono_Posix_Syscall_telldir (void *dir)
{
	return telldir ((DIR*) dir);
}

static void
copy_dirent (struct Mono_Posix_Syscall__Dirent *to, struct dirent *from)
{
	memset (to, 0, sizeof(*to));

	to->d_ino    = from->d_ino;
	to->d_name   = strdup (from->d_name);

#ifdef HAVE_STRUCT_DIRENT_D_OFF
	to->d_off    = from->d_off;
#endif
#ifdef HAVE_STRUCT_DIRENT_D_RECLEN
	to->d_reclen = from->d_reclen;
#endif
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
	to->d_type   = from->d_type;
#endif
}

gint32
Mono_Posix_Syscall_readdir (void *dirp, struct Mono_Posix_Syscall__Dirent *entry)
{
	struct dirent *d;

	if (entry == NULL) {
		errno = EFAULT;
		return -1;
	}

	d = readdir (dirp);

	if (d == NULL) {
		return -1;
	}

	copy_dirent (entry, d);

	return 0;
}

gint32
Mono_Posix_Syscall_readdir_r (void *dirp, struct Mono_Posix_Syscall__Dirent *entry, void **result)
{
	struct dirent _entry;
	int r;

	r = readdir_r (dirp, &_entry, (struct dirent**) result);

	if (r == 0 && result != NULL) {
		copy_dirent (entry, &_entry);
	}

	return r;
}

int
Mono_Posix_Syscall_rewinddir (void* dir)
{
	errno = 0;
	rewinddir (dir);
	return errno == 0 ? 0 : -1;
}

G_END_DECLS

/*
 * vim: noexpandtab
 */
