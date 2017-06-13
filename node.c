#include "node.h"

struct node *init_node(int32_t id, uint16_t *edges, uint16_t *socks, uint8_t num,
              FILE *globallog) {
  struct node *newnode;

  /*Initialize the node's reference to the global log*/
  newnode->globallog = globallog;

  /*log beginning of execution*/
  sleep(id);
  uint64_t stamp = (uint64_t) time(NULL);
  fprintf(newnode->globallog, "[%lu] Node %d has begun executing!\n",stamp,id);

  /*allocate memory for the node, and initialize its ID*/
  newnode = (struct node*) malloc(sizeof(struct node));
  newnode->id = id;

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

  return newnode;
}

void test_node(struct node *node) {
  uint8_t recvmsg[50];
  uint8_t sendmsg[50];
  memset(sendmsg, 0, 50);
  snprintf(sendmsg, 50, "Hi, I'm node %d!\n", node->id);

  fprintf(node->log, "Beginning test for node %d!\n", node->id);

  print_edges(node->neighs, node->log);

  uint8_t i;
  struct edge *link = node->neighs->head;
  for (i = 0; i < node->neighs->num; i++) {
    send(link->sock, sendmsg, strlen(sendmsg), 0);
    link = link->next;
  }

  link = node->neighs->head;
  for (i = 0; i < node->neighs->num; i++) {
    memset(recvmsg, 0, 50);
    recv(link->sock, recvmsg, 50, 0);
    fprintf(node->log, "%s", recvmsg);
    link = link->next;
  }

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
