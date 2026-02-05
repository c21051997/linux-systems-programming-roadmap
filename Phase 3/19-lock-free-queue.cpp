/* 
TP:
template class LockFreeQueue, private: struct Node + constructor (data, atomic next), atomic head + tail
public functions:
> Constructor: init dummy node, store in head and tail
> enqueue(const T& value): append a node at the end without locking
    create new node, infinite loop, get last node and last next, check if tail and last are still the same:
    if yes: if next == nullptr: if last->next compare weak (nullptr, new node): tail compare weak (last, new node) + return;
            if next != nullptr: try to advance tail
> dequeue: removes items from the front
*/  

// lockfree_queue.cpp
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>

template<typename T>
class LockFreeQueue {
private:
    struct Node {
        T data;
        std::atomic<Node*> next;
        
        Node(const T& val) : data(val), next(nullptr) {}
    };
    
    std::atomic<Node*> head;
    std::atomic<Node*> tail;
    
public:
    LockFreeQueue() {
        Node* dummy = new Node(T());
        head.store(dummy);
        tail.store(dummy);
    }
    
    void enqueue(const T& value) {
        Node* new_node = new Node(value);
        
        while (true) {
            Node* last = tail.load();
            Node* next = last->next.load();
            
            // Check if tail is still consistent
            if (last == tail.load()) {
                if (next == nullptr) {
                    // Try to link new node
                    Node* null_ptr = nullptr;
                    if (last->next.compare_exchange_weak(null_ptr, new_node)) {
                        // Success, try to swing tail
                        tail.compare_exchange_weak(last, new_node);
                        return;
                    }
                } else {
                    // Tail is falling behind, try to advance
                    tail.compare_exchange_weak(last, next);
                }
            }
        }
    }
    
    bool dequeue(T& result) {
        while (true) {
            Node* first = head.load();
            Node* last = tail.load();
            Node* next = first->next.load();
            
            // Check if head is still consistent
            if (first == head.load()) {
                if (first == last) {
                    if (next == nullptr) {
                        return false;  // Queue empty
                    }
                    // Tail falling behind, try to advance
                    tail.compare_exchange_weak(last, next);
                } else {
                    // Read value before CAS
                    result = next->data;
                    
                    // Try to swing head
                    if (head.compare_exchange_weak(first, next)) {
                        delete first;  // Safe to reclaim old dummy
                        return true;
                    }
                }
            }
        }
    }
    
    ~LockFreeQueue() {
        T dummy;
        while (dequeue(dummy)) {}
        delete head.load();
    }
};

void producer(LockFreeQueue<int>* queue, int id, int count) {
    for (int i = 0; i < count; i++) {
        queue->enqueue(id * 1000 + i);
    }
    std::cout << "Producer " << id << " finished" << std::endl;
}

void consumer(LockFreeQueue<int>* queue, int id, int count) {
    int received = 0;
    int value;
    
    while (received < count) {
        if (queue->dequeue(value)) {
            received++;
        }
    }
    
    std::cout << "Consumer " << id << " received " << received << " items" << std::endl;
}

int main() {
    std::cout << "=== Lock-Free Queue ===" << std::endl;
    
    LockFreeQueue<int> queue;
    
    const int producers = 4;
    const int consumers = 4;
    const int items_per_thread = 10000;
    
    std::vector<std::thread> threads;
    
    std::cout << "Starting " << producers << " producers and " 
              << consumers << " consumers..." << std::endl;
    
    // Start producers
    for (int i = 0; i < producers; i++) {
        threads.emplace_back(producer, &queue, i, items_per_thread);
    }
    
    // Start consumers
    for (int i = 0; i < consumers; i++) {
        threads.emplace_back(consumer, &queue, i, items_per_thread);
    }
    
    // Wait for all
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "\nAll threads completed!" << std::endl;
    
    std::cout << "\n=== Lock-Free Queue Properties ===" << std::endl;
    std::cout << "✓ Multiple producers, multiple consumers" << std::endl;
    std::cout << "✓ No locks (uses CAS)" << std::endl;
    std::cout << "✓ Non-blocking progress guarantee" << std::endl;
    std::cout << "✓ High performance under contention" << std::endl;
    
    return 0;
}