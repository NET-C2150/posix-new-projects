/*
 * <sys/mman.h> wrapper functions.
 *
 * Authors:
 *   Jonathan Pryor (jonpryor@vt.edu)
 *
 * Copyright (C) 2004-2006 Jonathan Pryor
 */

#if defined (HAVE_CONFIG_H)
#include <config.h>
#endif

#if !defined (__OpenBSD__) && !defined (_XOPEN_SOURCE)
#define _XOPEN_SOURCE 600
#endif

#if defined(__FreeBSD__) || defined(__OpenBSD__)
/* For mincore () */
#define __BSD_VISIBLE 1
#endif

#ifdef __NetBSD__
/* For mincore () */
#define _NETBSD_SOURCE
#endif

#include <sys/types.h>
#include <sys/mman.h>
#include <cerrno>

#include "map.hh"
#include "mph.hh"

void*
Mono_Posix_Syscall_mmap (void *start, mph_size_t length, int prot, int flags, int fd, mph_off_t offset)
{
	if (mph_have_off_t_overflow (offset) || mph_have_size_t_overflow (length)) {
		return MAP_FAILED;
	}

	int _prot, _flags;

	if (Mono_Posix_FromMmapProts (prot, &_prot) == -1)
		return MAP_FAILED;
	if (Mono_Posix_FromMmapFlags (flags, &_flags) == -1)
		return MAP_FAILED;

	return mmap (start, length, _prot, _flags, fd, offset);
}

int
Mono_Posix_Syscall_munmap (void *start, mph_size_t length)
{
	if (mph_have_size_t_overflow (length)) {
		return -1;
	}

	return munmap (start, length);
}

int
Mono_Posix_Syscall_mprotect (void *start, mph_size_t len, int prot)
{
	if (mph_have_size_t_overflow (len)) {
		return -1;
	}

	int _prot;
	if (Mono_Posix_FromMmapProts (prot, &_prot) == -1)
		return -1;

	return mprotect (start, len, _prot);
}

int
Mono_Posix_Syscall_msync (void *start, mph_size_t len, int flags)
{
	if (mph_have_size_t_overflow (len)) {
		return -1;
	}

	int _flags;
	if (Mono_Posix_FromMsyncFlags (flags, &_flags) == -1)
		return -1;

	return msync (start, len, _flags);
}

int
Mono_Posix_Syscall_mlock (void *start, mph_size_t len)
{
#if !defined (HAVE_MLOCK)
	return ENOSYS;
#else
	if (mph_have_size_t_overflow (len)) {
		return -1;
	}

	return mlock (start, len);
#endif
}

int
Mono_Posix_Syscall_munlock (void *start, mph_size_t len)
{
#if !defined (HAVE_MUNLOCK)
	return ENOSYS;
#else
	if (mph_have_size_t_overflow (len)) {
		return -1;
	}

	return munlock (start, len);
#endif
}

#ifdef HAVE_MREMAP
void*
Mono_Posix_Syscall_mremap (void *old_address, mph_size_t old_size, mph_size_t new_size, uint64_t flags)
{
	if (mph_have_size_t_overflow (old_size) || mph_have_size_t_overflow (new_size)) {
		return MAP_FAILED;
	}

	uint64_t _flags;

	if (Mono_Posix_FromMremapFlags (flags, &_flags) == -1)
		return MAP_FAILED;

#if defined(linux) || defined (__linux__) || defined (__linux)
	return mremap (old_address, old_size, new_size, static_cast<unsigned long>(_flags));
#elif defined(__NetBSD__)
	return mremap (old_address, old_size, old_address, new_size, static_cast<unsigned long>(_flags));
#else
#error Port me
#endif
}
#endif /* def HAVE_MREMAP */

int
Mono_Posix_Syscall_mincore (void *start, mph_size_t length, unsigned char *vec)
{
#if !defined (HAVE_MINCORE)
	return ENOSYS;
#else
	if (mph_have_size_t_overflow (length)) {
		return -1;
	}

#if defined (__linux__) || defined (HOST_WASM)
	typedef unsigned char T;
#else
	typedef char T;
#endif
	return mincore (start, length, reinterpret_cast<T*>(vec));
#endif
}

#ifdef HAVE_POSIX_MADVISE
int32_t
Mono_Posix_Syscall_posix_madvise (void *addr, mph_size_t len, int32_t advice)
{
	if (mph_have_size_t_overflow (len)) {
		return -1;
	}

	if (Mono_Posix_FromPosixMadviseAdvice (advice, &advice) == -1)
		return -1;

	return posix_madvise (addr, len, advice);
}
#endif /* def HAVE_POSIX_MADVISE */

#ifdef HAVE_REMAP_FILE_PAGES
int
Mono_Posix_Syscall_remap_file_pages (void *start, mph_size_t size, int prot, mph_ssize_t pgoff, int flags)
{
	if (mph_have_size_t_overflow (size) || mph_have_ssize_t_overflow (pgoff)) {
		return -1;
	}

	int _prot, _flags;
	if (Mono_Posix_FromMmapProts (prot, &_prot) == -1)
		return -1;
	if (Mono_Posix_FromMmapFlags (flags, &_flags) == -1)
		return -1;

	return remap_file_pages (start, size, _prot, pgoff, _flags);
}
#endif /* def HAVE_REMAP_FILE_PAGES */

// This has to be kept in sync with Syscall.cs
enum Mono_Posix_MremapFlags {
	Mono_Posix_MremapFlags_MREMAP_MAYMOVE       = 0x0000000000000001,
};

// Mono_Posix_FromMremapFlags() and Mono_Posix_ToMremapFlags() are not in map.c because NetBSD needs special treatment for MREMAP_MAYMOVE
int Mono_Posix_FromMremapFlags (uint64_t x, uint64_t *r)
{
	if (r == nullptr) {
		return -1;
	}

	*r = 0;
#ifndef __NetBSD__
	if ((x & Mono_Posix_MremapFlags_MREMAP_MAYMOVE) == Mono_Posix_MremapFlags_MREMAP_MAYMOVE)
#ifdef MREMAP_MAYMOVE
		*r |= MREMAP_MAYMOVE;
#else /* def MREMAP_MAYMOVE */
		{errno = EINVAL; return -1;}
#endif /* ndef MREMAP_MAYMOVE */
#else /* def __NetBSD__ */
	if ((x & Mono_Posix_MremapFlags_MREMAP_MAYMOVE) != Mono_Posix_MremapFlags_MREMAP_MAYMOVE)
		*r = MAP_FIXED;
#endif /* def __NetBSD__ */
	if (x == 0)
		return 0;
	return 0;
}

int Mono_Posix_ToMremapFlags (uint64_t x, uint64_t *r)
{
	if (r == nullptr) {
		return -1;
	}

	*r = 0;
#ifndef __NetBSD__
	if (x == 0)
		return 0;
#ifdef MREMAP_MAYMOVE
	if ((x & MREMAP_MAYMOVE) == MREMAP_MAYMOVE)
		*r |= Mono_Posix_MremapFlags_MREMAP_MAYMOVE;
#endif /* ndef MREMAP_MAYMOVE */
#else /* def __NetBSD__ */
	if ((x & MAP_FIXED) != MAP_FIXED)
		*r |= Mono_Posix_MremapFlags_MREMAP_MAYMOVE;
#endif
	return 0;
}

/*
 * vim: noexpandtab
 */
