#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#define MAX_MESSAGE_LENGTH 127

// Define the number of server instances
#define NUM_SERVERS 2

// Define server addresses and ports
const char* SERVER_ADDRESSES[NUM_SERVERS] = {"10.35.70.17", "10.35.70.18"};
const int SERVER_PORTS[NUM_SERVERS] = {33333, 33334};

int sSockets[NUM_SERVERS];

void* handleClient(void* arg);
void* readServer(void* arg);

int main() {
    // Create sockets for each server instance
    for (int i = 0; i < NUM_SERVERS; i++) {
        sSockets[i] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sSockets[i] == -1) {
            perror("Failed to create socket");
            exit(-1);
        }
        printf("Socket %d created successfully\n", i);
    }

    // Bind each socket to its respective server address and port
    for (int i = 0; i < NUM_SERVERS; i++) {
        struct sockaddr_in addr = { 0 };
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESSES[i]);
        addr.sin_port = htons(SERVER_PORTS[i]);

        int r = bind(sSockets[i], (struct sockaddr*)&addr, sizeof addr);
        if (r == -1) {
            perror("Failed to bind socket");
            exit(-1);
        }
        printf("Socket %d bound to address %s:%d\n", i, SERVER_ADDRESSES[i], SERVER_PORTS[i]);
    }

    // Start listening for client connections on each socket
    for (int i = 0; i < NUM_SERVERS; i++) {
        if (listen(sSockets[i], 5) == -1) {
            perror("Failed to listen for connections");
            exit(-1);
        }
        printf("Socket %d listening for connections\n", i);
    }

    // Accept incoming client connections and divide them between the servers
    int i = 0;
    while (1) {
        struct sockaddr_in clientAddr = { 0 };
        socklen_t clientAddrLen = sizeof clientAddr;
        int cSocket = accept(sSockets[i], (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (cSocket == -1) {
            perror("Failed to accept client connection");
            continue;
        }
        printf("Accepted client connection on socket %d\n", i);

        // Create a new thread to handle the client connection
        pthread_t thread;
        int* args = (int*)malloc(sizeof(int));
        *args = cSocket;
        pthread_create(&thread, NULL, handleClient, args);

        // Move on to the next server instance
        i = (i + 1) % NUM_SERVERS;
    }
}
