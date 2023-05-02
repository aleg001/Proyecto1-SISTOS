#include <stdio.h>      //Prints
#include <stdlib.h>     //Gestion memoria
#include <string.h>     //Manipulacion strings
#include <errno.h>      //Errores
#include <unistd.h>     //Funciones de sistema
#include <sys/types.h>  //Tipos de datos
#include <sys/socket.h> //Sockets de red
#include <netinet/in.h> //Direcciones de red
#include <pthread.h>    //Hilos
#include "chat.pb-c.h"
#include <protobuf-c/protobuf-c.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <curl/curl.h>
#include <ctype.h>
#include <ifaddrs.h>
#include <net/if.h>

#define BUFFER_SIZE 1024

#define OP_CHAT 1
#define OP_DM 2
#define OP_STATUS 3
#define OP_USUARIOS 4
#define OP_USUARIO 5
#define OP_HELP 6
#define OP_EXIT 7

int client_socket;
char *username;
char private_message[BUFFER_SIZE];
char destination[BUFFER_SIZE];
char content[BUFFER_SIZE];
int state;
char connected_users;
char user_information[BUFFER_SIZE];

void MENU()
{
    printf("\n");
    const char *comandos[] = {
        "1) Mensaje general (Chat)",
        "2) Mensaje privado (DM)",
        "3) Cambiar status",
        "4) Usuarios conectados",
        "5) Informacion usuario",
        "6) Help",
        "7) Salir"};

    printf("Comandos disponibles:\n");
    for (int i = 0; i < sizeof(comandos) / sizeof(comandos[0]); i++)
    {
        printf("- %s\n", comandos[i]);
    }
}

void AYUDA()
{
    const char *comandos[] = {
        "chat <mensaje>: envía un mensaje a todos los usuarios conectados",
        "dm <usuario> <mensaje>: envía un mensaje directo al usuario especificado",
        "status <estado>: cambia tu estado al valor especificado (0: activo, 1: ocupado, 2: inactivo)",
        "usuarios: muestra una lista de todos los usuarios conectados",
        "usuario <nombredeusuario>: muestra información sobre el usuario especificado",
        "help: muestra este mensaje de ayuda",
        "exit: sale del cliente de chat"};

    printf("Sintaxis comandos:\n");
    for (int i = 0; i < sizeof(comandos) / sizeof(comandos[0]); i++)
    {
        printf("- %s\n", comandos[i]);
    }
    printf("\n");
}

