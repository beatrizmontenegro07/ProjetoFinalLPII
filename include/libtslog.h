#ifndef LIBTSLOG_H
#define LIBTSLOG_H

#include <pthread.h>
#include <stdio.h>

// Enum para os níveis de log
typedef enum {
  INFO,
  WARNING,
  ERROR
} LogLevel;

/*
 - Inicializa o log
 - Parâmetros: 
    - file: Nome do arquivo que será salvo os logs
 - Return: 0 (sucesso) ou -1 (erro) 
*/
int tslog_init(const char *filename);


 /*
  - Escreve a mensagem de log no arquivo (thread-safe)
  - Parâmetros: 
    - level: indica o nível da mensagem do log (INFO, WARNING, ERROR)
    - message: menssagem a ser registrada
*/
void tslog_log(LogLevel level, const char *message);


 // Finaliza o logger, fechando o arquivo e destruindo o mutex
void tslog_destroy();

#endif 