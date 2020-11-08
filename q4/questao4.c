#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/time.h>
#define TAM_MAX 5
#define N 2 // Representa a quantidade de processadores ou núcleos do sistema

pthread_t threads[N];
// Parametros para as funexec
typedef struct parametro {
    int p1;
    int p2;
    int idRequisicao;
    int idThread;
}Parametro;

// Struct pra salvar cada requisicao no buffer
typedef struct requisicao {
    void *func;
    Parametro p;
}Requisicao;

// ------------------- DECLARAÇÃO DE VARIAVEIS ETC -------------------
Requisicao *bufferRequisicoes;
int *bufferResultados;
int threadOcupada[N] = {0};
int sizeRequisicoes = 0;
int sizeResultados = 0;
int primeiroReq = 0; // index para a prox posicao do buffer que vai ser despachada
int ultimoReq = 0; // index para a prox posicao do buffer a ser preenchida

pthread_cond_t newRequisicao = PTHREAD_COND_INITIALIZER; // sinal emitido quando houver uma nova requisicao, para a thread despachante ser acordada
pthread_cond_t newResultado = PTHREAD_COND_INITIALIZER; // sinal emitido quando houver um novo resultado no buffer, para a thread que espera o resultado ser acordada
pthread_mutex_t mutexResultado = PTHREAD_MUTEX_INITIALIZER; // mutex para tudo que for alterar as variaveis/buffers sobre os resultados
pthread_mutex_t mutexRequisicao = PTHREAD_MUTEX_INITIALIZER; // mutex para tudo que for alterar as variaveis/buffers sobre as requisicoes

// ------------------- FUNÇÕES PEDIDAS PELA QUESTÃO -------------------
int agendarExecucao(void *funexec, Parametro parameters)
{
    pthread_mutex_lock(&mutexRequisicao);
    Requisicao req; req.p = parameters; req.func = funexec;

    printf("\e[0;101m Agendando requisicao %d...\e[0m\n", ultimoReq);
    req.p.idRequisicao = ultimoReq;
    bufferRequisicoes[ultimoReq] = req; // coloca a requisicao no buffer
    sizeRequisicoes++; ultimoReq++;
    if (ultimoReq == TAM_MAX) ultimoReq = 0; // se chegou no tamanho máximo, temos que voltar a colocar as requisicoes no inicio do buffer, então ultimoReq = 0
    if (sizeRequisicoes == 1) pthread_cond_broadcast(&newRequisicao); // se nao tinha nenhuma outra requisicao ele emite o sinal de uma nova requisicao para a thread despachante
    pthread_mutex_unlock(&mutexRequisicao);

    return req.p.idRequisicao;
}

int pegarResultadoExecucao(int id)
{
    pthread_mutex_lock(&mutexResultado);
    int ans = 0;
    while (sizeResultados == 0 || (sizeResultados > 0 && bufferResultados[id] == 0)) {
        // Se o bufferResultados está vazio ou se o resultado que eu quero não chegou, vamos esperar o sinal newResultado até chegar o resultado.
        pthread_cond_wait(&newResultado, &mutexResultado);
    }
    // salvo a resposta e depois coloco 0 naquela posicao do buffer, esse vai ser o valor de quando não temos nada naquela posicao.
    printf("\e[44m Pegando o resultado do id %d...\e[0m\n", id);
    ans = bufferResultados[id];
    bufferResultados[id] = 0;
    sizeResultados--;
    pthread_mutex_unlock(&mutexResultado);
    return ans;
}

// ------------------- FUNÇÕES PARA EXECUTAR PEDIDAS PELA QUESTÃO -------------------
void *funexec1(void *parameters)
{
    Parametro *p = (Parametro *) parameters;
    int ans = p->p1 + p->p2;
    
    // Isso é só pra levar mais tempo pra concluir e ter uma diferença de tempo entre as duas funexec
    for (int i = 0 ; i < 100000000 ; i++) ans++;
    for (int i = 0 ; i < 100000000 ; i++) ans--;
    for (int i = 0 ; i < 100000000 ; i++) ans++;

    pthread_mutex_lock(&mutexResultado);
    bufferResultados[p->idRequisicao] = ans; // Salva a resposta no buffer
    sizeResultados++;
    pthread_cond_signal(&newResultado); // Emite o sinal para o pegarResultadoExecucao, precisa emitir toda vez porque o pegarResultadoExecucao quer um resultado específico, então não dá pra emitir só quando for o primeiro resultado no buffer vazio.
    pthread_mutex_unlock(&mutexResultado);

    threadOcupada[p->idThread] = 0; // Thread agora está desocupada e pode ser usada pra outra operação
    printf("funexec1 emitiu novo resultado, thread %d está livre.\n", p->idThread);
    pthread_exit(NULL);
}

