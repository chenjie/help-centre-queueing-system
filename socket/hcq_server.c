#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "hcq.h"
#include "hcq_server.h"

#define INPUT_BUFFER_SIZE 256
#define INPUT_ARG_MAX_NUM 3
#define DELIM " \n"
#define MAX_USERNAME 30
#define MAX_BACKLOG 5

#ifndef PORT
  #define PORT 30000
#endif


// ============================ helpcentre ============================
// Use global variables so we can have exactly one TA list and one student list
Ta *ta_list = NULL;
Student *stu_list = NULL;

Course *courses;
int num_courses = 3;
// ============================ END ============================

int init_server() {
    // Create the socket FD.
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket");
        exit(1);
    }

    // Reuse port
    int on = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    // Set information about the port (and IP) we want to be connected to.
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    // This should always be zero. On some systems, it won't error if you
    // forget, but on others, you'll get mysterious errors. So zero it.
    memset(&server.sin_zero, 0, 8);

    // Bind the selected port to the socket.
    if (bind(sock_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind");
        close(sock_fd);
        exit(1);
    }

    // Announce willingness to accept connections on this socket.
    if (listen(sock_fd, MAX_BACKLOG) < 0) {
        perror("listen");
        close(sock_fd);
        exit(1);
    }

    return sock_fd;
}

void append_client(Client **client_list_ptr, Client *new_client) {
    if (*client_list_ptr == NULL) {
        *client_list_ptr = new_client;
    } else {
        Client *cur = *client_list_ptr;
        while (cur->next != NULL) {
            cur = cur->next;
        }
        cur->next = new_client;
    }
}

Client *find_client(Client *client_list, int client_fd) {
    if (client_list == NULL) {
        return NULL;
    }
    while (client_list->client_fd != client_fd) {
        client_list = client_list->next;
    }
    return client_list;
}

