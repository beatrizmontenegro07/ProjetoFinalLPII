#include "libtslog.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>

// Variáveis globais
FILE *log_file = NULL; // indica o arquivo onde será registrado os logs
pthread_mutex_t log_mutex; //mutex para garantir exclusão mútua ao acessar o arquivo de logs

int tslog_init(const char *filename) {
  log_file = fopen(filename, "w");
  if (log_file == NULL) {
    printf("Erro ao abrir o arquivo.");
    return -1;
  }

  // Inicializa o mutex
  if (pthread_mutex_init(&log_mutex, NULL) != 0) {
    printf("Erro ao inicializar o mutex do log");
    fclose(log_file);
    return -1;
  }

  return 0;
}

void tslog_log(LogLevel level, const char *message) {
  if (log_file == NULL) {
    fprintf(stderr, "Logger ainda não foi inicializado.\n");
    return;
  }

  // trava o mutex para garantir a exclusão mútua na seção crítica
  pthread_mutex_lock(&log_mutex);

  // obtem o horário e a data atual 
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  char time_str[20];
  strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t);

  // mapeia o nível do log para o formato string
  char *level_str;
  switch (level) {
    case INFO: level_str = "INFO"; break;
    case WARNING: level_str = "WARNING"; break;
    case ERROR: level_str = "ERROR"; break;
    default: level_str = "UNKNOWN"; break;
  }

  // escreve no arquivo no formato [<DATA>] [<NÍVEL>] - <MENSSAGEM>
  fprintf(log_file, "[%s] [%s] - %s\n", time_str, level_str, message);
  
  // Garante que a mensagem seja escrita imediatamente no disco
  fflush(log_file);

  // libera o mutex
  pthread_mutex_unlock(&log_mutex);
}

void tslog_destroy() {
  if (log_file != NULL) {
    // trava o mutex para garantir que ao fexharo arquivo ainda não tenha nenhuma operação ainda sendo executada
    pthread_mutex_lock(&log_mutex);
    fclose(log_file);
    log_file = NULL;
    pthread_mutex_unlock(&log_mutex);

    pthread_mutex_destroy(&log_mutex); // destrói o mutex
  }
}