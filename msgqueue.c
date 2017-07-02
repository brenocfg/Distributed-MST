#include "msgqueue.h"

uint32_t dequeue(struct msgqueue *queue, uint8_t *buffer) {
  struct msg *aux;

  pthread_mutex_lock(&queue->mutex);

  /*let's not segfault shall we? do nothing for empty queues or NULL buffers*/
  if (is_empty(queue) || buffer == NULL) {
    return 0;
  }

  /*update front of the queue*/
  aux = queue->front;
  queue->front = aux->next;

  /*copy message content to buffer*/
  uint32_t len = aux->len;
  memcpy(buffer, aux->str, len);

  /*free the message's string pointer then the struct pointer itself*/
  free(aux->str);
  free(aux);

  pthread_mutex_unlock(&queue->mutex);

  /*return length of message copied*/
  return len;
}

void enqueue(struct msgqueue *queue, uint8_t *str, uint32_t len) {
  struct msg *newmsg;
  uint8_t *newstr;

  /*allocate some memory baby*/
  newmsg = (struct msg *) malloc(sizeof(struct msg));
  newstr = (uint8_t *) malloc(len*sizeof(uint8_t));

  /*copy over string content to the newly allocated msg*/
  memcpy(newstr, str, len);
  newmsg->str = newstr;
  newmsg->len = len;
  newmsg->next = NULL;

  pthread_mutex_lock(&queue->mutex);

  /*for empty queues we insert in the front*/
  if (queue->front == NULL) {
    queue->front = newmsg;
  }

  /*for queues of size 1 we need to update the front's next pointer*/
  else if (queue->back == NULL){
    queue->front->next = newmsg;
    queue->back = newmsg;
  }

  /*for the general case we only care about the back*/
  else {
    queue->back->next = newmsg;
    queue->back = newmsg;
  }

  pthread_mutex_unlock(&queue->mutex);
}

uint8_t is_empty(struct msgqueue *queue) {
  return (queue->front == NULL);
}

struct msgqueue *init_queue() {
  struct msgqueue *newqueue;

  /*init memory and initialize pointers as NULL*/
  newqueue = (struct msgqueue*) malloc(sizeof(struct msgqueue));
  newqueue->front = newqueue->back = NULL;

  /*and initialize its mutex variable*/
  pthread_mutex_init(&newqueue->mutex, NULL);

  return newqueue;
}

void free_queue(struct msgqueue *queue) {
  /*empty queue*/
  while (!is_empty(queue)) {
    dequeue(queue, NULL);
  }

  /*free its memory and destroy mutual exclusion variable*/
  pthread_mutex_destroy(&queue->mutex);
  free(queue);
}
