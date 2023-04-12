#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
//link to system socket library

#define MAX_MESSAGE_LENGTH 127
//define the maxium message length
int sSocket;
//int a varible sSocket
void* readServer(void* arg);
void* sendToServer(void* arg);
//function prototype in C++ in front of the main function

int main() {
    sSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //AF_INET: IPv4 address family 
 	//SOCK_STREAM: for TCP
 	//IPPROTO_TCP: TCP
    if (sSocket == -1) {
        perror("Failed to create socket");
        //error message
        return -1;
    }
    printf("Socket created successfully\n");
    //print the successful message
    struct sockaddr_in addr = { 0 };
    addr.sin_family = AF_INET;
    //AF_INET: IPv4 address family
    addr.sin_addr.s_addr = inet_addr("10.35.70.17");
    //Raspberry PI server address
    addr.sin_port = htons(33333);
    //Raspberry PI server port number
    int r = connect(sSocket, (struct sockaddr*)&addr, sizeof addr);
    //connect to the server
    if (r == -1) {
        perror("Failed to connect to server");
        //error message
        return -1;
    }
    printf("Connected to server successfully\n");
    //print the successful message
    pthread_t readThread, sendThread;
    //define two threads which are readThread and sendThread
    pthread_create(&readThread, NULL, readServer, NULL);
    //create threads to read meessages from the server
    pthread_create(&sendThread, NULL, sendToServer, NULL);
    //create threads to send meessages to the server

    pthread_join(readThread, NULL);
    //wait read thread ends
    pthread_join(sendThread, NULL);
    //wait send thread ends

    close(sSocket);
    //close socket
    return 0;
}

void* readServer(void* arg) {
//read server function
    char message[128];
    //char the message length to 128 bytes
    while (1) {
        int r = recv(sSocket, message, MAX_MESSAGE_LENGTH, 0);
        //receive the messages from the server
        if (r > 0) {
            message[r] = '\0';
            printf("%s\n", message);
            //print the messages in the terminal
        }
        else {
            perror("Failed to receive message from server");
            break;
        }
    }
    return NULL;
}

void* sendToServer(void* arg) {
    char message[128];
    //char the message length to 128 bytes
    while (1) {
        fgets(message, 128, stdin);
        //scan the messages from the keypad inputs
        int len = strlen(message);
        //len function to read the length of the message
        if (len > 0) {
            if (message[len - 1] == '\n') {
                message[len - 1] = '\0';
                //if the last message is \n, then message ends
            }
            if (send(sSocket, message, strlen(message), MSG_NOSIGNAL) == -1) {
                perror("Failed to send message to server");
                //otherwise, fail to send messages
                break;
            }
        }
    }
    return NULL;

}
