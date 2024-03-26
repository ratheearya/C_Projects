Thread-Safe Queue and Read/Write Lock Implimentation

Multi-Threaded Queue
 The queue.c file contains a Thread-Safe circular queue that supports the operations push and pop. A queue struct will contain the following attributes:
     
    void **data; //data containing the queue elements
    int head; //head var for dequeue
    int tail; //tail var for push
    int max_size; //max size for circular queue
    int size; //current size for knowing when full or empty
    sem_t empty; //empty semaphore
    sem_t full; //full semaphore
    sem_t mutex; //lock semaphore
 
 
 The functionality is as follows:
 - queue_t *queue_new(int n): creates a queue of max size n
 - void queue_delete(queue_t **q): Deletes the queue without leaking memory, and sets the queue_t *q to null
 - bool queue_push(queue_t *q, void *elem): Waits if the queue is full, then when it is no longer full it pushes and element to the queue (FIFO format still)
 - bool queue_pop(queue_t *q, void **elem): Waits if the queue is empty, then once it is no longer empty is pops an element from the queue (FIFO)

 Semaphores are used to lock the push and pop functions if the queue is full or empty, respectively.

 Read/Write Lock
  The rwlock_test.c file contains a lock system used for multithreaded read/write commands. It is used to avoid data race scenarios such as multiple writes occuring or a read and write occuring, for example. Depending on the priority, the locks will behave differently.

  The rwlock struct will contain the following attributes:

    pthread_mutex_t mutex; //mutex lock
    pthread_cond_t read_cond; //conditional variable for the read functions
    pthread_cond_t write_cond;//conditional variable for the write functions
    int readers; //the number of readers waiting to execute
    int writers; //the number of writers waiting to execute
    int active_readers; //the number of readers currently executed
    int active_writers; //the number of writers currently executed
    int n_readers; //the number of readers since the last Write call (for N_WAY prio)
    int n; //the number of readers allowed before a write must be performed(if there is one)
    PRIORITY priority; //the priority variable to state which reader has priority

    As shown the lock functions will act differently depending on the priority:
     - If READERS have priority, readers will be allowed to execute simultaniously while there are any waiting. They may execute simultaniously. Only once all te readers are finished, then the writers may execute, one by one.
     - If WRITERS have priority, writers will be allowed to execute one by one until there are non waiting. Only then will readers will be allowed to execute simultaniously
     - IF N_WAY priority is active. n readers (specified by the variable n) will have be allowed to read simultaniously. Once n readers are finished reading, only then will one writer be allowed to read. Then n readers will be allowed, then a writer and so forth. If there are no readers, the rest of the writers may write one by one. If there are no writers waiting, all of the readers may read simultaniously.

     It is important to keep track of te n_readers variable, and make sure you aren't prematurely increasing it, and you also want to make sure that you are only setting it to 0 again after a writer has acquired a lock, and nowhere else, to avoid confusion and deadlocking.


