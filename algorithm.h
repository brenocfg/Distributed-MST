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
  uint16_t num_neighs;
  uint8_t *edge_status;
  uint8_t in_branch;
  int16_t test_edge;
  int16_t best_edge;
  uint16_t best_weight;
};

/*Edges can be in one of three states: REJECT (not part of MSG), UNKNOWN (unde-
cided) or BRANCH (part of MSG). When algorithm terminates every edge in any gi-
ven node must be either BRANCH or REJECT.*/
enum EDGE_STATUS {
  EDGE_REJECT = -1,
  EDGE_UNKNOWN,
  EDGE_BRANCH
};

/*All the message types in the algorithm. This will be the first byte in any
message sent.*/
enum MSG_TYPES {
  MSG_CONNECT = 0,
  MSG_INITIATE,
  MSG_TEST,
  MSG_ACCEPT,
  MSG_REJECT,
  MSG_CHGROOT,
  MSG_REPORT
};

/*Nodes are always either in the FIND state, where they're waiting to discover
their lowest outgoing edge, or in the FOUND state, where the edge has been found
and they are in the process of reporting it. We do not implement the 'Sleeping'
state described in the original algorithm, because we technically 'wake up'
every node at the beginning of execution, so it would be pointless.*/
enum NODE_STATES {
  NODE_FIND = 0,
  NODE_FOUND
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

void process_test(struct node *node, struct node_data *ndata,uint8_t edge_index,
                                            uint8_t edge_sock, uint8_t *msg);

/*Performs each node's level 0 'wakeup' behaviour, where they find their lowest
weight edge, and send a CONNECT message through that edge*/
void wakeup(struct node *node, struct node_data *data);

/*Picks out the node's lowest cost edges which have not been rejected or inclu-
ded in the MST yet, and queries the node on the other end to find out if the
edge leads to outside the node's current fragment or not. Eventually either
picks the node's next candidate edge, or finds out that all of the node's edges
have already been rejected or included in the MST.*/
void test(struct node *node, struct node_data *ndata);

/*Creates a message of the specified type, placing its content in the buffer
provided in the input. Returns the length of the created message, in bytes.
Returns 0 if message creation failed.*/
uint8_t create_msg(uint8_t type, uint16_t weight, uint8_t level, uint16_t frag,
                                                uint8_t state, uint8_t *buffer);

#endif /* ALGORITHM_H */
