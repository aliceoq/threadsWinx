#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

#define N 5 // representa a quantidade de processadores ou núcleos do sistema
pthread_t threads[N];

typedef struct parametro {
    int p1;
    int p2;
    int id;
}Parametro;

typedef struct requisicao {
    void *func;
    Parametro p;
}Requisicao;

// ------------------- DECLARAÇÃO DE VARIAVEIS ETC -----------------------
Requisicao *bufferRequisicoes;
int sizeRequisicoes = 0;
int *bufferResultados;
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
    parameters.id = ultimoReq;
    Requisicao req;
    req.p = parameters;
    req.func = funexec;
    bufferRequisicoes[ultimoReq] = req;
    sizeRequisicoes++; ultimoReq++;
    // if (ultimo == sizeMaxRequisicoes) ~~
    if (sizeRequisicoes == 1) pthread_cond_broadcast(&newRequisicao);
    pthread_mutex_unlock(&mutex);
    return parameters.id;
}

void *funexec1(void *parameters)    // vai retornar inteiro
{
    Parametro *p = (Parametro *) parameters;
    printf("Id %d entrando na funexec1\n", p->id);
    int ans = p->p1 + p->p2;
    bufferResultados[p->id] = ans;
    sizeResultados++;
    if (sizeResultados == 1) pthread_cond_signal(&newResultado);
    pthread_exit(NULL);
}

int pegarResultadoExecucao(int id)
{
    pthread_mutex_lock(&mutex);
    printf("Pegando resultado...\n");
    int ans = 0;
    while (sizeResultados == 0) pthread_cond_wait(&newResultado, &mutex);
    if (bufferResultados[id] != 0) {
        ans = bufferResultados[id];
        bufferResultados[id] = 0;
        sizeResultados--;
    }
    return ans;
    pthread_mutex_unlock(&mutex);
}

// ------------------- FUNÇÕES AUXILIARES -------------------

void *despacha() {
    pthread_mutex_lock(&mutex);
    printf("Despachando...\n");

    while(sizeRequisicoes == 0) pthread_cond_wait(&newRequisicao, &mutex);

    for (int i = 0; i < N; i++) {
  	    int rc = pthread_create(&threads[i], NULL, bufferRequisicoes[primeiroReq].func, (void *)&bufferRequisicoes[primeiroReq].p); 
        sizeRequisicoes--; primeiroReq++;
        // if (primeiro == tamMax) 
        if (sizeRequisicoes == 0) break; // ????
    }

    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

int main(void) {
    bufferRequisicoes = (Requisicao *) calloc (100, sizeof(Requisicao));
    bufferResultados = (int *) calloc (100, sizeof(int));
    pthread_t despachante;
    Parametro p;
    p.p1 = 5;
    p.p2 = 3;

    int id = agendarExecucao((void *)funexec1, p);
    id = agendarExecucao((void *)funexec1, p);
    id = agendarExecucao((void *)funexec1, p);
    int rc = pthread_create(&despachante, NULL, despacha, NULL);
    
    int ans = pegarResultadoExecucao(id);
    if (ans == 0) printf("O id %d não terminou ainda.\n", id);
    else printf("Resultado de %d: %d\n", id, ans);


    pthread_exit(NULL);
}
