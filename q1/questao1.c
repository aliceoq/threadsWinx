#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#define c 1000000

int cont = 0; //declarando um contador global para ser incrementado até o valor c (1000000)
pthread_mutex_t mymutex = PTHREAD_MUTEX_INITIALIZER;

//essa é a função que faz o incremento
void *inc(void *id){
  //o id da thread é passado como parametro para sabermos qual thread atingiu o valor
  int tid = *((int *)id); 
  while(cont < c){
    //travando o mutex para que toda a operação seja feita e não ocorram erros pelo escalonamento
    pthread_mutex_lock(&mymutex);
    cont++;
    if (cont == c){
      //se o contador atinge o valor c, é printada a thread que estava executando
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
    //esse mutex é para garantir que sempre que a main for criar uma thread, ela vai passar pelo if, para saber se o valor ja foi atingido, pq se ja foi, a main termina e não cria mais nenhuma thread
    int continua = 1;
    pthread_mutex_lock(&mymutex);
    if (cont >= c){
      continua = 0;
    }
    pthread_mutex_unlock(&mymutex);
    if(continua == 0){
      break;
    }
    //aqui ocorre a criação das threads junto com seus respectivos ids
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
