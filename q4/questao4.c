#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/time.h>
#define TAM_MAX 100
#define N 2 // representa a quantidade de processadores ou núcleos do sistema
pthread_t threads[N];

// Parametro para as funexec
typedef struct parametro {
    int p1;
    int p2;
    int id;
}Parametro;

// Struct pra salvar cada requisicao no buffer
typedef struct requisicao {
    void *func;
    Parametro p;
}Requisicao;

// ------------------- DECLARAÇÃO DE VARIAVEIS ETC -------------------
Requisicao *bufferRequisicoes;
int *bufferResultados;
int sizeRequisicoes = 0;
int sizeResultados = 0;
int primeiroReq = 0;
int ultimoReq = 0;
pthread_cond_t newRequisicao = PTHREAD_COND_INITIALIZER;
pthread_cond_t newResultado = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// ------------------- FUNÇÕES PEDIDAS PELA QUESTÃO -------------------
int agendarExecucao(void *funexec, Parametro parameters)
{
    pthread_mutex_lock(&mutex);
    printf("\e[0;101m Agendando requisicao...\e[0m\n");
    parameters.id = ultimoReq;
    Requisicao req;
    req.p = parameters;
    req.func = funexec;
    bufferRequisicoes[ultimoReq] = req;
    sizeRequisicoes++; ultimoReq++;
    if (ultimoReq == TAM_MAX) ultimoReq = 0;
    if (sizeRequisicoes == 1) pthread_cond_broadcast(&newRequisicao);
    pthread_mutex_unlock(&mutex);
    return parameters.id;
}

int pegarResultadoExecucao(int id)
{
    pthread_mutex_lock(&mutex);
    printf("\e[0;101m Pegando resultado do id %d...\e[0m\n", id);
    int ans = 0;
    // se n tiver resultado nenhum ou se o resultado que eu quero não chegou, vou ficar esperando
    while (sizeResultados == 0 || (sizeResultados > 0 && bufferResultados[id] == 0)) {
        printf("Esperando novo resultado...\n");
        pthread_cond_wait(&newResultado, &mutex);
        // recebi um sinal, mas toda funcao vai emitir o sinal, entao pode nao ser do id que eu quero
        if (sizeResultados > 0 && bufferResultados[id] == 0)
            printf("Pegou sinal de resultado, mas ainda não achou. O id tem %d e o tamanho é %d\n", bufferResultados[id], sizeResultados);
    }
    ans = bufferResultados[id];
    bufferResultados[id] = 0;
    sizeResultados--;
    pthread_mutex_unlock(&mutex);
    return ans;
}

// ------------------- FUNÇÕES PARA EXECUTAR PEDIDAS PELA QUESTÃO -------------------
void *funexec1(void *parameters)
{
    pthread_mutex_lock(&mutex);
    Parametro *p = (Parametro *) parameters;
    printf("id %d entrando na funexec1\n", p->id);
    int ans = p->p1 + p->p2;

    // salva a resposta no buffer e incrementa a qntd de resultados
    bufferResultados[p->id] = ans; 
    sizeResultados++;
    pthread_cond_signal(&newResultado); // emite o sinal pra o pegarResultadoExecucao olhar se foi o resultado que ele quer
    printf("Emitiu sinal de novo resultado.\n");
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

void *funexec2(void *parameters)
{
    pthread_mutex_lock(&mutex);
    Parametro *p = (Parametro *) parameters;
    printf("id %d entrando na funexec2\n", p->id);
    int ans = p->p1 * p->p2;
    bufferResultados[p->id] = ans;
    sizeResultados++;
    pthread_cond_signal(&newResultado);
    printf("Emitiu sinal de novo resultado.\n");
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

// ------------------- FUNÇÕES AUXILIARES -------------------

void *despacha() {
    pthread_mutex_lock(&mutex);
    printf("\e[0;101m Despachando...\e[0m\n");
    
    while(sizeRequisicoes == 0) pthread_cond_wait(&newRequisicao, &mutex); // se n tem requisicao fica esperando infinitamente (pq seria esperando a primeira requisicao e tem q ter pelo menos 1 ne o___o)
    while (sizeRequisicoes > 0) {
        for (int i = 0; i < N && bufferRequisicoes[primeiroReq].func != NULL; i++) {
            // atribui a requisicao pra uma da threads
            int rc = pthread_create(&threads[i], NULL, bufferRequisicoes[primeiroReq].func, (void *)&bufferRequisicoes[primeiroReq].p); 
            printf("Despachou %d\n", primeiroReq);
            bufferRequisicoes[primeiroReq].func = NULL;
            sizeRequisicoes--; primeiroReq++;
            if (primeiroReq == TAM_MAX) primeiroReq = 0;
            if (sizeRequisicoes == 0) break;
        }
        struct timespec timeToWait;
        struct timeval now;
        int rt;
        gettimeofday(&now,NULL);
        timeToWait.tv_sec = now.tv_sec+2;
        timeToWait.tv_nsec = (now.tv_usec+1000UL)*1000UL;
        // aqui eu coloco o tempo que tem que esperar, pra nao ficar infinitamente esperando uma requisicao quando ja tiver chegado no fim do programa
        if (sizeRequisicoes == 0) pthread_cond_timedwait(&newRequisicao, &mutex, &timeToWait);
    }
    pthread_mutex_unlock(&mutex);
    printf("Despacha morreu\n");
    pthread_exit((void *)1);
}

int main(void) {
    bufferRequisicoes = (Requisicao *) calloc (100, sizeof(Requisicao));
    bufferResultados = (int *) calloc (100, sizeof(int));

    pthread_t despachante;
    Parametro p;
    p.p1 = 5;
    p.p2 = 3;
    // CHAMA PRIMEIRO A THREAD QUE DESPACHA, ELA FICA ATÉ 5 SEGUNDOS ESPERANDO REQUISICAO
    int rc = pthread_create(&despachante, NULL, despacha, NULL);

    // AGENDA E PEGA RESULTADO DO ID 0
    // resultado esperado = 8    
    int id = agendarExecucao((void *)funexec1, p);

    int ans = pegarResultadoExecucao(id);
    printf("\e[0;105m Resultado de %d: %d \e[0m\n", id, ans);

    // SÓ AGENDA
    id = agendarExecucao((void *)funexec1, p);
    id = agendarExecucao((void *)funexec1, p);

    // AGENDA E PEGA RESULTADO DO ID 3
    // resultado esperado = 15
    id = agendarExecucao((void *)funexec2, p);
    ans = pegarResultadoExecucao(id);
    printf("\e[0;105m Resultado de %d: %d \e[0m\n", id, ans);

    // SÓ AGENDA
    id = agendarExecucao((void *)funexec1, p);

 
    for (int i = 0 ; i < N ; i++) {
        pthread_join(threads[i], NULL);
    }
    pthread_join(despachante, NULL);
    printf("Todas as threads acabaram.\n");
    
    free(bufferResultados);
    free(bufferRequisicoes);
    pthread_exit(NULL);
}
