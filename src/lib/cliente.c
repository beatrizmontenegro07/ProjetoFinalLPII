#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "cliente.h"
#include "common.h"
#include "libtslog.h"

volatile int flag = 1; // flag para controlar a execução do loop de envio/recebimento

int client_connect(const char *ip, int port) {
  int socket_fd;
  struct sockaddr_in server_address;

  // cria o socket
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    tslog_log(ERROR, "Falha ao criar o socket do cliente."); // caso der erro, registra no logger
    exit(EXIT_FAILURE);
  }

  // configura o endereço do servidor
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);
  if (inet_pton(AF_INET, ip, &server_address.sin_addr) <= 0) {
    tslog_log(ERROR, "Endereço IP inválido."); // caso der erro, registra no logger
    close(socket_fd);
    exit(EXIT_FAILURE);
  }

  // conecta o cliente ao servidor
  if (connect(socket_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
    tslog_log(ERROR, "Falha ao conectar o cliente ao servidor."); // caso der erro, registra no logger
    close(socket_fd);
    exit(EXIT_FAILURE);
  }

  return socket_fd;
}

void *receive_messages(void *arg) {
  int socket_fd = *((int*)arg);
  char buffer[BUFFER_SIZE];

  while (flag) {
    memset(buffer, 0, BUFFER_SIZE); //limpa o buffer antes da nova leitura
    int receive_len = recv(socket_fd, buffer, BUFFER_SIZE - 1, 0); // aguarda o envio da mensagem pelo servidor

    if (receive_len > 0) {
      printf("%s", buffer);
      /* buffer[receive_len] = '\0';
      tslog_log(INFO, buffer); */
    } else if (receive_len == 0) {
      flag = 0; // sinaliza para a outra thread encerrar
      break;
    } else {
      if (flag) {
        tslog_log(ERROR, "Erro ao receber dados (recv)");
      }
      break;
    }
  }
  return NULL;
}

void send_messages(int socket_fd) {
  char name[NAME_SIZE];
  char message[BUFFER_SIZE];

  // Pede para o cliente digitar seu nome
  printf("Digite seu nome: ");
  fgets(name, NAME_SIZE, stdin);
  name[strcspn(name, "\r\n")] = 0; // tratamento do input
  send(socket_fd, name, strlen(name), 0); 
  
  printf("Conectado! Digite suas mensagens. (0 para sair)\n");

  // Loop para ler input do usuário e enviar a mensagem
  while (flag) {
    fgets(message, BUFFER_SIZE, stdin);
    
    if (strcmp(message, "0\n") == 0) {
      flag = 0; 
      break;
    }

    if (send(socket_fd, message, strlen(message), 0) < 0) {
      tslog_log(ERROR, "Falha ao enviar mensagem");
      break;
    }
  }
}