#include <stdio.h>
#include <pthread.h>
#include <unistd.h> 
#include "libtslog.h"

#define NUM_THREADS 5 // definição no número de threads
#define NUM_MESSAGES 10 // definição no número de menssagens por thread

// função que cada thread irá executar
void *writer_thread(void *arg) {
  long id = (long)arg;
  char message[100];

  for (int i = 0; i < NUM_MESSAGES; ++i) {
    sprintf(message, "Mensagem %d da Thread %ld", i + 1, id + 1);
    tslog_log(INFO, message); //escreve a menssagem no log
    usleep(100000); // simula o tempo de processamento
  }

  return NULL;
}

int main() {
  // inicializa o logger no arquivo "teste.log"
  if (tslog_init("logs/test.log") != 0) {
    return 1;
  }

  pthread_t threads[NUM_THREADS]; 

  printf("Iniciando %d threads...\n", NUM_THREADS);

  // Cria todas as threads, em cada uma irá executar a função writer_thread
  for (long i = 0; i < NUM_THREADS; ++i) {
    pthread_create(&threads[i], NULL, writer_thread, (void *)i);
  }

  // aguarda todas as threads finalizarem
  for (int i = 0; i < NUM_THREADS; ++i) {
      pthread_join(threads[i], NULL);
  }

  printf("Threads finalizadas.\n");

  // finaliza o logger
  tslog_destroy();

  return 0;
}