#include "neighlist.h"

void add_edge(struct neighbours *neighs, uint32_t weight, uint32_t sock) {
	struct edge *aux, *aux2;

	/*in these cases we need to insert at the head*/
	if (neighs->head == NULL || neighs->head->weight > weight) {
		aux = neighs->head;
		neighs->head = (struct edge*) malloc(sizeof(struct edge));
		neighs->head->weight = weight;
		neighs->head->sock = sock;
		neighs->head->next = aux;
	}

	/*otherwise...*/
	else {
		/*find insertion point*/
		aux = neighs->head;
		while (aux->next != NULL && aux->next->weight < weight) {
			aux = aux->next;
		}

		/*allocate new node and insert it in the proper position*/
		aux2 = aux;
		aux = (struct edge*) malloc(sizeof(struct edge));
		aux->next = aux2->next;
		aux->weight = weight;
		aux->sock = sock;
		aux2->next = aux;
	}

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

	/*allocate memory and initialize null/0 values*/
	newneighs = (struct neighbours*) malloc(sizeof(struct neighbours));
	newneighs->num = 0;
	newneighs->head = NULL;

	return newneighs;
}

void free_neighs(struct neighbours *neighs) {
	struct edge *aux;

	/*iterate over list, freeing each member*/
	aux = neighs->head;
	while(aux != NULL) {
		aux = aux->next;
		free(neighs->head);
		neighs->head = aux;
	}

	/*then free the base pointer itself*/
	free(neighs);
}
