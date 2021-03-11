/*
 * <stdlib.h> wrapper functions.
 *
 * Authors:
 *   Jonathan Pryor (jonpryor@vt.edu)
 *
 * Copyright (C) 2004-2005 Jonathan Pryor
 */

#include <stdlib.h>

#include "map.hh"
#include "mph.hh"

// See Stdlib.cs
void*
Mono_Unix_VersionString ()
{
	return (void *) "MonoProject-2015-12-1";
}

int32_t
Mono_Posix_Stdlib_EXIT_FAILURE (void)
{
	return EXIT_FAILURE;
}

int32_t
Mono_Posix_Stdlib_EXIT_SUCCESS (void)
{
	return EXIT_SUCCESS;
}

int32_t
Mono_Posix_Stdlib_MB_CUR_MAX (void)
{
	return MB_CUR_MAX;
}

int32_t
Mono_Posix_Stdlib_RAND_MAX (void)
{
	return RAND_MAX;
}

void*
Mono_Posix_Stdlib_calloc (mph_size_t nmemb, mph_size_t size)
{
	if (mph_have_size_t_overflow(nmemb) || mph_have_size_t_overflow(size))
		return nullptr;

	return calloc ((size_t) nmemb, (size_t) size);
}

void*
Mono_Posix_Stdlib_malloc (mph_size_t size)
{
	if (mph_have_size_t_overflow(size))
		return nullptr;

	return malloc ((size_t) size);
}

void*
Mono_Posix_Stdlib_realloc (void* ptr, mph_size_t size)
{
	if (mph_have_size_t_overflow(size))
		return nullptr;

	return realloc (ptr, (size_t) size);
}

void
Mono_Posix_Stdlib_free (void* p)
{
	free (p);
}

/*
 * vim: noexpandtab
 */