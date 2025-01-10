#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024

#pragma comment(lib, "ws2_32.lib")

int get_receive_window_size(SOCKET socket_fd) {
    int window_size;
    int optlen = sizeof(window_size);

    if (getsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF, (char *)&window_size, &optlen) == SOCKET_ERROR) {
        perror("getsockopt");
        return -1;
    }

    // Значение SO_RCVBUF включает удвоенную буферизацию в ядре, поэтому делим на 2
    return window_size / 2;
}

void *handle_client(void *arg) {
    SOCKET client_socket = *(SOCKET *)arg;
    free(arg);

    while (1) {
        time_t rawtime;
        struct tm *timeinfo;
        char time_buffer[80];

        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

        int window_size = get_receive_window_size(client_socket);

        char response[BUFFER_SIZE];
        snprintf(response, BUFFER_SIZE,
                 "Server window size: %d\nTime: %s",
                 window_size, time_buffer);

        if (send(client_socket, response, strlen(response), 0) == SOCKET_ERROR) {
            perror("send failed");
            break;
        }
        
        printf("Server information sent:\n%s\n", response);

        Sleep(3000); // задержка 3 секунды

        char buffer[1];
        int bytes_received = recv(client_socket, buffer, 1, MSG_DONTWAIT);
        if (bytes_received == 0) {
            printf("Client disconnected\n");
            break;
        }
    }

    closesocket(client_socket);
    printf("Connection closed\n");

    return NULL;
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup failed");
        return 1;
    }

    SOCKET server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        perror("Socket failed");
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt)) == SOCKET_ERROR) {
        perror("setsockopt failed");
        closesocket(server_fd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        perror("Bind failed");
        closesocket(server_fd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) == SOCKET_ERROR) {
        perror("Listen failed");
        closesocket(server_fd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", PORT);

    while (1) {
        printf("Waiting for a new connection...\n");

        SOCKET new_socket = accept(server_fd, NULL, NULL);
        if (new_socket == INVALID_SOCKET) {
            perror("Accept failed");
            continue;
        }

        printf("New connection accepted\n");

        HANDLE thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)handle_client, (LPVOID)&new_socket, 0, NULL);
        if (thread == NULL) {
            perror("Thread creation failed");
            closesocket(new_socket);
        } else {
            CloseHandle(thread);
        }
    }

    closesocket(server_fd);
    WSACleanup();
    return 0;
}