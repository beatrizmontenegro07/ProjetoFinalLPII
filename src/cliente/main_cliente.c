#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h> 
#include <unistd.h>

#include "common.h"  
#include "cliente.h" 

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Uso: %s <IP do Servidor>\n", argv[0]);
    return 1;
  }

  const char *server_ip = argv[1];

  // conecta o cliente ao servidor
  int socket_fd = client_connect(server_ip, SERVER_PORT);

  printf("Conexão com o servidor %s estabelecida com sucesso!\n", server_ip);

  // Inicia a thread para receber mensagens
  pthread_t recv_thread;
  pthread_create(&recv_thread, NULL, receive_messages, (void*)&socket_fd);

  // A thread principal envia mensagens 
  send_messages(socket_fd);

  printf("Desconectando...\n");
  shutdown(socket_fd, SHUT_RDWR);
  pthread_join(recv_thread, NULL);
  close(socket_fd);
  printf("Cliente encerrado com sucesso.\n");
  
  return 0;
}