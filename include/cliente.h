#ifndef CLIENT_H
#define CLIENT_H

/*
  - Conecta o cliente ao servidor
  - Parâmetros:
    - ip: edenreço ip do servidor
    - port: porta do servidor
  - Return: descritor de arquivo do socket ou -1 (em caso de erro)
*/
int client_connect(const char *ip, int port);

/*
  - Inicia uma thread para receber mensagens do servidor
  - Parâmetros:
    - arg: ponteiro para o descritor de arquivo do socket
*/
void *receive_messages(void *arg);

/*
  - envia a mensagem para o servidor
  - Parâmetros:
    - socket_fd: descritor de arquivo do socket
*/ 
void send_messages(int socket_fd);

#endif 