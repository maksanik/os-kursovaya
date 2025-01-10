#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <time.h>

#define PORT 8081
#define BUFFER_SIZE 1024

// Инициализация сокетов Windows
#pragma comment(lib, "ws2_32.lib")

int get_thread_count()
{
    // Для Windows можно использовать Windows API для получения информации о потоках
    // Эта функция должна быть реализована через WinAPI или другой подход
    return 1; // Заглушка
}

int get_process_count() {
    // Для Windows можно использовать функции Windows API для получения количества процессов
    // Эта функция должна быть реализована через WinAPI
    return 1; // Заглушка
}

DWORD WINAPI handle_client(LPVOID arg)
{
    SOCKET new_socket = *((SOCKET *)arg);
    free(arg);  // Освобождаем память для аргумента

    // Цикл для отправки данных раз в 1 секунду
    while (1)
    {
        // Получаем количество потоков
        int thread_count = get_thread_count();

        // Получаем количество процессов
        int process_count = get_process_count();

        // Получаем текущее время
        time_t rawtime;
        struct tm *timeinfo;
        char time_buffer[80];

        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(time_buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);

        // Формируем строку ответа
        char response[BUFFER_SIZE];
        snprintf(response, BUFFER_SIZE,
                 "Threads: %d\nProcesses: %d\nTime: %s",
                 (thread_count >= 0 ? thread_count : 0),
                 (process_count >= 0 ? process_count : 0),
                 time_buffer);

        // Отправляем ответ клиенту
        send(new_socket, response, strlen(response), 0);

        printf("Server information sent:\n%s\n", response);

        // Задержка 3 секунды
        Sleep(3000);

        // Проверка на разрыв соединения
        char buffer[1];
        int bytes_received = recv(new_socket, buffer, 1, MSG_DONTWAIT);
        if (bytes_received == 0)
        {
            printf("Client disconnected\n");
            break;
        }
    }

    // Закрываем соединение с текущим клиентом
    closesocket(new_socket);
    printf("Connection closed\n");

    return 0;
}

int main()
{
    WSADATA wsaData;
    SOCKET server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Инициализация Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        perror("WSAStartup failed");
        return 1;
    }

    // Создаем сокет
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        perror("Socket failed");
        WSACleanup();
        return 1;
    }

    // Настроиваем параметры сокета
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Привязываем сокет к порту
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR)
    {
        perror("Bind failed");
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    // Слушаем входящие соединения
    if (listen(server_fd, 3) == SOCKET_ERROR)
    {
        perror("Listen failed");
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    printf("Server is listening on port %d\n", PORT);

    // Основной цикл обработки подключений
    while (1)
    {
        printf("Waiting for a new connection...\n");

        // Принимаем новое подключение
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) == INVALID_SOCKET)
        {
            perror("Accept failed");
            closesocket(server_fd);
            WSACleanup();
            return 1;
        }

        printf("New connection accepted\n");

        // Создаем новый поток для обработки клиента
        HANDLE thread_id;
        SOCKET *new_sock =
23:39
malloc(sizeof(SOCKET)); // Выделяем память для сокета
        *new_sock = new_socket;

        thread_id = CreateThread(NULL, 0, handle_client, (LPVOID)new_sock, 0, NULL);
        if (thread_id == NULL)
        {
            perror("Failed to create thread");
            closesocket(new_socket);
        }
    }

    closesocket(server_fd);
    WSACleanup();
    return 0;
}