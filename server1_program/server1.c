#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>

#include <X11/Xlib.h>

#define PORT 8080
#define BUFFER_SIZE 1024


int get_receive_window_size(int socket_fd) {
    int window_size;
    socklen_t optlen = sizeof(window_size);

    if (getsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF, &window_size, &optlen) == -1) {
        perror("getsockopt");
        return -1;
    }

    // Значение SO_RCVBUF включает удвоенную буферизацию в ядре, поэтому делим на 2
    return window_size / 2;
}

int get_monitor_count() {
    Display *display = XOpenDisplay(NULL);
    if (!display) {
        perror("XOpenDisplay");
        return -1;
    }

    int screen_count = ScreenCount(display);
    XCloseDisplay(display);
    return screen_count;
}

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);

    while (1) {
        time_t rawtime;
        struct tm *timeinfo;
        char time_buffer[80];

        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

        int window_size = get_receive_window_size(client_socket);
        int monitor_count = get_monitor_count();

        char response[BUFFER_SIZE];
        snprintf(response, BUFFER_SIZE,
                 "Server window size: %d\nMonitors: %d\nTime: %s",
                 window_size, (monitor_count >= 0 ? monitor_count : 0), time_buffer);

        if (send(client_socket, response, strlen(response), 0) == -1) {
            perror("send failed");
            break;
        }
        
        printf("Server information sent:\n%s\n", response);

        sleep(3);

        char buffer[1];
        int bytes_received = recv(client_socket, buffer, 1, MSG_DONTWAIT);
        if (bytes_received == 0) {
            printf("Client disconnected\n");
            break;
        }
    }

    close(client_socket);
    printf("Connection closed\n");

    return NULL;
}


int main()
{
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0)
    {
        perror("Listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", PORT);

    while (1)
    {
        printf("Waiting for a new connection...\n");

        int *new_socket = malloc(sizeof(int));
        if (!new_socket)
        {
            perror("Malloc failed");
            continue;
        }

        *new_socket = accept(server_fd, NULL, NULL);
        if (*new_socket < 0)
        {
            perror("Accept");
            free(new_socket);
            continue;
        }

        printf("New connection accepted\n");

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, new_socket) != 0)
        {
            perror("Thread creation failed");
            free(new_socket);
        }
        else
        {
            pthread_detach(thread_id);
        }
    }

    close(server_fd);
    return 0;
}