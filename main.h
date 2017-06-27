#ifndef MAIN_H
#define MAIN_H

/*This file is the entry point for our program. The functions implemented here
are mostly related only to parsing the input parameters, computing the network
topology, then launching the processes that represent each node.*/

#include <unistd.h>     /*forking is fun, bombing is not*/
#include <time.h>       /*rand seeds and timestamps*/
#include <stdint.h>     /*standard types are pretty*/
#include <stdio.h>      /*what's computing without some input?*/
#include <stdlib.h>     /*because the heap wants to be used and abused*/
#include <sys/socket.h> /*UNIX sockets yay*/
#include <sys/wait.h>

#include "node.h"       /*implementation of a distributed node*/
#include "algorithm.h"  /*algorithm to be run (GHS in this case)*/

/*computes a connectivity matrix, where edges[i][j] being positive will
correspond to nodes i and j being neighbours, and the value of the cell itself
will be the weight of the edge.
this is the DENSE connectivity version, which computes (n-1)(n-2)/2 + 1 edges,
guaranteeing a connected undirected graph, for any number of nodes*/
uint16_t *compute_dense_connectivity(uint8_t num_nodes);

/*computes a connectivity matrix, where edges[i][j] being positive will
correspond to nodes i and j being neighbours, and the value of the cell itself
will be the weight of the edge.
this is the SPARSE connectivity version, which computes (n-1) edges in a line
at first, which connects the graph, then generates a random small additional
amount of edges, for some added complexity, while keeping the network smallish*/
uint16_t *compute_sparse_connectivity(uint8_t num_nodes);

/*initializes socket pairs for each edge in the graph, essentially creating
the communication channels between the nodes.
NOTE: this is not a very efficient way of doing this, since we're going through
n^2 matrix cells to create not as many sockets, but the number of nodes will
never be too large anyway, so we're ok with it*/
uint32_t *init_sockets(uint16_t *edges, uint8_t num_nodes);

/*prints adjacency matrix and socket map to given stream, for debug purposes*/
void print_network(uint16_t *edges, uint16_t *socks, uint8_t num, FILE *stream);

#endif /* MAIN_H */
