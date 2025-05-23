#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#ifndef HEADER_FILE
#define HEADER_FILE

typedef struct {
  struct element **buffer;
  int size;
  int count;
  int in;
  int out;
  pthread_mutex_t mutex;
  pthread_cond_t not_full;
  pthread_cond_t not_empty;
  int initialized;
} Queue;
struct element {
  int num_edition;
  int id_belt;
  int last;
};
Queue queue_create(int size, int*feedback);
int queue_init (Queue *);
int queue_destroy (Queue *queue);
int queue_put (Queue *queue, struct element *elem);
struct element * queue_get(Queue *queue);
int queue_empty (Queue *queue);
int queue_full(Queue *queue);

#endif
