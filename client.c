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
#include <netdb.h>
#include <curl/curl.h>
#include <ctype.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <time.h>
#include <sys/select.h>
#include <signal.h>
#include <iconv.h>

#define BUFFER_SIZE 1024

#define OP_CHAT 1
#define OP_DM 2
#define OP_STATUS 3
#define OP_USUARIOS 4
#define OP_USUARIO 5
#define OP_HELP 6
#define OP_EXIT 7
#define OP_SERVER_CLOSED 8

int client_socket;
char *username;
char private_message[BUFFER_SIZE];
char destination[BUFFER_SIZE];
char content[BUFFER_SIZE];
int state;
char connected_users;
char user_information[BUFFER_SIZE];
char input[BUFFER_SIZE];

int running = 1;

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
void ERRORMensaje(const char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

void AYUDA()
{
    const char *comandos[] = {
        "1 <mensaje>: envía un mensaje a todos los usuarios conectados",
        "2 <usuario> <mensaje>: envía un mensaje directo al usuario especificado",
        "3 <estado>: cambia tu estado al valor especificado (0: activo, 1: ocupado, 2: inactivo)",
        "4: muestra una lista de todos los usuarios conectados",
        "5 <nombredeusuario>: muestra información sobre el usuario especificado",
        "6: muestra este mensaje de ayuda",
        "7: sale del cliente de chat"};

    printf("\nSintaxis comandos:\n");
    for (int i = 0; i < sizeof(comandos) / sizeof(comandos[0]); i++)
    {
        printf("- %s\n", comandos[i]);
    }
}

void *listen_server(void *arg)
{
    int cliente_fd_server = *(int *)arg;
    while (1)
    {
        uint8_t buffer_rx_server[1024];
        ssize_t bytesRecibidos_server;
        bytesRecibidos_server = recv(cliente_fd_server, buffer_rx_server, 1024, 0);
        if (bytesRecibidos_server < 0)
        {
            ERRORMensaje("[CLIENT-ERROR]: Recepcion de respuesta fallida\n");
            continue;
        }

        if (bytesRecibidos_server == 0)
        {
            printf("[CLIENT]: El servidor ha cerrado la conexion\n");
            running = 0;
            exit(0);
            break;
        }

        ChatSistOS__Answer *answer_server = chat_sist_os__answer__unpack(NULL, bytesRecibidos_server, buffer_rx_server);
        int answer = answer_server->op;

        switch (answer)
        {
        case 1:
            printf("\n[%s]: %s\n", answer_server->message->message_sender, answer_server->message->message_content);
            break;
        case 2:
            if (answer_server->response_status_code != 400)
            {
                printf("[SERVER (%d)]->[%s] mensaje para [%s]: %s\n", answer_server->response_status_code, answer_server->message->message_sender, answer_server->message->message_destination, answer_server->message->message_content);
            }
            else
            {
                printf("[SERVER (%d)]->[%s]: %s\n", answer_server->response_status_code, answer_server->message->message_sender, answer_server->response_message);
            }
            break;
        case 3:
            printf("[SERVER (%d)]->[%s]: %s\n", answer_server->response_status_code, answer_server->user->user_name, answer_server->response_message);
            break;
        case 4:
            printf("[SERVER (%d)]: %s\n", answer_server->response_status_code, answer_server->response_message);

            printf("\nLista de usuarios conectados\n");
            for (int i = 0; i < answer_server->users_online->n_users; i++)
            {
                ChatSistOS__User *user = answer_server->users_online->users[i];
                char status[25] = "Estado desconocido";
                if (user->user_state == 1)
                {
                    strcpy(status, "En linea");
                }
                else if (user->user_state == 2)
                {
                    strcpy(status, "Ocupado");
                }
                else if (user->user_state == 3)
                {
                    strcpy(status, "Desconectado");
                }
                printf("- Usuario: %s con ip (%s) -> status: (%s)\n", user->user_name, user->user_ip, status);
            }
            printf("\n");
            break;
        case 5:
            if (answer_server->response_status_code != 400)
            {
                printf("[SERVER]: %s | IP: %s\n", answer_server->response_message, answer_server->user->user_ip);
            }
            else
            {
                printf("[SERVER]: %s\n", answer_server->response_message);
            }
            break;

        case 6:
            break;

        case 7:
            break;

        default:
            printf("Opcion INVALIDA, selecciona la veri help (6) para ayuda\n");
            break;
        }
        MENU();
        printf("Ingrese comando: \n");
        chat_sist_os__answer__free_unpacked(answer_server, NULL);
    }

    close(cliente_fd_server);
}

int main(int argc, const char **argv)
{

    char *username;
    const char *IPserver;
    int puerto;

    username = (char *)argv[1];
    IPserver = argv[2];
    puerto = atoi(argv[3]);

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
        if (getsockname(cliente_fd, (struct sockaddr *)&addr, &len) == -1)
        {
            perror("[CLIENT-ERROR]: Obtención de dirección IP del cliente fallida\n");
            exit(EXIT_FAILURE);
        }

        ChatSistOS__NewUser new_client = CHAT_SIST_OS__NEW_USER__INIT;
        new_client.username = username;
        new_client.ip = inet_ntoa(addr.sin_addr);

        size_t package_size = chat_sist_os__new_user__get_packed_size(&new_client);
        uint8_t *buffer_tx = malloc(package_size);
        chat_sist_os__new_user__pack(&new_client, buffer_tx);

        if (send(cliente_fd, buffer_tx, package_size, 0) < 0)
        {
            ERRORMensaje("[CLIENT-ERROR]: Envio de mensaje fallido\n");
        }

        free(buffer_tx);

        uint8_t buffer_rx[1024];

        ssize_t bytesRecibidos = recv(cliente_fd, buffer_rx, 1024, 0);
        if (bytesRecibidos < 0)
        {
            ERRORMensaje("[CLIENT-ERROR]: Recepcion de respuesta fallida\n");
            close(cliente_fd);
            return -1;
        }

        // Se obtiene el mensaje del servidor
        ChatSistOS__Answer *answer = chat_sist_os__answer__unpack(NULL, bytesRecibidos, buffer_rx);
        if (answer == NULL)
        {
            printf("ERROR: No se pudo desempaquetar el mensaje recibido\n");
            return -1;
        }
        printf("[SERVER (%d)]->[%s]: %s\n", answer->response_status_code, answer->user->user_name, answer->response_message);

        // Multithreading para broadcast y dm
        pthread_t listen_thread;
        int pthread_result = pthread_create(&listen_thread, NULL, listen_server, (void *)&cliente_fd);
        if (pthread_result < 0)
        {
            ERRORMensaje("Creacion de hilo para cliente fallida");
        }

        int veri = 0;
        while (running && veri != 7)
        {

            MENU();
            printf("Ingrese comando: \n");
            scanf("%d", &veri);

            switch (veri)
            {
            case OP_CHAT:
                printf("Mensaje general\n\n");

                printf("Ingrese mensaje: ");
                char input[1024];
                scanf(" %[^\n]", input);

                ChatSistOS__Message chat_message = CHAT_SIST_OS__MESSAGE__INIT;
                chat_message.message_private = 0;

                chat_message.message_content = strdup(input);
                chat_message.message_sender = strdup(username);

                ChatSistOS__UserOption user_option = CHAT_SIST_OS__USER_OPTION__INIT;
                user_option.op = veri;
                user_option.message = &chat_message;

                size_t package_size_chat = chat_sist_os__user_option__get_packed_size(&user_option);
                uint8_t *package_buffer_chat = malloc(package_size_chat);
                chat_sist_os__user_option__pack(&user_option, package_buffer_chat);

                if (send(cliente_fd, package_buffer_chat, package_size_chat, 0) < 0)
                {
                    perror("[CLIENT-ERROR]: Envio de mensaje fallido\n");
                    continue;
                }
                else
                {
                    free(package_buffer_chat);
                    printf("[CLIENT]: Mensaje enviado\n");
                }

                free(chat_message.message_sender);
                free(chat_message.message_content);
                break;

            case OP_DM:
                printf("Mensaje privado\n\n");

                printf("Ingrese el nombre del destinatario: ");
                char input_dest[1024];
                scanf(" %[^\n]", input_dest);

                printf("Ingrese el mensaje: ");
                char input_msg[1024];
                scanf(" %[^\n]", input_msg);

                ChatSistOS__Message dm_message = CHAT_SIST_OS__MESSAGE__INIT;
                dm_message.message_private = 1;
                dm_message.message_destination = input_dest;
                dm_message.message_content = input_msg;
                dm_message.message_sender = username;

                ChatSistOS__UserOption option_user = CHAT_SIST_OS__USER_OPTION__INIT;
                option_user.op = veri;
                option_user.message = &dm_message;

                size_t serializar = chat_sist_os__user_option__get_packed_size(&option_user);
                uint8_t *buffer_dm = malloc(serializar);
                chat_sist_os__user_option__pack(&option_user, buffer_dm);

                if (send(cliente_fd, buffer_dm, serializar, 0) < 0)
                {
                    ERRORMensaje("ERROR: Envio de mensaje fallido\n");
                    continue;
                }
                free(buffer_dm);
                printf("[CLIENT]: Mensaje enviado\n");
                break;

            case OP_STATUS:
                printf("Cambiando status...\n");

                ChatSistOS__Status status_usr = CHAT_SIST_OS__STATUS__INIT;
                status_usr.user_name = answer->user->user_name;
                status_usr.user_state = answer->user->user_state;

                ChatSistOS__UserOption usr_op_s = CHAT_SIST_OS__USER_OPTION__INIT;
                usr_op_s.op = veri;
                usr_op_s.status = &status_usr;

                size_t package_size_s = chat_sist_os__user_option__get_packed_size(&usr_op_s);
                uint8_t *buffer_option_s = malloc(package_size_s);
                chat_sist_os__user_option__pack(&usr_op_s, buffer_option_s);

                if (send(cliente_fd, buffer_option_s, package_size_s, 0) < 0)
                {
                    ERRORMensaje("[CLIENT-ERROR]: Envio de usuario fallido\n");
                }

                free(buffer_option_s);
                break;

            case OP_USUARIOS:
                printf("Listado usuarios\n");

                ChatSistOS__UserList new_user_all = CHAT_SIST_OS__USER_LIST__INIT;
                new_user_all.list = 1;

                ChatSistOS__UserOption usr_op_a = CHAT_SIST_OS__USER_OPTION__INIT;
                usr_op_a.op = veri;
                usr_op_a.userlist = &new_user_all;

                size_t package_size_all = chat_sist_os__user_option__get_packed_size(&usr_op_a);
                uint8_t *buffer_option_all = malloc(package_size_all);
                chat_sist_os__user_option__pack(&usr_op_a, buffer_option_all);

                if (send(cliente_fd, buffer_option_all, package_size_all, 0) < 0)
                {
                    ERRORMensaje("[CLIENT-ERROR]: Envio de usuario fallido\n");
                }

                free(buffer_option_all);
                break;

            case OP_USUARIO:
                printf("Informacion de usuario\n");

                printf("Ingrese el usuario: ");
                char usuarioname[1024];
                scanf(" %[^\n]", usuarioname);

                printf("%s", usuarioname);

                ChatSistOS__UserList new_user_sp = CHAT_SIST_OS__USER_LIST__INIT;
                new_user_sp.list = 0;
                new_user_sp.user_name = usuarioname;

                ChatSistOS__UserOption usr_op_sp = CHAT_SIST_OS__USER_OPTION__INIT;
                usr_op_sp.op = veri;
                usr_op_sp.userlist = &new_user_sp;

                size_t package_size_sp = chat_sist_os__user_option__get_packed_size(&usr_op_sp);
                uint8_t *buffer_option_sp = malloc(package_size_sp);
                chat_sist_os__user_option__pack(&usr_op_sp, buffer_option_sp);

                if (send(cliente_fd, buffer_option_sp, package_size_sp, 0) < 0)
                {
                    ERRORMensaje("[CLIENT-ERROR]: Envio de usuario fallido\n");
                }

                free(buffer_option_sp);
                break;

            case OP_HELP:
                AYUDA();
                break;

            case OP_EXIT:
                printf("Saliendo y liberando recursos...\n\n");
                close(cliente_fd);
                break;

            default:

                printf("Opcion INVALIDA, selecciona la veri help (6) para ayuda\n");
                break;
            }
        }
        chat_sist_os__answer__free_unpacked(answer, NULL);
        close(client_socket);
        return 0;
    }
    else
    {
        printf("Faltan argumentos. Comando: <Nombre Programa> <Nombre Usuario> <IP Server> <Puerto>\n");
    }
}
