/* vi: set ts=2:
 *
 * $Id: dragons.c,v 1.2 2001/01/26 16:19:41 enno Exp $
 * Eressea PB(E)M host Copyright (C) 1998-2000
 *      Christian Schlittchen (corwin@amber.kn-bremen.de)
 *      Katja Zedel (katze@felidae.kn-bremen.de)
 *      Henning Peters (faroul@beyond.kn-bremen.de)
 *      Enno Rehling (enno@eressea-pbem.de)
 *      Ingo Wilken (Ingo.Wilken@informatik.uni-oldenburg.de)
 *
 * This program may not be used, modified or distributed without
 * prior permission by the authors of Eressea.
 */

#include <config.h>
#include <eressea.h>
#include "dragons.h"

/* kernel includes */
#include <region.h>
#include <unit.h>

/* libc includes */
#include <stdlib.h>

#define age_chance(a,b,p) (max(0,a-b)*p)

#define DRAGONAGE                   27
#define WYRMAGE                     68

void
age_firedragon(unit *u)
{
	if (rand()%100 < age_chance(u->age, DRAGONAGE, 1)) {
		double q = (double) u->hp / (double) (unit_max_hp(u) * u->number);
		u->race = RC_DRAGON;
		u->irace = RC_DRAGON;
		scale_number(u,1);
		u->hp = (int) (unit_max_hp(u) * u->number * q);
	}
}

void
age_dragon(unit *u)
{
	if (rand()%100 < age_chance(u->age, WYRMAGE, 1)) {
		double q = (double) u->hp / (double) (unit_max_hp(u) * u->number);
		u->race = RC_WYRM;
		u->irace = RC_WYRM;
		scale_number(u,1);
		u->hp = (int) (unit_max_hp(u) * u->number * q);
	}
}

