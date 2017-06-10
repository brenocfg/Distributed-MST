#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*computes a connectivity matrix, where edges[i][j] being positive will
correspond to nodes i and j being neighbours, and the value of the cell itself
will be the weight of the edge, as well as the id for the file descriptor of
the socket used to implement the edge. This is safe because to simplify this
implementation of GHS, we can assume all edges have distinct weights

this is the DENSE connectivity version, which computes (n-1)(n-2)/2 + 1 edges,
which guarantees a connected undirected graph, for any number of nodes*/
uint16_t *compute_dense_connectivity(uint8_t num_nodes) {
	uint16_t *edges, *weights;
	uint16_t num_edges, goal;

	goal = (((num_nodes-1) * (num_nodes-2)) / 2) + 1;
	num_edges = 0;

	edges = calloc(num_nodes*num_nodes, sizeof(uint16_t));
	weights = calloc(num_nodes, sizeof(uint8_t));

	srand(time(NULL));
	while(num_edges < goal) {
		uint16_t v1 = rand();
		uint16_t v2 = rand();

		fprintf(stderr, "v1 = %d, v2 = %d\n", v1, v2);

		if (v1 == v2 || edges[v1*num_nodes + v2]) {
			fprintf(stderr, "invalid edge!\n");
			continue;
		}

		uint16_t weight = -1;
		while (weight < 1) {
			fprintf(stderr, "1 weight = %d\n", weight);
			weight = rand();
			fprintf(stderr, "2 weight = %d\n", weight);
			weight = (weights[weight]) ? weight : -1;
			fprintf(stderr, "3 weight = %d\n", weight);
		}
	}
	return NULL;
}

/*entry point*/
int main (int argc, char *argv[]) {
	uint8_t num_nodes;
	uint16_t *edges;

	/*check for number of input arguments*/
	if (argc != 2) {
		fprintf(stderr, "Not enough arguments!\n");
		fprintf(stderr, "Usage: ./ghs <number nodes> (range: [15:100])\n");
		return 0;
	}

	/*compute number of nodes and check for validity*/
	num_nodes = atoi(argv[1]);
	if (num_nodes > 100) {
		fprintf(stderr, "Too many nodes! (max: 100)\n");
		fprintf(stderr, "Fork bombing is bad and you should feel bad!\n");
		return 0;
	}
	if (num_nodes < 15) {
		fprintf(stderr, "Not enough nodes! (min: 15)\n");
		fprintf(stderr, "Why? Because the professor said so.\n");
		return 0;
	}

	/*initialize network connectivity (who is adjacent to whom)*/
	edges = compute_connectivity(num_nodes);

	return 0;
}
