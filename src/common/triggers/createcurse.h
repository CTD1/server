/* vi: set ts=2:
 *
 *	
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

#ifndef CREATECURSE_H
#define CREATECURSE_H

/* all types we use are defined here to reduce dependencies */
struct curse_type;
struct trigger_type;
struct trigger;
struct region;
struct faction;
struct unit;

extern struct trigger_type tt_createcurse;

extern struct trigger * trigger_createcurse(struct unit * mage, struct unit * target, const struct curse_type * ct, int vigour, int duration, int effect, int men);

#endif
