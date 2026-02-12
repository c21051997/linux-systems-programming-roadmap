// progress_guarantees.cpp
#include <iostream>

void explain_progress_guarantees() {
    std::cout << "=== Progress Guarantees ===" << std::endl;
    
    std::cout << "\n1. BLOCKING (Traditional Locks):" << std::endl;
    std::cout << "   - If one thread holds lock and stops, ALL threads stop" << std::endl;
    std::cout << "   - Example: mutex, rwlock" << std::endl;
    std::cout << "   - Guarantee: NONE (can deadlock)" << std::endl;
    
    std::cout << "\n2. OBSTRUCTION-FREE:" << std::endl;
    std::cout << "   - If thread runs in isolation, it completes in finite steps" << std::endl;
    std::cout << "   - Multiple threads can block each other" << std::endl;
    std::cout << "   - Weakest lock-free guarantee" << std::endl;
    
    std::cout << "\n3. LOCK-FREE:" << std::endl;
    std::cout << "   - System-wide progress guaranteed" << std::endl;
    std::cout << "   - At least ONE thread makes progress in finite steps" << std::endl;
    std::cout << "   - Individual thread may starve (but rare)" << std::endl;
    std::cout << "   - Example: CAS-based queues" << std::endl;
    
    std::cout << "\n4. WAIT-FREE:" << std::endl;
    std::cout << "   - EVERY thread makes progress in finite steps" << std::endl;
    std::cout << "   - Strongest guarantee (and hardest to achieve)" << std::endl;
    std::cout << "   - No thread can block another" << std::endl;
    std::cout << "   - Example: wait-free queues (very complex)" << std::endl;
    
    std::cout << "\n=== Visual Representation ===" << std::endl;
    std::cout << R"(
Blocking (Mutex):
  Thread A: [Lock]────────[Work]────[Unlock]
  Thread B:        [Wait................Wait][Lock]

Lock-Free (CAS):
  Thread A: [Try CAS]──[Success]──[Done]
  Thread B: [Try CAS]──[Fail]──[Retry]──[Success]──[Done]
            At least one always succeeds!

Wait-Free:
  Thread A: [Operation]──────[Done] (bounded steps)
  Thread B: [Operation]──────[Done] (bounded steps)
            Both always complete in finite time!
)" << std::endl;
}

int main() {
    explain_progress_guarantees();
    
    std::cout << "\n=== When to Use Each ===" << std::endl;
    std::cout << "Lock-Free: Most practical for high-performance systems" << std::endl;
    std::cout << "Wait-Free: Critical real-time systems (very complex)" << std::endl;
    std::cout << "Blocking:  Everything else (simpler, easier to reason about)" << std::endl;
    
    return 0;
}