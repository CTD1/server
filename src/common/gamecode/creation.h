/* vi: set ts=2:
 *
 *	$Id: creation.h,v 1.2 2001/01/26 16:19:39 enno Exp $
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
#ifndef CREATION_H
#define CREATION_H

/* entweder eine grosse Insel (Chance 2/3) mit 25 bis 34 Felder oder eine
 * kleine Insel (Chance 1/3) mit 11 bis 24 Feldern. */

enum {
	M_TERRAIN,
	M_FACTIONS,
	M_UNARMED,
	MAXMODES
};

void createmonsters(void);
void addunit(void);
void makeblock(int x1, int y1, char chaos);
void listnames(void);
void writemap(FILE * F, int mode);

void changeblockterrain(void);

void regionspells(void);
void moveunit(void);

#endif
