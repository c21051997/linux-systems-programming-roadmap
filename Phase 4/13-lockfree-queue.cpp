/*
Core design ideas:
> Dummy node: queue is empty when head->next = nullptr, NOT when head == nullptr
> Structure: Head points to dummy / old dummy, real data starts at head->next:
head                 tail
 ↓                    ↓
[D] → [A] → [B] → [C] → NULL



funcs:
> enqueue (const T& value): attach a new node at the end
create new node with value, while true loop
read tail and tail next (both with MO acquire)
ensure tail consistency:
case 1: next = nullptr, tail is pointing to the last node, try to link new node
    create null ptr node, if: compare last->next, with nullptr, set to new node, MO release, MO relaxed
        compare tail to last, set to new node, MO release, MO relaxed
case 2: tail is falling behind, try to advance it
    compare tail with last, set to next



*/

// michael_scott_queue.cpp
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>

template<typename T>
class MichaelScottQueue {
private:
    struct Node {
        T data;
        std::atomic<Node*> next;
        
        Node() : next(nullptr) {}
        Node(const T& value) : data(value), next(nullptr) {}
    };
    
    std::atomic<Node*> head;
    std::atomic<Node*> tail;
    
public:
    MichaelScottQueue() {
        // Create dummy node
        Node* dummy = new Node();
        head.store(dummy);
        tail.store(dummy);
    }
    
    ~MichaelScottQueue() {
        while (Node* node = head.load()) {
            head.store(node->next);
            delete node;
        }
    }
    
    void enqueue(const T& value) {
        Node* new_node = new Node(value);
        
        while (true) {
            Node* last = tail.load(std::memory_order_acquire);
            Node* next = last->next.load(std::memory_order_acquire);
            
            // Check if tail is still consistent
            if (last == tail.load(std::memory_order_acquire)) {
                if (next == nullptr) {
                    // Tail is pointing to last node, try to link new node
                    Node* null_ptr = nullptr;
                    if (last->next.compare_exchange_weak(null_ptr, new_node,
                                                         std::memory_order_release,
                                                         std::memory_order_relaxed)) {
                        // Success! Try to swing tail to new node
                        tail.compare_exchange_weak(last, new_node,
                                                   std::memory_order_release,
                                                   std::memory_order_relaxed);
                        return;
                    }
                } else {
                    // Tail is falling behind, try to advance it
                    tail.compare_exchange_weak(last, next,
                                               std::memory_order_release,
                                               std::memory_order_relaxed);
                }
            }
        }
    }
    
    bool dequeue(T& result) {
        while (true) {
            Node* first = head.load(std::memory_order_acquire);
            Node* last = tail.load(std::memory_order_acquire);
            Node* next = first->next.load(std::memory_order_acquire);
            
            // Check if head is still consistent
            if (first == head.load(std::memory_order_acquire)) {
                if (first == last) {
                    if (next == nullptr) {
                        return false;  // Queue is empty
                    }
                    
                    // Tail is falling behind, try to advance it
                    tail.compare_exchange_weak(last, next,
                                               std::memory_order_release,
                                               std::memory_order_relaxed);
                } else {
                    // Read value before CAS
                    result = next->data;
                    
                    // Try to swing head to next node
                    if (head.compare_exchange_weak(first, next,
                                                   std::memory_order_release,
                                                   std::memory_order_relaxed)) {
                        delete first;  // Safe to reclaim old dummy
                        return true;
                    }
                }
            }
        }
    }
    
    bool empty() const {
        Node* first = head.load(std::memory_order_acquire);
        Node* next = first->next.load(std::memory_order_acquire);
        return next == nullptr;
    }
};

void explain_michael_scott() {
    std::cout << "=== Michael-Scott Queue Algorithm ===" << std::endl;
    std::cout << R"(
Key Insights:
1. Uses dummy node (simplifies empty queue case)
2. Both head and tail can lag behind
3. Helper threads can advance tail

Structure:
    head            tail
     ↓               ↓
    [D] → [A] → [B] → [C] → NULL
    Dummy

Enqueue(X):
    1. Allocate new node X
    2. Try to link X after tail
    3. If success, try to swing tail to X
    4. If tail lags, helper thread can advance it

Dequeue():
    1. Read head and head->next
    2. If head == tail and next == NULL, empty
    3. If head == tail and next != NULL, advance tail (helper)
    4. Try to swing head to next
    5. Delete old dummy

Correctness:
    - Dummy node ensures head never equals NULL
    - Tail can lag (other threads help advance it)
    - Dequeue never touches tail directly
    - Enqueue never touches head directly
    
Performance:
    - Lock-free progress guarantee
    - Scales well with many threads
    - Industry standard implementation
)" << std::endl;
}

void benchmark_queue() {
    std::cout << "\n=== Queue Benchmark ===" << std::endl;
    
    MichaelScottQueue<int> queue;
    const int OPERATIONS = 1000000;
    const int NUM_THREADS = 4;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    
    // Producers
    for (int i = 0; i < NUM_THREADS / 2; i++) {
        threads.emplace_back([&]() {
            for (int j = 0; j < OPERATIONS; j++) {
                queue.enqueue(j);
            }
        });
    }
    
    // Consumers
    for (int i = 0; i < NUM_THREADS / 2; i++) {
        threads.emplace_back([&]() {
            int value;
            for (int j = 0; j < OPERATIONS; j++) {
                while (!queue.dequeue(value)) {
                    std::this_thread::yield();
                }
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Operations: " << OPERATIONS * NUM_THREADS << std::endl;
    std::cout << "Time: " << duration.count() << " ms" << std::endl;
    std::cout << "Throughput: " << (OPERATIONS * NUM_THREADS * 1000.0 / duration.count()) 
              << " ops/sec" << std::endl;
}

int main() {
    explain_michael_scott();
    benchmark_queue();
    
    return 0;
}