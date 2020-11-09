#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

int numArquivos, numLinhas;
//guarda QUANTOS arquivos ficam com cada thread
int *numF=NULL;
//guarda QUAIS arquivos ficam com cada thread
int **threadsF=NULL;
char **placa;
pthread_mutex_t *mutex;
pthread_mutex_t print = PTHREAD_MUTEX_INITIALIZER;
//essas strings farão a tela mudar de cor
char colors[8][10] = {"\e[41m", "\e[43m", "\e[44m", "\e[45m", "\e[42m", "\e[40m", "\e[46m", "\e[47m"};
char reset[10] = "\e[0m";

//a função que printa a placa da maneira correta
void printa(){
  system("clear");
  for(int i=0; i<numLinhas; i++){
  printf("%s" "%s" "%s\n", colors[i], placa[i], reset);
  }
}
//função que será passada para as threads, para que cada uma possa fazer a leitura de seus arquivos
void *func(void *id){
  char arquivo[9]="arq0.txt";
  char indice;
  int linha;
  char modif[300];
  int tid = *((int *)id); 
  for (int i=0;i<numF[tid];i++){
    if(threadsF[tid][i]){
      //aqui nós alteramos a string arquivo e colocamos o valor correto no caracter 3, por exemplo: a thread x vai ler o arquivo 1, logo, "arq0.txt" se torna "arq1.txt"
      indice = threadsF[tid][i]+'0';
      arquivo[3]=indice;
      FILE *file = fopen(arquivo,"r");
      if (file == NULL){
        printf("erro ao abrir arquivo %s com a thread %d\n", arquivo, tid);
      }
      while (!feof(file)){//aqui a thread lê o arquivo até que ele acabe
        fscanf(file,"%d",&linha); //salvamos a linha que deverá ser alterada
        pthread_mutex_lock(&mutex[linha-1]); //travamos o mutex daquela linha
        fscanf(file, " %[^\n]", modif); //salvamos a modificação que deve ser feita
        placa[linha-1]= (char *)realloc(placa[linha-1], (strlen(modif)+1)*sizeof(char));
        strcpy(placa[linha-1],modif); //fazemos a modificação
        threadsF[tid][i]=0; //aqui, estamos dizendo que, como o valor da matriz que fala qual arquivo fica com cada thread, será 0, significa que o arquivo já foi lido
        pthread_mutex_lock(&print);
        printa(); //printamos a placa
        pthread_mutex_unlock(&print);
        struct timespec timeToWait; 
        struct timeval now;
        gettimeofday(&now,NULL);
        timeToWait.tv_sec = now.tv_sec+2;
        do {
          gettimeofday(&now, NULL);
        } while (now.tv_sec != timeToWait.tv_sec); //esperamos o tempo necessário de 2 segundos para que a linha possa ser alterada novamente
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
    pthread_mutex_init(&mutex[i], NULL); //inicializamos os mutexes de cada linha
  }
  pthread_t threads[numThreads];
  int *taskids[numThreads];
  threadsF= (int **)malloc(numThreads*sizeof(int *));
  numF = (int *)calloc(numThreads, sizeof(int));
  i=0;
  int j=0;
  int repArq = 1;//numero que representa cada arquivo
  
  //define quais e quantos arquivos vão pra cada thread
  while(repArq<=numArquivos){
    threadsF[i]=(int *)realloc(threadsF[i],sizeof(int));
    threadsF[i][j] = repArq;
    numF[i]++;
    i++;
    if (i>=numThreads) {
      i=0; //aqui, se i já chegou ao final da linha da matriz, deverá voltar ao começo e passar para a proxima linha
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
  for(i=0;i<numThreads;i++){
    free(threadsF[i]);
  }
  free(threadsF);
  return 0;
}
