/* vi: set ts=2:
 *
 *
 *  Eressea PB(E)M host Copyright (C) 1998-2003
 *      Christian Schlittchen (corwin@amber.kn-bremen.de)
 *      Katja Zedel (katze@felidae.kn-bremen.de)
 *      Henning Peters (faroul@beyond.kn-bremen.de)
 *      Enno Rehling (enno@eressea-pbem.de)
 *      Ingo Wilken (Ingo.Wilken@informatik.uni-oldenburg.de)
 *
 *  based on:
 *
 * Atlantis v1.0  13 September 1993 Copyright 1993 by Russell Wallace
 * Atlantis v1.7                    Copyright 1996 by Alex Schr�der
 *
 * This program may not be used, modified or distributed without
 * prior permission by the authors of Eressea.
 * This program may not be sold or used commercially without prior written
 * permission from the authors.
 */

#include <config.h>
#include "eressea.h"
#include "curse.h"

/* kernel includes */
#include "magic.h"
#include "skill.h"
#include "unit.h"
#include "region.h"
#include "race.h"
#include "faction.h"
#include "building.h"
#include "ship.h"
#include "message.h"
#include "objtypes.h"

/* util includes */
#include <util/resolve.h>
#include <util/base36.h>
#include <util/nrmessage.h>
#include <util/rand.h>
#include <util/rng.h>
#include <util/goodies.h>
#include <util/variant.h>

/* libc includes */
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>


/* ------------------------------------------------------------- */

#define MAXENTITYHASH 7919
curse *cursehash[MAXENTITYHASH];

/* -------------------------------------------------------------------------- */
void
c_setflag(curse *c, unsigned int flags)
{
  assert(c);
  c->flags = (c->flags & ~flags) | (flags & (c->type->flags ^ flags));
}
/* -------------------------------------------------------------------------- */
void
c_clearflag(curse *c, unsigned int flags)
{
  assert(c);
  c->flags = (c->flags & ~flags) | (c->type->flags & flags);
}

void
chash(curse *c)
{
  curse *old = cursehash[c->no %MAXENTITYHASH];

  cursehash[c->no %MAXENTITYHASH] = c;
  c->nexthash = old;
}

static void
cunhash(curse *c)
{
  curse **show;

  for (show = &cursehash[c->no % MAXENTITYHASH]; *show; show = &(*show)->nexthash) {
    if ((*show)->no == c->no)
      break;
  }
  if (*show) {
    assert(*show == c);
    *show = (*show)->nexthash;
    c->nexthash = 0;
  }
}

curse *
cfindhash(int i)
{
  curse *old;

  for (old = cursehash[i % MAXENTITYHASH]; old; old = old->nexthash)
    if (old->no == i)
      return old;
  return NULL;
}

/* ------------------------------------------------------------- */
/* at_curse */
void
curse_init(attrib * a) {
  a->data.v = calloc(1, sizeof(curse));
}

int
curse_age(attrib * a)
{
  curse * c = (curse*)a->data.v;
  int result = 0;

  if (c->type->age) {
    result = c->type->age(c);
  }
  if (result!=0) {
    c->duration = 0;
  } else if (c_flags(c) & CURSE_NOAGE) {
    c->duration = 1;
  } else if (c->duration!=INT_MAX) {
    c->duration = max(0, c->duration-1);
  }
  return c->duration;
}

void
destroy_curse(curse * c)
{
  cunhash(c);

  if (c->data.v && c->type->typ == CURSETYP_UNIT) {
    free(c->data.v);
  }
  free(c);
}

void
curse_done(attrib * a) {
  destroy_curse((curse *)a->data.v);
}

/* ------------------------------------------------------------- */

