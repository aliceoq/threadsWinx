#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define n1 15
#define n2 2
#define p 5

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
long qtSub = 0;
char s1[n1+5], s2[n2+5];

void *contaSubstring(void * pos){
    long inicio = (long)pos;
    long tam = n1/p + inicio;
    long cont = 0;
    for(long i = inicio ; i < tam ; i++){
        if(s1[i] == s2[0] && i+n2-1 < tam){
            for(long j = 1 ; j < n2 ; j ++){
                if(s1[i+j] != s2[j]) break;
                if(j == n2 - 1) cont++;
            }
        }
    }
    pthread_mutex_lock(&mutex);
    qtSub += cont;
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}

int main(void) {
    scanf("%s", s1);
    scanf("%s", s2);

    pthread_t *thread = malloc (p*sizeof(pthread_t));
    pthread_attr_t attr;
    void *status;
    int rc = 0;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    long tam = n1/p;
    for (long i = 0 ; i < p ; i++) {   
        long par = i*tam;
        rc = pthread_create(&thread[i], &attr, contaSubstring, (void *)par);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }
    
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
    return 0;
}
