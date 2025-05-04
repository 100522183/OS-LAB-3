#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include "queue.h"
Queue queue_create(int size, int *feedback){
    Queue queue = { NULL, size, 0, 0, 0, PTHREAD_MUTEX_INITIALIZER, 
        PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER, 0 };
    * feedback = queue_init(&queue);
    return queue;
}

int queue_init(Queue *queue) {
    int size = queue->size;
    if (size <= 0) {
        return -1;
    }

    pthread_mutex_lock(&queue->mutex);

    if (queue->initialized) {
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }

    queue->buffer = malloc(size * sizeof(struct element *));
    if (!queue->buffer) {
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }

    queue->count = 0;
    queue->in = 0;
    queue->out = 0;

    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->not_full, NULL);
    pthread_cond_init(&queue->not_empty, NULL);
    queue->initialized = 1;

    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

int queue_destroy(Queue *queue) {
    pthread_mutex_lock(&queue->mutex);

    if (!queue->initialized) {
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }

    free(queue->buffer);
    queue->buffer = NULL;
    queue->size = 0;
    queue->count = 0;
    queue->in = 0;
    queue->out = 0;
    queue->initialized = 0;

    pthread_mutex_unlock(&queue->mutex);

    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->not_full);
    pthread_cond_destroy(&queue->not_empty);

    return 0;
}

int queue_put(Queue *queue, struct element *elem) {
    if (!queue->initialized || !elem) {
        return -1;
    }

    pthread_mutex_lock(&queue->mutex);

    while (queue->count == queue->size) {
        pthread_cond_wait(&queue->not_full, &queue->mutex);
    }

    queue->buffer[queue->in] = elem;
    queue->in = (queue->in + 1) % queue->size;
    queue->count++;

    pthread_cond_signal(&queue->not_empty);
    pthread_mutex_unlock(&queue->mutex);

    return 0;
}

struct element *queue_get(Queue *queue) {
    if (!queue->initialized) {
        return NULL;
    }

    pthread_mutex_lock(&queue->mutex);

    while (queue->count == 0) {
        pthread_cond_wait(&queue->not_empty, &queue->mutex);
    }

    struct element *elem = queue->buffer[queue->out];
    queue->out = (queue->out + 1) % queue->size;
    queue->count--;

    pthread_cond_signal(&queue->not_full);
    pthread_mutex_unlock(&queue->mutex);

    return elem;
}

int queue_empty(Queue *queue) {
    if (!queue->initialized) {
        return 1;
    }

    pthread_mutex_lock(&queue->mutex);
    int empty = (queue->count == 0);
    pthread_mutex_unlock(&queue->mutex);
    return empty;
}

int queue_full(Queue *queue) {
    if (!queue->initialized) {
        return 0;
    }

    pthread_mutex_lock(&queue->mutex);
    int full = (queue->count == queue->size);
    pthread_mutex_unlock(&queue->mutex);
    return full;
}