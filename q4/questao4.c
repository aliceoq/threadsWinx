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
// sinal emitido quando houver uma nova requisicao, para a thread despachante ser acordada
pthread_cond_t newRequisicao = PTHREAD_COND_INITIALIZER;
// sinal emitido quando houver um novo resultado no buffer, para a thread que espera o resultado ser acordada
pthread_cond_t newResultado = PTHREAD_COND_INITIALIZER;
// mutex para tudo que for alterar as variaveis/buffers sobre os resultados
pthread_mutex_t mutexResultado = PTHREAD_MUTEX_INITIALIZER;
// mutex para tudo que for alterar as variaveis/buffers sobre as requisicoes
pthread_mutex_t mutexRequisicao = PTHREAD_MUTEX_INITIALIZER;

// ------------------- FUNÇÕES PEDIDAS PELA QUESTÃO -------------------
int agendarExecucao(void *funexec, Parametro parameters)
{
    pthread_mutex_lock(&mutexRequisicao);
    printf("\e[0;101m Agendando requisicao %d...\e[0m\n", ultimoReq);
    parameters.id = ultimoReq;
    Requisicao req;
    req.p = parameters;
    req.func = funexec;
    // coloca a requisicao no buffer
    bufferRequisicoes[ultimoReq] = req;
    sizeRequisicoes++; ultimoReq++;
    // se chegou no tamanho máximo, temos que voltar a colocar as requisicoes no inicio do buffer, então ultimoReq = 0
    if (ultimoReq == TAM_MAX) ultimoReq = 0;
    // se nao tinha nenhuma outra requisicao ele emite o sinal de uma nova requisicao para a thread despachante
    if (sizeRequisicoes == 1) pthread_cond_broadcast(&newRequisicao);
    pthread_mutex_unlock(&mutexRequisicao);
    return parameters.id;
}

int pegarResultadoExecucao(int id)
{
    pthread_mutex_lock(&mutexResultado);
    printf("\e[0;101m Tentando pegar o resultado do id %d...\e[0m\n", id);
    int ans = 0;
    // se nao tiver resultado ou se o resultado que eu quero não chegou, vou ficar esperando
    while (sizeResultados == 0 || (sizeResultados > 0 && bufferResultados[id] == 0)) {
        printf("Esperando novo resultado...\n");
        pthread_cond_wait(&newResultado, &mutexResultado);
        // se o sinal nao foi emitido pelo resultado que ele quer, a thread vai dormir de novo
    }
    // salvo a resposta e depois coloco 0 naquela posicao do buffer, esse vai ser o valor de quando não temos nada naquela posicao.
    ans = bufferResultados[id];
    bufferResultados[id] = 0;
    sizeResultados--;
    pthread_mutex_unlock(&mutexResultado);
    return ans;
}

// ------------------- FUNÇÕES PARA EXECUTAR PEDIDAS PELA QUESTÃO -------------------
void *funexec1(void *parameters)
{
    pthread_mutex_lock(&mutexResultado);
    Parametro *p = (Parametro *) parameters;
    int ans = p->p1 + p->p2;
    // Salva a resposta no buffer e incrementa a qntd de resultados
    bufferResultados[p->id] = ans; 
    sizeResultados++;
    pthread_cond_signal(&newResultado); // Emite o sinal pra o pegarResultadoExecucao olhar se foi o resultado que ele quer, precisa emitir toda vez porque o pegarResultadoExecucao quer um resultado específico, então não dá pra emitir só quando for o primeiro resultado no buffer vazio.
    pthread_mutex_unlock(&mutexResultado);
    pthread_exit(NULL);
}

void *funexec2(void *parameters)
{
    pthread_mutex_lock(&mutexResultado);
    Parametro *p = (Parametro *) parameters;
    int ans = p->p1 * p->p2;
    bufferResultados[p->id] = ans;
    sizeResultados++;
    pthread_cond_signal(&newResultado);
    pthread_mutex_unlock(&mutexResultado);
    pthread_exit(NULL);
}

// ------------------- FUNÇÕES AUXILIARES -------------------

void *despacha() {
    pthread_mutex_lock(&mutexRequisicao);
    printf("\e[0;101m Despachando...\e[0m\n");
    
    while(sizeRequisicoes == 0) pthread_cond_wait(&newRequisicao, &mutexRequisicao); // Caso não tenha requisicao ele vai ficar esperando até que tenha.
    while (sizeRequisicoes > 0) {
        for (int i = 0; i < N && bufferRequisicoes[primeiroReq].func != NULL; i++) {
            // Atribui a requisicao pra uma da threads
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
        // Aqui tem um tempo máximo de espera, pra nao ficar infinitamente esperando uma requisicao quando ja tiver chegado no fim do programa
        if (sizeRequisicoes == 0) pthread_cond_timedwait(&newRequisicao, &mutexRequisicao, &timeToWait);
    }
    pthread_mutex_unlock(&mutexRequisicao);
    printf("Fim da espera por requisicao.\n");
    pthread_exit((void *)1);
}

int main(void) {
    bufferRequisicoes = (Requisicao *) calloc (TAM_MAX, sizeof(Requisicao));
    bufferResultados = (int *) calloc (TAM_MAX, sizeof(int));

    pthread_t despachante;
    Parametro p;
    p.p1 = 5;
    p.p2 = 3;
    // CHAMA PRIMEIRO A THREAD QUE DESPACHA, ELA FICA ATÉ 5 SEGUNDOS ESPERANDO REQUISICAO
    int rc = pthread_create(&despachante, NULL, despacha, NULL);

    // como o problema não especifica como as requisicoes serão feitas, colocamos todas as requisicoes na main.

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
    p.p1 = 10;
    p.p2 = 3;
    id = agendarExecucao((void *)funexec2, p);
    ans = pegarResultadoExecucao(id);
    printf("\e[0;105m Resultado de %d: %d \e[0m\n", id, ans);
 
    // espera o fim de todas as threads
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
