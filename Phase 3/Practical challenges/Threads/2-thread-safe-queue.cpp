/*
Exercise 2: Thread-Safe Queue
Implement a thread-safe queue with:

enqueue() - add item
dequeue() - remove item (blocks if empty)
size() - get current size
Use mutex and condition variables
*/

#include <iostream>
#include <pthread.h>
#include <queue>

class ThreadSafeQueue {
private:
    std::queue<int> q;
    pthread_mutex_t mtx;
    pthread_cond_t cond;
public:
    ThreadSafeQueue() {
        pthread_mutex_init(&mtx, nullptr);
        pthread_cond_init(&cond, nullptr);
    }

    ~ThreadSafeQueue() {
        pthread_mutex_destroy(&mtx);
        pthread_cond_destroy(&cond);
    }

    void enqueue(int item) {
        pthread_mutex_lock(&mtx);
        q.push(item);
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mtx);
    }

    int dequeue() {
        pthread_mutex_lock(&mtx);
        while (q.empty()) {
            pthread_cond_wait(&cond, &mtx);
        }

        int res = q.front();
        q.pop();

        pthread_mutex_unlock(&mtx);

        return res;
    }

    int size() {

        pthread_mutex_lock(&mtx);
        int size = q.size();
        pthread_mutex_unlock(&mtx);

        return size;
    }
};

void* producer_func(void* arg) {
    ThreadSafeQueue* tsq = (ThreadSafeQueue*) arg;

    for (int i = 0; i < 10; ++i) {
        tsq->enqueue(i);
        std::cout << "Produced: " << i << '\n';
    }
    return nullptr;
}

void* consumer_func(void* arg) {
    ThreadSafeQueue* tsq = (ThreadSafeQueue*) arg;

    for (int i = 0; i < 10; ++i) {
        int item = tsq->dequeue();
        std::cout << "Consumed: " << item << '\n'; 
    }
    return nullptr;
}

int main() {
    ThreadSafeQueue tsq;
    pthread_t producer, consumer;

    pthread_create(&producer, nullptr, producer_func, &tsq);
    pthread_create(&consumer, nullptr, consumer_func, &tsq);

    pthread_join(producer, nullptr);
    pthread_join(consumer, nullptr);
    
    return 0;
}