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
 * Error handling and diagnostic macros.
 *
 * @author Piotr Krzywicki <krzywicki.ptr@gmail.com>
 * @date 10.05.2019
 */

#ifndef _ERROR_H_
#define _ERROR_H_

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>

/**
 * Terminates program, displaying errno error message.
 */
#define ERROR(m) \
        fprintf(stderr, "ERROR at %s, %d: %s; %d; %s.\n Terminating.", \
            __FILE__, __LINE__, m, errno, strerror(errno)); \
        exit(EXIT_FAILURE);

/**
 * Calculates value, checks if less than zero, terminates if so.
 */
#define ERROR_CHECK(x) \
    if ((x) < 0) { \
        ERROR(#x) \
    } \
    (void) 0

/**
 * Calculates value, returns if zero.
 */
#define ZERO_RETURN(x) \
    if ((x) == 0) \
        return 0

/**
 * Calculates value, terminates if @p NULL.
 */
#define NULL_CHECK(x) \
    if ((x) == NULL) { \
        ERROR(#x " – should not be NULL") \
    } \
    (void) 0

/**
 * Calculates value, terminates if not zero.
 */
#define NON_ZERO_CHECK(x) \
    if ((x) != 0) { \
        ERROR(#x " — should be zero.") \
    } \
    (void) 0

/**
 * Displays diagnostic output if @p DEBUG defined.
 * @param format — format of the message to display
 * @param ... — format associated args
 */
static inline void diagnostic(const char *format, ...) {
#ifdef DEBUG
    va_list format_arguments;
    va_start(format_arguments, format);
    vfprintf(stderr, format, format_arguments);
    va_end(format_arguments);
#endif
    (void) format;
}

#endif /* _ERROR_H_ */