int main(int argc, char **argv) {
    int sock_fd = init_server();

    if ((courses = malloc(sizeof(Course) * 3)) == NULL) {
        perror("malloc for course list\n");
        exit(1);
    }
    strcpy(courses[0].code, "CSC108");
    strcpy(courses[1].code, "CSC148");
    strcpy(courses[2].code, "CSC209");

    // Initialize a linkedlist to store all client info
    Client *client_list = NULL;

    int max_fd = sock_fd;
    fd_set all_fds, listen_fds;
    FD_ZERO(&all_fds);
    FD_SET(sock_fd, &all_fds);

    while (1) {
        listen_fds = all_fds;
        if (select(max_fd + 1, &listen_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(1);
        }
        if (FD_ISSET(sock_fd, &listen_fds)) {
            // Accept connection
            int client_fd = accept(sock_fd, NULL, NULL);
            if (client_fd < 0) {
                perror("accept");
                close(sock_fd);
                exit(1);
            }
            max_fd = (client_fd > max_fd) ? client_fd : max_fd;
            FD_SET(client_fd, &all_fds);
            printf("=== Accepted connection ===\n");
            Client new_client;
            new_client.client_fd = client_fd;
            new_client.client_name[0] = '\0';
            new_client.client_state = 0;
            new_client.next = NULL;
            append_client(&client_list, &new_client);
            dprintf(client_fd, "Welcome to the Help Centre, what is your name?\r\n");
        }

        // Next, check the clients.
        // NOTE: We could do some tricks with nready to terminate this loop early.
        for (int fd = 3; fd <= max_fd; fd++) {
            if (fd != sock_fd && FD_ISSET(fd, &listen_fds)) {
                Client *target_client = find_client(client_list, fd);

                char read_buf[INPUT_BUFFER_SIZE];
                int num_read = read(fd, read_buf, INPUT_BUFFER_SIZE);
                printf("[%d]\n", num_read);
                if (num_read == 0) {
                    // If it was added to the database before, delete it!!
                    if (target_client->client_state == 3) {
                        if (remove_ta(&ta_list, target_client->client_name) == 1) {
                           dprintf(fd, "Unknown TA deletion error\r\n");
                        }
                    }
                    else if (target_client->client_state == 4) {
                        if (give_up_waiting(&stu_list, target_client->client_name) == 1) {
                            dprintf(fd, "Unknown Student deletion error\r\n");
                        }
                    }

                    target_client->client_fd = -1;
                    target_client->client_state = -1;
                    FD_CLR(fd, &all_fds);
                    printf("Client %d disconnected\n", fd);
                    continue;
                }
                else if (num_read == 2 && read_buf[1] == '\n') {
                    continue;
                }

                if (target_client->client_state == 0) {
                    // 0 - Just connected, no username yet.
                    if (read_buf[num_read-1] == '\n') {
                        read_buf[num_read-2] = '\0';
                        // Name is completed.
                        target_client->client_state = 1;
                    }
                    if (strlen(target_client->client_name) + strlen(read_buf) <= MAX_USERNAME) {
                        strcat(target_client->client_name, read_buf);
                    } else {
                        target_client->client_fd = -1;
                        printf("Username of fd=%d is %lu > 30\n", fd, strlen(read_buf));
                        FD_CLR(fd, &all_fds);
                        close(fd);
                    }
                    if (target_client->client_state == 1) {
                        printf("%s\n", target_client->client_name);
                        dprintf(fd, "Are you a TA or a Student (enter T or S)?\r\n");
                    }
                }
                else if (target_client->client_state == 1) {
                    // 1 - Username is completed, waiting for S or T
                    if (read_buf[0] == 'S' || read_buf[0] == 'T') {
                        if (read_buf[0] == 'S') {
                            dprintf(fd, "Valid courses:");
                            for (int i = 0; i < num_courses; i++) {
                                if (i > 0) {
                                    dprintf(fd, ",");
                                }
                                dprintf(fd, " %s", courses[i].code);
                            }
                            dprintf(fd, "\r\n");
                            dprintf(fd, "Which course are you asking about?\r\n");
                            target_client->client_state = 2;
                        } else if (read_buf[0] == 'T') {
                            add_ta(&ta_list, target_client->client_name);
                            dprintf(fd, "Valid commands for TA:\r\n\tstats\r\n\tnext\r\n\t(or use Ctrl-C to leave)\r\n");
                            target_client->client_state = 3;
                        }
                    } else {
                        dprintf(fd, "Invalid role (enter T or S)\r\n");
                    }
                }
                else if (target_client->client_state == 2) {
                    // 2 - S Waiting for the course code
                    int match = 0;
                    read_buf[num_read] = '\0';
                    printf("%s\n", read_buf);
                    for (int i = 0; i < num_courses; i++) {
                        if (strcmp(read_buf, courses[i].code) == 0) {
                            match = 1;
                            int result = add_student(&stu_list, target_client->client_name, courses[i].code, courses, num_courses);
                            if (result == 1) {
                                dprintf(fd, "You are already in the queue and cannot be added again for any course. Good-bye.\r\n");
                                target_client->client_fd = -1;
                                FD_CLR(fd, &all_fds);
                                close(fd);
                            } else if (result == 2) {
                                dprintf(fd, "This is not a valid course. Good-bye.\r\n");
                                target_client->client_fd = -1;
                                FD_CLR(fd, &all_fds);
                                close(fd);
                            } else {
                                target_client->client_state = 4;
                            }
                            break;
                        }
                    }
                    if (match == 0) {
                        dprintf(fd, "This is not a valid course. Good-bye.\r\n");
                        target_client->client_fd = -1;
                        FD_CLR(fd, &all_fds);
                        close(fd);
                    }
                }
                else if (target_client->client_state == 3) {
                    read_buf[num_read-2] = '\0';
                    if (strcmp(read_buf, "stats") == 0) {
                        char *full_queue = print_full_queue(stu_list);
                        dprintf(fd, "%s", full_queue);
                        free(full_queue);
                    } else if (strcmp(read_buf, "next") == 0) {
                        if (next_overall(target_client->client_name, &ta_list, &stu_list) == 1) {;
                           dprintf(fd, "Unknown TA\r\n");
                        }
                    } else {
                        dprintf(fd, "Incorrect syntax\r\n");
                    }
                }
            }
        }
    }

    // Should never get here.
    return 1;
}
