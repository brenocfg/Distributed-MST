#include "algorithm.h"

enum EDGE_STATUS {
  EDGE_REJECT = -1,
  EDGE_UNKNOWN,
  EDGE_BRANCH
};

enum MSG_TYPES {
  MSG_CONNECT = 0,
  MSG_INITIATE,
  MSG_TEST,
  MSG_ACCEPT,
  MSG_REJECT,
  MSG_CHGROOT,
  MSG_REPORT,
};

void ghs (struct node *node) {
  uint16_t num_neighs = node->neighs->num;
  uint8_t logmsg[50], outmsg[50], inmsg[50], edge_statuses[num_neighs];

  /*initialize all edges as UNKNOWN*/
  memset(edge_statuses, EDGE_UNKNOWN, sizeof(edge_statuses));

  /*we start at level 0*/
  uint8_t level = 0;

  /*at level 0, we get min weight edge, its socket and index in the edge order*/
  int16_t min_index = -1, min_weight = 5000, i;
  uint32_t min_sock;
  struct edge *link = node->neighs->head;
  for (i = 0; i < num_neighs; i++) {
    if (link->weight < min_weight) {
      min_weight = link->weight;
      min_sock = link->sock;
      min_index = i;
    }
    link = link->next;
  }

  /*log lowest cost edge, and update its status*/
  edge_statuses[min_index] = EDGE_BRANCH;
  snprintf(logmsg, 50, "My lowest edge has weight %d", min_weight);
  log_msg(logmsg, node->log);

  /*send lowest edge neighbour a CONNECT message*/
  memset(outmsg, 0, 50);
  outmsg[0] = MSG_CONNECT;
  outmsg[1] = node->id;
  outmsg[2] = level;
  send(min_sock, outmsg, 3, 0);

  while(1) {
    /*nothing to do when there are no messages to process*/
    if (is_empty(node->queue)) {
      continue;
    }

    memset(inmsg, 0, 50);
    int len = dequeue(node->queue, inmsg);
    snprintf(logmsg, 50, "Processing msg[%d]! Type: %d, Node ID: %d",
                                                      len, inmsg[0], inmsg[1]);
    log_msg(logmsg, node->log);

    uint8_t msg_type = inmsg[0];
    switch(msg_type) {
      case MSG_CONNECT: {
        process_connect(node, edge_statuses, &level, inmsg);
        break;
      }
      default: {
        fprintf(stderr, "Not implemented yet c:\n");
        break;
      }
    }
  }
}

void process_connect(struct node *node, uint8_t *edge_status, uint8_t *level,
                                                                uint8_t *msg) {
  uint8_t inid, inlevel;

  /*retrieve message data*/
  inid = msg[1];
  inlevel = msg[2];

}
