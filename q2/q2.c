// Compilar: gcc -pthread q2.c -o q2
// Executar: ./q2 <L> <T> <arquivo_inicial> <arq_atualizacao1> [arq_atualizacao2 ...]

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_LINHAS   100
#define MAX_CONTEUDO 256
#define MAX_ARQUIVOS 200

// cores ANSI para o fundo de cada linha
#define RESET "\033[0m"
const char *CORES[] = {
    "\033[41m",   // vermelho
    "\033[43m",   // amarelo
    "\033[46m",   // ciano
    "\033[42m",   // verde
    "\033[44m",   // azul
    "\033[45m",   // magenta
    "\033[47;30m" // branco
};
#define NUM_CORES 7

int L;  // numero de linhas da tela
int N;  // numero de arquivos de atualizacao
char *arquivos[MAX_ARQUIVOS];

// um mutex por linha (exclusao mutua refinada)
pthread_mutex_t mutex_linha[MAX_LINHAS];

// mutex para pegar o proximo arquivo dinamicamente
pthread_mutex_t mutex_arquivo;
int proximo = 0;

void desenhar_linha(int linha, const char *conteudo) {
    const char *cor = CORES[(linha - 1) % NUM_CORES];
    // move cursor para a linha, apaga ela e imprime o conteudo colorido
    printf("\033[%d;1H\033[2K%s %-40s " RESET, linha, cor, conteudo);
    fflush(stdout);
}

void *rotina(void *arg) {
    int id = *(int *)arg;

    while (1) {
        // pega o proximo arquivo disponivel
        pthread_mutex_lock(&mutex_arquivo);
        if (proximo >= N) {
            pthread_mutex_unlock(&mutex_arquivo);
            break;
        }
        int idx = proximo++;
        pthread_mutex_unlock(&mutex_arquivo);

        FILE *f = fopen(arquivos[idx], "r");
        if (!f) {
            fprintf(stderr, "thread %d: erro ao abrir %s\n", id, arquivos[idx]);
            continue;
        }

        int num_linha;
        char conteudo[MAX_CONTEUDO];

        // cada iteracao le um par (numero da linha, novo conteudo)
        while (fscanf(f, " %d ", &num_linha) == 1) {
            if (!fgets(conteudo, sizeof(conteudo), f)) break;
            conteudo[strcspn(conteudo, "\n")] = '\0';

            if (num_linha < 1 || num_linha > L) continue;

            // trava apenas o mutex da linha que vai ser modificada
            pthread_mutex_lock(&mutex_linha[num_linha - 1]);
            desenhar_linha(num_linha, conteudo);
            sleep(2); // segura o mutex por 2s para a mudanca ficar visivel
            pthread_mutex_unlock(&mutex_linha[num_linha - 1]);
        }

        fclose(f);
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Uso: %s <L> <T> <arq_inicial> <arq_atualizacao1> ...\n", argv[0]);
        return 1;
    }

    L = atoi(argv[1]);
    int T = atoi(argv[2]);
    const char *arq_inicial = argv[3];

    N = argc - 4;
    for (int i = 0; i < N; i++)
        arquivos[i] = argv[4 + i];

    if (T < 2 || T > N) {
        fprintf(stderr, "T deve satisfazer 1 < T <= N (N=%d)\n", N);
        return 1;
    }

    // inicializa os mutexes
    for (int i = 0; i < L; i++)
        pthread_mutex_init(&mutex_linha[i], NULL);
    pthread_mutex_init(&mutex_arquivo, NULL);

    // limpa a tela e exibe o estado inicial
    printf("\033[2J\033[H");
    fflush(stdout);

    FILE *fi = fopen(arq_inicial, "r");
    if (!fi) {
        fprintf(stderr, "erro ao abrir %s\n", arq_inicial);
        return 1;
    }
    char buf[MAX_CONTEUDO];
    for (int i = 1; i <= L; i++) {
        if (!fgets(buf, sizeof(buf), fi)) break;
        buf[strcspn(buf, "\n")] = '\0';
        desenhar_linha(i, buf);
    }
    fclose(fi);

    // cria as threads
    pthread_t threads[T];
    int ids[T];
    for (int i = 0; i < T; i++) {
        ids[i] = i + 1;
        pthread_create(&threads[i], NULL, rotina, &ids[i]);
    }

    for (int i = 0; i < T; i++)
        pthread_join(threads[i], NULL);

    printf("\033[%d;1H\n", L + 2);
    fflush(stdout);

    for (int i = 0; i < L; i++)
        pthread_mutex_destroy(&mutex_linha[i]);
    pthread_mutex_destroy(&mutex_arquivo);

    return 0;
}