void *funexec2(void *parameters)
{
    Parametro *p = (Parametro *) parameters;
    int ans = p->p1 * p->p2;

    pthread_mutex_lock(&mutexResultado);
    bufferResultados[p->idRequisicao] = ans;
    sizeResultados++;
    pthread_cond_signal(&newResultado);
    pthread_mutex_unlock(&mutexResultado);

    threadOcupada[p->idThread] = 0;
    printf("funexec2 emitiu novo resultado, thread %d está livre.\n", p->idThread);
    pthread_exit(NULL);
}

// ------------------- FUNÇÕES AUXILIARES -------------------

void *despacha() {
    pthread_mutex_lock(&mutexRequisicao);
    while (sizeRequisicoes == 0) pthread_cond_wait(&newRequisicao, &mutexRequisicao); // Caso não tenha requisicao ele vai ficar esperando até que tenha.
    while (sizeRequisicoes > 0) {
        for (int i = 0; i < N && bufferRequisicoes[primeiroReq].func != NULL; i++) {
            if (threadOcupada[i] == 0) { // Se a thread atual estiver desocupada, vamos atribuir a requisicao pra ela.
                threadOcupada[i] = 1;
                bufferRequisicoes[primeiroReq].p.idThread = i; // atualiza o id da thread que vai ficar responsável por aquela execução
                pthread_create(&threads[i], NULL, bufferRequisicoes[primeiroReq].func, (void *)&bufferRequisicoes[primeiroReq].p); // cria a thread que vai executar uma das funexec 
            }
            else continue;
            printf("Despachou %d\n", primeiroReq);
            bufferRequisicoes[primeiroReq].func = NULL;
            sizeRequisicoes--; primeiroReq++;

            if (primeiroReq == TAM_MAX) primeiroReq = 0; // Se a última requisicao foi na ultima posicao, agora vamos começar a colocar no inicio do buffer novamente.
            if (sizeRequisicoes == 0) break;
        }
        struct timespec timeToWait;
        struct timeval now;
        gettimeofday(&now,NULL);
        timeToWait.tv_sec = now.tv_sec+5;
        if (sizeRequisicoes == 0) pthread_cond_timedwait(&newRequisicao, &mutexRequisicao, &timeToWait); // Aqui tem um tempo máximo de espera, pra nao ficar infinitamente esperando uma requisicao quando ja tiver chegado no fim do programa.
    }
    pthread_mutex_unlock(&mutexRequisicao);
    printf("Fim da espera por requisicao.\n");
    pthread_exit(NULL);
}

int main(void) {
    bufferRequisicoes = (Requisicao *) calloc (TAM_MAX, sizeof(Requisicao));
    bufferResultados = (int *) calloc (TAM_MAX, sizeof(int));

    pthread_t despachante;
    pthread_create(&despachante, NULL, despacha, NULL); // Cria a thread despachante
    
    // Entrada, primeiros vamos agendar todas as requisicoes e depois pegar todos os resultados em ordem
    int ids[10]; 
    for (int i = 0 ; i < 5 ; i++) {
        Parametro p; p.p1 = i + 1; p.p2 = (i+1)*2;
        if (i%2 == 1) ids[i] = agendarExecucao((void *)funexec1, p);
        else ids[i] = agendarExecucao((void *)funexec2, p);
    }
    for (int i = 0 ; i < 5 ; i++) {
        int ans = pegarResultadoExecucao(ids[i]);
        printf("\e[0;105m Resultado de %d: %d \e[0m\n", ids[i], ans);
    }
    
    /* Outra opcao de entrada, agenda e imediatamente depois pega o resultado da requisicao q foi agendada
    for (int i = 0 ; i < 10 ; i++) {
        Parametro p; p.p1 = i; p.p2 = i+2;
        if (i%2 == 1) ids[i] = agendarExecucao((void *)funexec1, p);
        else ids[i] = agendarExecucao((void *)funexec2, p);
        int ans = pegarResultadoExecucao(ids[i]);
        printf("\e[0;105m Resultado de %d: %d \e[0m\n", ids[i], ans);
    }
    */

    // Espera o fim de todas as threads
    for (int i = 0 ; i < N ; i++) {
        pthread_join(threads[i], NULL);
    }
    pthread_join(despachante, NULL);
    printf("Todas as threads acabaram.\n");
    
    free(bufferResultados);
    free(bufferRequisicoes);
    pthread_exit(NULL);
    return 0;
}
