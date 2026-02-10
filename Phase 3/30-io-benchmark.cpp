// io_benchmark.cpp
#include <iostream>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <vector>
#include <thread>

void benchmark_summary() {
    std::cout << "=== I/O Multiplexing Performance Summary ===" << std::endl;
    std::cout << "\nTypical Performance (connections/second):" << std::endl;
    std::cout << "\nBlocking (one thread per connection):" << std::endl;
    std::cout << "  Max connections: ~1000 (memory limited)" << std::endl;
    std::cout << "  Throughput: ~10,000 req/sec" << std::endl;
    
    std::cout << "\nselect():" << std::endl;
    std::cout << "  Max connections: 1024 (FD_SETSIZE)" << std::endl;
    std::cout << "  Throughput: ~50,000 req/sec" << std::endl;
    std::cout << "  CPU usage: High (O(n) scanning)" << std::endl;
    
    std::cout << "\npoll():" << std::endl;
    std::cout << "  Max connections: Unlimited" << std::endl;
    std::cout << "  Throughput: ~50,000 req/sec" << std::endl;
    std::cout << "  CPU usage: High (O(n) scanning)" << std::endl;
    
    std::cout << "\nepoll() (level-triggered):" << std::endl;
    std::cout << "  Max connections: Millions" << std::endl;
    std::cout << "  Throughput: ~500,000 req/sec" << std::endl;
    std::cout << "  CPU usage: Low (O(1))" << std::endl;
    
    std::cout << "\nepoll() (edge-triggered):" << std::endl;
    std::cout << "  Max connections: Millions" << std::endl;
    std::cout << "  Throughput: ~700,000 req/sec" << std::endl;
    std::cout << "  CPU usage: Very low" << std::endl;
    
    std::cout << "\nio_uring:" << std::endl;
    std::cout << "  Max connections: Millions" << std::endl;
    std::cout << "  Throughput: ~1,000,000+ req/sec" << std::endl;
    std::cout << "  CPU usage: Lowest (batch operations)" << std::endl;
    
    std::cout << "\n=== Real-World Examples ===" << std::endl;
    std::cout << "\nNginx (epoll):" << std::endl;
    std::cout << "  - Handles 100,000+ concurrent connections" << std::endl;
    std::cout << "  - ~1M requests/sec per core" << std::endl;
    
    std::cout << "\nRedis (epoll):" << std::endl;
    std::cout << "  - 100,000+ operations/sec" << std::endl;
    std::cout << "  - Single-threaded event loop" << std::endl;
    
    std::cout << "\nNode.js (epoll on Linux):" << std::endl;
    std::cout << "  - Thousands of concurrent connections" << std::endl;
    std::cout << "  - Event-driven, non-blocking I/O" << std::endl;
}

int main() {
    benchmark_summary();
    
    std::cout << "\n=== Choosing the Right Approach ===" << std::endl;
    std::cout << "\nUse blocking I/O when:" << std::endl;
    std::cout << "  - Few connections (<100)" << std::endl;
    std::cout << "  - Simplicity is priority" << std::endl;
    std::cout << "  - Learning/prototyping" << std::endl;
    
    std::cout << "\nUse select() when:" << std::endl;
    std::cout << "  - Portability required (POSIX)" << std::endl;
    std::cout << "  - Few connections (<100)" << std::endl;
    std::cout << "  - Timeout support needed" << std::endl;
    
    std::cout << "\nUse poll() when:" << std::endl;
    std::cout << "  - More than 1024 connections" << std::endl;
    std::cout << "  - Portability required" << std::endl;
    std::cout << "  - Cleaner API than select()" << std::endl;
    
    std::cout << "\nUse epoll() when:" << std::endl;
    std::cout << "  - High performance needed" << std::endl;
    std::cout << "  - Many connections (1000+)" << std::endl;
    std::cout << "  - Linux-only is acceptable" << std::endl;
    
    std::cout << "\nUse io_uring when:" << std::endl;
    std::cout << "  - Maximum performance needed" << std::endl;
    std::cout << "  - Modern kernel (5.1+)" << std::endl;
    std::cout << "  - Complex I/O patterns" << std::endl;
    std::cout << "  - Zero-copy desired" << std::endl;
    
    return 0;
}