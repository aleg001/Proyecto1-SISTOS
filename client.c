#include <stdio.h> //Prints
#include <stdlib.h> //Gestion memoria
#include <string.h> //Manipulacion strings
#include <errno.h> //Errores
#include <unistd.h> //Funciones de sistema
#include <sys/types.h> //Tipos de datos
#include <sys/socket.h> //Sockets de red
#include <netinet/in.h> //Direcciones de red
#include <pthread.h> //Hilos
#include <arpa/inet.h> // Manipulacion direcciones IP
#include "chat.pb-c.h" // Protobuf

//ip vm: 10.0.2.15

int main(int argc, char **argv) {

    char *username, *IPserver;
    int puerto;

    username = argv[1];
    IPserver = argv[2];
    puerto = atoi(argv[3]);

    if(argc > 1){
        int sockfd, valread;
        struct sockaddr_in serv_addr;
        char buffer_tx[1024];
        char buffer_rx[1024];

        // Socket del cliente
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("[CLIENT-ERROR]: Creacion de socket fallida\n");
            exit(EXIT_FAILURE);
        } else {
            printf("[CLIENT]: Socket creado exitosamente\n");
        }

        memset(&serv_addr, 0, sizeof(serv_addr));

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(IPserver);
        serv_addr.sin_port = htons(puerto);

        if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0) {
            perror("[CLIENT-ERROR]: Conexion al servidor fallida\n");
            exit(EXIT_FAILURE);
        } else {
            printf("[CLIENT]: Conectado al servidor exitosamente\n");
        }

        // Obtener la dirección IP del cliente
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        if (getsockname(sockfd, (struct sockaddr *)&addr, &len) == -1) {
            perror("[CLIENT-ERROR]: Obtención de dirección IP del cliente fallida\n");
            exit(EXIT_FAILURE);
        }

        sprintf(buffer_tx, "El usuario %s se conecto", username);
        
        Chat__NewUser new_client = CHAT__NEW_USER__INIT;
        new_client.username = username;
        new_client.ip = inet_ntoa(addr.sin_addr);

        size_t package_size = chat__new_user__get_packed_size(&new_client);
        uint8_t *buffer = malloc(package_size);
        chat__new_user__pack(&new_client, buffer);

        if(send(sockfd, buffer, package_size, 0) < 0){
            perror("[CLIENT-ERROR]: Envio de mensaje fallido\n");
            exit(EXIT_FAILURE);
        }

        char input[100];
        while (1) {
            printf("Escribe un mensaje: ");
            fgets(input, sizeof(input), stdin);
            // Eliminamos el caracter de nueva línea del final del string
            input[strcspn(input, "\n")] = 0;

            if (strcmp(input, "exit") == 0) {
                break;
            }
        }

        free(buffer);
        close(sockfd);

        return 0;

    } else {
        printf("Faltan argumentos. Comando: <Nombre Programa> <Nombre Usuario> <IP Server> <Puerto>\n");
    }
}
