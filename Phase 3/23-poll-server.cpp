// poll_server.cpp
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <vector>
#include <cstring>


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

    std::cout << "=== poll() Server on port 8080 ===" << std::endl;

    // Vector of pollfd structures
    std::vector<struct pollfd> poll_fds;

    // Add server socket
    struct pollfd pfd;
    pfd.fd = server_fd;
    pfd.events = POLLIN; // Monitor for incoming data

    poll_fds.push_back(pfd);

    while (true) {
        std::cout << "\nMonitoring " << poll_fds.size() << " sockets..." << std::endl;
        
        // Wait for activity
        // i.e. block this thread until any of these fds has an event that i care about
        // -1 = wait forever
        int activity = poll(poll_fds.data(), poll_fds.size(), -1);

        if (activity < 0) {
            perror("poll() error");
            break;
        }

        // Check all FDs
        for (int i = 0; i < poll_fds.size(); ++i) {
            if (poll_fds[i].revents & POLLIN){
                if (poll_fds[i].fd == server_fd){
                    int client_fd = accept(server_fd, nullptr, nullptr);

                    std::cout << "New client: fd " << client_fd << std::endl;

                    struct pollfd client_pfd;
                    client_pfd.fd = client_fd;
                    client_pfd.events = POLLIN;

                    poll_fds.push_back(client_pfd);

                    const char* welcome = "Welcome to poll() server!\n";
                    write(client_fd, welcome, strlen(welcome));

                } else {
                    // Existing client
                    char buffer[1024];

                    ssize_t n = read(poll_fds[i].fd, buffer, sizeof(buffer) - 1);

                    if (n <= 0) {
                        std::cout << "Client disconnected: fd " << poll_fds[i].fd << std::endl;
                        close(poll_fds[i].fd);
                        poll_fds.erase(poll_fds.begin() + i);
                        i--;
                    } else {
                        buffer[n] = '\0';
                        std::cout << "Received: " << buffer;
                        write(poll_fds[i].fd, buffer, n);
                    }
                }
            }

            if (poll_fds[i].revents & (POLLERR | POLLHUP)) {
                std::cout << "Error on fd " << poll_fds[i].fd << std::endl;
                close(poll_fds[i].fd);
                poll_fds.erase(poll_fds.begin() + i);
                i--;
            }
        }
    }

    return 0;
}