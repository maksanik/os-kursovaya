#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdint.h>

#define BUFFER_SIZE 1024

static void cleanup_handler(void *arg) {
    int sock = (int)(intptr_t)arg;
    close(sock);
    printf("Закрытие сокета\n");
}

// Общая функция для работы с сервером
static void receive_updates_from_server(const char *server_ip, uint16_t port, void (*update_callback)(const char *)) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    // Создаём сокет
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Преобразуем IP-адрес из текста в бинарный вид
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(sock);
        return;
    }

    // Подключаемся к серверу
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return;
    }

    printf("Connected to server at %s:%d\n", server_ip, port);

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    pthread_cleanup_push(cleanup_handler, (void *)(intptr_t)sock); // Передаём sock в cleanup

    // Получаем данные в цикле
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = read(sock, buffer, BUFFER_SIZE);
        
        // Точка отмены потока
        pthread_testcancel();
        
        if (bytes_received > 0) {
            char res[BUFFER_SIZE + 50];
            sprintf(res, "Update from server %s:%d \n-----------\n", server_ip, port);
            strcat(res, buffer);
            strcat(res, "\n-----------\n");
            printf("Update from server %s:%d\n-----------\n%s\n-----------\n", server_ip, port, buffer);

            // Вызов внешнего модуля для обновления данных
            if (update_callback != NULL) {
                update_callback(res);
            }
        } else if (bytes_received == 0) {
            printf("Connection closed by server\n");
            break;
        } else {
            perror("Error receiving data");
            break;
        }
    }

    pthread_cleanup_pop(1);  // Вызов очистки перед завершением потока
    close(sock);
    printf("Disconnected from server\n");
}

// Функция для работы с сервером 1
void server1_getinfo(const char *server_ip, void (*update_callback)(const char *)) {
    receive_updates_from_server(server_ip, 8080, update_callback);
}

// Функция для работы с сервером 2
void server2_getinfo(const char *server_ip, void (*update_callback)(const char *)) {
    receive_updates_from_server(server_ip, 8081, update_callback);
}