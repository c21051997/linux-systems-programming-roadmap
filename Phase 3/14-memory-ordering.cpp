// memory_ordering.cpp
#include <iostream>
#include <atomic>
#include <thread>
#include <cassert>

// Shared variables
std::atomic<int> x(0);
std::atomic<int> y(0);
int r1, r2;

void thread1_relaxed() {
    x.store(1, std::memory_order_relaxed);
    r1 = y.load(std::memory_order_relaxed);
}

void thread2_relaxed() {
    y.store(1, std::memory_order_relaxed);
    r2 = x.load(std::memory_order_relaxed);
}

void thread1_seq_cst() {
    x.store(1, std::memory_order_seq_cst);
    r1 = y.load(std::memory_order_seq_cst);
}

void thread2_seq_cst() {
    y.store(1, std::memory_order_seq_cst);
    r2 = x.load(std::memory_order_seq_cst);
}

int main() {
    std::cout << "=== Memory Ordering ===" << std::endl;
    
    // Test with relaxed ordering
    std::cout << "\nTesting with relaxed ordering (many iterations):" << std::endl;
    int weird_cases = 0;
    
    for (int i = 0; i < 100000; i++) {
        x = 0;
        y = 0;
        r1 = 0;
        r2 = 0;
        
        std::thread t1(thread1_relaxed);
        std::thread t2(thread2_relaxed);
        
        t1.join();
        t2.join();
        
        // Can we see r1 == 0 && r2 == 0?
        // With reordering, YES! (counter-intuitive)
        if (r1 == 0 && r2 == 0) {
            weird_cases++;
        }
    }
    
    std::cout << "Cases where both reads saw 0: " << weird_cases << " / 100000" << std::endl;
    std::cout << "(This should be > 0 with relaxed ordering)" << std::endl;
    
    // Test with sequential consistency
    std::cout << "\nTesting with sequential consistency:" << std::endl;
    weird_cases = 0;
    
    for (int i = 0; i < 100000; i++) {
        x = 0;
        y = 0;
        r1 = 0;
        r2 = 0;
        
        std::thread t1(thread1_seq_cst);
        std::thread t2(thread2_seq_cst);
        
        t1.join();
        t2.join();
        
        if (r1 == 0 && r2 == 0) {
            weird_cases++;
        }
    }
    
    std::cout << "Cases where both reads saw 0: " << weird_cases << " / 100000" << std::endl;
    std::cout << "(This should be 0 with seq_cst)" << std::endl;
    
    std::cout << "\n=== Memory Order Types ===" << std::endl;
    std::cout << "memory_order_relaxed:  No ordering guarantees (fastest)" << std::endl;
    std::cout << "memory_order_acquire:  Loads/stores after can't move before" << std::endl;
    std::cout << "memory_order_release:  Loads/stores before can't move after" << std::endl;
    std::cout << "memory_order_acq_rel:  Both acquire and release" << std::endl;
    std::cout << "memory_order_seq_cst:  Total ordering (slowest, safest)" << std::endl;
    
    return 0;
}