#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>
#include <netinet/in.h>
#include <semaphore.h> 
#include "common.h"

#define QUEUE_SIZE 50

// struct para representar um cliente conectado
typedef struct {
  int socket_fd; // descritor de arquivo do socket do cliente
  struct sockaddr_in address; // endereço do cliente
  char nome[NAME_SIZE]; // nome do cliente
  int is_active; // flag para indicar se o cliente ta ativo
} client;

// struct para as mensagens
typedef struct {
  char content[BUFFER_SIZE + NAME_SIZE + 4];
  int sender_socket;
} message_t;

// struct do monitor (fila thread-safe)
typedef struct {
  message_t buffer[QUEUE_SIZE];
  int head, tail, count;
  pthread_mutex_t mutex;
  pthread_cond_t not_empty; // variavel conticional para sinalizar se a fila ta vazia ou não
} message_queue_t;


// -- FUNÇÕES PARA CONTROLAR A FILA DE MENSAGENS (MONITOR)

/*
  - função para inicializar a fila de mensagens
  - Parâmetros:
    - q: ponteiro para a struct message_queue
*/
void queue_init(message_queue_t *q);

/*
  - função para adicionar uma mensagem à fila
  - Parâmetros:
    - q: ponteiro para a struct message_queue
    - msg: ponteiro para struct message
*/
void queue_push(message_queue_t *q, message_t msg);

/*
  - função para remover uma mensagem à fila
  - Parâmetros:
    - q: ponteiro para a struct message_queue
*/
message_t queue_pop(message_queue_t *q);


/*
  - função para thread consumidora que faz o broadcast
*/
void *broadcast_thread(void *arg);


// FUNÇÕES DO SERVIDOR

/*
  - Inicia o servidor
*/
void server_start();

/*
  - função executada por cada thread para lidar com um cliente
  - recebe mensagens do cliente e inicia o broadcast
  - Parâmetros:
    - cliente: ponteiro para a struct cliente
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