void ERRORMensaje(const char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

int main(int argc, const char **argv)
{

    char *username, *IPserver;
    int puerto;

    username = argv[1];
    IPserver = argv[2];
    puerto = atoi(argv[3]);

    printf("BIENVENIDO AL ¡CHAT!, selecciona la opcion help para mas informacion\n");
    char input[BUFFER_SIZE];

    if (argc > 1)
    {
        int cliente_fd, valread;
        struct sockaddr_in serv_addr;
        cliente_fd = socket(AF_INET, SOCK_STREAM, 0);

        if (cliente_fd < 0)
        {
            ERRORMensaje("ERROR: Creacion al socket fallida\n");
        }
        else
        {
            printf("[CLIENT]: Socket creado exitosamente\n");
        }

        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(IPserver);
        serv_addr.sin_port = htons(puerto);

        if (connect(cliente_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0)
        {
            ERRORMensaje("ERROR: Conexion al servidor fallida\n");
        }
        else
        {
            printf("[CLIENT]: Conectado al servidor exitosamente\n");
        }

        // Obtener la dirección IP del cliente
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        if (getsockname(cliente_fd, (struct sockaddr *)&addr, &len) == -1) {
            perror("[CLIENT-ERROR]: Obtención de dirección IP del cliente fallida\n");
            exit(EXIT_FAILURE);
        }

        ChatSistOS__NewUser new_client = CHAT_SIST_OS__NEW_USER__INIT;
        new_client.username = username;
        new_client.ip = inet_ntoa(addr.sin_addr);

        size_t package_size = chat_sist_os__new_user__get_packed_size(&new_client);
        uint8_t *buffer_tx = malloc(package_size);
        chat_sist_os__new_user__pack(&new_client, buffer_tx);

        if(send(cliente_fd, buffer_tx, package_size, 0) < 0){
            ERRORMensaje("[CLIENT-ERROR]: Envio de mensaje fallido\n");
        }

        free(buffer_tx);

        uint8_t buffer_rx[1024];
        ssize_t bytesRecibidos = recv(cliente_fd, buffer_rx, 1024, 0);
        if(bytesRecibidos < 0){
            ERRORMensaje("[CLIENT-ERROR]: Recepcion de respuesta fallida\n");
        }

        // Se obtiene el mensaje del servidor
        ChatSistOS__Answer *answer = chat_sist_os__answer__unpack(NULL, bytesRecibidos, buffer_rx);
        if (answer == NULL) {
            printf("ERROR: No se pudo desempaquetar el mensaje recibido\n");
            return -1;  
        }
        printf("[SERVER (%d)]->[%s]: %s\n",answer->response_status_code, answer->user->user_name, answer->response_message);

        if(answer->response_status_code != 400){
            int veri = 1;
            while (veri)
            {
                MENU();
                printf("\nIngrese comando: ");
                if (fgets(input, sizeof(input), stdin) == NULL) {
                    continue;
                }
                input[strcspn(input, "\r\n")] = '\0';

                if (strlen(input) == 0)
                {
                    continue;
                }
                
                int opcion = atoi(input);
                uint8_t bufferExit = 1;

                ChatSistOS__UserOption option_user = CHAT_SIST_OS__USER_OPTION__INIT;
                option_user.op = opcion;

                size_t package_size = chat_sist_os__user_option__get_packed_size(&option_user);
                uint8_t *buffer_option = malloc(package_size);
                chat_sist_os__user_option__pack(&option_user, buffer_option);

                if(send(cliente_fd, buffer_option, package_size, 0) < 0){
                    ERRORMensaje("[CLIENT-ERROR]: Envio de opcion fallido\n");
                }

                free(buffer_option);

                switch (opcion){
                    case OP_CHAT:
                        printf("Mensaje general\n\n");

                        if (fgets(input, sizeof(input), stdin) == NULL)
                        {
                            printf("FORMATO INCORRECTO\n");
                            continue;
                        }
                        input[strcspn(input, "\n")] = '\0';

                        char *txt_mensaje = strtok(input, " ");
                        txt_mensaje = strtok(NULL, "");

                        printf("Mensaje: %s\n", txt_mensaje);

                        if (txt_mensaje == NULL)
                        {
                            printf("FORMATO INCORRECTO\n");
                            continue;
                        }

                        ChatSistOS__Message chat_message = CHAT_SIST_OS__MESSAGE__INIT;
                        chat_message.message_private = 0;
                        chat_message.message_destination = "";
                        chat_message.message_content = txt_mensaje;
                        chat_message.message_sender = username;

                        size_t serializado = chat_sist_os__message__get_packed_size(&chat_message);
                        uint8_t *buffer = malloc(serializado);
                        chat_sist_os__message__pack(&chat_message, buffer);

                        if (send(cliente_fd, buffer, serializado, 0) < 0)
                        {
                            ERRORMensaje("ERROR: Envio de mensaje fallido\n");
                        }
                        else
                        {
                            free(buffer);
                            printf("[CLIENT]: Mensaje enviado\n");
                        }
                        continue;
                        break;

                    case OP_DM:
                        printf("Mensaje privado\n\n");
                        if (fgets(input, sizeof(input), stdin) == NULL)
                        {
                            printf("FORMATO INCORRECTO\n");
                            continue;
                        }
                        input[strcspn(input, "\n")] = '\0';

                        char *usuarioname = strtok(input, " ");
                        if (usuarioname == NULL)
                        {
                            printf("FORMATO INCORRECTO\n");
                            continue;
                        }
                        char *dm_mensaje = strtok(NULL, "");
                        if (dm_mensaje == NULL)
                        {
                            printf("FORMATO INCORRECTO\n");
                            continue;
                        }

                        ChatSistOS__Message dm_message = CHAT_SIST_OS__MESSAGE__INIT;
                        dm_message.message_private = 1;
                        dm_message.message_destination = usuarioname;
                        dm_message.message_content = dm_mensaje;
                        dm_message.message_sender = username;

                        size_t dm_message_size = chat_sist_os__message__get_packed_size(&dm_message);
                        uint8_t *dm_message_buffer = malloc(dm_message_size);
                        chat_sist_os__message__pack(&dm_message, dm_message_buffer);

                        printf("Mensaje: %s\n", dm_mensaje);
                        printf("Destinatario: %s\n", usuarioname);

                        if (send(cliente_fd, dm_message_buffer, dm_message_size, 0) < 0)
                        {
                            ERRORMensaje("ERROR: Envio de mensaje fallido\n");
                            continue;
                        }
                        else
                        {
                            free(dm_message_buffer);
                            printf("[CLIENT]: Mensaje enviado\n");
                            continue;
                        }
                        break;

                    case OP_STATUS:
                        printf("Status\n\n");
                        // char *status_str = strtok(NULL, " ");
                        // if (status_str == NULL)
                        // {
                        //     printf("FORMATO INVALIDO DE ESTADO\n");
                        //     continue;
                        // }
                        // int status = atoi(status_str);
                        // if (status < 0 || status > 2)
                        // {
                        //     printf("Estados pueden ser 0, 1 y 2\n");
                        //     continue;
                        // }

                        // ChatSistOS__Status status_update = CHAT_SIST_OS__STATUS__INIT;
                        // status_update.user_name = username;
                        // status_update.user_state = status;

                        // size_t status_update_size = chat_sist_os__status__get_packed_size(&status_update);
                        // uint8_t *status_update_buffer = malloc(status_update_size);
                        // chat_sist_os__status__pack(&status_update, status_update_buffer);

                        // if (send(cliente_fd, status_update_buffer, status_update_size, 0) < 0)
                        // {
                        //     ERRORMensaje("ERROR: Envio de estado fallido\n");
                        // }
                        // else
                        // {
                        //     free(status_update_buffer);
                        //     printf("[CLIENT]: Estado actualizado\n");
                        // }
                        break;

                    case OP_USUARIOS:
                        printf("Listado usuarios\n");

                        uint8_t buffer_recepcion[1024];
                        ssize_t bytes_recepcion = recv(cliente_fd, buffer_recepcion, 1024, 0);
                        if(bytes_recepcion < 0){
                            ERRORMensaje("[CLIENT-ERROR]: Recepcion de lista de usuarios fallida\n");
                        }
                        ChatSistOS__UsersOnline *users_online = chat_sist_os__users_online__unpack(NULL, bytes_recepcion, buffer_recepcion);
                        if (users_online == NULL) {
                            ERRORMensaje("Error al deserializar los datos recibidos del servidor");
                        }

                        for (int i = 0; i < users_online->n_users; i++) {
                            ChatSistOS__User *user = users_online->users[i];
                            char status[25] = "Estado desconocido";
                            if(user->user_state == 1){
                                strcpy(status, "En linea");
                            } else if (user->user_state == 2) {
                                strcpy(status, "Ocupado");
                            } else if (user->user_state == 3) {
                                strcpy(status, "Desconectado");
                            }
                            printf("- Usuario: %s con ip (%s) -> status: (%s)\n", user->user_name, user->user_ip, status);
                        }

                        chat_sist_os__users_online__free_unpacked(users_online, NULL);
                        printf("\n");
                        break;

                    case OP_USUARIO:
                        printf("Informacion usuario\n\n");

                        char usuarioname[50];
                        printf("Ingrese su nombre de usuario: ");
                        fgets(usuarioname, 50, stdin);
                        usuarioname[strcspn(usuarioname, "\n")] = 0;

                        ChatSistOS__NewUser new_us = CHAT_SIST_OS__NEW_USER__INIT;
                        new_us.username = usuarioname;

                        size_t package_size = chat_sist_os__new_user__get_packed_size(&new_us);
                        uint8_t *buffer_option = malloc(package_size);
                        chat_sist_os__new_user__pack(&new_us, buffer_option);

                        if(send(cliente_fd, buffer_option, package_size, 0) < 0){
                            ERRORMensaje("[CLIENT-ERROR]: Envio de usuario fallido\n");
                        }

                        free(buffer_option);

                        uint8_t buffer_rx[1024];
                        ssize_t bytesRecibidos = recv(cliente_fd, buffer_rx, 1024, 0);
                        if(bytesRecibidos < 0){
                            ERRORMensaje("[CLIENT-ERROR]: Recepcion de respuesta fallida\n");
                        }

                        // Se obtiene el mensaje del servidor
                        ChatSistOS__Answer *answer = chat_sist_os__answer__unpack(NULL, bytesRecibidos, buffer_rx);
                        if (answer == NULL) {
                            printf("ERROR: No se pudo desempaquetar el mensaje recibido\n");
                            return -1;  
                        }

                        if(answer->response_status_code != 400){
                            printf("[SERVER]: %s | IP: %s\n", answer->response_message, answer->user->user_ip);
                        } else {
                            printf("[SERVER]: %s\n", answer->response_message);
                        }
                        break;

                    case OP_HELP:
                        printf("Ayuda\n\n");
                        AYUDA();
                        break;
                        
                    case OP_EXIT:
                        printf("Salir\n\n");
                        close(cliente_fd);
                        veri=0;
                        break;
                    default:
                        printf("Comando INVALIDO, escribe 'help' pa q te ayudemos\n");
                        break;
                }
            }
        }
        chat_sist_os__answer__free_unpacked(answer, NULL);
        return EXIT_SUCCESS;
    }
    else
    {
        printf("Faltan argumentos. Comando: <Nombre Programa> <Nombre Usuario> <IP Server> <Puerto>\n");
    }
}