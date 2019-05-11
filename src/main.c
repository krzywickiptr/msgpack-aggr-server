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
 * Implementation of key-value aggregation server.
 *
 * @author Piotr Krzywicki <krzywicki.ptr@gmail.com>
 * @date 10.05.2019
 */

#include "hashtable.h"
#include "protocol.h"
#include "input.h"
#include "error.h"

#include <netinet/in.h>
#include <unistd.h>
#include <inttypes.h>

/**
 * Maximum number of pending connections on main-thread socket.
 */
#define BACKLOG_LENGTH 64
/**
 * Port, on which server listen to incoming connections.
 */
#define PORT 8080

/**
 * Mutex to @ref hashtable, accessed by multiple @ref handle_connection.
 */
pthread_mutex_t hashtable_mutex;
/**
 * Hashtable to store id – value mapping with partial aggregation.
 */
struct entry_t *hashtable[HASHTABLE_SIZE] = {NULL};

/**
 * Definition of container of thread-local input buffers.
 */
pthread_key_t input_buffer_key;


/**
 * Reads incoming packets and aggregates them.
 * @param sock_ptr[owner] — wrapped client socket
 * @return EXIT_SUCCESS or terminates program at serious failure.
 */
void *handle_connection(void *sock_ptr) {
    int sock = *(int *) sock_ptr;
    /* Init thread local input buffer */
    init_input_buffer();

    /* Iterate over every message in a stream */
    for (struct message m; next_message(sock, &m);) {
        diagnostic("Got message; id: %" PRIuLEAST64 ", value: %" PRIuLEAST64 ".\n", m.id, m.value);

        /* Process entry in critical section */
        ERROR_CHECK(pthread_mutex_lock(&hashtable_mutex));

        /* Add id — value mapping to hashtable */
        struct entry_t *entry;
        NULL_CHECK(entry = hashtable_get(m.id));
        entry->values[entry->count++] = m.value;

        if (entry->count == VALUES_THRESHOLD) {
            /* Print and remove aggregated entries */
            printf("id: %" PRIuLEAST64 ", values: %" PRIuLEAST64 ", %" PRIuLEAST64 ", %" PRIuLEAST64 "\n",
                   entry->id, entry->values[0], entry->values[1], entry->values[2]);
            fflush(stdout);
            hashtable_remove(entry->id);
        }

        ERROR_CHECK(pthread_mutex_unlock(&hashtable_mutex));
    }

    /* Release resources */
    free(sock_ptr);
    destroy_input_buffer();
    close(sock);

    /* Terminate thread */
    diagnostic("Thread exited.\n");
    pthread_exit(EXIT_SUCCESS);
}

/**
 * Initializes server and listen to incoming connections.
 * @return [noreturn]
 */
int main() {
    /* Initialize thread local input buffers */
    pthread_key_create(&input_buffer_key, NULL);

    /* Init synchronization mechanisms */
    ERROR_CHECK(pthread_mutex_init(&hashtable_mutex, NULL));

    /* Create server socket */
    int server_sock;
    ERROR_CHECK(server_sock = socket(AF_INET, SOCK_STREAM, 0));

    /* Ignore TIME_WAIT tcp state on server termination */
    int enable = 1;
    ERROR_CHECK(setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)));

    /* Bind server socket */
    struct sockaddr_in listen_addr;
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    listen_addr.sin_port = htons(PORT);
    ERROR_CHECK(bind(server_sock, (struct sockaddr *) &listen_addr, sizeof(listen_addr)));

    /* Switch to listening mode */
    ERROR_CHECK(listen(server_sock, BACKLOG_LENGTH));
    diagnostic("main-thread: Listening on port %d.\n", PORT);

    /* Pthread attribute to create detached threads */
    pthread_attr_t detached_attr;
    NON_ZERO_CHECK(pthread_attr_init(&detached_attr));
    NON_ZERO_CHECK(pthread_attr_setdetachstate(&detached_attr, PTHREAD_CREATE_DETACHED));

    for (;;) {
        /* Accept incoming connection */
        int client_sock;
        ERROR_CHECK(client_sock = accept(server_sock, NULL, NULL));
        diagnostic("main-thread: Accepted client connection.\n");

        /* Launch new thread to handle it */
        pthread_t thread;
        int *client_sock_ptr;
        NULL_CHECK(client_sock_ptr = malloc(sizeof(int)));
        *client_sock_ptr = client_sock;
        ERROR_CHECK(pthread_create(&thread, &detached_attr, handle_connection, client_sock_ptr));
        diagnostic("main-thread: Launched new detached thread.\n");
    }
}