int
curse_read(attrib * a, FILE * f)
{
  variant mageid;
  curse * c = (curse*)a->data.v;
  const curse_type * ct;

  char cursename[64];
  unsigned int flags;

  if (global.data_version >= CURSEVIGOURISFLOAT_VERSION) {
    fscanf(f, "%d %s %u %d %lf %d %d ", &c->no, cursename, &flags,
      &c->duration, &c->vigour, &mageid.i, &c->effect.i);
  } else {
    int vigour;
    fscanf(f, "%d %s %u %d %d %d %d ", &c->no, cursename, &flags,
      &c->duration, &vigour, &mageid.i, &c->effect.i);
    c->vigour = vigour;
  }
  ct = ct_find(cursename);
  assert(ct!=NULL);
  c->type = ct;

  if (global.data_version < CURSEFLAGS_VERSION) {
    c_setflag(c, flags);
  } else {
    c->flags = flags;
  }
  c_clearflag(c, CURSE_ISNEW);

#ifdef CONVERT_DBLINK
  if (global.data_version<DBLINK_VERSION) {
    static const curse_type * cmonster = NULL;
    if (!cmonster) cmonster=ct_find("calmmonster");
    if (ct==cmonster) {
      c->effect.v = uniquefaction(c->effect.i);
    }
  }
#endif

  /* beim Einlesen sind noch nicht alle units da, muss also
   * zwischengespeichert werden. */
  if (mageid.i == -1){
    c->magician = (unit *)NULL;
  } else {
    ur_add(mageid, (void**)&c->magician, resolve_unit);
  }

  if (c->type->read) c->type->read(f, c);
  else if (c->type->typ==CURSETYP_UNIT) {
    curse_unit * cc = calloc(1, sizeof(curse_unit));

    c->data.v = cc;
    fscanf(f, "%d ", &cc->cursedmen);
  }
  if (c->type->typ == CURSETYP_REGION) {
    read_region_reference((region**)&c->data.v, f);
  }
  chash(c);

  return AT_READ_OK;
}

void
curse_write(const attrib * a, FILE * f)
{
  unsigned int flags;
  int mage_no;
  curse * c = (curse*)a->data.v;
  const curse_type * ct = c->type;

  /* copied from c_clearflag */
  flags = (c->flags & ~CURSE_ISNEW) | (c->type->flags & CURSE_ISNEW);

  if (c->magician) {
    mage_no = c->magician->no;
  } else {
    mage_no = -1;
  }

  fprintf(f, "%d %s %d %d %f %d %d ", c->no, ct->cname, flags,
      c->duration, c->vigour, mage_no, c->effect.i);

  if (c->type->write) c->type->write(f, c);
  else if (c->type->typ == CURSETYP_UNIT) {
    curse_unit * cc = (curse_unit*)c->data.v;
    fprintf(f, "%d ", cc->cursedmen);
  }
  if (c->type->typ == CURSETYP_REGION) {
    write_region_reference((region*)c->data.v, f);
  }
}

attrib_type at_curse =
{
  "curse",
  curse_init,
  curse_done,
  curse_age,
  curse_write,
  curse_read,
  ATF_CURSE
};
/* ------------------------------------------------------------- */
/* Spruch identifizieren */

#include "umlaut.h"

typedef struct cursetype_list {
  struct cursetype_list * next;
  const curse_type * type;
} cursetype_list;

cursetype_list * cursetypes[256];

void
ct_register(const curse_type * ct)
{
  unsigned int hash = tolower(ct->cname[0]);
  cursetype_list ** ctlp = &cursetypes[hash];
  while (*ctlp) {
    cursetype_list * ctl = *ctlp;
    if (ctl->type==ct) return;
    ctlp=&ctl->next;
  }
  *ctlp = calloc(1, sizeof(cursetype_list));
  (*ctlp)->type = ct;
}

const curse_type *
ct_find(const char *c)
{
  unsigned int hash = tolower(c[0]);
  cursetype_list * ctl = cursetypes[hash];
  while (ctl) {
    size_t k = min(strlen(c), strlen(ctl->type->cname));
    if (!strncasecmp(c, ctl->type->cname, k)) return ctl->type;
    ctl = ctl->next;
  }
  /* disable this assert to be able to remoce certain curses from the game
   * make sure that all locations using that curse can deal with a NULL
   * return value.
   */
  assert(!"unknown cursetype");
  return NULL;
}

