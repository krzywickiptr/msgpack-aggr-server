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
 * Implementation for limited part of messagepack format.
 * Sufficient to handle messages like `{"id": uint8_t – uint64_t, "value": uint8_t – uint64_t}`.
 *
 * @author Piotr Krzywicki <krzywicki.ptr@gmail.com>
 * @date 10.05.2019
 */

#include "protocol.h"
#include "error.h"
#include "input.h"

#include <netinet/in.h>

/**
 * Reads and parses int from @p sock.
 * @param sock[buf] — sock to read from
 * @param result[out] — output to read into
 * @return @p true on success, @p false otherwise – disconnect or invalid header
 */
bool read_msgpack_int(int sock, struct msgpack_int *result) {
    /* Read variable number of bytes, switching on result->kind header */
    switch (result->kind) {
        case MSGPACK_UINT8:
            ZERO_RETURN(buffered_read_all(sock, &result->value.ui8,
                                          sizeof(result->value.ui8)) == sizeof(result->value.ui8));
            break;

        case MSGPACK_UINT16:
            ZERO_RETURN(buffered_read_all(sock, &result->value.ui16,
                                          sizeof(result->value.ui16)) == sizeof(result->value.ui16));
            result->value.ui16 = ntohs(result->value.ui16);
            break;

        case MSGPACK_UINT32:
            ZERO_RETURN(buffered_read_all(sock, &result->value.ui32,
                                          sizeof(result->value.ui32)) == sizeof(result->value.ui32));
            result->value.ui32 = ntohl(result->value.ui32);
            break;

        case MSGPACK_UINT64:
            ZERO_RETURN(buffered_read_all(sock, &result->value.ui64,
                                          sizeof(result->value.ui64)) == sizeof(result->value.ui64));
            result->value.ui64 = ntohll(result->value.ui64);
            break;

        default:
            /* We don't accept signed ints */
            if (result->kind & MSGPACK_UINT8_FIXNUM_MASK) return false;
            /* Fixnum is encoded in the header */
            result->value.ui8 = result->kind;
    }
    return true;
}

/**
 * Converts messagepack int to uint_least64_t.
 * @param x — to convert
 * @return converted @p x
 */
uint_least64_t msgpack_int_value(struct msgpack_int x) {
    switch (x.kind) {
        case MSGPACK_UINT16:
            return x.value.ui16;
        case MSGPACK_UINT32:
            return x.value.ui32;
        case MSGPACK_UINT64:
            return x.value.ui64;
        default:
            /* Fixint or one byte int with header */
            return x.value.ui8;
    }
}

/**
 * Reads and parse messagepack packet from @p sock.
 * @param sock[buf] — sock to read from
 * @param result[out] — output to read into
 * @return @p true on success, @p false otherwise – disconnect or invalid headers
 */
bool read_packet(int sock, struct msgpack_packet *result) {
    /* Read, parse, validate fixmap header */
    ZERO_RETURN(buffered_read_all(sock, &result->map_header,
                                  sizeof(result->map_header)) == sizeof(result->map_header));
    ZERO_RETURN(result->map_header == (MSGPACK_FIXMAP | NO_KEYS));

    /* Read, parse, validate id section */
    ZERO_RETURN(buffered_read_all(sock, &result->id_key.kind,
                                  sizeof(result->id_key.kind)) == sizeof(result->id_key.kind));
    ZERO_RETURN(result->id_key.kind == (MSGPACK_STRING | ID_KEY_NAME_LENGTH));

    ZERO_RETURN(buffered_read_all(sock, result->id_key.value,
                                  ID_KEY_NAME_LENGTH) == ID_KEY_NAME_LENGTH);
    result->id_key.value[ID_KEY_NAME_LENGTH] = '\0';
    ZERO_RETURN(strcmp(result->id_key.value, ID_KEY_NAME) == 0);

    ZERO_RETURN(buffered_read_all(sock, &result->id_value.kind,
                                  sizeof(result->id_value.kind)) == sizeof(result->id_value.kind));
    ZERO_RETURN(read_msgpack_int(sock, &result->id_value));

    /* Read, parse, validate value section */
    ZERO_RETURN(buffered_read_all(sock, &result->value_key.kind,
                                  sizeof(result->value_key.kind)) == sizeof(result->value_key.kind));
    ZERO_RETURN(result->value_key.kind == (MSGPACK_STRING | VALUE_KEY_NAME_LENGTH));

    ZERO_RETURN(buffered_read_all(sock, result->value_key.value,
                                  VALUE_KEY_NAME_LENGTH) == VALUE_KEY_NAME_LENGTH);
    result->value_key.value[VALUE_KEY_NAME_LENGTH] = '\0';
    ZERO_RETURN(strcmp(result->value_key.value, VALUE_KEY_NAME) == 0);

    ZERO_RETURN(buffered_read_all(sock, &result->value_value.kind,
                                  sizeof(result->value_value.kind)) == sizeof(result->value_value.kind));
    ZERO_RETURN(read_msgpack_int(sock, &result->value_value));

    return true;
}

/**
 * Calls @ref read_packet, rewrites it to @ref message.
 * @param sock[buf] — sock to read from
 * @param result[out] — output to rewrite @ref read_packet result
 * @return @p true on success, @p false otherwise – on disconnect or invalid data
 */
bool next_message(int sock, struct message *result) {
    /* Read message from sock */
    struct msgpack_packet packet;
    ZERO_RETURN(read_packet(sock, &packet));

    /* Convert to message struct */
    result->id = msgpack_int_value(packet.id_value);
    result->value = msgpack_int_value(packet.value_value);
    return true;
}