#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// Como o pthread_create só passa um argumento, crio uma struct que pode passar mais
typedef struct {
    int id_thread;
    int total_threads; // Tamanho do passo da thread
    int tamanho_matriz;
    int soma_esperada;
    int **matriz; // Ponteiro para a matriz quadrada
    int *eh_magico; // Ponteiro para flag
} DadosThread;



// Função que verifica se a soma está correta
void* verificar_soma (void *arg) {
    DadosThread* dados = (DadosThread*) arg;
    int M = dados->tamanho_matriz;
    int total_tarefas = 2 * M + 2;

    for (int i = dados->id_thread; i < total_tarefas; i += dados->total_threads) {

        // Se alguma thread já descobriu que não é mágico, eu paro de gastar CPU.
        if (*(dados->eh_magico) == 0) {
            break;
        }

        int soma_atual = 0;

        // Passando pelas linhas
        if (i < M) {
            int linha = i;
            for (int j = 0; j < M; j++) {
                soma_atual += dados->matriz[linha][j];
            }
        }

        // Passando pelas colunas
        else if (i < 2 * M) {
            int coluna = i - M;
            for (int j = 0; j < M; j++) {
                soma_atual += dados->matriz[j][coluna];
            }
        }

        // Passando pela diagonal principal
        else if (i == 2 * M) {
            for (int j = 0; j < M; j++) {
                soma_atual += dados->matriz[j][j];
            }
        }

        // Passando pela diagonal secundária
        else {
            for (int j = 0; j < M; j++) {
                soma_atual += dados->matriz[j][M - 1 - j];
            }
        }


        // Comparando o valor da soma
        if (soma_atual != dados->soma_esperada) {
            *(dados->eh_magico) = 0;
            break;
        }
    }

    return NULL;
}






int main() {

    // Declarando a matriz e o tamanho dela
    int tamanho;
    int soma = 0;
    int quadrado_eh_magico = 1;
    scanf("%d", &tamanho);
    int **matriz = (int **)malloc(tamanho * sizeof(int *));
    for (int i = 0; i < tamanho; i++) {
        matriz[i] = (int *)malloc(tamanho * sizeof(int));
    }

    printf("Digite os valores da matriz linha por linha:\n");
    for (int i = 0; i < tamanho; i++) {
        for (int j = 0; j < tamanho; j++) {
            scanf("%d", &matriz[i][j]);
        }
    }

    // Encontrando a soma de referência da primeira linha
    for (int i = 0; i < tamanho; i++) {
        soma += matriz[0][i];
    }

    // Criando as threads
    int N = 4;
    pthread_t threads[N];
    DadosThread dados[N];

    // Laço de criação das threads
    for (int i = 0; i < N; i++) {
        dados[i].id_thread = i;
        dados[i].total_threads = N;
        dados[i].tamanho_matriz = tamanho;
        dados[i].soma_esperada = soma;
        dados[i].matriz = matriz;
        dados[i].eh_magico = &quadrado_eh_magico;

        if (pthread_create(&threads[i], NULL, verificar_soma, (void*)&dados[i]) != 0) {
            printf("Erro: Falha ao criar a thread %d\n", i);
            return 1;
        }
    }

    // Laço de sincronização das threads
    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }

    // Resultado final
    if (quadrado_eh_magico) {
        printf("O quadrado é mágico\n");
    }
    else {
        printf("O quadrado NÃO é mágico\n");
    }
    
    // Liberação da memória alocada dinamicamente
    for (int i = 0; i < tamanho; i++) {
        free(matriz[i]);
    }
    free(matriz);

    return 0;
}