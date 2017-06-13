#include "node.h"
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

	/*initialize global log file (shared by all nodes)*/
	FILE *globallog = fopen("global.log", "w");
	fflush(globallog);

	/*spawn child processes for each node, and let them run*/
	int32_t i, pid;
	for (i = 0; i < num_nodes; i++) {
		/*child processes will run this, and init and run their own node*/
		if ((pid = fork()) == 0) {
			/*declare and initialize the node*/
			struct node *newnode;
			newnode = init_node(i, edges, sockets, num_nodes, globallog);

			/*Run whatever algorithm here. At this point, the nodes should be agnostic
			to any global information from the parent process, such as the edge/socket
			map, and should only rely on information that is self-contained to their
			own initialized structure*/
			test_node(newnode);

			/*free the node's memory in the child process*/
			free_node(newnode);

			/*child process cannot keep iterating!*/
			break;
		}
	}

	free(edges);
	free(sockets);
	fclose(globallog);

	/*child processes are done at this point, so return them*/
	if (pid == 0) {
		return 1;
	}

	/*parent needs to wait for all child processes (nodes) to finish executing*/
	int32_t status = 0;
	while(wait(&status) > 0) {}

	return 1;
}


uint16_t *init_sockets(uint16_t *edges, uint8_t num_nodes) {
	uint16_t *weights, *sockets;

	/*weights for unique edges, sockets is our socket map*/
	weights = calloc(num_nodes*num_nodes, sizeof(uint16_t));
	sockets = calloc(num_nodes*num_nodes, sizeof(uint16_t));

	/*compute socket pairs for each edge*/
	int16_t i, j;
	for (i = 0; i < num_nodes; i++) {
		for (j = 0; j < num_nodes; j++) {
			int16_t weight = edges[i*num_nodes + j];

			if (weight && !weights[weight]) {
				int fd_pair[2];

				if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd_pair) == -1) {
					fprintf(stderr, "Error when creating socket pair!\n");
					return NULL;
				}

				/*each node gets a different socket*/
				sockets[i*num_nodes + j] = fd_pair[0];
				sockets[j*num_nodes + i] = fd_pair[1];
				weights[weight] = 1;
			}
		}
	}

	free(weights);
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
	weights = calloc((num_nodes*(num_nodes-1))+1, sizeof(uint16_t));

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
	weights = calloc((num_nodes*(num_nodes-1))+1, sizeof(uint16_t));

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

	free(weights);
	return edges;
}

void print_network(uint16_t *edges, uint16_t *socks, uint8_t num, FILE *stream){
	int16_t i, j;
	for (i = 0; i < num; i++) {
		for (j = 0; j < num; j++) {
			fprintf(stream, "%d\t", edges[i*num + j]);
		}
		fprintf(stream, "\n");
	}
	fprintf(stream, "\n\n");

	for (i = 0; i < num; i++) {
		for (j = 0; j < num; j++) {
			fprintf(stream, "%d\t", socks[i*num + j]);
		}
		fprintf(stream, "\n");
	}
	fprintf(stream, "\n\n");
}