/* ------------------------------------------------------------- */
/* get_curse identifiziert eine Verzauberung �ber die ID und gibt
 * einen pointer auf die struct zur�ck.
 */

boolean
cmp_curse(const attrib * a, const void * data) {
  const curse * c = (const curse*)data;
  if (a->type->flags & ATF_CURSE) {
    if (!data || c == (curse*)a->data.v) return true;
  }
  return false;
}

boolean
cmp_cursetype(const attrib * a, const void * data)
{
  const curse_type * ct = (const curse_type *)data;
  if (a->type->flags & ATF_CURSE) {
    if (!data || ct == ((curse*)a->data.v)->type) return true;
  }
  return false;
}

curse *
get_cursex(attrib *ap, const curse_type * ctype, variant data, boolean(*compare)(const curse *, variant))
{
  attrib * a = a_select(ap, ctype, cmp_cursetype);
  while (a) {
    curse * c = (curse*)a->data.v;
    if (compare(c, data)) return c;
    a = a_select(a->next, ctype, cmp_cursetype);
  }
  return NULL;
}

curse *
get_curse(attrib *ap, const curse_type * ctype)
{
  attrib * a = ap;
  while (a) {
    if (a->type->flags & ATF_CURSE) {
      const attrib_type * at = a->type;
      while (a && a->type==at) {
        curse* c = (curse *)a->data.v;
        if (c->type==ctype) return c;
        a = a->next;
      }
    } else {
      a = a->nexttype;
    }
  }
  return NULL;
}

/* ------------------------------------------------------------- */
/* findet einen curse global anhand seiner 'curse-Einheitnummer' */

curse *
findcurse(int cid)
{
  return cfindhash(cid);
}

/* ------------------------------------------------------------- */
void
remove_curse(attrib **ap, const curse *c)
{
  attrib *a = a_select(*ap, c, cmp_curse);
  if (a) a_remove(ap, a);
}

/* gibt die allgemeine St�rke der Verzauberung zur�ck. id2 wird wie
 * oben benutzt. Dies ist nicht die Wirkung, sondern die Kraft und
 * damit der gegen Antimagie wirkende Widerstand einer Verzauberung */
static double
get_cursevigour(const curse *c)
{
  if (c) return c->vigour;
  return 0;
}

/* setzt die St�rke der Verzauberung auf i */
static void
set_cursevigour(curse *c, double vigour)
{
  assert(c && vigour > 0);
  c->vigour = vigour;
}

/* ver�ndert die St�rke der Verzauberung um +i und gibt die neue
 * St�rke zur�ck. Sollte die Zauberst�rke unter Null sinken, l�st er
 * sich auf.
 */
double
curse_changevigour(attrib **ap, curse *c, double vigour)
{
  vigour += get_cursevigour(c);

  if (vigour <= 0) {
    remove_curse(ap, c);
    vigour = 0;
  } else {
    set_cursevigour(c, vigour);
  }
  return vigour;
}

/* ------------------------------------------------------------- */

int
curse_geteffect(const curse *c)
{
  if (c) return c->effect.i;
  return 0;
}

/* ------------------------------------------------------------- */
static void
set_curseingmagician(struct unit *magician, struct attrib *ap_target, const curse_type *ct)
{
  curse * c = get_curse(ap_target, ct);
  if (c) {
    c->magician = magician;
  }
}

/* ------------------------------------------------------------- */
/* gibt bei Personenbeschr�nkten Verzauberungen die Anzahl der
 * betroffenen Personen zur�ck. Ansonsten wird 0 zur�ckgegeben. */
int
get_cursedmen(unit *u, curse *c)
{
  int cursedmen = u->number;

  if (!c) return 0;

  /* je nach curse_type andere data struct */
  if (c->type->typ == CURSETYP_UNIT) {
    curse_unit * cc = (curse_unit*)c->data.v;
    cursedmen = cc->cursedmen;
  }

  return min(u->number, cursedmen);
}

