#include "main.h"

/*entry point*/
uint8_t main (int argc, char *argv[]) {
	/*check for number of input arguments*/
	if (argc < 2) {
		fprintf(stderr, "Not enough arguments!\n");
		fprintf(stderr, "Usage: ./ghs <number nodes> <connectivity flag>\n");
		fprintf(stderr, "Use flag as anything but 0 for dense network.\n");
		return 0;
	}

	/*type of connectivity*/
	uint8_t con_flag = 0;
	if (argc == 3) {
		con_flag = atoi(argv[2]);
	}


	/*compute number of nodes and check for validity*/
	uint8_t num_nodes;
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
	uint16_t *edges;
	if (con_flag) {
		edges = compute_dense_connectivity(num_nodes);
	}
	else {
		edges = compute_sparse_connectivity(num_nodes);
	}

	/*initialize socket pairs for each edge*/
	uint16_t *sockets;
	sockets = init_sockets(edges, num_nodes);
	if (sockets == NULL) {
		return 0;
	}

	print_edges(sockets, num_nodes);
	
	return 1;
}


uint8_t init_sockets(uint16_t *edges, uint8_t num_nodes) {
	uint16_t *weights, *sockets;

	weights = calloc(num_nodes*num_nodes, sizeof(uint16_t));
	sockets = calloc(num_nodes*num_nodes, sizeof(uint16_t));

	int16_t i, j;
	for (i = 0; i < num_nodes; i++) {
		for (j = 0; j< num_nodes; j++) {
			int16_t weight = edges[i*num_nodes + j];

			if (weight && !weights[weight]) {
				int16_t fd_pair[2];

				if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd_pair) == -1) {
					fprintf(stderr, "Error when creating socket pair!\n");
					return NULL;
				}

				sockets[i*num_nodes + j] = fd_pair[0];
				sockets[j*num_nodes + i] = fd_pair[1];
				weights[weight] = 1;
			}
		}
	}

	return sockets;
}

uint16_t *compute_dense_connectivity(uint8_t num_nodes) {
	uint16_t *edges, *weights;
	uint16_t num_edges, goal;

	/*we want (n-1)*(n-2)/2 + 1 edges total*/
	goal = (((num_nodes-1) * (num_nodes-2)) / 2) + 1;
	num_edges = 0;

	/*edges will be our connectivity matrix, weights will guarantee uniqueness*/
	edges = calloc(num_nodes*num_nodes, sizeof(uint16_t));
	weights = calloc(num_nodes*(num_nodes-1), sizeof(uint8_t));

	/*generate random edges until we reach goal*/
	srand(time(NULL));
	while(num_edges < goal) {
		/*randomize nodes*/
		uint16_t v1 = rand()%num_nodes;
		uint16_t v2 = rand()%num_nodes;

		/*can't have self edges or repeat ones*/
		if (v1 == v2 || edges[v1*num_nodes + v2]) {
			continue;
		}

		/*get a valid, unique weight*/
		int16_t weight = -1;
		while (weight < 1) {
			weight = (rand()%((num_nodes)*(num_nodes-1)))+1;
			weight = (weights[weight]) ? -1 : weight;
		}

		/*update graph and unique weight flag map*/
		weights[weight] = 1;
		edges[v1*num_nodes + v2] = weight;
		edges[v2*num_nodes + v1] = weight;
		num_edges++;
	}

	return edges;
}

uint16_t *compute_sparse_connectivity(uint8_t num_nodes) {
	uint16_t *edges, *weights;
	uint16_t num_edges;

	num_edges = 0;

	/*edges will be our connectivity matrix, weights will guarantee uniqueness*/
	edges = calloc(num_nodes*num_nodes, sizeof(uint16_t));
	weights = calloc(num_nodes*(num_nodes-1), sizeof(uint8_t));

	/*generate random n-1 random weight edges (which connects the graph)*/
	srand(time(NULL));
	int16_t i;
	for (i = 0; i < num_nodes-1; i++) {
		int16_t weight = -1;
		while (weight < 1) {
			weight = (rand()%((num_nodes)*(num_nodes-1)))+1;
			weight = (weights[weight]) ? -1 : weight;
		}

		weights[weight] = 1;
		edges[i*num_nodes + (i+1)] = weight;
		edges[(i+1)*num_nodes + i] = weight;
		num_edges++;
	}

	/*now generate an additional [(n-5) : (n+5)] additional edges*/
	uint16_t goal = num_edges + (rand()%(num_nodes)+5);
	while(num_edges < goal) {
		/*randomize nodes*/
		uint16_t v1 = rand()%num_nodes;
		uint16_t v2 = rand()%num_nodes;

		/*can't have self edges or repeat ones*/
		if (v1 == v2 || edges[v1*num_nodes + v2]) {
			continue;
		}

		/*get a valid, unique weight*/
		int16_t weight = -1;
		while (weight < 1) {
			weight = (rand()%((num_nodes)*(num_nodes-1)))+1;
			weight = (weights[weight]) ? -1 : weight;
		}

		/*update graph and unique weight flag map*/
		weights[weight] = 1;
		edges[v1*num_nodes + v2] = weight;
		edges[v2*num_nodes + v1] = weight;
		num_edges++;
	}

	return edges;
}

void print_edges(uint16_t *edges, uint8_t num_nodes) {
	int16_t i, j;
	for (i = 0; i < num_nodes; i++) {
		for (j = 0; j < num_nodes; j++) {
			fprintf(stdout, "%d\t", edges[i*num_nodes + j]);
		}
		fprintf(stdout, "\n");
	}
}
