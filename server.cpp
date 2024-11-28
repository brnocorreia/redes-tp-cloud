#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cmath>

using namespace std;

string getLocalIPAddress() {
    char hostname[1024];
    if (gethostname(hostname, sizeof(hostname)) == -1) {
        perror("Error getting hostname");
        return "unknown_host";
    }

    addrinfo hints{}, *res;
    hints.ai_family = AF_INET6; // IPv6 support
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(hostname, nullptr, &hints, &res) != 0) {
        perror("Error resolving hostname");
        return "unknown_host";
    }

    // Extract IP address from the first result
    char ipstr[INET6_ADDRSTRLEN];
    void* addr;
    if (res->ai_family == AF_INET) {
        sockaddr_in* ipv4 = (sockaddr_in*)res->ai_addr;
        addr = &(ipv4->sin_addr);
    } else {
        sockaddr_in6* ipv6 = (sockaddr_in6*)res->ai_addr;
        addr = &(ipv6->sin6_addr);
    }

    inet_ntop(res->ai_family, addr, ipstr, sizeof(ipstr));
    freeaddrinfo(res);
    return string(ipstr);
}

int main() {
    int buffer_size = pow(2, 16);
    int server_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        return 1;
    }

    sockaddr_in6 address{};
    address.sin6_family = AF_INET6;
    address.sin6_addr = in6addr_any;
    address.sin6_port = htons(8080);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("Failed to listen");
        return 1;
    }

    cout << "Server listening on port 8080..." << endl;

    int client_fd = accept(server_fd, nullptr, nullptr);
    if (client_fd < 0) {
        perror("Failed to accept client");
        return 1;
    }

    cout << "Client connected. Waiting for messages..." << endl;

    // Receiving message from client
    char buffer[buffer_size] = {};

    int bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0 || strcmp(buffer, "READY") != 0) {
        cerr << "Erro: Initial message 'READY' not received from client." << endl;
        close(client_fd);
        close(server_fd);
        return 1;
    }
    cout << "Message 'READY' received from client." << endl;

    // Sending READY ACK to the client
    const char* ready_ack_message = "READY ACK";
    send(client_fd, ready_ack_message, strlen(ready_ack_message), 0);

    // Receiving directory name from the client
    memset(buffer, 0, sizeof(buffer)); // Limpar o buffer
    bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0) {
        cerr << "Error: Error found while receiving directory name from client." << endl;
        close(client_fd);
        close(server_fd);
        return 1;
    }

    string dir_name(buffer);
    cout << "Directory name received from client: " << dir_name << endl;

    // Identifying output filename based on the server hostname and directory name
    string server_host = getLocalIPAddress();
    string output_filename = server_host + "_" + dir_name + ".txt";

    ofstream output_file(output_filename, ios::out | ios::trunc);
    if (!output_file.is_open()) {
        cerr << "Error: Failed to create output file " << output_filename << endl;
        close(client_fd);
        close(server_fd);
        return 1;
    }

    // Receiving file names from the client and saving them to the output file
    while (true) {
        memset(buffer, 0, sizeof(buffer)); // Clearing the buffer before each read
        bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            perror("Error: Error found while receiving file name from client.");
            break;
        }

        string received_message(buffer);
        if (received_message == "bye") {
            cout << "Bye message received from client. Saving file and closing connection." << endl;
            break;
        } else {
            cout << "Recebido: " << received_message << endl;
            output_file << received_message << endl; // Saving the file name to the output file
        }
    }

    output_file.close();
    
    close(client_fd);
    close(server_fd);

    return 0;
}
