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
 * Implementation of buffered reads from file descriptor.
 *
 * @author Piotr Krzywicki <krzywicki.ptr@gmail.com>
 * @date 10.05.2019
 */

#include "input.h"
#include "error.h"

#include <sys/ioctl.h>
#include <unistd.h>

/**
 * Initializes new input buffer for a thread.
 * Has to be destroyed by @ref destroy_input_buffer.
 */
void init_input_buffer() {
    /* Init buffer */
    struct buffer *b;
    NULL_CHECK(b = malloc(sizeof(struct buffer)));
    b->current = b->available = 0;

    /* Save in input_buffer_key store */
    ERROR_CHECK(pthread_setspecific(input_buffer_key, b));
}

/**
 * Frees memory associated with input-buffer.
 */
void destroy_input_buffer() {
    free(pthread_getspecific(input_buffer_key));
    ERROR_CHECK(pthread_setspecific(input_buffer_key, NULL));
}

/**
 * Reads exactly @p bytes, unless connection closes.
 * @param sock — sock to read from
 * @param output[out] — place to store read result
 * @param bytes — number of bytes to store
 * @return number of bytes stored
 */
int read_all(int sock, void *output, size_t bytes) {
    size_t already_read = 0;
    ssize_t read_len;

    /* Read until read all bytes */
    while (already_read != bytes) {
        /* Try to read remaining */
        read_len = read(sock, output + already_read, bytes - already_read);
        already_read += read_len;

        /* Check for socket closure */
        if (read_len == 0)
            return already_read;

        /* Check for read errors */
        ERROR_CHECK(read_len);
    }

    return already_read;
}

/**
 * Reads exactly @p bytes from buffer or from @p sock, prefetches available bytes.
 * @param sock — sock to read from, to buffer, or to @p output
 * @param output[out] — output to store read bytes
 * @param bytes — number of bytes to fetch
 * @return number of bytes read
 */
int buffered_read_all(int sock, void *output, size_t bytes) {
    struct buffer *input_buffer = pthread_getspecific(input_buffer_key);

    size_t available = input_buffer->available - input_buffer->current;
    size_t transfered = MIN(available, bytes);

    /* Copy available bytes from buffer */
    memcpy(output, input_buffer->buffer + input_buffer->current, transfered);
    bytes -= transfered;
    input_buffer->current += transfered;

    /* On buffer boundary fetch rest of bytes from socket */
    size_t total = transfered + read_all(sock, output + transfered, bytes);

    /* If buffer is empty */
    if (input_buffer->current == input_buffer->available) {
        input_buffer->current = 0;

        /* Read available bytes to buffer */
        int count;
        ERROR_CHECK(ioctl(sock, FIONREAD, &count));
        input_buffer->available = read(sock, input_buffer->buffer, MIN(count, INPUT_BUFFER_SIZE));
    }

    return total;
}