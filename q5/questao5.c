#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define I 2
#define maxP 100

pthread_barrier_t barrier;
long P = 10;
long N = 1;
double A[I][I] = {{2, 1},
                {5,7}};

double b[I] = {11, 13};
double respostas[maxP+1][I] = {};

void *calculaResult(void * pos){
    long inicio = (long)pos;

    //cada interação no primeiro laço é uma interação do método jacobi
    for(long k = 1 ; k <= P ; k++){
        //cada thread fica responsável pelas incógnitas de 
        //número congruente ao índice dela mod N
        //N sendo o número de threads
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
        //após o final de cada interação, é acionada uma barreira
        //que aguarda todas as N threads terminarem para que
        //todas possam seguir juntas para a próxima interação
        pthread_barrier_wait(&barrier);
    }

    pthread_exit(NULL);
}


int main() {
    printf("Matriz A:\n");
    for(long i = 0 ; i < I ; i++){
        for(long j = 0 ; j < I ; j++){
            printf("%2.0lf%c", A[i][j], j==I-1?'\n':' ');
        }
    }
    printf("Matriz B:\n");
    for(long i = 0 ; i < I ; i++){
      printf("%2.0lf%c", b[i], i==I-1?'\n':' ');
    }

    for(long i = 0 ; i < I ; i++) respostas[0][i] = 1;
    printf("Digite a quantidade de threads desejada: ");
    scanf("%ld", &N);

    printf("Digite a quantidade de interações desejada (max = 100): ");
    scanf("%ld", &P);

    pthread_t *thread = malloc (N*sizeof(pthread_t));
    pthread_attr_t attr;
    void *status;
    int rc = 0;

    pthread_attr_init(&attr);
    pthread_barrier_init(&barrier,NULL,N); 

    //Criação das threads
    for (long i = 0 ; i < N ; i++) {   
        rc = pthread_create(&thread[i], &attr, calculaResult, (void *)i);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    printf("Respostas encontradas:\n");
    for (long i = 0 ; i < I ; i++){
        printf("%lf%c", respostas[P][i], i==I-1?'\n':' ');
    }
    pthread_exit(NULL);
    free(thread);
    return 0;
}
