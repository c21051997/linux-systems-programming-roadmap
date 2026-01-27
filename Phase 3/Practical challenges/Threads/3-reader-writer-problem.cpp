/*
Exercise 3: Reader-Writer Problem
Implement a database simulation:

Multiple reader threads
Multiple writer threads
Use read-write locks
Track statistics (reads/writes per second)
*/
#include <iostream>
#include <pthread.h>
#include <vector>
#include <unistd.h>
#include <chrono>
#include <atomic>

// Database struct
struct Database {
    std::vector<int> data;      // Shared data
    pthread_rwlock_t rwlock;    // Read-write lock
};

// Statistics
std::atomic<int> total_reads(0);
std::atomic<int> total_writes(0);

// Reader function
void* reader_func(void* arg) {
    Database* db = (Database*) arg;

    for (int i = 0; i < 10; ++i) { // Each reader reads 10 times
        pthread_rwlock_rdlock(&db->rwlock);   // Acquire read lock

        // Critical section (reading)
        int sum = 0;
        for (int val : db->data) {
            sum += val;   // Example read operation
        }

        pthread_rwlock_unlock(&db->rwlock);   // Release read lock

        total_reads++;                          // Update stats
        std::cout << "Reader " << pthread_self() % 1000 
                  << " read sum: " << sum << std::endl;

        usleep(100000); // Sleep 0.1s to simulate work
    }

    return nullptr;
}

// Writer function
void* writer_func(void* arg) {
    Database* db = (Database*) arg;

    for (int i = 0; i < 5; ++i) { // Each writer writes 5 times
        pthread_rwlock_wrlock(&db->rwlock);   // Acquire write lock

        // Critical section (writing)
        db->data.push_back(i);

        pthread_rwlock_unlock(&db->rwlock);   // Release write lock

        total_writes++;                         // Update stats
        std::cout << "Writer " << pthread_self() % 1000 
                  << " wrote: " << i << std::endl;

        usleep(200000); // Sleep 0.2s to simulate work
    }

    return nullptr;
}

int main() {
    Database db;
    db.data = {1, 2, 3};                     // Initial data
    pthread_rwlock_init(&db.rwlock, nullptr);

    const int num_readers = 3;
    const int num_writers = 2;
    pthread_t readers[num_readers];
    pthread_t writers[num_writers];

    auto start_time = std::chrono::high_resolution_clock::now();

    // Create reader threads
    for (int i = 0; i < num_readers; ++i) {
        pthread_create(&readers[i], nullptr, reader_func, &db);
    }

    // Create writer threads
    for (int i = 0; i < num_writers; ++i) {
        pthread_create(&writers[i], nullptr, writer_func, &db);
    }

    // Join reader threads
    for (int i = 0; i < num_readers; ++i) {
        pthread_join(readers[i], nullptr);
    }

    // Join writer threads
    for (int i = 0; i < num_writers; ++i) {
        pthread_join(writers[i], nullptr);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);

    pthread_rwlock_destroy(&db.rwlock);

    std::cout << "\n=== Statistics ===" << std::endl;
    std::cout << "Total reads: " << total_reads.load() 
              << ", Reads per second: " 
              << (total_reads.load() / (duration.count() + 1)) << std::endl;
    std::cout << "Total writes: " << total_writes.load() 
              << ", Writes per second: " 
              << (total_writes.load() / (duration.count() + 1)) << std::endl;

    return 0;
}
