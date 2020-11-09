 
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

//função para criar a fila bloqueante
BlockingQueue *newBlockingQueue(unsigned int SizeBuffer){
    BlockingQueue *aux = NULL;
    aux = (BlockingQueue *) malloc(sizeof(BlockingQueue));
    aux->head = NULL;
    aux->last = NULL;
    aux->sizeBuffer = SizeBuffer;
    aux->statusBuffer = 0;
    return aux;
}
//Colocando elementos na fila
void putBlockingQueue(BlockingQueue *Q, int intnewValue){
    //bloqueio o mutex
    pthread_mutex_lock(&mutex);
    //vejo se o buffer ta cheio. se ele estiver, nao posso colocar inserir elementos na fila, então fico esperando
    while(Q->statusBuffer==buffer_size){
        printf("Nao posso colocar...\n");
        pthread_cond_wait(&empty, &mutex);
    }
    //quando chega aqui, já posso inserir elementos
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
    //aqui eu vejo se o buffer tem pelo menos 1 item, se tiver, eu sinalizo com o fill para saber que eu posso retirar elementos da fila
    if(Q->statusBuffer>0){
        pthread_cond_broadcast(&fill);
    }
    //desbloqueio o mutex
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

//função para remover elementos da fila bloqueante
int takeBlockingQueue(BlockingQueue* Q){
    int result;
    //bloqueio o mutex
    pthread_mutex_lock(&mutex);

	//Vejo se o buffer está vazio, se sim, eu espero ser colocado um item pra prosseguir 
    while(Q->statusBuffer==0){
        printf("Nao posso tirar...\n");
        pthread_cond_wait(&fill, &mutex);
    }
    //quando chega aqui, significa que eu tenho algum item para retirar e então eu o retiro
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
	//ponteiro de estrutura, onde possuo a fila bloqueante e o elemento que vou inserir na fila
    structAux structQueue[1];
    
	//criando a fila bloqueante
    structQueue->bq = newBlockingQueue(buffer_size);

	//criando as threads consuidoras e produtoras
    pthread_t prod[qtd_threads];
    pthread_t cons[qtd_threads];
    int ids[qtd_threads];
	
	//inicializo o mutex
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

