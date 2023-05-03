# Chat Cliente-Servidor con Multithreading

El ambiente utilizado para desarrollar este programa fue Debian 11 con ayuda de máquinas virtuales. Este proyecto está basado en el propuesto por Bob Dugan y Erik Véliz en 2006. Con este trabajo se buscó reforzar los conocimientos sobre procesos, threads, concurrencia y comunicación entre procesos. El objetivo era desarrollar una aplicación de chat en C.

La aplicación debe estar separada dos partes:
- Servidor: mantiene una lista de todos los clientes/usuarios conectados al sistema. Sólo puede existir un servidor durante una ejecución del sistema de chat. El servidor se podrá ejecutar en cualquier máquina Linux que provea la infraestructura necesaria y los clientes se conectarán al servidor mediante la dirección IP de la máquina donde se ejecute el servidor.
- Cliente: se conecta y se registra con el servidor. Un sistema de chat permite la existencia de uno o más clientes durante su ejecución. Posee una pantalla mediante la que se despliegan los mensajes que el usuario ha recibido del servidor, enviados por un usuario remoto; y en la que se permite el ingreso de texto para envío de mensajes a otros usuarios. Cada cliente debe presentar su propia y única pantalla de chat.

## Comandos a utilizar
- Necesario instalar (Sirven para compilar los archivos utilizados):
  - `sudo apt-get install libcurl4-openssl-dev`
  - `sudo apt-get install protobuf-c-compiler libprotobuf-c-dev`
- Para compilar el archivo .proto:
  - `protoc --c_out=. chat.proto`
  
- Para compilar el servidor:
  - `gcc -o server server.c chat.pb-c.c -lprotobuf-c -pthread`
  
- Para compilar el cliente:
  - `gcc -o client client.c chat.pb-c.c -lprotobuf-c -pthread`
  
- Para ejecutar el servidor
  - `./server 9000` (9000 es el puerto donde se escuchara al cliente)
  
- Para ejecutar el cliente
  - `./client test 10.0.2.15 9000` (test es el nombre del usuario, 10.0.2.15 la IP del servidor (en nuestro caso esta era de la Virtual Machine), 9000 el puerto del servidor)
  
Posterior a esto se podrán usar todas las opciones del cliente:
1. Chatear con todos los usuarios (broadcasting).
2. Enviar y recibir mensajes directos, privados, aparte del chat general.
3. Cambiar de status.
4. Listar los usuarios conectados al sistema de chat.
5. Desplegar información de un usuario en particular.
6. Ayuda.
7. Salir.
