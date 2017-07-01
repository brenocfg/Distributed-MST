#ifndef ALGORITHM_H
#define ALGORITHM_H

/*This file implements the algorithm to be run on each node. In this case we
are implementing the GHS algorithm for building a distributed Minimum Spanning
Tree. Note that the algorithm itself is agnostic to the node implementation. The
only thing each node does at the beginning of execution is call the function
sent to it through a function pointer.*/

#include <stdio.h>      /*we'll probably have to print something at some point*/

#include "node.h"       /*can't run an algorithm without some guinea pigs*/
#include "neighlist.h"  /*the guinea pigs need to know the other guinea pigs*/

/*For the GHS algorithm, we need to embed some additional data onto nodes. Since
we want to keep the node implementation isolated from the algorithm itself, we
create a new struct to contain such data, rather than change the underlying im-
plementation. This structure will always be passed around along with the node's
base structure itself.
Here's a brief description of the data we store for each node:
  state       -> the node's current state amongst {FIND, FOUND}
  level       -> the node's current fragment level
  fcount      -> the count of neighbours still not reported to this node
  frag_id     -> weight of the edge that identifies the node's current fragment
  num_neighs  -> number of neighbours the node has
  edge_status -> array that keeps track of each of the node's edges' status
  in_branch   -> stores the node's parent in the MST
  test_edge   -> the node's current best candidate edge, which is being tested
  best_edge   -> index of the node's edge that leads to best frag edge
  best_weight -> weight of best_edge, which is minimum outgoing weight*/
struct node_data {
  uint8_t state;
  uint8_t level;
  uint8_t fcount;
  uint8_t frag_id;
  uint8_t num_neighs;
  uint8_t *edge_status;
  uint8_t in_branch;
  int16_t test_edge;
  int16_t best_edge;
  uint16_t best_weight;
};

/*Entry point for the GHS algorithm. Initializes additional data structures for
each node (edge status and whatnot), performs the level 0 behaviour, and runs
the main loop that reacts to messages received*/
void ghs(struct node *node);

/*Processes an incoming CONNECT message, reacting appropriately depending on
the incoming node's level, ID and whatnot*/
void process_connect(struct node *node, struct node_data *ndata,
                          uint8_t edge_index, uint8_t edge_sock, uint8_t *msg);

/*Processes an incoming INITIATE message, which signals the node to begin a new
discovery phase and to propagate the message to its fragment neighbours.*/
void process_initiate(struct node *node, struct node_data *ndata,
                                              uint8_t edge_index, uint8_t *msg);

/*Performs each node's level 0 'wakeup' behaviour, where they find their lowest
weight edge, and send a CONNECT message through that edge*/
void wakeup(struct node *node, struct node_data *data);

/*Creates a message of the specified type, placing its content in the buffer
provided in the input. Returns the length of the created message, in bytes.
Returns 0 if message creation failed.*/
uint8_t create_message(uint8_t type, uint16_t weight, uint8_t level,
                                                uint8_t state, uint8_t *buffer);

#endif /* ALGORITHM_H */
