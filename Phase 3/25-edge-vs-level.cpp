#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

void demonstrate_modes() {
    std::cout << "=== Edge-Triggered vs Level-Triggered ===" << std::endl;
    
    std::cout << "\nLevel-Triggered (default):" << std::endl;
    std::cout << "  Socket has 100 bytes available" << std::endl;
    std::cout << "  1. epoll_wait() returns → 'fd ready'" << std::endl;
    std::cout << "  2. You read 50 bytes" << std::endl;
    std::cout << "  3. epoll_wait() returns AGAIN → 'fd ready'" << std::endl;
    std::cout << "  4. You read remaining 50 bytes" << std::endl;
    std::cout << "  5. epoll_wait() blocks (no more data)" << std::endl;
    std::cout << "\n  Easy to use, but less efficient" << std::endl;
    
    std::cout << "\nEdge-Triggered (EPOLLET):" << std::endl;
    std::cout << "  Socket has 100 bytes available" << std::endl;
    std::cout << "  1. epoll_wait() returns → 'fd ready'" << std::endl;
    std::cout << "  2. You read 50 bytes" << std::endl;
    std::cout << "  3. epoll_wait() blocks! (no notification)" << std::endl;
    std::cout << "  4. You MUST read until EAGAIN!" << std::endl;
    std::cout << "\n  More efficient, but requires careful handling" << std::endl;
    
    std::cout << "\n=== Edge-Triggered Requirements ===" << std::endl;
    std::cout << "1. MUST use non-blocking I/O" << std::endl;
    std::cout << "2. MUST read until EAGAIN/EWOULDBLOCK" << std::endl;
    std::cout << "3. MUST write until EAGAIN/EWOULDBLOCK" << std::endl;
    std::cout << "4. Handles spurious wakeups gracefully" << std::endl;
}


int main() {
    demonstrate_modes();
    std::cout << "\n=== Example: Reading in Edge-Triggered ===" << std::endl;
    std::cout << R"(
while (true) {
    ssize_t n = read(fd, buffer, sizeof(buffer));
    
    if (n == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No more data - this is NORMAL!
            break;
        } else {
            // Real error
            return -1;
        }
    }
    else if (n == 0) {
        // EOF (connection closed)
        return 0;
    }
    
    // Process n bytes...
}
)" << std::endl;
    
    return 0;
}