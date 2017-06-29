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

/*Entry point for the GHS algorithm. Initializes algorithm specific information
(such as edge statuses and whatnot), performs the level 0 behaviour, and runs
the main loop that reacts to messages received*/
void ghs(struct node *node);

/*Processes an incoming CONNECT message, reacting appropriately depending on
the incoming node's level, ID and whatnot*/
void process_connect(struct node *node, uint8_t *edge_status, uint8_t *level,
                                                                  uint8_t *msg);
                                                                  
/*Performs each node's level 0 'wakeup' behaviour, where they find their lowest
weight edge, and send a CONNECT message through that edge*/
void wakeup(struct node *node, uint8_t *edge_status, uint8_t *level);

#endif /* ALGORITHM_H */
