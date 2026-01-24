// critical_section.cpp
#include <iostream>
#include <pthread.h>
#include <chrono>

struct BankAccount {
    int balance;
    pthread_mutex_t mutex;
};

void init_account(BankAccount* acc, int initial) {
    acc->balance = initial;
    pthread_mutex_init(&acc->mutex, nullptr);
}

void* withdraw_thread(void* arg) {
    BankAccount* account = (BankAccount*)arg;
    
    for (int i = 0; i < 10000; i++) {
        pthread_mutex_lock(&account->mutex);
        
        // Critical section - only one thread at a time!
        if (account->balance >= 10) {
            account->balance -= 10;
        }
        
        pthread_mutex_unlock(&account->mutex);
    }
    
    return nullptr;
}

void* deposit_thread(void* arg) {
    BankAccount* account = (BankAccount*)arg;
    
    for (int i = 0; i < 10000; i++) {
        pthread_mutex_lock(&account->mutex);
        
        // Critical section
        account->balance += 10;
        
        pthread_mutex_unlock(&account->mutex);
    }
    
    return nullptr;
}

int main() {
    std::cout << "=== Bank Account Simulation ===" << std::endl;
    
    BankAccount account;
    init_account(&account, 1000);
    
    std::cout << "Initial balance: $" << account.balance << std::endl;
    
    pthread_t depositor, withdrawer;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    pthread_create(&depositor, nullptr, deposit_thread, &account);
    pthread_create(&withdrawer, nullptr, withdraw_thread, &account);
    
    pthread_join(depositor, nullptr);
    pthread_join(withdrawer, nullptr);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Final balance: $" << account.balance << std::endl;
    std::cout << "Time taken: " << duration.count() << " ms" << std::endl;
    
    pthread_mutex_destroy(&account.mutex);
    
    return 0;
}

/*
// FINE-GRAINED (many small locks) - Better parallelism
for (int i = 0; i < 1000000; i++) {
    pthread_mutex_lock(&mutex);    // Lock
    counter++;                      // Small critical section
    pthread_mutex_unlock(&mutex);  // Unlock
}
// Pro: Other threads can run between iterations
// Con: Lock/unlock overhead is expensive!

// COARSE-GRAINED (few large locks) - Less overhead
pthread_mutex_lock(&mutex);        // Lock once
for (int i = 0; i < 1000000; i++) {
    counter++;                      // Large critical section
}
pthread_mutex_unlock(&mutex);      // Unlock once
// Pro: Less lock/unlock overhead
// Con: Other threads blocked for longer

*/