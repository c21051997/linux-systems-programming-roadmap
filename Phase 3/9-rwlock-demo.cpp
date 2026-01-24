// rwlock_demo.cpp
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <vector>

struct Database {
    std::vector<int> data;
    pthread_rwlock_t rwlock;
};

void init_database(Database* db) {
    db->data = {1, 2, 3, 4, 5};
    pthread_rwlock_init(&db->rwlock, nullptr);
}

void* reader_thread(void* arg) {
    Database* db = (Database*)arg;
    int id = pthread_self() % 1000;
    
    for (int i = 0; i < 5; i++) {
        pthread_rwlock_rdlock(&db->rwlock);  // Read lock (shared)
        
        std::cout << "Reader " << id << " reading: ";
        for (int val : db->data) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
        
        usleep(100000);  // Simulate read
        
        pthread_rwlock_unlock(&db->rwlock);
        
        usleep(500000);
    }
    
    return nullptr;
}

void* writer_thread(void* arg) {
    Database* db = (Database*)arg;
    int id = pthread_self() % 1000;
    
    for (int i = 0; i < 3; i++) {
        sleep(1);
        
        pthread_rwlock_wrlock(&db->rwlock);  // Write lock (exclusive)
        
        std::cout << "Writer " << id << " writing..." << std::endl;
        for (int& val : db->data) {
            val++;
        }
        
        usleep(200000);  // Simulate write
        
        pthread_rwlock_unlock(&db->rwlock);
    }
    
    return nullptr;
}

int main() {
    std::cout << "=== Read-Write Lock Demo ===" << std::endl;
    
    Database db;
    init_database(&db);
    
    pthread_t readers[5];
    pthread_t writers[2];
    
    // Create readers
    for (int i = 0; i < 5; i++) {
        pthread_create(&readers[i], nullptr, reader_thread, &db);
    }
    
    // Create writers
    for (int i = 0; i < 2; i++) {
        pthread_create(&writers[i], nullptr, writer_thread, &db);
    }
    
    // Wait for all
    for (int i = 0; i < 5; i++) {
        pthread_join(readers[i], nullptr);
    }
    for (int i = 0; i < 2; i++) {
        pthread_join(writers[i], nullptr);
    }
    
    pthread_rwlock_destroy(&db.rwlock);
    
    std::cout << "\nFinal data: ";
    for (int val : db.data) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
    
    return 0;
}

// **Read-Write Lock States:**
// 
// State 1: Multiple readers (OK!)
// ┌────────┐  ┌────────┐  ┌────────┐
// │Reader 1│  │Reader 2│  │Reader 3│  All reading simultaneously
// └────────┘  └────────┘  └────────┘

// State 2: One writer (others wait)
// ┌────────┐
// │Writer  │  Exclusive access
// └────────┘
//     ↓
// ┌────────┐  ┌────────┐
// │Reader  │  │Reader  │  Waiting...
// │(wait)  │  │(wait)  │
// └────────┘  └────────┘