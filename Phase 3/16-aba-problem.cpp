/*
ABA problem explained: when value changes from A->B->A, and CAS cannot tell that anything changed
*/

#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>

struct Node {
    int data;
    Node* next;

    Node(int d) : data(d), next(nullptr) {}
};

class VulnerableStack {
private:
    std::atomic<Node*> head;
public:
    VulnerableStack() : head(nullptr) {}

    void push(int value) {
        Node* new_node = new Node(value);
        Node* old_head = head.load();

        do {
            new_node->next = old_head;
        } while (!head.compare_exchange_weak(old_head, new_node));
    }

    bool pop(int& result) {
        Node* old_head = head.load();

        while (old_head != nullptr) {
            Node* next = old_head->next;

            // Simulate slow operation
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            // ABA problem can occur here!
            if (head.compare_exchange_weak(old_head, next)) {
                result = old_head->data;
                delete old_head;
                return true;
            }
        }
        return false;
    }
};

void demonstrate_aba() {
    std::cout << "=== ABA Problem Demonstration ===" << std::endl;
    std::cout << "\nScenario:" << std::endl;
    std::cout << "1. Stack: A → B → C" << std::endl;
    std::cout << "2. Thread 1 reads head (A), gets interrupted" << std::endl;
    std::cout << "3. Thread 2: pop(A), pop(B), push(A)" << std::endl;
    std::cout << "4. Stack is now: A → C (but A is same address!)" << std::endl;
    std::cout << "5. Thread 1 resumes, CAS succeeds (thinks nothing changed!)" << std::endl;
    std::cout << "6. Result: B is lost, C might be leaked!" << std::endl;
    
    std::cout << "\n=== Solutions ===" << std::endl;
    std::cout << "1. Tagged pointers (add version counter)" << std::endl;
    std::cout << "2. Hazard pointers (mark nodes as 'in use')" << std::endl;
    std::cout << "3. Garbage collection" << std::endl;
    std::cout << "4. Use double-width CAS (DWCAS)" << std::endl;
}



// Solution: tagged pointer
class SafeStack {
private:
    struct TaggedPointer {
        Node* ptr;
        uintptr_t tag;
    };

    std::atomic<TaggedPointer> head;

    static bool compare_exchange(std::atomic<TaggedPointer>& atomic_tp,
                                 TaggedPointer& expected,
                                 TaggedPointer desired) {
        // On some platforms, need to use special 128-bit CAS
        // For x86-64: CMPXCHG16B instruction
        return atomic_tp.compare_exchange_weak(expected, desired);
    }
public:
    SafeStack() {
        TaggedPointer tp;
        tp.ptr = nullptr;
        tp.tag = 0;
        head.store(tp);
    }

    void push(int value) {
        Node* new_node = new Node(value);
        TaggedPointer old_head = head.load();
        TaggedPointer new_head;
        
        do {
            new_node->next = old_head.ptr;
            new_head.ptr = new_node;
            new_head.tag = old_head.tag + 1;  // Increment version!
        } while (!compare_exchange(head, old_head, new_head));
    }
    
    bool pop(int& result) {
        TaggedPointer old_head = head.load();
        TaggedPointer new_head;
        
        while (old_head.ptr != nullptr) {
            new_head.ptr = old_head.ptr->next;
            new_head.tag = old_head.tag + 1;  // Increment version!
            
            if (compare_exchange(head, old_head, new_head)) {
                result = old_head.ptr->data;
                delete old_head.ptr;
                return true;
            }
        }
        return false;
    }
};

int main() {
    demonstrate_aba();
    
    std::cout << "\n=== Tagged Pointer Example ===" << std::endl;
    std::cout << "Pointer: 0x7fff1234 | Tag: 5" << std::endl;
    std::cout << "Even if pointer value repeats, tag differs!" << std::endl;
    std::cout << "CAS checks BOTH pointer and tag" << std::endl;
    
    return 0;
}