#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#define MaxClientNum 100
#define MaxThreadNum 10

int count;
int cSocket[MaxClientNum];
pthread_t threads[MaxThreadNum];
int thread_count = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* communicate(void* a);

int main() {
    count = 0;
    int sSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sSocket == -1) {
        perror("Failed to create socket");
        return -2;
    }
    printf("Successfully created socket\n");

    struct sockaddr_in addr = { 0 };
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("10.35.70.17");
    addr.sin_port = htons(33333);
    int r = bind(sSocket, (struct sockaddr*)&addr, sizeof addr);
    if (r == -1) {
        perror("Failed to bind socket");
        close(sSocket);
        return -2;
    }
    printf("Bind Socket Successfully!\n");

    r = listen(sSocket, SOMAXCONN);
    if (r == -1) {
        perror("Failed to listen socket");
        close(sSocket);
        return -2;
    }
    printf("Successfully listen to socket\n");

    while (1) {
        int c = accept(sSocket, NULL, NULL);
        if (c == -1) {
            perror("Server crushed");
            close(sSocket);
            return -3;
        }
        char password[100];
        printf("Please enter the password to connect to the server: ");
        scanf("%s", password);
        if (strcmp(password, "mypassword") != 0) {
            printf("Incorrect password, disconnecting client.\n");
            close(c);
            continue;
        }
        printf("Client number %d has successfully connected to server\n", count + 1);
        cSocket[count++] = c;

        pthread_mutex_lock(&mutex);
        if (thread_count < MaxThreadNum) {
            pthread_create(&threads[thread_count++], NULL, communicate, (void*)(intptr_t)(count-1));
        } else {
            pthread_mutex_unlock(&mutex);
            communicate((void*)(intptr_t)(count-1));
        }
        pthread_mutex_unlock(&mutex);
    }

    return 0;
}

void* communicate(void* a) {
    int index = (int)(intptr_t)a;
    char buff[300];
    int r;
    char temp[300];

    while (1) {
        r = recv(cSocket[index], buff, 299, 0);
        if (r > 0) {
            buff[r] = 0;
            printf(">> Client %d: %s \n", index + 1, buff);

            memset(temp, 0, 300);
            sprintf(temp, "Client %d: %s", index + 1, buff);

            pthread_mutex_lock(&mutex);
            for (int i = 0; i < count; i++) {
                send(cSocket[i], temp, strlen(temp), 0);
            }
            pthread_mutex_unlock(&mutex);
        }
    }
    return 0;
}
