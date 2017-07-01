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
  branch_sock -> stores a reference to the in-branch edge's socket
  test_edge   -> the node's current best candidate edge, which is being tested
  best_edge   -> index of the node's edge that leads to best frag edge
  best_weight -> weight of best_edge, which is minimum outgoing weight
  best_sock   -> tracks the socket for the node's best_edge*/
struct node_data {
  uint8_t state;
  uint8_t level;
  uint8_t fcount;
  uint8_t frag_id;
  uint16_t num_neighs;
  uint8_t *edge_status;
  uint8_t in_branch;
  uint32_t branch_sock;
  int16_t test_edge;
  int16_t best_edge;
  uint16_t best_weight;
  uint32_t best_sock;
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
                        uint16_t edge_index, uint32_t edge_sock, uint8_t *msg);

/*Processes an incoming INITIATE message, which signals the node to begin a new
discovery phase and to propagate the message to its fragment neighbours.*/
void process_initiate(struct node *node, struct node_data *ndata,
                        uint16_t edge_index, uint32_t edge_sock, uint8_t *msg);

/*Processes an incoming test message, meaning a neighbour is probing the node
for whether they are in the same fragment. This will either ACCEPT or REJECT the
edge in question. The node might also have to delay its response, if the probing
neighbour is at a higher level.*/
void process_test(struct node *node,struct node_data *ndata,uint16_t edge_index,
                                            uint32_t edge_sock, uint8_t *msg);

/*Processes an incoming ACCEPT message, meaning the edge the node just probed
should become its best edge, and be a candidate for next edge to be included in
the MST for that fragment.*/
void process_accept(struct node *node, struct node_data *ndata,
                        uint16_t edge_index, uint32_t edge_sock, uint8_t *msg);

/*Processes an incoming REJECT message, meaning the edge we just probed leads
to the same fragment and should be rejected. The node simply updates that edge's
status to REJECT and moves on to testing its other edges.*/
void process_reject(struct node *node, struct node_data *ndata,
                                            uint16_t edge_index, uint8_t *msg);

/*Processes an incoming REPORT message, meaning one the node's neighbours
finished its discovery phase and reported its LWOE. The node simply updates its
best edge if the reported edge has lower cost, and reports to its 'parent'.
More importantly, this function is also what determines termination for each
node. If a node receives a report with maximum weight, it means no edge was
selected, which means the algorithm is done, so it terminates.*/
uint8_t process_report(struct node *node, struct node_data *ndata,
                        uint16_t edge_index, uint32_t edge_sock, uint8_t *msg);

/*Passes the CHGROOT message forward, if the node is not the root for the newly
formed fragment. If node is the new root, pass the CONNECT message across
fragment boundaries to create new fragment.*/
void changeroot(struct node *node, struct node_data *ndata);

/*Performs each node's level 0 'wakeup' behaviour, where they find their lowest
weight edge, and send a CONNECT message through that edge*/
void wakeup(struct node *node, struct node_data *data);

/*Picks out the node's lowest cost edges which have not been rejected or inclu-
ded in the MST yet, and queries the node on the other end to find out if the
edge leads to outside the node's current fragment or not. Eventually either
picks the node's next candidate edge, or finds out that all of the node's edges
have already been rejected or included in the MST.*/
void test(struct node *node, struct node_data *ndata);

/*Reports the node's LWOE to its 'parent' in the MST, effectively ending its
discovery phase. Now it waits to either be elected the new core, or to be sent
into a new discovery phase, by receiving an INITIATE message from the next
fragment level*/
void report(struct node *node, struct node_data *ndata);

/*Creates a message of the specified type, placing its content in the buffer
provided in the input. Returns the length of the created message, in bytes.
Returns 0 if message creation failed.*/
uint8_t create_msg(uint8_t type, uint16_t weight, uint8_t level, uint16_t frag,
                                                uint8_t state, uint8_t *buffer);

#endif /* ALGORITHM_H */