/* setzt die Anzahl der betroffenen Personen auf cursedmen */
static void
set_cursedmen(curse *c, int cursedmen)
{
  if (!c) return;

  /* je nach curse_type andere data struct */
  if (c->type->typ == CURSETYP_UNIT) {
    curse_unit * cc = (curse_unit*)c->data.v;
    cc->cursedmen = cursedmen;
  }
}

/* ------------------------------------------------------------- */
/* Legt eine neue Verzauberung an. Sollte es schon einen Zauber
 * dieses Typs geben, gibt es den bestehenden zur�ck.
 */
static curse *
make_curse(unit *mage, attrib **ap, const curse_type *ct, double vigour,
    int duration, variant effect, int men)
{
  curse *c;
  attrib * a;

  a = a_new(&at_curse);
  a_add(ap, a);
  c = (curse*)a->data.v;

  c->type = ct;
  c->flags = 0;
  c->vigour = vigour;
  c->duration = duration;
  c->effect = effect;
  c->magician = mage;

  c->no = newunitid();
  chash(c);

  switch (c->type->typ) {
    case CURSETYP_NORM:
      break;

    case CURSETYP_UNIT:
    {
      curse_unit *cc = calloc(1, sizeof(curse_unit));
      cc->cursedmen += men;
      c->data.v = cc;
      break;
    }

  }
  return c;
}


/* Mapperfunktion f�r das Anlegen neuer curse. Automatisch wird zum
 * passenden Typ verzweigt und die relevanten Variablen weitergegeben.
 */
curse *
create_curse(unit *magician, attrib **ap, const curse_type *ct, double vigour,
    int duration, variant effect, int men)
{
  curse *c;

  /* die Kraft eines Spruchs darf nicht 0 sein*/
  assert(vigour > 0);

  c = get_curse(*ap, ct);

  if (c && (c_flags(c) & CURSE_ONLYONE)){
    return NULL;
  }
  assert(c==NULL || ct==c->type);

  /* es gibt schon eins diese Typs */
  if (c && ct->mergeflags != NO_MERGE) {
    if(ct->mergeflags & M_DURATION){
      c->duration = max(c->duration, duration);
    }
    if(ct->mergeflags & M_SUMDURATION){
      c->duration += duration;
    }
    if(ct->mergeflags & M_SUMEFFECT){
      c->effect.i += effect.i;
    }
    if(ct->mergeflags & M_MAXEFFECT){
      c->effect.i = max(c->effect.i, effect.i);
    }
    if(ct->mergeflags & M_VIGOUR){
      c->vigour = max(vigour, c->vigour);
    }
    if(ct->mergeflags & M_VIGOUR_ADD){
      c->vigour = vigour + c->vigour;
    }
    if(ct->mergeflags & M_MEN){
      switch (ct->typ) {
        case CURSETYP_UNIT:
        {
          curse_unit * cc = (curse_unit*)c->data.v;
          cc->cursedmen += men;
        }
      }
    }
    set_curseingmagician(magician, *ap, ct);
  } else {
    c = make_curse(magician, ap, ct, vigour, duration, effect, men);
  }
  return c;
}

/* ------------------------------------------------------------- */
/* hier m�ssen alle c-typen, die auf Einheiten gezaubert werden k�nnen,
 * ber�cksichtigt werden */

