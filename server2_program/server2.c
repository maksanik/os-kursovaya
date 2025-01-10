#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>

#ifdef __linux__
#include <X11/Xlib.h>
#endif

#define PORT 8081
#define BUFFER_SIZE 1024

int get_thread_count()
{
    FILE *file = fopen("/proc/self/status", "r");
    if (!file)
    {
        perror("Failed to open /proc/self/status");
        return -1;
    }

    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "Threads:", 8) == 0)
        {
            fclose(file);
            return atoi(line + 8);
        }
    }

    fclose(file);
    return -1;
}

int get_process_count() {
    FILE *fp;
    char buffer[128];
    int count = 0;

    // Открываем команду "ps -e" для получения списка процессов
    fp = popen("ps -e | wc -l", "r");
    if (fp == NULL) {
        perror("Ошибка при выполнении команды");
        return -1;
    }

    // Читаем количество процессов из вывода команды
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        count = atoi(buffer) - 1; // Вычитаем 1, так как wc -l также считает заголовок
    }

    // Закрываем файл
    fclose(fp);
    return count;
}

void *handle_client(void *arg)
{
    int new_socket = *((int *)arg);
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
        ssize_t bytes_sent = send(new_socket, response, strlen(response), 0);
        
        printf("Server information sent:\n%s\n", response);

        // Задержка 3 секунды
        sleep(3);

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
    close(new_socket);
    printf("Connection closed\n");

    return NULL;
}

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Создаем сокет
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Настроиваем параметры сокета
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Привязываем сокет к порту
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Слушаем входящие соединения
    if (listen(server_fd, 3) < 0)
    {
        perror("Listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", PORT);

    // Основной цикл обработки подключений
    while (1)
    {
        printf("Waiting for a new connection...\n");

        // Принимаем новое подключение
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("Accept");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        printf("New connection accepted\n");

        // Создаем новый поток для обработки клиента
        pthread_t thread_id;
        int *new_sock = malloc(sizeof(int)); // Выделяем память для сокета
        *new_sock = new_socket;

        if (pthread_create(&thread_id, NULL, handle_client, (void *)new_sock) != 0)
        {
            perror("Failed to create thread");
            close(new_socket);
        }
        else
        {
            pthread_detach(thread_id); // Отсоединяем поток, чтобы не ждать его завершения
        }
    }

    close(server_fd);
    return 0;
}