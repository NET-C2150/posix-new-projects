/*
 * Common/shared macros and routines.
 *
 * This file contains macros of the form
 *
 *   mph_return_if_TYPE_overflow(val);
 *
 * Which tests `val' for a TYPE underflow/overflow (that is, is `val' within
 * the range for TYPE?).  If `val' can't fit in TYPE, errno is set to
 * EOVERFLOW, and `return -1' is executed (which is why it's a macro).
 *
 * Assumptions:
 *
 * I'm working from GLibc, so that's the basis for my assumptions.  They may
 * not be completely portable, in which case I'll need to fix my assumptions.
 * :-(
 *
 * See the typedefs for type size assumptions.  These typedefs *must* be kept
 * in sync with the types used in Mono.Posix.dll.
 */

#ifndef INC_mph_H
#define INC_mph_H

#include <config.h>

#include <limits.h>             /* LONG_MAX, ULONG_MAX */
#include <errno.h>              /* for ERANGE */
#include <glib/gtypes.h>        /* for g* types, etc. */

#ifdef HAVE_STDINT_H
#include <stdint.h>             /* for SIZE_MAX */
#endif

typedef    gint64 mph_blkcnt_t;
typedef    gint64 mph_blksize_t;
typedef   guint64 mph_dev_t;
typedef   guint64 mph_ino_t;
typedef   guint64 mph_nlink_t;
typedef    gint64 mph_off_t;
typedef   guint64 mph_size_t;
typedef    gint64 mph_ssize_t;
typedef    gint32 mph_pid_t;
typedef   guint32 mph_gid_t;
typedef   guint32 mph_uid_t;
typedef    gint64 mph_time_t;
typedef    gint64 mph_clock_t;

#ifdef HAVE_LARGE_FILE_SUPPORT
#define MPH_OFF_T_MAX G_MAXINT64
#define MPH_OFF_T_MIN G_MININT64
#else
#define MPH_OFF_T_MAX G_MAXINT32
#define MPH_OFF_T_MIN G_MININT32
#endif

#ifdef SIZE_MAX
#define MPH_SIZE_T_MAX SIZE_MAX
#elif SIZEOF_SIZE_T == 8
#define MPH_SIZE_T_MAX  G_MAXUINT64
#elif SIZEOF_SIZE_T == 4
#define MPH_SIZE_T_MAX  G_MAXUINT32
#else
#error "sizeof(size_t) is unknown!"
#endif

#define _mph_return_val_if_cb_(val, ret, cb) G_STMT_START{ \
	if (cb (val)) { \
		errno = EOVERFLOW; \
		return ret; \
	}}G_STMT_END

#define mph_have_long_overflow(var) ((var) > LONG_MAX || (var) < LONG_MIN)

#define mph_return_val_if_long_overflow(var, ret) \
	_mph_return_val_if_cb_(var, ret, mph_have_long_overflow)

#define mph_return_if_long_overflow(var) mph_return_val_if_long_overflow(var, -1)

#define mph_have_ulong_overflow(var) ((var) > ULONG_MAX)

#define mph_return_val_if_ulong_overflow(var, ret) \
	_mph_return_val_if_cb_(var, ret, mph_have_ulong_overflow)

#define mph_return_if_ulong_overflow(var) mph_return_val_if_ulong_overflow(var, -1)

#define mph_have_size_t_overflow(var) ((var) > MPH_SIZE_T_MAX)

#define mph_return_val_if_size_t_overflow(var, ret) \
	_mph_return_val_if_cb_(var, ret, mph_have_size_t_overflow)

#define mph_return_if_size_t_overflow(var) mph_return_val_if_size_t_overflow(var, -1)

#define mph_have_off_t_overflow(var) \
	(((var) < MPH_OFF_T_MIN) || ((var) > MPH_OFF_T_MAX))

#define mph_return_val_if_off_t_overflow(var, ret) \
	_mph_return_val_if_cb_(var, ret, mph_have_off_t_overflow)

#define mph_return_if_off_t_overflow(var) mph_return_val_if_size_t_overflow(var, -1)

#define mph_return_if_time_t_overflow(var) mph_return_if_long_overflow(var)

/*
 * Helper function for functions which use ERANGE (such as getpwnam_r and
 * getgrnam_r).  These functions accept buffers which are dynamically
 * allocated so that they're only as large as necessary.  However, Linux and
 * Mac OS X differ on how to signal an error value.
 *
 * Linux returns the error value directly, while Mac OS X is more traditional,
 * returning -1 and setting errno accordingly.
 *
 * Unify the checking in one place.
 */
static inline int
recheck_range (int ret)
{
	if (ret == ERANGE)
		return 1;
	if (ret == -1)
		return errno == ERANGE;
	return 0;
}

#endif /* ndef INC_mph_H */

/*
 * vim: noexpandtab
 */
