#ifndef MSGQUEUE_H
#define MSGQUEUE_H

#include <stdio.h>      /*because we're weak and need to debug*/
#include <stdint.h>     /*because some datatypes are better than others*/
#include <stdlib.h>     /*because dynamic allocation is all the rage*/
#include <string.h>     /*because 'string' sounds better than 'char pointer'*/
#include <pthread.h>    /*because msgqueues are promiscuous sluts*/
#include <sys/socket.h> /*because messages need roads to travel through*/

/*Struct that represents a queue of messages. The queue has pointers to its
first and last members, as well as a mutex variable, to guarantee mutual
exclusion to all accesses to it, since it will be manipulated by multiple
threads (one per socket).*/
struct msgqueue {
  struct msg *front, *back;
  pthread_mutex_t mutex;
};

/*Struct that represents a single message in the queue. The messages are not
null-terminated. They have a pointer to their content, as well as their length
in bytes and a pointer to the next message in the queue.*/
struct msg {
  uint8_t *str;
  uint32_t len;
  struct msg *next;
};

/*Removes the first message from the queue and copies the content of the message
to the given buffer. Returns the length of the message copied (0 for failure).
This is all done as an atomic operation, to avoid corrupting the queue.*/
uint32_t dequeue(struct msgqueue *queue, uint8_t *buffer);

/*Inserts a message in the back of the queue. We need to know the message's
length when inserting, since messages are not null-terminated. Therefore, it's
up to whoever creates the message (or receives it) to compute its length
properly before inserting in the queue.*/
void enqueue(struct msgqueue *queue, uint8_t *str, uint32_t len);

/*Returns 1 if the given queue is empty. 0 otherwise.*/
uint8_t is_empty(struct msgqueue *queue);

/*Initializes an initially empty queue structure, allocating memory and
initializing its mutual exclusion lock*/
struct msgqueue *init_queue();

/*Destroys a given message queue, emptying it (if necessary) then freeing all
its memory. We also destroy its mutex variable, and we don't bother locking it,
since in theory this is only called when the queue is no longer in use.*/
void free_queue(struct msgqueue *queue);

#endif /* MSGQUEUE_H */
