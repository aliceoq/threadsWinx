#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

int numArquivos, numLinhas;
//guarda quantos arquivos ficam com cada thread
int *numF=NULL;
//guarda quais arquivos ficam com cada thread
int **threadsF=NULL;
char **placa;
pthread_mutex_t *mutex;
pthread_mutex_t print = PTHREAD_MUTEX_INITIALIZER;
char colors[8][10] = {"\e[41m", "\e[43m", "\e[44m", "\e[45m", "\e[42m", "\e[40m", "\e[46m", "\e[47m"};
char reset[10] = "\e[0m";

clock_t *before;

void *func(void *id){
  char arquivo[9]="arq0.txt";
  char indice;
  int linha;
  char modif[300];
  int tid = *((int *)id); 
  for (int i=0;i<numF[tid];i++){
    if(threadsF[tid][i]){
      indice = threadsF[tid][i]+'0';
      arquivo[3]=indice;
      FILE *file = fopen(arquivo,"r");
      if (file == NULL){
        printf("erro ao abrir arquivo %s com a thread %d\n", arquivo, tid);
      }
      while (!feof(file)){
        fscanf(file,"%d",&linha);
        pthread_mutex_lock(&mutex[linha-1]);
        fscanf(file, " %[^\n]", modif);
        placa[linha-1]= (char *)realloc(placa[linha-1], (strlen(modif)+1)*sizeof(char));
        strcpy(placa[linha-1],modif);
        threadsF[tid][i]=0;
        before[linha-1]=clock();
        system("clear");
        for(int i=0; i<numLinhas; i++){
          printf("%s" "%s" "%s\n", colors[i], placa[i], reset);
        }
        printf("\n");
        while(clock()-before[linha-1]<2000){
        }
        pthread_mutex_unlock(&mutex[linha-1]);
      }
      fclose(file);
    }
  }
  pthread_exit(NULL);
}
int main(){
  int numThreads;
  int i = 0, rc;
  printf("Digite quantos arquivos serão lidos: ");
  scanf("%d", &numArquivos);
  printf("Digite quantas threads serão usadas: ");
  scanf("%d", &numThreads);
  printf("Digite quantas linhas tem a placa: ");
  scanf("%d", &numLinhas);
  placa = (char **)malloc((numLinhas+1)*sizeof(char *));
  mutex = (pthread_mutex_t *)malloc((numLinhas+1)*sizeof(pthread_mutex_t));
  for(i = 0; i<=numLinhas;i++){
    pthread_mutex_init(&mutex[i], NULL);
  }
  pthread_t threads[numThreads];
  int *taskids[numThreads];
  threadsF= (int **)malloc(numThreads*sizeof(int *));
  numF = (int *)calloc(numThreads, sizeof(int));
  before = (clock_t *)malloc(numLinhas*sizeof(clock_t));
  i=0;
  int j=0;
  //numero q representa cada arquivo
  int repArq = 1;
  //define quais e quantos arquivos vão pra cada thread
  while(repArq<=numArquivos){
    threadsF[i]=(int *)realloc(threadsF[i],sizeof(int));
    threadsF[i][j] = repArq;
    numF[i]++;
    i++;
    if (i>=numThreads) {
      i=0;
      j++;
    }
    repArq++;
  }
  for (i=0;i<numThreads;i++){
    taskids[i] = (int *) malloc(sizeof(int));
    *taskids[i] = i;
    rc = pthread_create(&threads[i], NULL, func, (void *)taskids[i]);
    if (rc){
      printf("erro ao criar thread\n");
      exit(-1);
    }
  }
  pthread_exit(NULL);
  free(threads);
  free(taskids);
  free(mutex);
  free(numF);
  free(placa);
  free(before);
  for(i=0;i<numThreads;i++){
    free(threadsF[i]);
  }
  free(threadsF);
  return 0;
}
