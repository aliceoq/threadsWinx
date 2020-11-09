#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define n1 15 //tamanho de s1
#define n2 2  //tamanho de s2
#define p 5   //quantidade de threads

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
long qtSub = 0;
char s1[n1+5], s2[n2+5];

void *contaSubstring(void * pos){
    long inicio = (long)pos;
    long tam = n1/p + inicio;
    long cont = 0; //cada thread tem um contador local para calcular a 
                   //quantidade de ocorrências de s2 na sua determinada substring de s1

    //para contar a quantidade de substrings, o primeiro loop indica 
    //qual vai ser a primeira letra a ser conferida com a string s2
    for(long i = inicio ; i < tam ; i++){
        if(s1[i] == s2[0] && i+n2-1 < tam){
            for(long j = 1 ; j < n2 ; j ++){
                if(s1[i+j] != s2[j]) break;
                if(j == n2 - 1) cont++; 
                //caso tenha chegado no final da interação do segundo loop, 
                //significa que todas as letras são iguais, fazendo com que 
                //exista uma ocorrência de s2 em s1 começando da posição i, 
                //então incrementamos no contador local
            }
        }
    }

    //ao checar todas as ocorrências de s2 na substring determinada, 
    //a thread aciona o mutex para poder adicionar a quantidade 
    //calculada localmente no contador global
    pthread_mutex_lock(&mutex);
    qtSub += cont;
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}

int main(void) {
    printf("Digite s1 (%d digitos): ", n1);
    scanf("%s", s1);
    printf("Digite s2 (%d digitos): ", n2);
    scanf("%s", s2);

    pthread_t *thread = malloc (p*sizeof(pthread_t));
    pthread_attr_t attr;
    void *status;
    int rc = 0;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    //Criação das threads
    long tam = n1/p;
    for (long i = 0 ; i < p ; i++) { 
        //o índice de cada thread vai ser a posição que ela irá começar a checar na string  
        long par = i*tam;
        rc = pthread_create(&thread[i], &attr, contaSubstring, (void *)par);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }
    
    //Espera todas as threads acabarem para poder dar o resultado
    pthread_attr_destroy(&attr);
    for (int i = 0 ; i < p ; i++) {
        rc = pthread_join(thread[i], &status);
        if (rc) {
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            exit(-1);
        }
    }
    
    printf("Quantidade de substrings: %ld\n", qtSub);
    pthread_exit(NULL);
    free(thread);
    return 0;
}
