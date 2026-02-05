// select_server.cpp
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <cstring>
#include <vector>

int main() {
    // Create listening socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);
    
    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 10);
    
    std::cout << "=== select() Server on port 8080 ===" << std::endl;
    std::cout << "Can handle multiple clients!\n" << std::endl;
    
    fd_set master_set;
    FD_ZERO(&master_set);
    FD_SET(server_fd, &master_set);
    
    int max_fd = server_fd;
    
    while (true) {
        // Copy master set (select modifies it!)
        fd_set read_fds = master_set;
        
        std::cout << "Waiting for activity on " << (max_fd + 1) 
                  << " file descriptors..." << std::endl;
        
        // Wait for activity on any socket
        int activity = select(max_fd + 1, &read_fds, nullptr, nullptr, nullptr);
        
        if (activity < 0) {
            std::cerr << "select() error" << std::endl;
            break;
        }
        
        std::cout << "Activity detected on " << activity << " sockets" << std::endl;
        
        // Check all file descriptors
        for (int fd = 0; fd <= max_fd; fd++) {
            if (FD_ISSET(fd, &read_fds)) {
                if (fd == server_fd) {
                    // New connection
                    int client_fd = accept(server_fd, nullptr, nullptr);
                    
                    std::cout << "New client connected: fd " << client_fd << std::endl;
                    
                    FD_SET(client_fd, &master_set);
                    if (client_fd > max_fd) {
                        max_fd = client_fd;
                    }
                    
                    const char* welcome = "Welcome! Type a message.\n";
                    write(client_fd, welcome, strlen(welcome));
                } 
                else {
                    // Existing client has data
                    char buffer[1024];
                    ssize_t n = read(fd, buffer, sizeof(buffer) - 1);
                    
                    if (n <= 0) {
                        // Client disconnected
                        std::cout << "Client disconnected: fd " << fd << std::endl;
                        close(fd);
                        FD_CLR(fd, &master_set);
                    } 
                    else {
                        // Echo back
                        buffer[n] = '\0';
                        std::cout << "Received from fd " << fd << ": " << buffer;
                        write(fd, buffer, n);
                    }
                }
            }
        }
    }
    
    close(server_fd);
    return 0;
}