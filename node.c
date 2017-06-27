#include "node.h"

struct node *init_node(int32_t id, uint16_t *edges, uint32_t *socks, uint8_t num,
              FILE *globallog) {
  struct node *newnode;
  uint8_t logmsg[50];

  /*Initialize the node's memory, ID and global log pointer*/
  newnode = (struct node*) malloc(sizeof(struct node));
  newnode->globallog = globallog;
  newnode->id = id;

  /*And initialize its message queue*/
  newnode->queue = init_queue();

  /*log beginning of execution*/
  sleep(id);
  snprintf(logmsg, 50, "Node %d has begun executing!", id);
  log_msg(logmsg, globallog);

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
  snprintf(logmsg, 50, "Node %d has finished computing edges!", id);
  log_msg(logmsg, globallog);

  /*initialize local log file, named after the node's ID*/
  char logfilename[7];
  memset(logfilename, 0, 7);
  snprintf(logfilename, 7, "%d.log", id);
  newnode->log = fopen(logfilename, "w");
  setbuf(newnode->log, NULL);

  return newnode;
}

void run_node(struct node *node, void(*algo) (struct node *node)) {
  uint32_t num_neighs = node->neighs->num;
  struct thread_data tdata[num_neighs];
  pthread_t tids[num_neighs];
  uint32_t i;
  uint8_t logmsg[60];

  /*log beginning of node execution*/
  snprintf(logmsg, 50, "Node %d is beginning algorithm execution!", node->id);
  log_msg(logmsg, node->log);

  /*start message-receiving threads for each of the node's sockets*/
  struct edge *aux = node->neighs->head;
  for (i = 0; i < num_neighs; i++) {
    tdata[i].sock = aux->sock;
    tdata[i].queue = node->queue;
    pthread_create(&tids[i],NULL,receiver_thread,(void*)&tdata[i]);
    snprintf(logmsg, 60, "Node %d now has thread %li receiving on fd %d",
                                              node->id, tids[i], tdata[i].sock);
    log_msg(logmsg, node->log);
    aux = aux->next;
  }

  /*run the node's algorithm implementation*/
  algo(node);

  /*log node's execution finish*/
  snprintf(logmsg, 50, "Node %d has finished algorithm execution!", node->id);
  log_msg(logmsg, node->log);

  /*terminate all of the node's receiving threads (we can't simply join them
  because they run forever, so we forcibly terminate them first)*/
  for (i = 0; i < num_neighs; i++) {
    pthread_cancel(tids[i]);
    pthread_join(tids[i], NULL);
  }
}

void log_msg(uint8_t *msg, FILE *logfile) {
  struct timeval tv;
  struct tm *tm_info;
  uint32_t ms;
  uint8_t timestamp[40], hms[26];

  /*get time of day in secs and ms/usecs*/
  gettimeofday(&tv, NULL);

  /*round up to the nearest second, if needed*/
  ms = lrint(tv.tv_usec/1000.0);
  if (ms >= 1000) {
    ms -= 1000;
    tv.tv_sec++;
  }

  /*convert secs to local time*/
  tm_info = localtime(&tv.tv_sec);

  /*concatenate localtime + milliseconds to final timestamp*/
  strftime(hms, 50, "%H:%M:%S", tm_info);
  snprintf(timestamp, 40, "%s.%03d", hms, ms);

  /*print given log message with the appropriate timestamp*/
  fprintf(logfile, "[%s] %s.\n", timestamp, msg);
}

void free_node(struct node *node) {
  uint8_t logmsg[50];

  /*log the node's inevitable demise*/
  snprintf(logmsg, 50, "Node %d says so long, and thanks for all the fish!",
                                                                      node->id);
  log_msg(logmsg, node->globallog);

  /*close its fds and free its memory*/
  fclose(node->log);
  free_neighs(node->neighs);
  free_queue(node->queue);
  free(node);
}

void *receiver_thread(void *thread_data) {
  /*retrieve thread data and initialize structures*/
  struct thread_data *data = (struct thread_data *) thread_data;
  struct msgqueue *queue = data->queue;
  uint32_t sock = data->sock;

  /*message buffer*/
  uint8_t msg[50];
  uint32_t len;

  /*receive messages indefinitely, and insert them in the node's queue*/
  while (1) {
    memset(msg, 0, 50);
    if ((len = recv(sock, msg, 50, 0)) > 0) {
      enqueue(queue, msg, len);
    }
  }
}
