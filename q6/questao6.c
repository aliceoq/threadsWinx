#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>

#define buffer_size 100
#define num_items 50
#define qtd_threads 5

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;


typedef struct elem{
   int value;
   struct elem *prox;
}Elem;
 
typedef struct blockingQueue{
   unsigned int sizeBuffer, statusBuffer;
   Elem *head,*last;
}BlockingQueue;

typedef struct{
    BlockingQueue *bq;
    int element;
}structAux;

//so criando a filinha
BlockingQueue *newBlockingQueue(unsigned int SizeBuffer){
    BlockingQueue *aux = NULL;
    aux = (BlockingQueue *) malloc(sizeof(BlockingQueue));
    aux->head = NULL;
    aux->last = NULL;
    aux->sizeBuffer = SizeBuffer;
    aux->statusBuffer = 0;
    return aux;
}
void putBlockingQueue(BlockingQueue *Q, int intnewValue){
    //bloqueio rapidao
    pthread_mutex_lock(&mutex);
    //vejo se o buffer ta cheio, se estiver nao posso colocar as coisas e fico esperando
    while(Q->statusBuffer==buffer_size){
        printf("Nao posso colocar...\n");
        pthread_cond_wait(&empty, &mutex);
    }
    //aqui eu posso colocar, aí so faço as coisas que guga ensinou
    Elem *aux = NULL;
    Elem *aux2;
    aux = (Elem*) malloc(sizeof(Elem));
    aux->value = intnewValue;
    aux->prox = NULL;
    if(Q->statusBuffer==0){
        (*Q).head = aux;
        (*Q).last = aux;
        Q->statusBuffer += 1;
    }else{
        aux2 = (*Q).last;
        aux2->prox = aux;
        (*Q).last = aux;
        Q->statusBuffer += 1;
    }
    printf("Adicionei: %d\n", intnewValue);
    //aqui eu vejo se o buffer tem pelo menos 1 item, se tiver eu posso ir tirando
    if(Q->statusBuffer>0){
        pthread_cond_broadcast(&fill);
    }
    //desbloqueio apenas
    pthread_mutex_unlock(&mutex);
}
void *producer(void* arg){
    structAux *my_queue;
    my_queue = (structAux *) arg;
    int i;
    printf("Produtor\n");
    //vou de produçao
    for(i=0; i<num_items; i++){
        putBlockingQueue(my_queue->bq, i+my_queue[0].element);
    }
    printf("Produtor terminou\n");
    pthread_exit(NULL);
}
int takeBlockingQueue(BlockingQueue* Q){
    int result;
    //vou de block
    pthread_mutex_lock(&mutex);
    while(Q->statusBuffer==0){
      //se o buffer tiver vazio eu n posso tirar as coisas, ent tem q esperar
        printf("Nao posso tirar...\n");
        pthread_cond_wait(&fill, &mutex);
    }
    //qnd o buffer deixa de ser 0 eu posso pegar as coisas
    Elem *aux;
    aux = (*Q).head;
    (*Q).head = aux->prox;
    result = aux->value;
    if(Q->statusBuffer == 1){
        (*Q).last = NULL;
    }
    Q->statusBuffer -= 1;
    free(aux);
    if(Q->statusBuffer<Q->sizeBuffer){
        pthread_cond_broadcast(&empty);
    }
    pthread_mutex_unlock(&mutex);
    return result;
}
void *consumer(void* arg){
    structAux *my_queue;
    my_queue = (structAux *) arg;
    int i, take;
    //indo de consumo
    for(i=0; i<num_items; i++){
        take = takeBlockingQueue(my_queue->bq);
        printf("Removi %d\n", take);
    }
    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    long i;
    //se perguntarem pq fiz struct... tambem nao sei, mas eu uso um negocio dela dps
    structAux structQueue[1];
    //minha fila ta na struc ent eu crio ela
    structQueue->bq = newBlockingQueue(buffer_size);
    pthread_t prod[qtd_threads];
    pthread_t cons[qtd_threads];
    int ids[qtd_threads];

    pthread_mutex_init(&mutex, NULL);    
    //crio as threads
    for(i=0; i<qtd_threads; i++){
        structQueue[0].element = i;
        pthread_create(&(prod[i]), NULL, producer, (void*) &structQueue[0]);
    }
    
    for(i=0; i<qtd_threads; i++){
        structQueue[0].element = i;
        pthread_create(&(cons[i]), NULL, consumer, (void*) &structQueue[0]);
    }

    for(i=0; i<qtd_threads; i++)
        pthread_join(prod[i], NULL);

    for(i=0; i<qtd_threads; i++)
        pthread_join(cons[i], NULL);

    pthread_exit(NULL);
}
