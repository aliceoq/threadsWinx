#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define I 2
#define P 10 

pthread_barrier_t barrier;
long N = 1;
double A[I][I] = {{2, 1},
                {5,7}};

double b[I] = {11, 13};
double respostas[P+1][I] = {};

void *calculaResult(void * pos){
    long inicio = (long)pos;

    for(long k = 1 ; k <= P ; k++){
        for(long i = inicio ; i < I ; i += N){
            double x = 0;
            for(long j = 0 ; j < I ; j++){
                if(j == i) continue;
                x += A[i][j]*respostas[k-1][j];
            }
            x = b[i] - x;
            x /= A[i][i];
            respostas[k][i] = x;
            //printf("interação: %ld -> x[%ld] = %lf\n", k, i, x);
        }

        pthread_barrier_wait(&barrier);
    }

    pthread_exit(NULL);
}


int main() {
    for(long i = 0 ; i < I ; i++) respostas[0][i] = 1;

    scanf("%ld", &N);

    pthread_t *thread = malloc (N*sizeof(pthread_t));
    pthread_attr_t attr;
    void *status;
    int rc = 0;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_barrier_init(&barrier,NULL,N); 

    for (long i = 0 ; i < N ; i++) {   
        rc = pthread_create(&thread[i], &attr, calculaResult, (void *)i);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    pthread_attr_destroy(&attr);
    for (long i = 0 ; i < N ; i++) {
        rc = pthread_join(thread[i], &status);
        if (rc) {
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            exit(-1);
        }
    }

    for (long i = 0 ; i < I ; i++){
        printf("%lf%c", respostas[P][i], i==I-1?'\n':' ');
    }

    return 0;
}
