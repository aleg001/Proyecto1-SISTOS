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

#define MAX_CLIENTS 50

// Estructura basada en el .proto
typedef struct {
    char username[75];
    char user_ip[20];
    int status;
    int sockfd;
} Client;

Client clients[MAX_CLIENTS]; // Lista de clientes
int cantidad_clientes = 0;

void* handle_newclient(void *arg){

    // Se obtiene el cliente y su informacion
    int client_socket = *(int*)arg;
    uint8_t buffer[1024];
    ssize_t bytes_received = recv(client_socket, buffer, 1024, 0);

    // Se verifica si existe informacion
    if (bytes_received < 0){
        perror("[SERVER-ERROR]: Recepcion de cliente fallida\n");
        exit(EXIT_FAILURE);
    } else {
        // Se crea un nuevo usuario en el server para manejar mejor la informacion
        Chat__User* new_client = chat__new_user__unpack(NULL, bytes_received, buffer); 

        if(new_client == NULL){
            perror("[SERVER-ERROR]: Registro de usuario fallido\n");
            exit(EXIT_FAILURE);
        }

        // Se imprime informacion y se libera memoria
        printf("Bienvenido: %s de ip: %s\n", new_client->user_name, new_client->user_ip);

        // Respuesta del servidor
        

        // Se agrega el nuevo cliente a la lista
        Client client;
        strcpy(client.username, new_client->user_name);
        strcpy(client.user_ip, new_client->user_ip);
        client.status = 1;
        client.sockfd = client_socket;

        clients[cantidad_clientes] = client;
        cantidad_clientes++;

        printf("Lista de clientes conectados:\n");
        for (int i = 0; i < cantidad_clientes; i++) {
            printf("- %s (%s)\n", clients[i].username, clients[i].user_ip);
        }
        chat__new_user__free_unpacked(new_client, NULL);
    }
}


int main(int argc, char **argv) {

    if(argc > 1){
        // descriptor del socket del servidor y cliente, puerto y la cantidad de bytes leidos
        int server_fd, client_fd, puerto, valread;

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
        if (listen(server_fd, MAX_CLIENTS) != 0) {
            perror("[SERVER-ERROR]: Socket listen fallido");
            exit(EXIT_FAILURE);    
        } else {
            printf("[SERVER]: Escuchando en el puerto %d\n", puerto);
        }

        while (1) {
            client_fd = accept(server_fd, (struct sockaddr *)&client, (socklen_t*)&addrlenclient);
            if (client_fd < 0) {
                perror("[SERVER-ERROR]: Conexion no aceptada\n");
                exit(EXIT_FAILURE);
            }

            // printf("[SERVER]: %s conectado desde la ip: %s y puerto: %d\n", buffer_rx, inet_ntoa(client.sin_addr), ntohs(client.sin_port));
       
            pthread_t thread;
            int pthread_result = pthread_create(&thread, NULL, handle_newclient, (void *)&client_fd);
            if(pthread_result < 0){
                perror("[SERVER-ERROR]: Creacion de hilo para cliente fallida\n");
                exit(EXIT_FAILURE);
            }
        
        }

        return 0;

    } else {
        printf("Puerto no especificado. Comando: <Nombre Programa> <Puerto>\n");
    }

    
}
