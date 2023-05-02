#include <stdio.h>      //Prints
#include <stdlib.h>     //Gestion memoria
#include <string.h>     //Manipulacion strings
#include <errno.h>      //Errores
#include <unistd.h>     //Funciones de sistema
#include <sys/types.h>  //Tipos de datos
#include <sys/socket.h> //Sockets de red
#include <netinet/in.h> //Direcciones de red
#include <pthread.h>    //Hilos
#include <arpa/inet.h>  // Manipulacion direcciones IP
#include "chat.pb-c.h"  // Protobuf
#pragma GCC ignored - Wall

#define MAX_CLIENTS 50

// Estructura basada en el .proto
typedef struct
{
    char username[75];
    char user_ip[20];
    int status;
    int sockfd;
} Client;

Client clients[MAX_CLIENTS]; // Lista de clientes
int cantidad_clientes = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void listado_clientes()
{
    printf("Lista de clientes conectados:\n");
    for (int i = 0; i < cantidad_clientes; i++)
    {
        char status[25] = "Estado desconocido";
        if (clients[i].status == 1)
        {
            strcpy(status, "En linea");
        }
        else if (clients[i].status == 2)
        {
            strcpy(status, "Ocupado");
        }
        else if (clients[i].status == 3)
        {
            strcpy(status, "Desconectado");
        }
        printf("- %s (%s) -> status: (%s)\n", clients[i].username, clients[i].user_ip, status);
    }
}

char *getStatusString(int status)
{
    char *result;

    switch (status)
    {
    case 1:
        result = "En linea";
        break;
    case 2:
        result = "Ocupado";
        break;
    case 3:
        result = "Desconectado";
        break;
    default:
        result = "Estado desconocido";
        break;
    }

    return result;
}

ChatSistOS__User *find_user(char *username)
{
    for (int i = 0; i < cantidad_clientes; i++)
    {
        if (strcmp(clients[i].username, username) == 0)
        {
            ChatSistOS__User *user_find_response = malloc(sizeof(ChatSistOS__User));
            chat_sist_os__user__init(user_find_response);
            user_find_response->user_name = clients[i].username;
            user_find_response->user_ip = clients[i].user_ip;
            user_find_response->user_state = clients[i].status;

            return user_find_response;
        }
    }

    return NULL;
}

void update_user_status(char *username, char *ip, Client cliente)
{
    for (int i = 0; i < cantidad_clientes; i++)
    {
        if (strcmp(clients[i].username, username) == 0 && strcmp(clients[i].user_ip, ip) == 0)
        {
            clients[i] = cliente;
        }
    }
}

int search_user(char *username, char *ip)
{
    for (int i = 0; i < cantidad_clientes; i++)
    {
        if (strcmp(clients[i].username, username) == 0 && strcmp(clients[i].user_ip, ip) == 0)
        {
            return i; // Si el usuario existe, se devuelve la posicion
        }
    }
    return -1;
}

int remove_users(int index)
{
    if (index < 0 || index >= cantidad_clientes)
    {
        return -1;
    }

    for (int i = index; i < cantidad_clientes - 1; i++)
    {
        clients[i] = clients[i + 1];
    }

    cantidad_clientes--;
}

int check_users(char *username, char *ip)
{
    for (int i = 0; i < cantidad_clientes; i++)
    {
        if (strcmp(clients[i].username, username) == 0 && strcmp(clients[i].user_ip, ip) == 0)
        {
            return 1; // Si el usuario existe, se devuelve 1
        }
    }
    return 0; // Si el usuario no existe
}

