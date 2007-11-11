/*
 * $Id: hashtab.c,v 1.2 2007/07/22 13:33:26 khansen Exp $
 * $Log: hashtab.c,v $
 * Revision 1.2  2007/07/22 13:33:26  khansen
 * convert tabs to whitespaces
 *
 * Revision 1.1  2004/06/30 07:55:43  kenth
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

/**
 * This file contains an implementation of a hash table.
 * Both keys and values are of type void *.
 * To implement hashing and key comparison, you have to supply two functions
 * for doing this job. If keys are strings, then you can use the default
 * functions below.
 */

#include <stdlib.h>
#include <string.h>
#include "hashtab.h"

#define SAFE_FREE(a) if (a) { free(a); a = NULL; }

/*---------------------------------------------------------------------------*/

/**
 * Supplies default hashing function when key is a string.
 */
int HASHTAB_STRKEYHSH(void *key)
{
    int i;
    unsigned char *s = (unsigned char *)key;
    int sum = 0;
    for (i=0; s[i] != '\0'; i++) {
        sum += s[i];
    }
    return sum;
}

/**
 * Supplies default key comparison function when key is a string.
 */
int HASHTAB_STRKEYCMP(void *key1, void *key2)
{
    return (strcmp((char *)key1, (char *)key2) == 0) ? 1 : 0;
}

/*---------------------------------------------------------------------------*/

/**
 * Creates a hash table.
 * @param size Number of slots
 * @param keyhsh Procedure which computes h(k)
 * @param keycmp Procedure which compares two keys
 * @return The newly created hash table
 */
hashtab *hashtab_create(int size, keyhashproc keyhsh, keycompareproc keycmp)
{
    int i;
    /* Allocate space for hash table */
    hashtab *ht = (hashtab *)malloc( sizeof(hashtab) );
    if (ht != NULL) {
        /* Allocate space for slots */
        ht->size = size;
        ht->slots = (hashtab_slot **)malloc( sizeof(hashtab_slot *) * size );
        if (ht->slots != NULL) {
            /* Set the hash and comparator procedures */
            ht->keyhsh = keyhsh;
            ht->keycmp = keycmp;
            /* Initialize the slots */
            for (i=0; i<size; i++) {
                ht->slots[i] = NULL;
            }
        }
    }
    /* Return the created hash table */
    return ht;
}

/**
 * Puts something in a hash table.
 * @param ht Hash table
 * @param key Key
 * @param data Data
 */
void hashtab_put(hashtab *ht, void *key, void *data)
{
    int i;
    hashtab_slot *s;
    hashtab_slot *fresh;
    /* Create a new entry */
    fresh = (hashtab_slot *)malloc( sizeof(hashtab_slot) );
    if (fresh != NULL) {
        fresh->key = key;
        fresh->data = data;
        fresh->next = NULL;
        /* Figure out in which slot to put it */
        i = ht->keyhsh(key) % ht->size;
        /* Now put it! */
        if (ht->slots[i] == NULL) {
            /* Start the list */
            ht->slots[i] = fresh;
        }
        else {
            /* Add to end of list */
            for (s = ht->slots[i]; s->next != NULL; s = s->next) ;
            s->next = fresh;
        }
    }
}

/**
 * Gets the data associated with given key.
 * @param ht Hash table
 * @param key Key
 * @return The data associated with key, or <code>NULL</code> if key not found
 */
void *hashtab_get(hashtab *ht, void *key)
{
    hashtab_slot *s;
    int i = ht->keyhsh(key) % ht->size;
    for (s = ht->slots[i]; s != NULL; s = s->next) {
        if (ht->keycmp(key, s->key)) {
            return s->data;
        }
    }
    return NULL;
}

/**
 * Removes something from hash table.
 * @param ht Hash table
 * @param key Key
 * @return Data associated with key
 */
void *hashtab_remove(hashtab *ht, void *key)
{
    hashtab_slot *s;
    int i = ht->keyhsh(key) % ht->size;
    for (s = ht->slots[i]; s != NULL; s = s->next) {
        if (ht->keycmp(key, s->key)) {
            // TODO: remove s...
            return NULL;
        }
    }
    return NULL;
}

/**
 * Finalizes hash table.
 * @param ht The hash table to finalize
 */
void hashtab_finalize(hashtab *ht)
{
    hashtab_slot *s;
    hashtab_slot *t;
    int i;
    for (i=0; i<ht->size; i++) {
        s = ht->slots[i];
        while (s != NULL) {
            t = s->next;
            SAFE_FREE(s);
            s = t;
        }
    }
    SAFE_FREE(ht->slots);
    free(ht);
}
