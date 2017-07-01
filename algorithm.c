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
  MSG_REPORT
};

enum NODE_STATES {
  NODE_FIND = 0,
  NODE_FOUND
};

void ghs (struct node *node) {
  uint8_t inmsg[50];
  struct node_data node_data;

  /*print edge information, for clarity's sake*/
  print_edges(node->neighs, node->log);

  /*we 'wake up' every node by default*/
  wakeup(node, &node_data);

  while(1) {
    /*nothing to do when there are no messages to process*/
    if (is_empty(node->queue)) {
      continue;
    }

    /*process first message in the queue*/
    memset(inmsg, 0, 50);
    dequeue(node->queue, inmsg);

    /*Retrieve information about which link the message came from beforehand.
    This is very hacky and should have been externalized to a function, but
    since we can't return both the socket and index of the edge at the same
    time, and we need to find out the edge's status using its index...*/
    struct edge *link = node->neighs->head;
    uint16_t i, inweight = (inmsg[1] << 8) | inmsg[2];
    uint8_t sock;
    for (i = 0; i < node_data.num_neighs; i++) {
      if (link->weight == inweight) {
        sock = link->sock;
        break;
      }
      link = link->next;
    }

    /*react based on incoming message type*/
    uint8_t msg_type = inmsg[0];
    switch(msg_type) {
      case MSG_CONNECT: {
        process_connect(node, &node_data, i, sock, inmsg);
        break;
      }
      case MSG_INITIATE: {
        process_initiate(node, &node_data, i, inmsg);
        break;
      }
      default: {
        fprintf(stderr, "Invalid message type!\n");
        break;
      }
    }
  }
}

void wakeup(struct node *node, struct node_data *data) {
  uint8_t outmsg[50];
  char logmsg[60];

  /*Initialize node's data. All nodes start at state FOUND, with level, fcount
  and fragment id 0. All edges begin as UNKNOWN. We also keep track of the
  node's number of neighbours, which is a bit redundant but whatever*/
  data->state = NODE_FOUND;
  data->level = 0;
  data->fcount = 0;
  data->frag_id = 0;
  data->num_neighs = node->neighs->num;
  data->edge_status = (uint8_t*) malloc(data->num_neighs*sizeof(uint8_t));
  memset(data->edge_status, EDGE_UNKNOWN, data->num_neighs);

  /*at wakeup we haven't touched any edges yet, so lowest is first in the list*/
  struct edge *lowest = node->neighs->head;

  /*log lowest cost edge, and update its status*/
  data->edge_status[0] = EDGE_BRANCH;
  snprintf(logmsg, 60, "My lowest edge has weight %d", lowest->weight);
  log_msg(logmsg, node->log);

  /*send lowest edge neighbour a CONNECT message*/
  uint8_t msg_len;
  msg_len = create_message(MSG_CONNECT, lowest->weight, data->level, 0, outmsg);
  send(lowest->sock, outmsg, msg_len, 0);

  /*and log the send event*/
  snprintf(logmsg, 60, "Sending CONNECT message with level %d to lowest edge!",
                                                                  data->level);
  log_msg(logmsg, node->log);
}

void process_connect(struct node *node, struct node_data *ndata,
                        uint8_t edge_index, uint8_t edge_sock, uint8_t *msg) {

  uint8_t inlevel, outmsg[50];
  char logmsg[60];
  uint16_t inweight;

  /*retrieve message data*/
  inlevel = msg[3];
  inweight = (msg[1] << 8) | msg[2];

  snprintf(logmsg, 60, "Received CONNECT msg, lvl: %d, weight: %d",
                                                            inlevel, inweight);
  log_msg(logmsg, node->log);

  /*received connect from lower level, sender node's fragment can be absorbed*/
  if (inlevel < ndata->level) {
    /*mark edge as part of MST*/
    ndata->edge_status[edge_index] = EDGE_BRANCH;

    /*send INITIATE message and log it*/
    uint8_t msg_len;
    msg_len = create_message(MSG_INITIATE, inweight, 0, ndata->state, outmsg);
    send(edge_sock, outmsg, msg_len, 0);
    snprintf(logmsg, 60, "Sending INITIATE message to absorbable fragment!");
    log_msg(logmsg, node->log);

    /*if we were in a discovering state count the node as reported*/
    if (ndata->state == NODE_FIND) {
      ndata->fcount += 1;
    }
  }

  /*if level is same or above and edge isn't classified, we delay the response*/
  else if (ndata->edge_status[edge_index] == EDGE_UNKNOWN) {
    /*place message at end of queue, CONNECT messages always have len 4*/
    enqueue(node->queue, msg, 4);
  }

  /*only case left is a merge, so we send the INITIATE message with next level*/
  else {
    uint8_t len;
    len=create_message(MSG_INITIATE,inweight,(ndata->level)+1,NODE_FIND,outmsg);
    send(edge_sock, outmsg, len, 0);
    snprintf(logmsg, 60, "Sending INITIATE message to ADVANCE LEVEL!");
    log_msg(logmsg, node->log);
  }
}

