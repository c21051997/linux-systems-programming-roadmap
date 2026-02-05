/*
Goal: implement a lock free stack using CAS

Technical planning:
Template class: privates: atmoic<Node*> (head), struct (Node): T (data), Node* (next) + constructor
publics:  constructor, push func, pop func, destructor
void push (const T& value): create new node, load old head (use memory ordering relaxed), do while loop: do: set new node next = old head, while: not CAS weak, old head and new node
bool pop(T& result): find old head, while old head != nullptr, if compare weak (old head, old head next), result = old head data, delete old head, return true

*/
// compare_and_swap.cpp
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>

// Lock-free stack using CAS

template <typename T>
class LockFreeStack {
private:
    struct Node {
        T data;
        Node* next;

        Node(T d) : data(d), next(nullptr) {}
    }
    std::atomic<Node*> head;
public:
    LockFreeStack() : head(nullptr) {}

    void push(const T& value) {
        Node* new_node = new Node(value);

        Node* old_head = head.load(std::memory_ordering_relaxed);

        do {
            new_node->next = old_head;
            // CAS: if head is still old_head, set it to new_node
            // on success it does first memory order, on failure it does the second
        } while (!head.compare_exchange_weak(old_head, new_node, std::memory_order_release, std::memory_order_relaxed))
    }

    bool pop(T& result) {
        Node* old_head = head.load(std::memory_order_relaxed);

        // Keep trying until successful
        while (old_head != nullptr) {
            // CAS: If head is still old_head, set it to old_head->next
            if (head.compare_exchange_weak(old_head, old_head->next, std::memory_order_acquire, std::memory_order_relaxed)) {
                result = old_head->data;
                delete old_head;
                return true;
            }
            // If CAS failed, old_head is updated to current value, retry
        }

        return false;
    }

    ~LockFreeStack() {
        Node* current = head.load();
        while (current) {
            Node* next = current->next;
            delete current;
            current = next;
        }
    }
};

void* pusher_thread(void* arg) {
    LockFreeStack<int>* stack = (LockFreeStack<int>*)arg;

    for (int i = 0; i < 10000; ++i) {
        stack->push(i);
    }

    return nullptr;
}

void* popper_thread(void* arg) {
    LockFreeStack<int>* stack = (LockFreeStack<int>*)arg;
    int value;
    int count = 0;

    for (int i = 0; i < 10000; ++i) {
        if (stack->pop(value)) {
            count++;
        }
    }

    return nullptr;
}

int main() {
    std::cout << "=== Lock-Free Stack using CAS ===" << std::endl;
    
    LockFreeStack<int> stack;
    
    pthread_t pushers[4];
    pthread_t poppers[4];
    
    std::cout << "Starting 4 pusher and 4 popper threads..." << std::endl;
    
    // Create pushers
    for (int i = 0; i < 4; i++) {
        pthread_create(&pushers[i], nullptr, pusher_thread, &stack);
    }
    
    // Create poppers
    for (int i = 0; i < 4; i++) {
        pthread_create(&poppers[i], nullptr, popper_thread, &stack);
    }
    
    // Wait for all
    for (int i = 0; i < 4; i++) {
        pthread_join(pushers[i], nullptr);
    }
    for (int i = 0; i < 4; i++) {
        pthread_join(poppers[i], nullptr);
    }
    
    std::cout << "All threads completed!" << std::endl;
    
    std::cout << "\n=== How CAS Works ===" << std::endl;
    std::cout << "compare_exchange(expected, desired):" << std::endl;
    std::cout << "  1. Read current value" << std::endl;
    std::cout << "  2. Compare with 'expected'" << std::endl;
    std::cout << "  3. If equal, write 'desired' (ATOMIC)" << std::endl;
    std::cout << "  4. If not equal, update 'expected' with current value" << std::endl;
    std::cout << "  5. Return true if successful, false otherwise" << std::endl;
    
    std::atomic<int> x(100);
    int expected = 100;
    bool success = x.compare_exchange_strong(expected, 200);
    std::cout << "\nExample: x=100, CAS(100, 200) → " << (success ? "SUCCESS" : "FAIL") << std::endl;
    std::cout << "x is now: " << x.load() << std::endl;
    
    expected = 100;
    success = x.compare_exchange_strong(expected, 300);
    std::cout << "\nExample: x=200, CAS(100, 300) → " << (success ? "SUCCESS" : "FAIL") << std::endl;
    std::cout << "x is now: " << x.load() << std::endl;
    std::cout << "expected updated to: " << expected << std::endl;
    
    return 0;
}

/*
**CAS Visualization:**
```
Lock-Free Stack Push:

Initial: head → [3] → [2] → [1] → NULL

Thread A wants to push 4:
1. Read head: [3]
2. Create new node: [4] → [3]
3. CAS(head, [3], [4]):
   - If head still [3]: head = [4] ✓
   - If head changed:   retry

Thread B concurrent push 5:
1. Read head: [3]
2. Create new node: [5] → [3]
3. CAS(head, [3], [5]):
   - If A succeeded first: head = [4], CAS fails
   - Retry with new head value

Result: Both succeed, one retries
No locks needed!
*/