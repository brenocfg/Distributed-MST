#ifndef NEIGHLIST_H
#define NEIGHLIST_H

/*This file implements the list of neighbours for any given node. A list of
neighbours is a simple linked list of edges, where each edge represents a
communication link between the node and one of its neighbours. Each edge keeps
track of its own weight, as well as the socket used to communicate through it*/

#include <stdio.h>	//for printing debug info
#include <stdlib.h>	//mallocs, frees and whatnot
#include <stdint.h>	//portable size types (uint8_t, uint32_t, etc)

/*Struct that represents an edge between a node and one of its neighbours. As
our network model describes, each node knows only the weight of the incoming
edges. We also keep track of the socket that the node must use to communicate
through that channel.*/
struct edge {
	uint32_t weight;
	uint32_t sock;
	struct edge *next;
};

/*Struct that represents an entire list of neighbours, with pointer to the first
element, and the number of neighbours (for iterating)*/
struct neighbours {
	struct edge *head;
	uint32_t num;
};

/*Adds a given edge to the node's neighbour list, with the provided weight and
associated socket*/
void add_edge(struct neighbours *neighs, uint32_t weight, uint32_t sock);

/*Prints a node's list of edges. Only for debugging purposes*/
void print_edges(struct neighbours *neighs, FILE *stream);

/*Initializes a list of neighbours. Initially the list has size 0, obviously*/
struct neighbours *init_neighs();

/*Frees the memory allocated for a neighbours list, removing it from existence
:(*/
void free_neighs(struct neighbours *neighs);

#endif /* NEIGHLIST_H */
