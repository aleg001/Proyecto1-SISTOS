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
#include "chat.pb-c.h"

void handle_newuser(){

}


int main(int argc, char **argv) {

    if(argc > 1){
        // descriptor del socket del servidor y cliente, puerto y la cantidad de bytes leidos
        int server_fd, cliente_fd, puerto, valread;

        // se obtiene el puerto de los argumentos
        puerto = atoi(argv[1]);

        // estructura del servidor y del cliente
        struct sockaddr_in server;
        struct sockaddr_in client;

        // size de la estructura
        int addrlenserver = sizeof(server);
        int addrlenclient = sizeof(client);

        // buffer_rx para almacenar datos
        char buffer_rx[1024];

        // buffer para mandar datos
        char buffer_tx[1024] = "Soy el servidor";

        // Creacion del socket del servidor
        // AF_INET para IPv4 y SOCK_STREAM para socket tipo TCP
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == 0) {
            perror("[SERVER-ERROR]: Creacion de socket fallida\n");
            exit(EXIT_FAILURE);
        } else {
            printf("[SERVER]: Socket creado exitosamente\n");
        }

        memset(&server, 0, sizeof(server));

        // Se inicializa la estructura con la direccion y puerto donde se escucharan solicitudes
        server.sin_family = AF_INET;
        server.sin_port = htons(puerto);
        server.sin_addr.s_addr = INADDR_ANY;

        // Con bind se asocia la direccion y el puerto al socket del servidor
        if (bind(server_fd, (struct sockaddr *)&server, sizeof(server))<0) {
            perror("[SERVER-ERROR]: Socket bind fallido\n");
            exit(EXIT_FAILURE);
        } else {
            printf("[SERVER]: Socket asociado exitosamente\n");
        }

        // listen espera las conexiones
        // primer argumento es el socket del servidor
        // segundo argumento la capacidad de cola
        if (listen(server_fd, 10) != 0) {
            perror("[SERVER-ERROR]: Socket listen fallido");
            exit(EXIT_FAILURE);    
        } else {
            printf("[SERVER]: Escuchando en el puerto %d\n", puerto);
        }

        while (1) {

            cliente_fd = accept(server_fd, (struct sockaddr *)&client, (socklen_t*)&addrlenclient);
            if (cliente_fd < 0) {
                perror("[SERVER-ERROR]: Conexion no aceptada\n");
                exit(EXIT_FAILURE);
            } else {
                char* client_ip = inet_ntoa(client.sin_addr);
                while (1) {
                    valread = read(cliente_fd, buffer_rx, sizeof(buffer_rx));

                    if(valread == -1){
                        fprintf(stderr, "[SERVER-ERROR]: FD del cliente no pudo leerse\n");
                    } else if(valread == 0) {
                        printf("[SERVER]: Socket del cliente cerrado\n\n");
                        close(cliente_fd);
                        break;
                    } else {
                        write(cliente_fd, buffer_tx, strlen(buffer_tx));
                        printf("[SERVER]: %s conectado desde la ip: %s\n", buffer_rx, inet_ntoa(client.sin_addr));
                    }
                }
            }
        }

        return 0;
    } else {
        printf("Puerto no especificado. Comando: <Nombre Programa> <Puerto>\n");
    }

    
}
