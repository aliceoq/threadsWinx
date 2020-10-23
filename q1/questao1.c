#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#define c 1000000

int cont = 0;
pthread_mutex_t mymutex = PTHREAD_MUTEX_INITIALIZER;

void *inc(void *id){
  int tid = *((int *)id); 
  while(cont < c){
    pthread_mutex_lock(&mymutex);
    cont++;
    if (cont == c){
      printf("A thread %d atingiu o valor %d :)\n", tid, cont);
    }
    pthread_mutex_unlock(&mymutex);
  }
  pthread_exit(NULL);
}

int main(void) {
  
  int n;
  int rc;

  printf("Escolha quantas threads: ");
  scanf("%d", &n);

  pthread_t threads[n];
  int *taskids[n];
  
  for(int t = 0; t < n; t++){
    //esse mutex eh p garantir que sempre q a main for criar uma thread, ela vai passar pelo if, p saber se o valor ja foi atingido, pq se ja foi, a main termina e n cria mais nenhuma thread
    int continua = 1;
    pthread_mutex_lock(&mymutex);
    if (cont >= c){
      continua = 0;
    }
    pthread_mutex_unlock(&mymutex);
    if(continua == 0){
      break;
    }
    taskids[t] = (int *) malloc(sizeof(int));
    *taskids[t] = t;
    rc = pthread_create(&threads[t], NULL, inc, (void *)taskids[t]);
    if (rc){
      printf("erro");
      exit(-1);
    }
  }
  pthread_exit(NULL);
  return 0;
}
