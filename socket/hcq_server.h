#ifndef HCQ_SERVER_H
#define HCQ_SERVER_H

#define MAX_USERNAME 30

// 0 - Just connected, no username yet.
// 1 - Username is completed, waiting for S or T
// 2 - S Waiting for the course code
// 3 - T Done adding the TA
// 4 - Student is added to the database
struct client {
    int client_fd;
    char client_name[MAX_USERNAME+1];
    int client_state;
    struct client *next;
};

typedef struct client Client;

int init_server();
void append_client(Client **client_list_ptr, Client *new_client);
Client *find_client(Client *client_list, int client_fd);


#endif
