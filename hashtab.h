/*
 * $Id: hashtab.h,v 1.2 2007/07/22 13:35:20 khansen Exp $
 * $Log: hashtab.h,v $
 * Revision 1.2  2007/07/22 13:35:20  khansen
 * convert tabs to whitespaces
 *
 * Revision 1.1  2004/06/30 07:56:26  kenth
 * Initial revision
 *
 */

/**
 *    (C) 2004 Kent Hansen
 *
 *    The XORcyst is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    The XORcyst is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with The XORcyst; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef HASHTAB_H
#define HASHTAB_H

/** Linked list of entries in hash table slot */
struct tag_hashtab_slot
{
    void *key;
    void *data;
    struct tag_hashtab_slot *next;
};

typedef struct tag_hashtab_slot hashtab_slot;

/** Signature for procedure that hashes a key to a slot */
typedef int (*keyhashproc)(void *);

/** Signature for procedure that compares two keys */
typedef int (*keycompareproc)(void *, void *);

/** Hash table */
struct tag_hashtab
{
    hashtab_slot **slots;
    int size;
    keyhashproc keyhsh;
    keycompareproc keycmp;
};

typedef struct tag_hashtab hashtab;

/** Function prototypes */
hashtab *hashtab_create(int, keyhashproc, keycompareproc);
void hashtab_put(hashtab *, void *, void *);
void *hashtab_get(hashtab *, void *);
void *hashtab_remove(hashtab *, void *);
void hashtab_finalize(hashtab *);

extern int HASHTAB_STRKEYHSH(void *);
extern int HASHTAB_STRKEYCMP(void *, void *);

#endif  /* !HASHTAB_H */
