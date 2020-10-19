#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int *ans;
int **matriz;

void *transforma(void * i){
    long index = (long)i;
    ans[index] = matriz[index][0]*0.30 + matriz[index][1]*0.59 + matriz[index][2]*0.11;
    pthread_exit(NULL);
}

int main(void) {
    char arquivo[2];
    int colunas, linhas, qtdPixel, max;

    FILE *file = fopen("in.ppm", "r");
    fscanf(file, "%s", arquivo);
    fscanf(file, "%d", &colunas);
    fscanf(file, "%d", &linhas);
    fscanf(file, "%d", &max);
    qtdPixel = colunas*linhas;

    matriz = (int **) malloc(qtdPixel*sizeof(int *));
    for (int i = 0 ; i < qtdPixel ; i++) matriz[i] = (int *) malloc (3*sizeof(int));

    ans = (int *) malloc(qtdPixel*sizeof(int *));

    for (int i = 0 ; i < qtdPixel ; i++)
        for (int j = 0 ; j < 3 ; j++)
            fscanf(file, "%d", &matriz[i][j]);

    fclose(file);

    pthread_t *thread = malloc (qtdPixel*sizeof(pthread_t));
    pthread_attr_t attr;
    void *status;
    int rc = 0;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for (long i = 0 ; i < qtdPixel ; i++)
    {   
        //printf("criando thread #%ld\n", i);
        rc = pthread_create(&thread[i], &attr, transforma, (void *)i);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }
    
    pthread_attr_destroy(&attr);
    for (int i = 0 ; i < qtdPixel ; i++) {
        rc = pthread_join(thread[i], &status);
        if (rc) {
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            exit(-1);
        }
    }
    
    file = fopen("out.ppm", "w");
    fprintf(file, "%s\n%d %d\n%d\n", arquivo, colunas, linhas, max);
    for (int i = 0 ; i < qtdPixel ; i++) {
        for (int j = 0 ; j < 3 ; j++)
            fprintf(file, "%3d ", ans[i]);
        fprintf(file, "\n");
    }
    
    pthread_exit(NULL);
    return 0;
}