static void
do_transfer_curse(curse *c, unit * u, unit * u2, int n)
{
  int cursedmen = 0;
  int men = 0;
  boolean dogive = false;
  const curse_type *ct = c->type;

  switch (ct->typ) {
    case CURSETYP_UNIT:
    {
      curse_unit * cc = (curse_unit*)c->data.v;
      men = cc->cursedmen;
      break;
    }
    default:
      cursedmen = u->number;
  }

  switch ((ct->flags | c->flags) & CURSE_SPREADMASK) {
    case CURSE_SPREADALWAYS:
      dogive = true;
      men = u2->number + n;
      break;

    case CURSE_SPREADMODULO:
    {
      int i;
      int u_number = u->number;
      for (i=0;i<n+1 && u_number>0;i++){
        if (rng_int()%u_number < cursedmen){
          ++men;
          --cursedmen;
          dogive = true;
        }
        --u_number;
      }
      break;
    }
    case CURSE_SPREADCHANCE:
      if (chance(u2->number/(double)(u2->number + n))) {
        men = u2->number + n;
        dogive = true;
      }
      break;
    case CURSE_SPREADNEVER:
      break;
  }

  if (dogive == true) {
    curse * cnew = make_curse(c->magician, &u2->attribs, c->type, c->vigour,
      c->duration, c->effect, men);
    cnew->flags = c->flags;

    if (ct->typ == CURSETYP_UNIT) set_cursedmen(cnew, men);
  }
}

void
transfer_curse(unit * u, unit * u2, int n)
{
  attrib * a;

  a = a_find(u->attribs, &at_curse);
  while (a && a->type==&at_curse) {
    curse *c = (curse*)a->data.v;
    do_transfer_curse(c, u, u2, n);
    a = a->next;
  }
}

/* ------------------------------------------------------------- */

boolean
curse_active(const curse *c)
{
  if (!c) return false;
  if (c_flags(c) & CURSE_ISNEW) return false;
  if (c->vigour <= 0) return false;

  return true;
}

boolean
is_cursed_internal(attrib *ap, const curse_type *ct)
{
  curse *c = get_curse(ap, ct);

  if (!c)
    return false;

  return true;
}


boolean
is_cursed_with(attrib *ap, curse *c)
{
  attrib *a = ap;

  while (a) {
    if ((a->type->flags & ATF_CURSE) && (c == (curse *)a->data.v)) {
      return true;
    }
    a = a->next;
  }

  return false;
}
/* ------------------------------------------------------------- */
/* cursedata */
/* ------------------------------------------------------------- */
/*
 * typedef struct curse_type {
 *  const char *cname; (Name der Zauberwirkung, Identifizierung des curse)
 *  int typ;
 *  spread_t spread;
 *  unsigned int mergeflags;
 *  const char *info_str;  Wirkung des curse, wird bei einer gelungenen Zauberanalyse angezeigt
 *  int (*curseinfo)(const struct locale*, const void*, int, curse*, int);
 *  void (*change_vigour)(curse*, double);
 *  int (*read)(FILE * F, curse * c);
 *  int (*write)(FILE * F, const curse * c);
 * } curse_type;
 */

void *
resolve_curse(variant id)
{
   return cfindhash(id.i);
}

static const char * oldnames[MAXCURSE] = {
  "fogtrap",
  "antimagiczone",
  "farvision",
  "gbdream",
  "auraboost",
  "maelstrom",
  "blessedharvest",
  "drought",
  "badlearn",
  "stormwind",
  "flyingship",
  "nodrift",
  "depression",
  "magicwalls",
  "strongwall",
  "astralblock",
  "generous",
  "peacezone",
  "disorientationzone",
  "magicstreet",
  "magicrunes",
  "badmagicresistancezone",
  "goodmagicresistancezone",
  "slavery",
  "shipdisorientation",
  "calmmonster",
  "oldrace",
  "fumble",
  "riotzone",
  "nocostbuilding",
  "godcursezone",
  "speed",
  "orcish",
  "magicboost",
  "insectfur",
  "strength",
  "worse",
  "magicresistance",
  "itemcloak",
  "sparkle",
  "skillmod"
};

const char *
oldcursename(int id)
{
  return oldnames[id];
}

/* ------------------------------------------------------------- */
message *
cinfo_simple(const void * obj, typ_t typ, const struct curse *c, int self)
{
  struct message * msg;

  unused(typ);
  unused(self);
  unused(obj);

  msg = msg_message(mkname("curseinfo", c->type->cname), "id", c->no);
  if (msg==NULL) {
    log_error(("There is no curseinfo for %s.\n", c->type->cname));
  }
  return msg;
}
