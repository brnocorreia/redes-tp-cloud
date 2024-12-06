#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <cmath>
#include <algorithm>
#include <filesystem>

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

string getLocalIP() {
    const char* google_dns_server = "8.8.8.8";
    int dns_port = 53;

    struct sockaddr_in serv;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    //Socket could not be created
    if(sock < 0)
    {
        std::cout << "Socket error" << std::endl;
    }

    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr(google_dns_server);
    serv.sin_port = htons(dns_port);

    int err = connect(sock, (const struct sockaddr*)&serv, sizeof(serv));
    if (err < 0)
    {
        std::cout << "Error number: " << errno
            << ". Error message: " << strerror(errno) << std::endl;
    }

    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    err = getsockname(sock, (struct sockaddr*)&name, &namelen);

    char buffer[80];
    const char* p = inet_ntop(AF_INET, &name.sin_addr, buffer, 80);
    if(p != NULL)
    {
        std::cout << "Local IP address is: " << buffer << std::endl;
        return buffer;
    }
    else
    {
        std::cout << "Error number: " << errno
            << ". Error message: " << strerror(errno) << std::endl;
    }

    close(sock);
    return 0;
}

string sanitize_directory_name(const string& dir_name) {
    string sanitized = dir_name;

    // Removing relative prefixes like "./"
    if (sanitized.find("./") == 0) {
        sanitized = sanitized.substr(2); // Removes it
    }

    // Removing relative prefixes like "../"
    while (sanitized.find("../") == 0) {
        sanitized = sanitized.substr(3); // Removes it
    }

    // Replacing slashes with underscores
    replace(sanitized.begin(), sanitized.end(), '/', '_');

    return sanitized;
}

int main(int argc, char* argv[]) {
    // Checking if the correct number of arguments was provided
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <server_port>" << " <i_size>" << endl;
        return 1;
    }

    // Converting command line arguments
    int server_port = stoi(argv[1]);
    int i_size = stoi(argv[2]);

    int buffer_size = pow(2, i_size);
    int server_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        return 1;
    }

    sockaddr_in6 address{};
    address.sin6_family = AF_INET6;
    address.sin6_addr = in6addr_any;
    address.sin6_port = htons(server_port);
    string server_host = getLocalIP();

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("Failed to listen");
        return 1;
    }
    
    cout << "Server listening on IP: " << server_host << " at port " << server_port << endl;

    int client_fd = accept(server_fd, nullptr, nullptr);
    if (client_fd < 0) {
        perror("Failed to accept client");
        return 1;
    }

    cout << "Client connected. Waiting for messages..." << endl;

    // Receiving message from client
    char buffer[buffer_size] = {};
    char init_buffer[256] = {};

    int bytes_received = recv(client_fd, init_buffer, sizeof(init_buffer), 0);
    if (bytes_received <= 0 || strcmp(init_buffer, "READY") != 0) {
        cerr << "Erro: Initial message 'READY' not received from client." << endl;
        close(client_fd);
        close(server_fd);
        return 1;
    }
    cout << "Message 'READY' received from client." << endl;

    // Sending READY ACK to the client
    const char* ready_ack_message = "READY ACK";
    send(client_fd, ready_ack_message, strlen(ready_ack_message) + 1, 0);

    // Receiving directory name from the client
    memset(init_buffer, 0, sizeof(init_buffer)); // Limpar o buffer
    bytes_received = recv(client_fd, init_buffer, sizeof(init_buffer), 0);
    if (bytes_received <= 0) {
        cerr << "Error: Error found while receiving directory name from client." << endl;
        close(client_fd);
        close(server_fd);
        return 1;
    }

    string dir_name(init_buffer);
    cout << "Dirty directory name received from client: " << dir_name << endl;

    // Sanitizing the directory name
    dir_name = sanitize_directory_name(dir_name);

    // Identifying output filename based on the server hostname and directory name
    string results_dir = "./results";
    string output_filename = results_dir + "/" + server_host + "_" + dir_name + ".txt";

    if (!filesystem::exists(results_dir)) {
        if (!filesystem::create_directory(results_dir)) {
            cerr << "Error: Failed to create /results directory." << endl;
            return 1;
        }
    }

    ofstream output_file(output_filename, ios::out | ios::trunc);
    if (!output_file.is_open()) {
        cerr << "Error: Failed to create output file " << output_filename << endl;
        close(client_fd);
        close(server_fd);
        return 1;
    }

    const char* ack_message = "ACK";
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
            cout << "Received: " << received_message << endl;
            output_file << received_message << endl; // Saving the file name to the output file
        }
        // Sending ACK to the client
        send(client_fd, ack_message, strlen(ack_message) + 1, 0);
    }

    output_file.close();
    
    close(client_fd);
    close(server_fd);

    return 0;
}
