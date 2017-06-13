#include "neighlist.h"

/*This file implements the data type that represents the given node's
neighbourhood, that is, its connections to its adjacent nodes. We model a node's
neighbours as a simple linked list of edges, with each edge's weight and
associated socket. We only need an operation for adding edges, since nodes
cannot lose connectivity.*/

void add_edge(struct neighbours *neighs, uint32_t weight, uint32_t sock) {
	struct edge *aux;

	/*we insert new edges at the beginning*/
	aux = neighs->head;

	/*allocate and initialize edge*/
	neighs->head = (struct edge*) malloc(sizeof(struct edge));
	neighs->head->weight = weight;
	neighs->head->sock = sock;
	neighs->head->next = aux;

	/*increment number of neighbours*/
	neighs->num += 1;
}

void print_edges(struct neighbours *neighs, FILE *stream) {
	struct edge *aux;
	uint32_t i;

	fprintf(stream, "Edge list [%u]: ", neighs->num);

	/*iterate over edges and print*/
	aux = neighs->head;
	for (i = 0; i < neighs->num; i++) {
		fprintf(stream, "%u[%u] -> ", aux->weight, aux->sock);
		aux = aux->next;
	}
	fprintf(stream, "\n");
}

struct neighbours *init_neighs() {
	struct neighbours *newneighs;

	newneighs = (struct neighbours*) malloc(sizeof(struct neighbours));

	newneighs->num = 0;
	newneighs->head = NULL;

	return newneighs;
}

void free_neighs(struct neighbours *neighs) {
	struct edge *aux;

	aux = neighs->head;
	while(aux != NULL) {
		aux = aux->next;
		free(neighs->head);
		neighs->head = aux;
	}

	free(neighs);
}
