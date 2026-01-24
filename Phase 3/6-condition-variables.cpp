// condition_variable.cpp
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <queue>

std::queue<int> task_queue;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;
bool done = false;

void* producer_thread(void* arg) {
    for (int i = 1; i <= 10; i++) {
        sleep(1);  // Simulate work
        
        pthread_mutex_lock(&queue_mutex);
        
        task_queue.push(i);
        std::cout << "Producer: Added task " << i << std::endl;
        
        pthread_cond_signal(&queue_cond);  // Wake up one consumer
        
        pthread_mutex_unlock(&queue_mutex);
    }
    
    pthread_mutex_lock(&queue_mutex);
    done = true;
    pthread_cond_broadcast(&queue_cond);  // Wake up all consumers
    pthread_mutex_unlock(&queue_mutex);
    
    return nullptr;
}

void* consumer_thread(void* arg) {
    int id = *(int*)arg;
    
    while (true) {
        pthread_mutex_lock(&queue_mutex);
        
        // Wait while queue is empty and not done
        while (task_queue.empty() && !done) {
            std::cout << "Consumer " << id << ": Waiting..." << std::endl;
            pthread_cond_wait(&queue_cond, &queue_mutex);
        }
        
        if (!task_queue.empty()) {
            int task = task_queue.front();
            task_queue.pop();
            
            pthread_mutex_unlock(&queue_mutex);
            
            std::cout << "Consumer " << id << ": Processing task " 
                      << task << std::endl;
            sleep(2);  // Process task
        }
        else if (done) {
            pthread_mutex_unlock(&queue_mutex);
            break;  // Exit loop
        }
        else {
            pthread_mutex_unlock(&queue_mutex);
        }
    }
    
    std::cout << "Consumer " << id << ": Exiting" << std::endl;
    return nullptr;
}

int main() {
    std::cout << "=== Producer-Consumer with Condition Variables ===" << std::endl;
    
    pthread_t producer;
    pthread_t consumers[3];
    int ids[] = {1, 2, 3};
    
    // Create producer
    pthread_create(&producer, nullptr, producer_thread, nullptr);
    
    // Create consumers
    for (int i = 0; i < 3; i++) {
        pthread_create(&consumers[i], nullptr, consumer_thread, &ids[i]);
    }
    
    // Wait for all
    pthread_join(producer, nullptr);
    for (int i = 0; i < 3; i++) {
        pthread_join(consumers[i], nullptr);
    }
    
    std::cout << "\nAll done!" << std::endl;
    
    pthread_mutex_destroy(&queue_mutex);
    pthread_cond_destroy(&queue_cond);
    
    return 0;
}