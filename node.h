#ifndef NODE_H
#define NODE_H

/*This file implements all the structural parts of a given node in our network.
Each node has its own local log, plus access to the global distributed log, as
well as its own ID, a list of neighbours with their respective sockets for
communication, and a message queue from which they dequeue messages in order*/

#include <stdio.h>      /*debugging is nice*/
#include <stdint.h>     /*hipster datatypes*/
#include <string.h>     /*memset is king*/
#include <time.h>       /*timestamps*/
#include <unistd.h>     /*sleeps and stuff*/
#include <math.h>       /*because timestamps require work*/

#include <sys/time.h>   /*BETTER timestamps!*/
#include <sys/socket.h> /*communication is the staple of a stable relationship*/

#include "neighlist.h"  /*implementation of neighbour list*/
#include "msgqueue.h"   /*implementation of the node's message queue*/

/*Struct that represents a given node in the network.
A node only knows two things: its own unique ID, and which incoming edges it
has, and their respective weights. Therefore, each node has an ID and a list of
edges, with their weights and associated sockets (so the node can communicate
through them). A node also keeps track of its own local log file, where it
outputs local events, and the global log file, which it inherits from the parent
process.
Each node also has a general message queue, which contains all the messages it
receives from its neighbours, in the proper order*/
struct node {
  uint8_t id;
  FILE *log;
  FILE *globallog;
  struct neighbours *neighs;
  struct msgqueue *queue;
};

/*Struct that stores all the data required for a socket-receiving thread to run.
The threads need to know their respective sockets, as well as a pointer to the
queue where they need to insert the incoming messages*/
struct thread_data {
  struct msgqueue *queue;
  uint32_t sock;
};

/*Initializes the structure to represent a node. At this point we compute all
the information that a node actually has access to, such as its ID, which edges
it has and their respective weights/associated sockets, the number of neighbours
they have, and their local log file.*/
struct node *init_node(int32_t id, uint16_t *edges, uint32_t *socks, uint8_t num,
              FILE *globallog);

/*Initializes the node's algorithm execution, through function implemented in
algo*/
void run_node(struct node *node, void (*algo) (struct node *node));

/*Writes the given log message to given log file. The log file will be either
the node's own local log, or the global distributed log. The log message will
be appropriately timestamped, down to millisecond precision (hopefully).*/
void log_msg(char *msg, FILE *logfile);

/*Essentially terminates a node's existence, freeing its memory, closing its
file descriptors, etc. Technically we don't need to do this, because once all
child processes that run the nodes finish executing they will be deallocated
automatically anyway, but we do this manually to stop Valgrind being a nag*/
void free_node(struct node *node);

/*Receives a socket as input, and waits for incoming messages on the given
socket, adding them to the node's message queue whenever they arrive. This
function will be instantiated by several threads in a given node, with each
thread receiving messages on one socket*/
void *receiver_thread(void *thread_data);

#endif /* NODE_H */
