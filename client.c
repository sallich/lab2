//
// Created by sallich on 27.06.2023.
//

#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char* argv[]) {


    // Получение адреса и порта из аргументов командной строки
    char buffer[PATH_MAX];
    char *address = "0.0.0.0";
    int port = 8080;
    int opt;
    while ((opt = getopt(argc, argv, "l:a:p:vh")) != -1) {
        switch (opt) {
            case 'v':
                printf("\nVersion 1\n"
                       "Usmanova Regina\n"
                       "Group N32481\n"
                       "Var 3\n\n");
                break;
            case 'h':
                printf("\nUsage: ./server [-a address] [-p port]\n"
                       "\nOption -v\n"
                       "Displays program version and information, "
                       "including executor's full name, group number, and lab variant number.\n\n"
                       "Option -a   set ip address what server listening to\n"
                       "Option -p   set port address what server listening to\n"
                       "Option -h:\nDisplays this message.\n\n");
                break;
            case 'a':
                // Обработка опции -a (адрес)
                // optarg содержит переданный адрес
                address = buffer;
                strcpy(address, optarg);
                break;
            case 'p':
                port = atoi(optarg);
                // Обработка опции -p (порт)
                // optarg содержит переданный порт
                break;
            default:
                fprintf(stderr, "Usage: %s [-a address] [-p port]\n", argv[0]);
                exit(EXIT_FAILURE);
        }

        // Создание TCP-сокета
        int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == -1) {
            perror("Socket creation failed");
            return 1;
        }

        // Настройка адреса сервера
        struct sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port);
        if (inet_pton(AF_INET, address, &(serverAddress.sin_addr)) <= 0) {
            perror("Invalid address");
            return 1;
        }

        // Подключение к серверу
        if (connect(clientSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == -1) {
            perror("Connection failed");
            return 1;
        }

        printf("Connected to server.\n");

        while (1) {
            // Ввод MAC-адреса с клавиатуры
            char mac[6];
            printf("Enter MAC address (format: XX:XX:XX:XX:XX:XX): ");
            if (scanf("%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx%*c", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) !=
                6) {
                printf("Invalid MAC address format.\n");
                close(clientSocket);
                return 1;
            }

            // Ввод типа запроса с клавиатуры
            int requestType;
            printf("Enter request type (0 - Add, 1 - Remove, 2 - Check): ");
            if (scanf("%d", &requestType) != 1) {
                printf("Invalid request type.\n");
                close(clientSocket);
                return 1;
            }

            // Формирование запроса
            char request[7];
            memcpy(request, mac, 6);
            request[6] = (char) requestType;

            // Отправка запроса серверу
            if (write(clientSocket, request, sizeof(request)) != sizeof(request)) {
                perror("Sending request failed");
                close(clientSocket);
                return 1;
            }


            // Получение ответа от сервера
            char response;
            if (read(clientSocket, &response, sizeof(response)) != sizeof(response)) {
                perror("Receiving response failed");
                close(clientSocket);
                return 1;
            }

            // Обработка ответа
            if (response == 0) {
                printf("MAC address operation completed successfully.\n");
            } else if (response == -3) {
                printf("Error: Maximum number of addresses exceeded.\n");
            } else {
                printf("Error: Unknown response from server.\n");
            }

            printf("You want exit?[y/n]\n");
            char exit;
            scanf("%c", &exit);
            if (exit == 'y') {
                break;
            }
        }

        // Закрытие клиентского сокета
        close(clientSocket);

        return 0;
    }
}