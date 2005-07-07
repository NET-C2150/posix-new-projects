#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "supportw.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/class.h"
#include "mono/metadata/object.h"
#include "mono/metadata/tabledefs.h"
#include "mono/io-layer/wapi.h"

typedef struct {
	const char *fname;
	void *fnptr;
} FnPtr;

gpointer FindWindowExW (gpointer hwndParent, gpointer hwndChildAfter,
			const char *classw, const char *window);

gpointer HeapAlloc (gpointer unused1, gint32 unused2, gint32 nbytes);
gpointer HeapCreate (gint32 flags, gint32 initial_size, gint32 max_size);
gboolean HeapSetInformation (gpointer handle, gpointer heap_info_class,
				gpointer heap_info, gint32 head_info_length);

gboolean HeapQueryInformation (gpointer handle, gpointer heap_info_class,
			gpointer heap_info, gint32 head_info_length, gint32 *ret_length);

gpointer HeapAlloc (gpointer handle, gint32 flags, gint32 nbytes);
gpointer HeapReAlloc (gpointer handle, gint32 flags, gpointer mem, gint32 nbytes);
gint32 HeapSize (gpointer handle, gint32 flags, gpointer mem);
gboolean HeapFree (gpointer handle, gint32 flags, gpointer mem);
gboolean HeapValidate (gpointer handle, gpointer mem);
gboolean HeapDestroy (gpointer handle);
gpointer GetProcessHeap (void);

static FnPtr functions [] = {
	{ "FindWindowExW", NULL }, /* user32 */
};
#define NFUNCTIONS	(sizeof (functions)/sizeof (FnPtr))

static int swf_registered;

static int
compare_names (const void *key, const void *p)
{
	FnPtr *ptr = (FnPtr *) p;
	return strcmp (key, ptr->fname);
}

static gpointer
get_function (const char *name)
{
	FnPtr *ptr;

	ptr = bsearch (name, functions, NFUNCTIONS, sizeof (FnPtr),
			compare_names);

	if (ptr == NULL) {
		g_warning ("Function '%s' not not found.", name);
		return NULL;
	}

	return ptr->fnptr;
}

gboolean
supportw_register_delegate (const char *function_name, void *fnptr)
{
	FnPtr *ptr;

	g_return_val_if_fail (function_name && fnptr, FALSE);

	ptr = bsearch (function_name, functions, NFUNCTIONS, sizeof (FnPtr),
			compare_names);

	if (ptr == NULL) {
		g_warning ("Function '%s' not supported.", function_name);
		return FALSE;
	}

	ptr->fnptr = fnptr;
	return TRUE;
}

#define M_ATTRS (METHOD_ATTRIBUTE_PUBLIC | METHOD_ATTRIBUTE_STATIC)
static gboolean
register_assembly (const char *name, int *registered)
{
/* we can't use mono or wapi funcions in a support lib */
#if 0
	MonoAssembly *assembly;
	MonoImageOpenStatus status;
	MonoImage *image;
	MonoClass *klass;
	MonoMethod *method;
	MonoObject *exc;

	if (*registered)
		return TRUE;

	assembly = mono_assembly_load_with_partial_name (name, &status);
	if (assembly == NULL) {
		g_warning ("Cannot load assembly '%s'.", name);
		return FALSE;
	}

	image = mono_assembly_get_image (assembly);
	klass = mono_class_from_name (image, name, "LibSupport");
	if (klass == NULL) {
		g_warning ("Cannot load class %s.LibSupport", name);
		mono_assembly_close (assembly);
		return FALSE;
	}

	method = mono_class_get_method_from_name_flags (klass, "Register", 0, M_ATTRS);
	if (klass == NULL) {
		g_warning ("Cannot load method Register from klass %s.LibSupport", name);
		mono_assembly_close (assembly);
		return FALSE;
	}

	exc = NULL;
	mono_runtime_invoke (method, NULL, NULL, &exc);
	if (exc != NULL) {
		mono_assembly_close (assembly);
		mono_print_unhandled_exception (exc);
		return FALSE;
	}
	*registered = 1;
	mono_assembly_close (assembly);
	return TRUE;
#else
	return FALSE;
#endif
}

void
supportw_test_all ()
{
	int i;

	register_assembly ("System.Windows.Forms", &swf_registered);
	for (i = 0; i < NFUNCTIONS; i++) {
		FnPtr *ptr = &functions [i];
		if (ptr->fnptr == NULL)
			g_warning ("%s wasn't registered.", ptr->fname);
	}
}

gpointer
FindWindowExW (gpointer hwndParent, gpointer hwndChildAfter, const char *classw, const char *window)
{
	typedef gpointer (*func_type) (gpointer hwndParent, gpointer hwndChildAfter,
					const char *classw, const char *window);
	static func_type func;

	g_return_val_if_fail (register_assembly ("System.Windows.Forms", &swf_registered), NULL);
	if (func == NULL)
		func = (func_type) get_function ("FindWindowExW");

	return func (hwndParent, hwndChildAfter, classw, window);
}

/* begin Heap* functions */
gpointer
HeapCreate (gint32 flags, gint32 initial_size, gint32 max_size)
{
	return (gpointer) 0xDEADBEEF;
}

gboolean
HeapSetInformation (gpointer handle, gpointer heap_info_class, gpointer heap_info,
			gint32 head_info_length)
{
	return TRUE;
}

gboolean
HeapQueryInformation (gpointer handle, gpointer heap_info_class, gpointer heap_info,
			gint32 head_info_length, gint32 *ret_length)
{
	*ret_length = 0;
	return TRUE;
}

gpointer
HeapAlloc (gpointer handle, gint32 flags, gint32 nbytes)
{
	return g_malloc0 (nbytes);
}

gpointer
HeapReAlloc (gpointer handle, gint32 flags, gpointer mem, gint32 nbytes)
{
	return g_realloc (mem, nbytes);
}

gint32
HeapSize (gpointer handle, gint32 flags, gpointer mem)
{
	return 0;
}

gboolean
HeapFree (gpointer handle, gint32 flags, gpointer mem)
{
	g_free (mem);
	return TRUE;
}

gboolean
HeapValidate (gpointer handle, gpointer mem)
{
	return TRUE;
}

gboolean
HeapDestroy (gpointer handle)
{
	return TRUE;
}


gpointer 
GetProcessHeap ()
{
	return (gpointer) 0xDEADBEEF;
}
/* end Heap* functions */

