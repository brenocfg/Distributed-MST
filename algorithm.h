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

void ghs(struct node *node);

#endif /* ALGORITHM_H */
