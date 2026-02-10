// nonblocking_io.cpp
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>

void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        std::cerr << "fcntl F_GETFL failed" << std::endl;
        return;
    }
    
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "fcntl F_SETFL failed" << std::endl;
    }
}

ssize_t read_all_nonblocking(int fd, char* buffer, size_t size) {
    size_t total = 0;
    
    while (total < size) {
        ssize_t n = read(fd, buffer + total, size - total);
        
        if (n == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No more data available right now
                std::cout << "Would block - read " << total << " bytes so far" << std::endl;
                break;
            } else if (errno == EINTR) {
                // Interrupted by signal, retry
                continue;
            } else {
                // Real error
                std::cerr << "Read error: " << strerror(errno) << std::endl;
                return -1;
            }
        }
        else if (n == 0) {
            // EOF
            std::cout << "EOF reached" << std::endl;
            break;
        }
        
        total += n;
    }
    
    return total;
}

ssize_t write_all_nonblocking(int fd, const char* buffer, size_t size) {
    size_t total = 0;
    
    while (total < size) {
        ssize_t n = write(fd, buffer + total, size - total);
        
        if (n == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Socket buffer full - would block
                std::cout << "Would block - wrote " << total << " bytes so far" << std::endl;
                break;
            } else if (errno == EINTR) {
                continue;
            } else {
                std::cerr << "Write error: " << strerror(errno) << std::endl;
                return -1;
            }
        }
        
        total += n;
    }
    
    return total;
}

int main() {
    std::cout << "=== Non-Blocking I/O Patterns ===" << std::endl;
    
    std::cout << "\nBlocking I/O:" << std::endl;
    std::cout << "  read(fd, buf, 1024)  → BLOCKS until data arrives" << std::endl;
    std::cout << "  write(fd, buf, 1024) → BLOCKS until buffer space available" << std::endl;
    
    std::cout << "\nNon-Blocking I/O:" << std::endl;
    std::cout << "  read(fd, buf, 1024)  → Returns immediately" << std::endl;
    std::cout << "    - Returns bytes read, or -1 with EAGAIN" << std::endl;
    std::cout << "  write(fd, buf, 1024) → Returns immediately" << std::endl;
    std::cout << "    - Returns bytes written, or -1 with EAGAIN" << std::endl;
    
    std::cout << "\n=== Common Patterns ===" << std::endl;
    std::cout << "Read until EAGAIN:" << std::endl;
    std::cout << R"(
    while (true) {
        n = read(fd, buf, size);
        if (n == -1 && errno == EAGAIN) break;
        process(buf, n);
    }
)" << std::endl;
    
    std::cout << "Write with retry:" << std::endl;
    std::cout << R"(
    while (remaining > 0) {
        n = write(fd, buf, remaining);
        if (n == -1 && errno == EAGAIN) {
            // Re-add to epoll with EPOLLOUT
            // Wait for socket to become writable
            break;
        }
        buf += n;
        remaining -= n;
    }
)" << std::endl;
    
    return 0;
}