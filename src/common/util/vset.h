/* vi: set ts=2:
 *
 *	$Id: vset.h,v 1.2 2001/01/26 16:19:41 enno Exp $
 *	Eressea PB(E)M host Copyright (C) 1998-2000
 *      Christian Schlittchen (corwin@amber.kn-bremen.de)
 *      Katja Zedel (katze@felidae.kn-bremen.de)
 *      Henning Peters (faroul@beyond.kn-bremen.de)
 *      Enno Rehling (enno@eressea-pbem.de)
 *      Ingo Wilken (Ingo.Wilken@informatik.uni-oldenburg.de)
 *
 * This program may not be used, modified or distributed without
 * prior permission by the authors of Eressea.
 */

#ifndef VOIDPTR_SETS
#define VOIDPTR_SETS
#include <stddef.h>
typedef struct vset vset;
struct vset {
	void **data;
	size_t size;
	size_t maxsize;
};
void vset_init(vset * s);
void vset_destroy(vset * s);
unsigned int vset_add(vset * s, void *);
int vset_erase(vset * s, void *);

#endif
