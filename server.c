//
// Created by sallich on 27.06.2023.
//

#include "server.h"

#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>

#define MAX_MAC_ADDRESSES 100

// Структура для хранения MAC-адресов
typedef struct {
    unsigned char mac[6];
} MacAddress;

// Функция для сохранения MAC-адресов в файл
void saveMacAddresses(const MacAddress* macAddresses, int count) {
    FILE* file = fopen("mac_addresses.txt", "wb");
    if (file != NULL) {
        fwrite(macAddresses, sizeof(MacAddress), count, file);
        fclose(file);
    }
}

// Функция для загрузки MAC-адресов из файла
int loadMacAddresses(MacAddress* macAddresses) {
    FILE* file = fopen("mac_addresses.txt", "rb");
    if (file != NULL) {
        int count = fread(macAddresses, sizeof(MacAddress), MAX_MAC_ADDRESSES, file);
        fclose(file);
        return count;
    }
    return 0;
}

void getCurrentTime(char* currentTime) {
    time_t rawTime;
    struct tm* timeInfo;
    time(&rawTime);
    timeInfo = localtime(&rawTime);
    strftime(currentTime, 20, "%d.%m.%y %H:%M:%S", timeInfo);
}

int main(int argc, char* argv[]) {
    int isDaemon = 0;
    int logToFile = 0;
    char* logFilePath = NULL;

    int port = 0;
    int opt;
    while ((opt = getopt(argc, argv, "p:dl:")) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'd':
                isDaemon = 1;
                break;
            case 'l':
                logToFile = 1;
                logFilePath = optarg;
                break;
            default:
                printf("Usage: %s -p <port> [-d] [-l <log_file>]\n", argv[0]);
                return 1;
        }
    }

    // Запуск в режиме демона
    if (isDaemon) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("Forking failed");
            return 1;
        } else if (pid > 0) {
            // Родительский процесс - завершаем его
            return 0;
        }
        // Дочерний процесс продолжает выполнение
        setsid();
    }

    // Запись логов в файл, если указан путь к файлу
    int logFile = STDOUT_FILENO; // Используем стандартный вывод по умолчанию
    if (logToFile) {
        logFile = open(logFilePath, O_CREAT | O_WRONLY | O_APPEND, 0644);
        if (logFile == -1) {
            perror("Opening log file failed");
            return 1;
        }
    }

    char currentTime[20];
    getCurrentTime(currentTime);
    dprintf(logFile, "[%s] Server started\n", currentTime);

    if (argc < 3) {
        dprintf(logFile, "Usage: %s -p <port> [-d] [-l <log_file>]\n", argv[0]);
        return 1;
    }

    // Загрузка сохраненных MAC-адресов из файла
    MacAddress macAddresses[MAX_MAC_ADDRESSES];
    int macCount = loadMacAddresses(macAddresses);

    // Создание TCP-сокета
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Socket creation failed");
        dprintf(logFile, "Socket creation failed\n");
        return 1;
    }

    // Настройка адреса сервера
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Привязка сокета к адресу и порту
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("Binding failed");
        dprintf(logFile, "Binding failed\n");
        return 1;
    }

    // Ожидание подключений
    if (listen(serverSocket, 1) == -1) {
        perror("Listening failed");
        dprintf(logFile, "Listening failed\n");
        return 1;
    }

    getCurrentTime(currentTime);
    dprintf(logFile, "[%s] Server listening on port %d\n", currentTime, port);

    while (1) {
        // Принятие подключения
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        if (clientSocket == -1) {
            perror("Accepting connection failed");
            dprintf(logFile, "[%s] Accepting connection failed\n", currentTime);
            continue;
        }

        getCurrentTime(currentTime);
        dprintf(logFile, "[%s] New request received\n", currentTime);

        // Получение запроса от клиента
        char request[7];
        if (read(clientSocket, request, sizeof(request)) != sizeof(request)) {
            perror("Receiving request failed");
            dprintf(logFile, "[%s] Receiving request failed\n", currentTime);
            close(clientSocket);
            continue;
        }

        // Обработка запроса
        unsigned char* mac = (unsigned char*)request;
        int requestType = request[6];

        int response = 0; // Значение ответа по умолчанию

        if (requestType == 0) {
            // Запрос на добавление MAC-адреса
            if (macCount >= MAX_MAC_ADDRESSES) {
                response = -3; // Ошибка: превышено максимальное количество адресов
            } else {
                memcpy(macAddresses[macCount].mac, mac, 6);
                macCount++;
                saveMacAddresses(macAddresses, macCount);
            }
        } else if (requestType == 1) {
            // Запрос на удаление MAC-адреса
            int foundIndex = -1;
            for (int i = 0; i < macCount; i++) {
                if (memcmp(macAddresses[i].mac, mac, 6) == 0) {
                    foundIndex = i;
                    break;
                }
            }

            if (foundIndex != -1) {
                // Найден MAC-адрес, удаляем его
                memmove(&macAddresses[foundIndex], &macAddresses[foundIndex + 1], (macCount - foundIndex - 1) * sizeof(MacAddress));
                macCount--;
                saveMacAddresses(macAddresses, macCount);
            } else {
                response = -4; // Ошибка: указанный MAC-адрес не найден
            }
        } else if (requestType == 2) {
            // Запрос на проверку наличия MAC-адреса
            int found = 0;
            for (int i = 0; i < macCount; i++) {
                if (memcmp(macAddresses[i].mac, mac, 6) == 0) {
                    found = 1;
                    break;
                }
            }

            if (found) {
                response = 1; // MAC-адрес найден
            } else {
                response = 0; // MAC-адрес не найден
            }
        } else {
            response = -5; // Ошибка: неизвестный тип запроса
        }

        // Отправка ответа клиенту
        if (write(clientSocket, &response, sizeof(response)) != sizeof(response)) {
            perror("Sending response failed");
            dprintf(logFile, "[%s] Sending response failed\n", currentTime);
            close(clientSocket);
            continue;
        }

        // Закрытие клиентского сокета
        close(clientSocket);

        getCurrentTime(currentTime);
        dprintf(logFile, "[%s] Request processed\n", currentTime);
    }

    // Закрытие серверного сокета
    close(serverSocket);

    if (logToFile) {
        close(logFile);
    }

    return 0;
}
