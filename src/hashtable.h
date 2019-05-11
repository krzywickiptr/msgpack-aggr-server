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
 * Interface for static hashtable with linear collision resolving.
 *
 * @author Piotr Krzywicki <krzywicki.ptr@gmail.com>
 * @date 10.05.2019
 */

#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

#include <stdint.h>

/**
 * Number of buckets.
 */
#define HASHTABLE_SIZE 16384
#define HASHTABLE_SIZE_LOG 14
/**
 * Max number of same values associated with key.
 */
#define VALUES_THRESHOLD 3

/**
 * Linked hashtable entry.
 */
struct entry_t {
    uint_least64_t id;
    uint8_t count;
    uint_least64_t values[VALUES_THRESHOLD];
    struct entry_t *next;
};

/**
 * Static hashtable, as an array of buckets.
 */
extern struct entry_t *hashtable[HASHTABLE_SIZE];

struct entry_t *hashtable_get(uint_least64_t id);

void hashtable_remove(uint_least64_t id);

#endif /* _HASHTABLE_H_ */