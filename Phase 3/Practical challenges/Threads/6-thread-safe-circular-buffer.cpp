#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 10
#define PRODUCER_COUNT 5
#define CONSUMER_COUNT 5
#define ITEMS_PER_PRODUCER 20

typedef struct {
    int buffer[BUFFER_SIZE];
    int head; // Where to write next
    int tail; // Where to read next
    int count; // Current number of items in buffer

    // --- YOUR TOOLS GO HERE ---
    // Hint: You need a mutex and TWO condition variables.
    pthread_mutex_t mtx;
    pthread_cond_t cond;
} shared_buffer_t;

shared_buffer_t shared_data;

void* producer(void* arg) {
    int id = *(int*)arg;
    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        int item = (id * 1000) + i; // Unique-ish item value

        // TODO: START CRITICAL SECTION
        // 1. Wait if the buffer is full (count == BUFFER_SIZE)
        
        // 2. Add 'item' to buffer[head]

        // 3. Update 'head' using: $head = (head + 1) \pmod{BUFFER\_SIZE}$
        // 4. Update 'count'
        // 5. Signal that the buffer is no longer empty
        // TODO: END CRITICAL SECTION

        printf("Producer %d produced %d\n", id, item);
    }
    return NULL;
}

void* consumer(void* arg) {
    int id = *(int*)arg;
    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        int item;

        // TODO: START CRITICAL SECTION
        // 1. Wait if the buffer is empty (count == 0)
        // 2. Read 'item' from buffer[tail]
        // 3. Update 'tail' using: $tail = (tail + 1) \pmod{BUFFER\_SIZE}$
        // 4. Update 'count'
        // 5. Signal that the buffer is no longer full
        // TODO: END CRITICAL SECTION

        printf("Consumer %d consumed %d\n", id, item);
    }
    return NULL;
}

int main() {
    pthread_t producers[PRODUCER_COUNT], consumers[CONSUMER_COUNT];
    int p_ids[PRODUCER_COUNT], c_ids[CONSUMER_COUNT];

    // Initialize shared_data (don't forget to init your mutex/conds!)
    shared_data.head = 0;
    shared_data.tail = 0;
    shared_data.count = 0;

    // Create threads
    for (int i = 0; i < PRODUCER_COUNT; i++) {
        p_ids[i] = i;
        pthread_create(&producers[i], NULL, producer, &p_ids[i]);
    }
    for (int i = 0; i < CONSUMER_COUNT; i++) {
        c_ids[i] = i;
        pthread_create(&consumers[i], NULL, consumer, &c_ids[i]);
    }

    // Join threads
    for (int i = 0; i < PRODUCER_COUNT; i++) pthread_join(producers[i], NULL);
    for (int i = 0; i < CONSUMER_COUNT; i++) pthread_join(consumers[i], NULL);

    // Clean up
    printf("Mission complete. No deadlocks (hopefully)!\n");
    return 0;
}