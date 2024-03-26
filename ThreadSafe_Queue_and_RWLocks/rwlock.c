#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <semaphore.h>
#include <assert.h>
#include "rwlock.h"

typedef struct rwlock {
    pthread_mutex_t mutex;
    pthread_cond_t read_cond;
    pthread_cond_t write_cond;

    //waiting
    int readers;
    int writers;
    //active
    int active_readers;
    int active_writers;
    //total
    int n_readers;

    int n;
    PRIORITY priority;
} rw_lock_t;

rwlock_t *rwlock_new(PRIORITY p, uint32_t n) {
    rwlock_t *rw = (rwlock_t *) malloc(sizeof(rwlock_t));
    if (rw == NULL)
        return rw;

    rw->priority = p;
    rw->n = n;
    rw->readers = 0;
    rw->writers = 0;
    rw->active_readers = 0;
    rw->active_writers = 0;
    rw->n_readers = 0;

    int rc = 0;

    rc = pthread_mutex_init(&rw->mutex, NULL);

    if (rc != 0)
        free(rw);
    assert(!rc);

    pthread_cond_init(&rw->read_cond, NULL);

    if (rc != 0)
        free(rw);
    assert(!rc);

    pthread_cond_init(&rw->write_cond, NULL);
    if (rc != 0)
        free(rw);
    assert(!rc);

    return rw;
}

void rwlock_delete(rwlock_t **rw) {
    if (rw == NULL || *rw == NULL) {
        return;
    }

    int rc = 0;
    rc = pthread_mutex_destroy(&(*rw)->mutex);
    assert(!rc);
    rc = pthread_cond_destroy(&(*rw)->read_cond);
    assert(!rc);
    rc = pthread_cond_destroy(&(*rw)->write_cond);
    assert(!rc);

    free(*rw);
    *rw = NULL;
    rw = NULL;
}

//when reader tries to get lock, if writer has lock its denied, but if a readers has lock it is "ok"
//when writer tries to get lock, if reader or writer has lock it is denied.
void reader_lock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->mutex);

    if (rw->priority == READERS) {
        rw->readers++;

        while (rw->active_writers > 0)
            pthread_cond_wait(&rw->read_cond, &rw->mutex);

        rw->readers--;
        rw->active_readers++;

    } else if (rw->priority == WRITERS) {

        rw->readers++;

        while (rw->active_writers > 0 || rw->writers > 0)
            pthread_cond_wait(&rw->read_cond, &rw->mutex);

        rw->readers--;
        rw->active_readers++;

    } else if (rw->priority == N_WAY) {

        fprintf(stderr, "%d \n", rw->n_readers);
        rw->readers++;
        while (rw->active_writers > 0 || (rw->writers > 0 && rw->n_readers >= rw->n)) {
            pthread_cond_wait(&rw->read_cond, &rw->mutex); //must be unlocked from writer unlock
        }
        rw->readers--;
        rw->active_readers++;
        rw->n_readers++;
    }
    pthread_mutex_unlock(&rw->mutex);
}

void reader_unlock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->mutex);

    if (rw->priority == READERS) {

        if (rw->active_readers > 0)
            rw->active_readers--;

        if (rw->active_readers == 0 && rw->readers == 0) {
            pthread_cond_signal(&rw->write_cond);
        } else {
            pthread_cond_broadcast(&rw->read_cond);
        }

    } else if (rw->priority == WRITERS) {

        if (rw->active_readers > 0)
            rw->active_readers--;

        if (rw->active_readers == 0 && rw->writers > 0 && rw->active_writers == 0) {
            pthread_cond_signal(&rw->write_cond);
        } else if (rw->readers > 0) {
            pthread_cond_signal(&rw->read_cond);
        }

    } else if (rw->priority == N_WAY) {
        if (rw->active_readers > 0)
            rw->active_readers--;

        if (rw->active_writers == 0 && rw->active_readers == 0
            && (rw->readers == 0 || rw->n_readers >= rw->n)) {
            pthread_cond_signal(&rw->write_cond);
        }
    }

    pthread_mutex_unlock(&rw->mutex);
}

void writer_lock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->mutex);

    if (rw->priority == READERS) {

        rw->writers++;

        while (rw->active_readers > 0 || rw->readers > 0 || rw->active_writers > 0)
            pthread_cond_wait(&rw->write_cond, &rw->mutex);

        rw->writers--;
        rw->active_writers++;

    } else if (rw->priority == WRITERS) {

        rw->writers++;
        while (rw->active_writers > 0 || rw->active_readers > 0) {
            pthread_cond_wait(&rw->write_cond, &rw->mutex);
        }
        rw->writers--;
        rw->active_writers++;

    } else if (rw->priority == N_WAY) {

        rw->writers++;
        //wait while active writers or active readers or (n_readers < n and there are readers)

        while ((rw->active_writers > 0 || rw->active_readers > 0
                || (rw->readers > 0 && rw->n_readers < rw->n))) {
            pthread_cond_wait(&rw->write_cond, &rw->mutex);
        }

        rw->writers--;
        rw->active_writers++;
        rw->n_readers = 0;
    }

    pthread_mutex_unlock(&rw->mutex);
}

void writer_unlock(rwlock_t *rw) {

    pthread_mutex_lock(&rw->mutex);

    if (rw->priority == READERS) {

        if (rw->active_writers > 0)
            rw->active_writers--;

        if (rw->active_writers == 0 && rw->readers > 0) {
            pthread_cond_broadcast(&rw->read_cond);
        } else {
            pthread_cond_signal(&rw->write_cond);
        }
    } else if (rw->priority == WRITERS) {

        if (rw->active_writers > 0)
            rw->active_writers--;

        if (rw->writers > 0) {
            pthread_cond_signal(&rw->write_cond);
        } else {
            pthread_cond_broadcast(&rw->read_cond);
        }

    } else if (rw->priority == N_WAY) {

        if (rw->active_writers > 0)
            rw->active_writers--;

        if (rw->writers == 0) {
            pthread_cond_broadcast(&rw->read_cond);
        } else if ((rw->readers > 0 && rw->n_readers < rw->n)) {
            for (int i = rw->n_readers; i <= rw->n; i++)
                pthread_cond_signal(&rw->read_cond);
        } else if (rw->writers > 0) {
            pthread_cond_signal(&rw->write_cond);
        }
    }

    pthread_mutex_unlock(&rw->mutex);
}
