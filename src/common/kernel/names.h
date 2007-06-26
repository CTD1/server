/* vi: set ts=2:
 *
 *	
 *	Eressea PB(E)M host Copyright (C) 1998-2003
 *      Christian Schlittchen (corwin@amber.kn-bremen.de)
 *      Katja Zedel (katze@felidae.kn-bremen.de)
 *      Henning Peters (faroul@beyond.kn-bremen.de)
 *      Enno Rehling (enno@eressea-pbem.de)
 *      Ingo Wilken (Ingo.Wilken@informatik.uni-oldenburg.de)
 *
 * This program may not be used, modified or distributed without
 * prior permission by the authors of Eressea.
 */


#ifndef H_KRNL_NAMES
#define H_KRNL_NAMES
#ifdef __cplusplus
extern "C" {
#endif
extern void register_names(void);
const xmlChar *undead_name(const struct unit * u);
const xmlChar *skeleton_name(const struct unit * u);
const xmlChar *zombie_name(const struct unit * u);
const xmlChar *ghoul_name(const struct unit * u);
const xmlChar *dragon_name(const struct unit *u);
const xmlChar *dracoid_name(const struct unit *u);
const xmlChar *generic_name(const struct unit *u);
const xmlChar *abkz(const xmlChar *s, size_t max);

#ifdef __cplusplus
}
#endif
#endif
