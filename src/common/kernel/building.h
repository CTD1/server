/* vi: set ts=2:
 *
 *	$Id: building.h,v 1.2 2001/01/26 16:19:39 enno Exp $
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

#ifndef BUILDING_H
#define BUILDING_H

/* maintenance::flags */
#define MTF_NONE     0x00
#define MTF_VARIABLE 0x01	/* resource usage scales with size */
#define MTF_VITAL    0x02	/* if resource missing, building may crash */

typedef struct maintenance {
	resource_t type; /* type of resource required */
	int number;         /* amount of resources */
	unsigned int flags; /* misc. flags */
} maintenance;

/* building_type::flags */
#define BTF_NONE           0x00
#define BTF_INDESTRUCTIBLE 0x01 /* cannot be torm down */
#define BTF_NOBUILD        0x02 /* special, can't be built */
#define BTF_UNIQUE         0x04 /* only one per struct region (harbour) */
#define BTF_DECAY          0x08 /* decays when not occupied */
#define BTF_DYNAMIC        0x10 /* dynamic type, needs bt_write */

typedef struct building_type {
	const char * _name;

	int flags;  /* flags */
	int capacity; /* Kapazität pro Größenpunkt */
	int maxcapacity; /* Max. Kapazität */
	int maxsize;	/* how big can it get, with all the extensions? */
	const struct maintenance * maintenance; /* array of requirements */
	const struct construction * construction; /* construction of 1 building-level */

	const char * (*name)(int, const struct locale *);
	struct attrib * attribs;
} building_type;

extern const building_type * bt_find(const char* name);
extern void bt_register(const building_type * type);
extern void init_buildings(void);

typedef struct building_typelist {
	struct building_typelist * next;
	const building_type * type;
} building_typelist;

extern struct building_typelist *buildingtypes;
/* buildingt => building_type
 * Name => locale_string(name)
 * MaxGroesse => levels
 * MinBauTalent => construction->minskill
 * Kapazitaet => capacity, maxcapacity
 * Materialien => construction->materials
 * UnterSilber, UnterSpezialTyp, UnterSpezial => maintenance
 * per_size => !maintenance->fixed
 */
#define BFL_NONE           0x00
#define BLD_MAINTAINED     0x01 /* vital maintenance paid for */
#define BLD_WORKING        0x02 /* full maintenance paid, it works */
#define BLD_SAVEMASK       0x00 /* mask for persistent flags */

typedef struct building {
	struct building *next;
	struct building *nexthash;

	const struct building_type * type;
	struct region *region;
	char *name;
	char *display;
	struct attrib * attribs;
	int no;
	int size;
	int sizeleft;	/* is only used during battle. should be a temporary attribute */
	int besieged;	/* should be an attribute */
	unsigned int flags;
} building;

extern attrib_type at_building_generic_type;
extern const char * buildingtype(const struct building * b, int bsize, const struct locale * language);
extern const char * buildingname(const struct building * b);
extern int buildingmaintenance(const building * b, resource_t rtype);
extern int buildingcapacity(const struct building * b);
extern struct building *new_building(const struct building_type * typ, struct region * r, const struct locale * lang);
void build_building(struct unit * u, const struct building_type * typ, int size);

/* Alte Gebäudetypen: */

extern struct building_type bt_castle;
extern struct building_type bt_lighthouse;
extern struct building_type bt_mine;
extern struct building_type bt_quarry;
extern struct building_type bt_harbour;
extern struct building_type bt_academy;
extern struct building_type bt_magictower;
extern struct building_type bt_smithy;
extern struct building_type bt_sawmill;
extern struct building_type bt_stables;
extern struct building_type bt_monument;
extern struct building_type bt_dam;
extern struct building_type bt_caravan;
extern struct building_type bt_tunnel;
extern struct building_type bt_inn;
extern struct building_type bt_stonecircle;
extern struct building_type bt_blessedstonecircle;
extern struct building_type bt_illusion;
extern struct building_type bt_generic;

/* old functions, still in build.c: */
int buildingeffsize(const building * b, boolean img);
void bhash(struct building * b);
void bunhash(struct building * b);
int buildingcapacity(const struct building * b);
void destroy_building(struct building * b);

const struct building_type * findbuildingtype(const char * name, const struct locale * lang);

extern struct building_type * bt_read(FILE * F);
extern void bt_write(FILE * F, const building_type * bt);
extern struct building_type * bt_make(const char * name, int flags, int capacity, int maxcapacity, int maxsize);

#include "build.h"
extern const struct building_type * oldbuildings[MAXBUILDINGTYPES];
#define NOBUILDING NULL

extern void * resolve_building(void * data);
extern void building_write(const struct building * b, FILE * F);
extern int building_read(struct building ** b, FILE * F);

extern struct building *findbuilding(int n);

extern struct unit * buildingowner(const struct region * r, const struct building * b);

#endif