void ERRORMensaje(const char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

void handle_incoming_message(uint8_t *buffer, size_t len)
{
    ChatSistOS__Message *received_message = chat_sist_os__message__unpack(NULL, len, buffer);
    if (received_message == NULL)
    {
        printf("[CLIENT-ERROR]: Failed to unpack message\n");
        return;
    }

    if (received_message->message_private)
    {
        printf("[DM] %s: %s\n", received_message->message_sender, received_message->message_content);
    }
    else
    {
        printf("[BROADCAST] %s: %s\n", received_message->message_sender, received_message->message_content);
    }

    chat_sist_os__message__free_unpacked(received_message, NULL);
}

void *handle_newclient(void *arg)
{
    // Se obtiene el cliente y su informacion
    int client_socket = *(int *)arg;

    uint8_t buffer[1024];
    ssize_t bytes_received = recv(client_socket, buffer, 1024, 0);

    // Se verifica si existe informacion
    if (bytes_received < 0)
    {
        perror("[SERVER-ERROR]: Recepcion de cliente fallida\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        // Se crea un nuevo usuario en el server para manejar mejor la informacion
        ChatSistOS__NewUser *new_client = chat_sist_os__new_user__unpack(NULL, bytes_received, buffer);

        if (new_client == NULL)
        {
            perror("[SERVER-ERROR]: Registro de usuario fallido\n");
            exit(EXIT_FAILURE);
        }

        // Se imprime informacion
        printf("Ingreso usuario: %s | IP: (%s)\n", new_client->username, new_client->ip);

        // Se agrega el nuevo cliente a la lista
        Client user;
        strcpy(user.username, new_client->username);
        strcpy(user.user_ip, new_client->ip);
        user.status = 1;
        user.sockfd = client_socket;

        ChatSistOS__User user_response = CHAT_SIST_OS__USER__INIT;
        user_response.user_name = user.username;
        user_response.user_ip = user.user_ip;
        user_response.user_state = user.status;

        int user_exist = check_users(new_client->username, new_client->ip);
        if (user_exist == 0)
        {
            clients[cantidad_clientes] = user;
            cantidad_clientes++;
            // Respuesta de registro
            ChatSistOS__Answer answer = CHAT_SIST_OS__ANSWER__INIT;
            answer.response_status_code = 200;
            answer.response_message = "Usuario registrado";
            answer.user = &user_response;

            printf("[SERVER (%d)]->[CLIENT %s]: %s\n", answer.response_status_code, answer.user->user_name, answer.response_message);

            size_t package_size = chat_sist_os__answer__get_packed_size(&answer);
            uint8_t *buffer_envioA = malloc(package_size);
            chat_sist_os__answer__pack(&answer, buffer_envioA);

            if (send(client_socket, buffer_envioA, package_size, 0) < 0)
            {
                perror("[SERVER-ERROR]: Envio de respuesta fallido\n");
                exit(EXIT_FAILURE);
            }

            free(buffer_envioA);

            while (1)
            {
                // Se obtiene opcion ingresada
                uint8_t buffer_option[1024];
                ssize_t bytesRecibidos = recv(client_socket, buffer_option, 1024, 0);
                if (bytesRecibidos < 0)
                {
                    perror("[CLIENT-ERROR]: Recepcion de opcion fallida\n");
                    exit(EXIT_FAILURE);
                }

                ChatSistOS__UserOption *user_option = chat_sist_os__user_option__unpack(NULL, bytesRecibidos, buffer_option);
                printf("Opcion ingresada por %s: %d\n", new_client->username, user_option->op);

                // Manejar opciones

                if (user_option->op == 1)
                {
                    if (user_option->message)
                    {
                        printf("[SERVER]: Mensaje de: %s\n", user_option->message->message_sender);
                        printf("[SERVER]: Contenido: %s\n", user_option->message->message_content);
                        printf("[SERVER]: Destino: Usuarios conectados\n");
                        ChatSistOS__Message *broadcast_message = user_option->message;
                        ChatSistOS__Message new_message = CHAT_SIST_OS__MESSAGE__INIT;
                        new_message.message_private = broadcast_message->message_private;
                        new_message.message_destination = broadcast_message->message_destination ? strdup(broadcast_message->message_destination) : NULL;
                        new_message.message_content = strdup(broadcast_message->message_content);
                        new_message.message_sender = strdup(broadcast_message->message_sender);

                        for (int i = 0; i < cantidad_clientes; i++)
                        {
                            printf("Sending message to client %d\n", i);

                            ChatSistOS__Answer answer = CHAT_SIST_OS__ANSWER__INIT;
                            answer.op = 1;
                            answer.response_status_code = 200;
                            answer.message = &new_message;
                            // answer.user = &user;

                            size_t msg_size = chat_sist_os__answer__get_packed_size(&answer);
                            uint8_t *msg_buffer = malloc(msg_size);
                            chat_sist_os__answer__pack(&answer, msg_buffer);

                            if (send(clients[i].sockfd, msg_buffer, msg_size, 0) < 0)
                            {
                                perror("[SERVER-ERROR]: Envio de respuesta fallido\n");
                                exit(EXIT_FAILURE);
                            }

                            free(msg_buffer);
                        }

                        free(new_message.message_destination);
                        free(new_message.message_content);
                        free(new_message.message_sender);
                    }
                }

                else if (user_option->op == 2)
                {
                    printf("[SERVER]: Mensaje privado\n");

                    ChatSistOS__Message *dm_message = user_option->message;
                    char *dm_destination = dm_message->message_destination;
                    char *dm_content = dm_message->message_content;
                    char *dm_sender = dm_message->message_sender;

                    int dm_socket_fd = -1;
                    for (int i = 0; i < cantidad_clientes; i++)
                    {
                        if (strcmp(clients[i].username, dm_destination) == 0)
                        {
                            dm_socket_fd = clients[i].sockfd;
                            break;
                        }
                    }

                    if (dm_socket_fd == -1)
                    {
                        // Destination user not found
                        ChatSistOS__Answer answer = CHAT_SIST_OS__ANSWER__INIT;
                        answer.op = 2;
                        answer.response_status_code = 404;

                        size_t msg_size = chat_sist_os__answer__get_packed_size(&answer);
                        uint8_t *msg_buffer = malloc(msg_size);
                        chat_sist_os__answer__pack(&answer, msg_buffer);

                        if (send(client_socket, msg_buffer, msg_size, 0) < 0)
                        {
                            perror("[SERVER-ERROR]: Envio de respuesta fallido\n");
                            exit(EXIT_FAILURE);
                        }

                        free(msg_buffer);
                    }
                    else
                    {

                        ChatSistOS__Answer answer = CHAT_SIST_OS__ANSWER__INIT;
                        answer.op = 2;
                        answer.response_status_code = 200;
                        answer.message = dm_message;
                        answer.user = &user_response;

                        size_t msg_size = chat_sist_os__answer__get_packed_size(&answer);
                        uint8_t *msg_buffer = malloc(msg_size);
                        chat_sist_os__answer__pack(&answer, msg_buffer);

                        if (send(dm_socket_fd, msg_buffer, msg_size, 0) < 0)
                        {
                            perror("[SERVER-ERROR]: Envio de respuesta fallido\n");
                            exit(EXIT_FAILURE);
                        }

                        if (send(client_socket, msg_buffer, msg_size, 0) < 0)
                        {
                            perror("[SERVER-ERROR]: Envio de respuesta fallido\n");
                            exit(EXIT_FAILURE);
                        }

                        free(msg_buffer);
                    }
                }

                else if (user_option->op == 3)
                {

                    uint8_t buffer_recepcion[1024];
                    ssize_t bytes_recepcion = recv(client_socket, buffer_recepcion, 1024, 0);
                    if (bytes_recepcion < 0)
                    {
                        ERRORMensaje("[CLIENT-ERROR]: Recepcion de lista de usuarios fallida\n");
                    }

                    ChatSistOS__UserOption *usr_op_st = chat_sist_os__user_option__unpack(NULL, bytes_recepcion, buffer_recepcion);
                    if (usr_op_st == NULL)
                    {
                        ERRORMensaje("Error al deserializar los datos recibidos del servidor");
                    }

                    ChatSistOS__User *new_usr_st = find_user(usr_op_st->status->user_name);

                    if (new_usr_st != NULL)
                    {
                        if (new_usr_st->user_state > 3 || new_usr_st->user_state < 1)
                        {
                            // Error
                            printf("Status de usuario erroneo: %d\n", new_usr_st->user_state);
                        }
                        else
                        {
                            printf("Status de usuario: %d\n", new_usr_st->user_state);

                            int new_Status_temp = (new_usr_st->user_state == 1) ? 2 : 1;

                            printf("New status: %d\n", new_Status_temp);
                            printf("%s\n", (new_Status_temp == 1) ? "Ocupado -> En linea" : "En linea -> Ocupado");

                            user.status = new_Status_temp;

                            update_user_status(new_usr_st->user_name, new_usr_st->user_ip, user);

                            // Avisar al cliente
                            ChatSistOS__Answer answer_b = CHAT_SIST_OS__ANSWER__INIT;

                            answer_b.response_status_code = 200;
                            answer_b.response_message = ("Status cambiado: %s\n", (new_Status_temp == 1) ? "Ocupado -> En linea" : "En linea -> Ocupado");
                            answer_b.user = &user_response;

                            size_t package_size = chat_sist_os__answer__get_packed_size(&answer_b);
                            uint8_t *buffer_envioB = malloc(package_size);
                            chat_sist_os__answer__pack(&answer_b, buffer_envioB);

                            if (send(client_socket, buffer_envioB, package_size, 0) < 0)
                            {
                                perror("[SERVER-ERROR]: Envio de respuesta fallido\n");
                                exit(EXIT_FAILURE);
                            }

                            free(buffer_envioB);
                        }
                    }
                }
                else if (user_option->op == 4)
                {

                    uint8_t buffer_recepcion[1024];
                    ssize_t bytes_recepcion = recv(client_socket, buffer_recepcion, 1024, 0);
                    if (bytes_recepcion < 0)
                    {
                        ERRORMensaje("[CLIENT-ERROR]: Recepcion de lista de usuarios fallida\n");
                    }

                    ChatSistOS__UserOption *usr_op_a = chat_sist_os__user_option__unpack(NULL, bytes_recepcion, buffer_recepcion);
                    if (usr_op_a == NULL)
                    {
                        ERRORMensaje("Error al deserializar los datos recibidos del servidor");
                    }

                    if (usr_op_a->userlist->list == 1)
                    {
                        ChatSistOS__UsersOnline users_online = CHAT_SIST_OS__USERS_ONLINE__INIT;
                        users_online.n_users = cantidad_clientes;
                        users_online.users = malloc(sizeof(ChatSistOS__User *) * cantidad_clientes);
                        for (int i = 0; i < cantidad_clientes; i++)
                        {
                            ChatSistOS__User *user_l_response = malloc(sizeof(ChatSistOS__User));
                            chat_sist_os__user__init(user_l_response);
                            user_l_response->user_name = clients[i].username;
                            user_l_response->user_ip = clients[i].user_ip;
                            user_l_response->user_state = clients[i].status;
                            users_online.users[i] = user_l_response;
                        }

                        ChatSistOS__Answer uo_answer = CHAT_SIST_OS__ANSWER__INIT;
                        uo_answer.response_message = "Enviando listado de usuarios...";
                        uo_answer.response_status_code = 200;
                        uo_answer.users_online = &users_online;

                        size_t response_size_uo = chat_sist_os__answer__get_packed_size(&uo_answer);
                        uint8_t *response_buffer_uo = malloc(response_size_uo);
                        chat_sist_os__answer__pack(&uo_answer, response_buffer_uo);

                        // Se envía la respuesta al cliente que se acaba de conectar
                        ssize_t bytes_sent = send(client_socket, response_buffer_uo, response_size_uo, 0);
                        if (bytes_sent < 0)
                        {
                            perror("[SERVER-ERROR]: Envío de respuesta fallido\n");
                            exit(EXIT_FAILURE);
                        }

                        // Se libera la memoria utilizada
                        for (int i = 0; i < cantidad_clientes; i++)
                        {
                            free(users_online.users[i]);
                        }
                        free(users_online.users);
                        free(response_buffer_uo);
                    }

                    chat_sist_os__user_option__free_unpacked(usr_op_a, NULL);
                }
                else if (user_option->op == 5)
                {

                    uint8_t buffer_recepcion[1024];
                    ssize_t bytes_recepcion = recv(client_socket, buffer_recepcion, 1024, 0);
                    if (bytes_recepcion < 0)
                    {
                        ERRORMensaje("[CLIENT-ERROR]: Recepcion de lista de usuarios fallida\n");
                    }

                    ChatSistOS__UserOption *usr_op = chat_sist_os__user_option__unpack(NULL, bytes_recepcion, buffer_recepcion);
                    if (usr_op == NULL)
                    {
                        ERRORMensaje("Error al deserializar los datos recibidos del servidor");
                    }

                    printf("Buscando usuario: %s...\n", usr_op->userlist->user_name);

                    ChatSistOS__User *user_response = find_user(usr_op->userlist->user_name);
                    ChatSistOS__Answer response = CHAT_SIST_OS__ANSWER__INIT;

                    if (user_response == NULL || usr_op->userlist->list == 1)
                    {
                        answer.response_status_code = 400;
                        answer.response_message = "Usuario no encontrado";

                        size_t package_size = chat_sist_os__answer__get_packed_size(&answer);
                        uint8_t *buffer_envioB = malloc(package_size);
                        chat_sist_os__answer__pack(&answer, buffer_envioB);

                        if (send(client_socket, buffer_envioB, package_size, 0) < 0)
                        {
                            perror("[SERVER-ERROR]: Envio de respuesta fallido\n");
                            exit(EXIT_FAILURE);
                        }

                        free(buffer_envioB);
                    }
                    else
                    {
                        answer.response_status_code = 200;
                        answer.response_message = "Usuario encontrado";
                        answer.user = user_response;

                        size_t package_size = chat_sist_os__answer__get_packed_size(&answer);
                        uint8_t *buffer_envioB = malloc(package_size);
                        chat_sist_os__answer__pack(&answer, buffer_envioB);

                        if (send(client_socket, buffer_envioB, package_size, 0) < 0)
                        {
                            perror("[SERVER-ERROR]: Envio de respuesta fallido\n");
                            exit(EXIT_FAILURE);
                        }

                        free(buffer_envioB);
                    }

                    chat_sist_os__user_option__free_unpacked(usr_op, NULL);
                }

                else if (user_option->op == 6)
                {
                    printf("6\n");
                }

                else if (user_option == NULL || user_option->op == 7 || user_option->op == 0)
                {
                    int index = search_user(user.username, user.user_ip);
                    if (index != -1)
                    {
                        remove_users(index);
                    }
                    break;
                }

                chat_sist_os__user_option__free_unpacked(user_option, NULL);
            }

            chat_sist_os__new_user__free_unpacked(new_client, NULL);
        }
        else
        {
            // Respuesta de registro
            ChatSistOS__Answer answer = CHAT_SIST_OS__ANSWER__INIT;
            answer.response_status_code = 400;
            answer.response_message = "Usuario ya registrado";
            answer.user = &user_response;

            printf("[SERVER (%d)]->[CLIENT %s]: %s\n", answer.response_status_code, answer.user->user_name, answer.response_message);

            size_t package_size = chat_sist_os__answer__get_packed_size(&answer);
            uint8_t *buffer_envioB = malloc(package_size);
            chat_sist_os__answer__pack(&answer, buffer_envioB);

            if (send(client_socket, buffer_envioB, package_size, 0) < 0)
            {
                perror("[SERVER-ERROR]: Envio de respuesta fallido\n");
                exit(EXIT_FAILURE);
            }

            free(buffer_envioB);
            chat_sist_os__new_user__free_unpacked(new_client, NULL);
        }
    }
}

int main(int argc, char **argv)
{

    if (argc > 1)
    {
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
        if (server_fd == 0)
        {
            perror("[SERVER-ERROR]: Creacion de socket fallida\n");
            exit(EXIT_FAILURE);
        }
        else
        {
            printf("[SERVER]: Socket creado exitosamente\n");
        }

        memset(&server, 0, sizeof(server));

        // Se inicializa la estructura con la direccion y puerto donde se escucharan solicitudes
        server.sin_family = AF_INET;
        server.sin_port = htons(puerto);
        server.sin_addr.s_addr = INADDR_ANY;

        // Con bind se asocia la direccion y el puerto al socket del servidor
        if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) < 0)
        {
            perror("[SERVER-ERROR]: Socket bind fallido\n");
            exit(EXIT_FAILURE);
        }
        else
        {
            printf("[SERVER]: Socket asociado exitosamente\n");
        }

        if (listen(server_fd, MAX_CLIENTS) != 0)
        {
            perror("[SERVER-ERROR]: Socket listen fallido");
            exit(EXIT_FAILURE);
        }
        else
        {
            printf("[SERVER]: Escuchando en el puerto %d\n", puerto);
        }

        while (1)
        {

            client_fd = accept(server_fd, (struct sockaddr *)&client, (socklen_t *)&addrlenclient);
            if (client_fd < 0)
            {
                perror("[SERVER-ERROR]: Conexion no aceptada\n");
                exit(EXIT_FAILURE);
            }

            pthread_t thread;
            int pthread_result = pthread_create(&thread, NULL, handle_newclient, (void *)&client_fd);
            if (pthread_result < 0)
            {
                perror("[SERVER-ERROR]: Creacion de hilo para cliente fallida\n");
                exit(EXIT_FAILURE);
            }
        }

        return 0;
    }
    else
    {
        printf("Puerto no especificado. Comando: <Nombre Programa> <Puerto>\n");
    }
}
