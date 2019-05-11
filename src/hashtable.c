/*
    Simple key-value aggregation concurrent server.
    Copyright (C) 2019  Piotr Krzywicki

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file
 * Implementation of static hashtable with linear collision resolving.
 *
 * @author Piotr Krzywicki <krzywicki.ptr@gmail.com>
 * @date 10.05.2019
 */

#include "hashtable.h"

#include <malloc.h>

/**
 * Calculates (0 =<) hash (< number of buckets) for a key.
 * @param id — key to calculate hash for
 * @return uniform hash for key @p id
 */
static uint16_t hashtable_hash(uint_least64_t id) {
    /* MurmurHash3 Mixer,
     * Read http://zimbry.blogspot.com/2011/09/better-bit-mixing-improving-on.html */
    id ^= id >> 33;
    id *= 0xff51afd7ed558ccd;
    id ^= id >> 33;
    id *= 0xc4ceb9fe1a85ec53;
    id ^= id >> 33;

    /* Right shift calculated hash to fit in index space */
    return (uint16_t) (id >> (64 - HASHTABLE_SIZE_LOG));
}

/**
 * Adds empty entry at the beginning of bucket corresponding to @p id.
 * @param id — id associated with new entry
 * @return pointer to new entry, or @p NULL on @p malloc failure
 */
static struct entry_t *hashtable_add(uint_least64_t id) {
    /* Create empty entry */
    struct entry_t *new = malloc(sizeof(struct entry_t));
    if (!new) return NULL;
    new->id = id;
    new->count = 0;

    /* Append to the corresponding bucket */
    uint16_t hash = hashtable_hash(id);
    new->next = hashtable[hash];
    return hashtable[hash] = new;
}

/**
 * Finds or create entry associated with key @p id.
 * @param id — key, for which entry is being searched or created
 * @return pointer to found or newly created entry
 */
struct entry_t *hashtable_get(uint_least64_t id) {
    /* Find entry corresponding to id */
    for (struct entry_t *it = hashtable[hashtable_hash(id)]; it != NULL; it = it->next)
        if (it->id == id)
            return it;

    /* Otherwise add new entry */
    return hashtable_add(id);
}

/**
 * Removes entry associated with key @p id
 * @param id — key, for which entry is being removed
 */
void hashtable_remove(uint_least64_t id) {
    /* Find link pointing to entry with given id */
    struct entry_t **it;
    for (it = &hashtable[hashtable_hash(id)];
         *it != NULL && (*it)->id != id;
         it = &(*it)->next);

    if (*it != NULL) {
        /* Remove entry from the bucket */
        struct entry_t *subsititue = (*it)->next;
        free(*it);
        *it = subsititue;
    }
}