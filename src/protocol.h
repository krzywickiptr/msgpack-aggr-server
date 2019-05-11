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
 * Interface for limited part of messagepack format.
 * Sufficient to handle messages like `{"id": uint8_t – uint64_t, "value": uint8_t – uint64_t}`.
 *
 * @author Piotr Krzywicki <krzywicki.ptr@gmail.com>
 * @date 10.05.2019
 */

#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <stdint.h>
#include <stdbool.h>

#define MSGPACK_FIXMAP 0x80
#define MSGPACK_STRING 0xa0

#define MSGPACK_UINT8 0xcc
#define MSGPACK_UINT16 0xcd
#define MSGPACK_UINT32 0xce
#define MSGPACK_UINT64 0xcf
#define MSGPACK_UINT8_FIXNUM_MASK 0x80

#define ID_KEY_NAME "id"
#define ID_KEY_NAME_LENGTH 2
#define VALUE_KEY_NAME "value"
#define VALUE_KEY_NAME_LENGTH 5
#define MAX_KEY_NAME_LENGTH 5
#define NO_KEYS 2

/**
 * Converts 64bit integer byte order from network to host.
 */
#define ntohll(x) \
    ((1==ntohl(1)) ? (x) : \
    ((uint_least64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))

/**
 * High-level message format.
 */
struct message {
    uint_least64_t id;
    uint_least64_t value;
};

/**
 * Messagepack available int types.
 */
union ints {
    uint8_t ui8;
    uint16_t ui16;
    uint32_t ui32;
    uint_least64_t ui64;
};

/**
 * Messagepack int type with header.
 */
struct msgpack_int {
    uint8_t kind;
    union ints value;
};

/**
 * Messagepack string type with header.
 */
struct msgpack_string {
    uint8_t kind;
    char value[MAX_KEY_NAME_LENGTH + 1];
};

/**
 * Messagepack packet of the form `{"id": uint8_t – uint_least64_t, "value": uint8_t – uint_least64_t}`.
 */
struct msgpack_packet {
    uint8_t map_header;

    struct msgpack_string id_key;
    struct msgpack_int id_value;

    struct msgpack_string value_key;
    struct msgpack_int value_value;
};

bool read_msgpack_int(int sock, struct msgpack_int *result);

uint_least64_t msgpack_int_value(struct msgpack_int x);

bool read_packet(int sock, struct msgpack_packet *result);

bool next_message(int sock, struct message *result);

#endif /* _PROTOCOL_H_ */