#ifndef NODE_H
#define NODE_H

#include <stdio.h>      /*debugging is nice*/
#include <string.h>     /*memset is king*/
#include <time.h>       /*timestamps*/
#include <unistd.h>     /*sleeps and stuff*/
#include <sys/socket.h> /*communication is the staple of a stable relationship*/
#include "neighlist.h"

/*Struct that represents a given node in the network.
A node only knows two things: its own unique ID, and which incoming edges it
has, and their respective weights. Therefore, each node has an ID and a list of
edges, with their weights and associated sockets (so the node can communicate
through them). A node also keeps track of its own local log file, where it
outputs local events, and the global log file, which it inherits from the parent
process.*/
struct node {
  uint8_t id;
  FILE *log;
  FILE *globallog;
  struct neighbours *neighs;
};

/*Initializes the structure to represent a node. At this point we compute all
the information that a node actually has access to, such as its ID, which edges
it has and their respective weights/associated sockets, the number of neighbours
they have, and their local log file.*/
struct node *init_node(int32_t id, uint16_t *edges, uint16_t *socks, uint8_t num,
              FILE *globallog);

void test_node(struct node *node);

/*Essentially terminates a node's existence, freeing its memory, closing its
file descriptors, etc. Technically we don't need to do this, because once all
child processes that run the nodes finish executing they will be deallocated
automatically anyway, but we do this manually to stop Valgrind being a nag*/
void free_node(struct node *node);

#endif /* NODE_H */
