#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>
#include <netinet/in.h>
#include "common.h"

// struct para representar um cliente conectado
typedef struct {
  int socket_fd; // descritor de arquivo do socket do cliente
  struct sockaddr_in address; // endereço do cliente
  char nome[NAME_SIZE]; // nome do cliente
  int is_active; // flag para indicar se o cliente ta ativo
} client;


/*
  - Inicia o servidor
*/
void server_start();

/*
  - função executada por cada thread para lidar com um cliente
  - recebe mensagens do cliente e inicia o broadcast
  - Parâmetros:
    - arg: ponteiro para a struct cliente
*/
void *handle_client(void *cliente);


/*
  - adiciona um novo cliente
  - Parâmetros: 
    - new_client: dados do novo cliente a ser adicionado
*/
void add_client(client* new_client);

/*
  - remove um cliente
  - Parâmetros: 
    - socket_fd: descritor de arquivo do socket do cliente a ser removido
*/
void remove_client(int socket_fd);

/*
  - envia uma mensagem para todos os clientes ativos
  - Parâmetros:
    - sender_socket:  socket do cliente que enviou a mensagem original
    - message: mensagem a ser enviada
*/
void broadcast_message(int sender_socket, const char *message);


#endif // SERVER_H