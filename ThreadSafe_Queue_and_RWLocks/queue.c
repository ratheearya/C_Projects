#include "queue.h"
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <assert.h>

//wait is for down post is for up
struct queue {
    void **data; //data
    int head; //head for dequeue
    int tail; //tail for push
    int max_size; //max size for circular queue
    int size; //current size
    sem_t empty; //empty semaphore
    sem_t full; //full semaphore
    sem_t mutex; //lock semaphore
};

queue_t *queue_new(int n) {
    queue_t *q = (queue_t *) malloc(sizeof(queue_t));
    if (q == NULL)
        return q;

    q->data = (void **) malloc(n * sizeof(void *));
    if (q->data == NULL) {
        free(q);
        return NULL;
    }

    q->head = 0;
    q->tail = 0;
    q->max_size = n;
    q->size = 0;

    int rc;
    rc = sem_init(&(q->empty), 0, 0);
    if (rc != 0)
        free(q);
    assert(!rc);
    rc = sem_init(&(q->full), 0, q->max_size);
    if (rc != 0)
        free(q);
    assert(!rc);
    rc = sem_init(&(q->mutex), 0, 1);
    if (rc != 0)
        free(q);
    assert(!rc);

    return q;
}

void queue_delete(queue_t **q) {
    if (q == NULL || *q == NULL)
        return;

    int rc;
    rc = sem_destroy(&(*q)->full);
    assert(!rc);
    rc = sem_destroy(&(*q)->empty);
    assert(!rc);
    rc = sem_destroy(&(*q)->mutex);
    assert(!rc);

    free((*q)->data);
    q = NULL;
    free(q);
}

bool queue_push(queue_t *q, void *elem) {
    if (q == NULL)
        return false;

    sem_wait(&(q->full));
    sem_wait(&(q->mutex));

    q->data[q->tail] = elem;
    q->tail = (q->tail + 1) % (q->max_size);
    q->size++;

    sem_post(&(q->empty));
    sem_post(&(q->mutex));

    return true;
}

bool queue_pop(queue_t *q, void **elem) {
    if (q == NULL)
        return false;

    sem_wait(&(q->empty));
    sem_wait(&(q->mutex));

    *elem = q->data[q->head];
    q->head = (q->head + 1) % q->max_size;
    q->size--;

    sem_post(&(q->full));
    sem_post(&(q->mutex));

    return true;
}
