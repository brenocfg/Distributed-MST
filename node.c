#include "node.h"

struct node *init_node(int32_t id, uint16_t *edges, uint16_t *socks, uint8_t num,
              FILE *globallog) {
  struct node *newnode;

  /*Initialize the node's memory, ID and global log pointer*/
  newnode = (struct node*) malloc(sizeof(struct node));
  newnode->globallog = globallog;
  newnode->id = id;

  /*log beginning of execution*/
  sleep(id);
  uint64_t stamp = (uint64_t) time(NULL);
  fprintf(newnode->globallog, "[%lu] Node %d has begun executing!\n",stamp,id);

  /*allocate and initialize edge list, with proper weight/socket pairs*/
  uint8_t i;
  newnode->neighs = init_neighs();
  for (i = 0; i < num; i++) {
    if(edges[id*num + i]) {
      uint16_t weight, socket;
      weight = edges[id*num + i];
      socket = socks[id*num + i];
      add_edge(newnode->neighs, weight, socket);
    }
  }

  /*log edge initialization*/
  sleep(id);
  stamp = (uint64_t) time(NULL);
  fprintf(newnode->globallog,"[%lu] Node %d finished computing edges!\n",stamp,id);

  /*initialize local log file, named after the node's ID*/
  char logfilename[7];
  memset(logfilename, 0, 7);
  snprintf(logfilename, 7, "%d.log", id);
  newnode->log = fopen(logfilename, "w");
  setbuf(newnode->log, NULL);

  return newnode;
}

void run_node(struct node *node, void(*algo) (struct node *node)) {
  fprintf(node->log, "Beginning test for node %d!\n", node->id);

  algo(node);

  fprintf(node->log, "Ended test for node %d!\n", node->id);
}

void free_node(struct node *node) {
  uint64_t stamp;

  stamp = (uint64_t) time(NULL);
  fprintf(node->log, "[%lu] Node %d is saying bye bye!\n", stamp, node->id);
  fclose(node->log);

  free_neighs(node->neighs);

  free(node);
}
