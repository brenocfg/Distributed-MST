#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*computes a connectivity matrix, where edges[i][j] being positive will
correspond to nodes i and j being neighbours, and the value of the cell itself
will be the weight of the edge, as well as the id for the file descriptor of
the socket used to implement the edge

this is safe because to simplify this implementation of GHS, we can assume all
edges have distinct weights*/
uint16_t *compute_connectivity(uint8_t num_nodes) {
	uint16_t *edges;
	uint16_t num_edges;
	uint16_t goal;

	goal = (((num_nodes-1) * (num_nodes-2)) / 2) + 1;
	num_edges = 0;

	edges = calloc(num_nodes*num_nodes, sizeof(uint16_t));

	srand(time(NULL));
	while(num_edges < goal) {
		uint16_t v1 = rand()
		
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
