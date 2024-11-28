#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h> // Hostname resolution
#include <sys/time.h> // Time measurement
#include <dirent.h> // List files in a directory
#include <sys/stat.h> // Verify file type

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <server_host> <server_port> <dir_name>" << endl;
        return 1;
    }

    // Converting command line arguments
    const char* server_host = argv[1];
    int server_port = stoi(argv[2]);
    const char* dir_name = argv[3];

    cout << "Trying to connect to the server at " << server_host 
              << " on port " << server_port
              << " using directory " << dir_name << endl;

    int sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Socket creation failed!");
        return 1;
    }

    sockaddr_in6 server{};
    server.sin6_family = AF_INET6;
    server.sin6_port = htons(server_port);

    // Resolving hostname to IP address
    addrinfo hints{}, *res;
    hints.ai_family = AF_INET6; // IPv6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_V4MAPPED | AI_ALL; // Allow IPv4 mapping on IPv6

    if (getaddrinfo(server_host, nullptr, &hints, &res) != 0) {
        perror("Failed to resolve hostname!");
        close(sock);
        return 1;
    }

    memcpy(&server.sin6_addr, 
           &((sockaddr_in6*)res->ai_addr)->sin6_addr, 
           sizeof(server.sin6_addr));
    freeaddrinfo(res);

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("Failed to connect to server!");
        close(sock);
        return 1;
    }

    cout << "Connection stablished succesfully!" << endl;

    // Sending READY message to the server
    const char* ready_message = "READY";
    send(sock, ready_message, strlen(ready_message), 0);

    // Start time measurement
    timeval start_time{}, end_time{};
    gettimeofday(&start_time, nullptr);

    cout << "Ready message sent to the server! Waiting for response..." << endl;

    // Receiving "READY ACK" from the server
    char buffer[1024] = {};
    int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        cout << "Server response: " << buffer << endl;
    } else {
        cerr << "Error receiving message from server." << endl;
        return 1;
    }

    // Verificar se o servidor respondeu "READY ACK"
    if (strcmp(buffer, "READY ACK") == 0) {
        cout << "Starting file transfer from: " << dir_name << endl;

        DIR* dir = opendir(dir_name);
        if (!dir) {
            perror("Error while opening directory");
            close(sock);
            return 1;
        }

        struct dirent* entry;
        int total_bytes_sent = 0;

        // Sending directory name to the server
        int bytes_sent = send(sock, dir_name, strlen(dir_name) + 1, 0); // Includes the '\0'
        if (bytes_sent < 0) {
            perror("Error while sending directory name");
            closedir(dir);
            close(sock);
            return 1;
        }

        total_bytes_sent += bytes_sent;

        while ((entry = readdir(dir)) != nullptr) {
            // Ignoring special entries "." and ".."
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            // Building the full path of the file
            string full_path = string(dir_name) + "/" + entry->d_name;

            // Verifying if it is a regular file
            struct stat file_info;
            if (stat(full_path.c_str(), &file_info) == 0 && S_ISREG(file_info.st_mode)) {
                // Send the file name to the server
                int bytes_sent = send(sock, entry->d_name, strlen(entry->d_name) + 1, 0); // Includes the '\0'
                if (bytes_sent < 0) {
                    perror("Error while sending file name");
                    closedir(dir);
                    close(sock);
                    return 1;
                }

                total_bytes_sent += bytes_sent;
                cout << "Sent file name: " << entry->d_name << " (" << bytes_sent << " bytes)" << endl;
                cout << "Waiting for ACK from server..." << endl;

                // Receiving ACK from the server
                char ack_buffer[1024] = {};
                int ack_bytes_received = recv(sock, ack_buffer, sizeof(ack_buffer) - 1, 0);
                if (ack_bytes_received <= 0 || strcmp(ack_buffer, "ACK") != 0) {
                    cerr << "Error: Server did not respond with ACK!" << endl;
                    close(sock);
                    return 1;
                }
            }
        }

        closedir(dir);

        // Sending "bye" message to the server to indicate the end of the transmission
        const char* end_message = "bye";
        send(sock, end_message, strlen(end_message) + 1, 0);

        // Finalizing time measurement
        gettimeofday(&end_time, nullptr);

        // Calculating elapsed time
        double elapsed_time = (end_time.tv_sec - start_time.tv_sec) + 
                              (end_time.tv_usec - start_time.tv_usec) / 1e6;

        cout << "Transfer completed in " << elapsed_time << " seconds." << endl;
        cout << "Total bytes sent: " << total_bytes_sent << endl;
    
    } else {
        cout << "Server did not respond with READY ACK! Closing connection..." << endl;
    }

    close(sock);

    return 0;
}
