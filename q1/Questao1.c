#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Struct que define o Pixel
typedef struct {
    int red;
    int green;
    int blue;
} Pixel;

int      n_linhas, n_colunas;
Pixel  **imagem;
// Função que converte cada Pixel em Cinza
void *converter_para_cinza(void *arg) {
    int linha = *(int *)arg;
    for (int j = 0; j < n_colunas; j++) {
        int cinza = (int)
        (imagem[linha][j].red   * 0.30 +
        imagem[linha][j].green * 0.59 +
        imagem[linha][j].blue  * 0.11);

        imagem[linha][j].red   = cinza;
        imagem[linha][j].green = cinza;
        imagem[linha][j].blue  = cinza;
    }   
    free(arg);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    char PPM[3];
    int  max_valor;
    FILE *entrada = fopen("Questao1_Entrada.ppm", "r");
    FILE *saida   = fopen("saida.ppm",   "w");

    if (!entrada) { perror("Erro ao abrir Questao1_Entrada.ppm"); return 1; }
    if (!saida)   { perror("Erro ao abrir saida.ppm");   return 1; }
    
    fscanf(entrada, "%s", PPM);
    fscanf(entrada, "%d %d", &n_colunas, &n_linhas);
    fscanf(entrada, "%d", &max_valor);  
    
    // Alocação da imagem no tamanho pixel x n_linhas
    imagem = malloc(n_linhas * sizeof(Pixel *));
    for (int i = 0; i < n_linhas; i++){
        imagem[i] = malloc(n_colunas * sizeof(Pixel));}

    // Leitura das entradas da imagem
    for (int i = 0; i < n_linhas; i++){
        for (int j = 0; j < n_colunas; j++)
            fscanf(entrada, "%d %d %d",
                   &imagem[i][j].red,
                   &imagem[i][j].green,
                   &imagem[i][j].blue);}
    // Criação das Threads para cada linha, 1 thread por linha
    pthread_t *threads = malloc(n_linhas * sizeof(pthread_t));
    for (int i = 0; i < n_linhas; i++) {
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&threads[i], NULL, converter_para_cinza, id);
    }
    // Espera todas Threads terminarem
    for (int i = 0; i < n_linhas; i++){
        pthread_join(threads[i], NULL);}
    free(threads);
    // Print do resultado das operações 

    fprintf(saida, "P3\n%d %d\n%d\n", n_colunas, n_linhas, max_valor);
    for (int i = 0; i < n_linhas; i++)
        for (int j = 0; j < n_colunas; j++)
            fprintf(saida, "%d %d %d\n",
                    imagem[i][j].red,
                    imagem[i][j].green,
                    imagem[i][j].blue);

    // Liberação da memória alocada para a matriz
    for (int i = 0; i < n_linhas; i++) {free(imagem[i]);}
    free(imagem);

    fclose(entrada);
    fclose(saida);

    printf("Feito!\n");
    return 0;
}

/*

O arquivo entrada.ppm pode ser alterado

1.copie exatamente essa parte e rode no terminal:

gcc Questao1.c -o conversor -lpthread
./conversor
cat saida.ppm


*/