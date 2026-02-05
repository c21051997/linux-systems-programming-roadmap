// blocking_server.cpp - The OLD, BAD way
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>


void handle_client_blocking(int client_fd) {
    char buffer[1024];

    // BLOCKS here if client doesn't send data!
    ssize_t n = read(client_fd, buffer, sizeof(buffer) - 1);

    if (n > 0) {
        buffer[n] = '\0';
        std::cout << "Received: " << buffer << std::endl;
        write(client_fd, "OK\n", 3);
    }

    close(client_fd);
}

int main(){
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);

    bind(server_fd, (struct sockaddr*) &addr, sizeof(addr));
    listen(server_fd, 10);

    std::cout << "Blocking server on port 8080" << std::endl;
    std::cout << "This can only handle ONE client at a time!" << std::endl;

    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        handle_client_blocking(client_fd); // Blocks until client done!
    }

    return 0;
}