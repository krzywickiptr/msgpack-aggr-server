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
 * Interface for buffered reads from file descriptor.
 *
 * @author Piotr Krzywicki <krzywicki.ptr@gmail.com>
 * @date 10.05.2019
 */

#ifndef _INPUT_H_
#define _INPUT_H_

#include <pthread.h>

/**
 * Usual min macro, using < as comparison.
 */
#define MIN(x, y) (x) < (y) ? (x) : (y)

/**
 * Size of buffers stored in @ref input_buffer_key.
 */
#define INPUT_BUFFER_SIZE 1024

/**
 * Input buffer, with two-way fill indicator.
 */
struct buffer {
    char buffer[INPUT_BUFFER_SIZE];
    int current, available;
};

/**
 * Container of thread-local input buffers, used by @ref buffered_read_all.
 * Has to be initialized before use by @ref pthread_key_create
 */
extern pthread_key_t input_buffer_key;

void init_input_buffer();

void destroy_input_buffer();

int read_all(int sock, void *output, size_t bytes);

int buffered_read_all(int sock, void *output, size_t bytes);

#endif /* _INPUT_H_ */