void process_initiate(struct node *node, struct node_data *ndata,
                                            uint8_t edge_index, uint8_t *msg) {
  uint8_t inlevel, instate, outmsg[50];
  uint16_t inweight;
  char logmsg[60];

  /*retrieve message data*/
  inlevel = msg[3];
  instate = msg[4];
  inweight = (msg[1] << 8) | msg[2];

  /*log message arrival and its parameters*/
  snprintf(logmsg, 60, "Received INITIATE message. Lvl: %d, W: %d, St: %d",
                                                    inlevel, inweight, instate);
  log_msg(logmsg, node->log);

  /*node is advancing level, update node data to match new level and fragment*/
  ndata->level = inlevel;
  ndata->frag_id = inweight;
  ndata->state = instate;
  ndata->edge_status[edge_index] = EDGE_BRANCH;
  ndata->in_branch = edge_index;
  ndata->best_edge = -1;
  ndata->best_weight = ~0;

  /*all neighbouring fragment nodes will receive the same INITIATE message, so
  we create it beforehand*/
  uint8_t len;
  len = create_message(MSG_INITIATE, inweight, inlevel, NODE_FIND, outmsg);

  /*now for each MST neighbour (edge is BRANCH), who isn't the node who just
  sent us the INITIATE message, we propagate the new fragment's INITIATE*/
  uint8_t i;
  struct edge *link = node->neighs->head;
  for (i = 0; i < ndata->num_neighs; i++, link = link->next) {
    if (i == edge_index || ndata->edge_status[i] != EDGE_BRANCH) {
      continue;
    }

    /*propagate INITIATE forward, and log*/
    send(link->sock, outmsg, len, 0);
    snprintf(logmsg, 60, "Propagating INITIATE message on edge with weight %d",
                                                                  link->weight);
    log_msg(logmsg, node->log);

    /*if we were in the discovery state, this node must report to us later*/
    if (ndata->state == NODE_FIND) {
      ndata->fcount += 1;
    }
  }

  /*if in discovery state, begin testing edges*/
  if (ndata->state == NODE_FIND) {
    //test(node, ndata);
  }
}

uint8_t create_message(uint8_t type, uint16_t weight, uint8_t level,
                        uint8_t state, uint8_t *buffer) {

  /*avoid messing with invalid pointers*/
  if (buffer == NULL) {
    return 0;
  }

  uint8_t msg_len;

  /*all messages share the first three bytes, for msg type and weight. All
  messages piggyback edge weight so the receiving node can identify the edge*/
  buffer[0] = type;
  buffer[1] = (weight >> 8) & 0xFF;
  buffer[2] = weight & 0xFF;
  msg_len = 3;

  /*fill out message content based on type*/
  switch(type) {
    /*CONNECT messages piggyback the fragment level*/
    case MSG_CONNECT: {
      buffer[3] = level;
      msg_len++;
      break;
    }
    /*INITIATE messages piggyback fragment level and state*/
    case MSG_INITIATE: {
      buffer[3] = level;
      buffer[4] = state;
      msg_len += 2;
      break;
    }
    default: {
      fprintf(stderr, "Invalid message type!\n");
      break;
    }
  }
  return msg_len;
}
