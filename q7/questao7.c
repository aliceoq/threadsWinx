#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define NUM_THREADS 4

int *ans;       // vetor com os valores RGB na escada de cinza para cada pixel
int **matriz;   // matriz de pixels
int qtdPixel = 0;

// essa funcao eh a responsavel por deixar o pixel na escala de cinza;
// cada thread modifica apenas o pixel que possui o indice = indice da thread + NUM_THREADS*n, onde n vai de 0 até a quantidade de pixels que ela vai modificar menos um.
// ex: thread[0] modificando 3 pixels: matriz[0+4*0] + matriz[0+4*1] e matriz[0+4*2]
void *transforma(void * i){
    long index = (long)i;
    for (int i = index ; i < qtdPixel ; i += NUM_THREADS) {
        ans[i] = matriz[i][0]*0.30 + matriz[i][1]*0.59 + matriz[i][2]*0.11;
    }
    pthread_exit(NULL);
}

int main(void) {
    char arquivo[2];
    int colunas, linhas, max;

    // LEITURA DO ARQUIVO DE ENTRADA
    FILE *file = fopen("in.ppm", "r");
    if (file == NULL) {
        printf("ERRO AO LER ARQUIVO DE ENTRADA");
        return -1;
    }
    fscanf(file, "%s", arquivo);
    fscanf(file, "%d %d", &colunas,&linhas);
    fscanf(file, "%d", &max);
    qtdPixel = colunas*linhas;
   
    matriz = (int **) malloc(qtdPixel*sizeof(int *));
    for (int i = 0 ; i < qtdPixel ; i++) matriz[i] = (int *) malloc (3*sizeof(int));

    ans = (int *) malloc(qtdPixel*sizeof(int *));

    // SALVANDO OS VALORES DE RGB
    for (int i = 0 ; i < qtdPixel ; i++)
        for (int j = 0 ; j < 3 ; j++)
            fscanf(file, "%d", &matriz[i][j]);

    fclose(file);
    
    // criacao das threads
    pthread_t thread[NUM_THREADS];
    int rc = 0;

    for (long i = 0 ; i < NUM_THREADS ; i++) {   
        rc = pthread_create(&thread[i], NULL, transforma, (void *)i);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }
    
    // para garantir que todas as threads terminaram antes de imprimir o resultado
    for (int i = 0 ; i < NUM_THREADS ; i++) {
        rc = pthread_join(thread[i], NULL);
        if (rc) {
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            exit(-1);
        }
    }
    
    // criacao do arquivo de saida
    file = fopen("out.ppm", "w");
    fprintf(file, "%s\n%d %d\n%d\n", arquivo, colunas, linhas, max);
    for (int i = 0 ; i < qtdPixel ; i++) {
        for (int j = 0 ; j < 3 ; j++)
            fprintf(file, "%3d ", ans[i]);
        fprintf(file, "\n");
    }
    fclose(file);
    
    for (int i = 0 ; i < qtdPixel ; i++) free(matriz[i]);
    free(matriz);
    free(ans);
    pthread_exit(NULL);
    return 0;
}
