#include <stdio.h> //Prints
#include <stdlib.h> //Gestion memoria
#include <string.h> //Manipulacion strings
#include <errno.h> //Errores
#include <unistd.h> //Funciones de sistema
#include <sys/types.h> //Tipos de datos
#include <sys/socket.h> //Sockets de red
#include <netinet/in.h> //Direcciones de red
#include <pthread.h> //Hilos

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

        // Creating socket file descriptor
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

        sprintf(buffer_tx, "El usuario %s se conecto", username);
        write(sockfd, buffer_tx, sizeof(buffer_tx));
        read(sockfd, buffer_rx, sizeof(buffer_rx));
        printf("[CLIENT]: %s \n", buffer_rx);

        while (1) {
            printf("Ingrese un mensaje: ");
            fgets(buffer_tx, 1024, stdin);

            // Verificar si el usuario quiere salir
            if (strncmp(buffer_tx, "exit", 4) == 0) {
                printf("[CLIENT]: Saliendo...\n");
                break;
            }

            // Enviar mensaje al servidor
            write(sockfd, buffer_tx, sizeof(buffer_tx));
            read(sockfd, buffer_rx, sizeof(buffer_rx));
            printf("[SERVER]: %s \n", buffer_rx);
        }

        close(sockfd);

        return 0;

    } else {
        printf("Faltan argumentos. Comando: <Nombre Programa> <Nombre Usuario> <IP Server> <Puerto>\n");
    }
}
