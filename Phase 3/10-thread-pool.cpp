// thread_pool.cpp
#include <iostream>
#include <pthread.h>
#include <queue>
#include <unistd.h>

struct Task {
    void (*function)(int);
    int argument;
};

struct ThreadPool {
    pthread_t* threads;
    int num_threads;
    std::queue<Task> tasks;
    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_cond;
    bool shutdown;
};

void example_task(int n) {
    std::cout << "Task executing with argument " << n 
              << " on thread " << pthread_self() % 1000 << std::endl;
    sleep(1);  // Simulate work
}

void* worker_thread(void* arg) {
    ThreadPool* pool = (ThreadPool*)arg;

    while (true) {
        pthread_mutex_lock(&pool->queue_mutex);

        while (pool->tasks.empty() && !pool->shutdown) {
            pthread_cond_wait(&pool->queue_cond, &pool->queue_mutex);
        }

        if (pool->shutdown && pool->tasks.empty()) {
            pthread_mutex_unlock(&pool->queue_mutex);
            break;
        }

        Task task = pool->tasks.front();
        pool->tasks.pop();

        pthread_mutex_unlock(&pool->queue_mutex);

        task.function(task.argument);
    }

    return nullptr;
}

void threadpool_init(ThreadPool* pool, int num_threads) {
    pool->num_threads = num_threads;
    pool->threads = new pthread_t[num_threads];
    pool->shutdown = false;

    pthread_mutex_init(&pool->queue_mutex, nullptr);
    pthread_cond_init(&pool->queue_cond, nullptr);

    for (int i = 0; i < num_threads; ++i) {
        pthread_create(&pool->threads[i], nullptr, worker_thread, pool);
    }
}

void threadpool_add_task(ThreadPool* pool, void (*function)(int), int arg) {
    pthread_mutex_lock(&pool->queue_mutex);

    Task task {function, arg};
    pool->tasks.push(task);

    pthread_cond_signal(&pool->queue_cond);

    pthread_mutex_unlock(&pool->queue_mutex);
}

void threadpool_destroy(ThreadPool* pool) {
    pthread_mutex_lock(&pool->queue_mutex);
    pool->shutdown = true;

    pthread_cond_broadcast(&pool->queue_cond);
    pthread_mutex_unlock(&pool->queue_mutex);

    // Wait for all threads
    for (int i = 0; i < pool->num_threads; ++i) {
        pthread_join(pool->threads[i], nullptr);
    }

    delete[] pool->threads;
    pthread_mutex_destroy(&pool->queue_mutex);
    pthread_cond_destroy(&pool->queue_cond);
}

int main() {
    std::cout << "=== Thread Pool Demo ===" << std::endl;
    
    ThreadPool pool;
    threadpool_init(&pool, 4);  // 4 worker threads
    
    std::cout << "Thread pool created with 4 workers\n" << std::endl;
    
    // Add 10 tasks
    for (int i = 1; i <= 10; i++) {
        threadpool_add_task(&pool, example_task, i);
        std::cout << "Added task " << i << std::endl;
    }
    
    std::cout << "\nWaiting for tasks to complete..." << std::endl;
    sleep(15);  // Give tasks time to finish
    
    threadpool_destroy(&pool);
    std::cout << "\nThread pool destroyed" << std::endl;
    
    return 0;
}