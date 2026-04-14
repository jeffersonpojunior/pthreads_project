// Compilar: gcc -pthread q4.c -o q4
// Executar: ./q4 <C> <T>   (C = cabines, T = carros)

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int C;            // número de cabines
int *cabine_livre;
int total_livres;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond   = PTHREAD_COND_INITIALIZER;

void *carro(void *arg) {
    int id = *((int *)arg);
    free(arg);

    printf("Carro %d chegou ao pedágio\n", id);

    pthread_mutex_lock(&mutex);

    if (total_livres == 0)
        printf("Carro %d aguardando cabine livre\n\n", id);

    // wakeups
    while (total_livres == 0)
        pthread_cond_wait(&cond, &mutex); // libera o mutex e dorme ate ser sinalizado

    // ocupa a primeira cabine livre
    int cabine = -1;
    for (int i = 0; i < C; i++) {
        if (cabine_livre[i]) {
            cabine = i + 1;
            cabine_livre[i] = 0;
            total_livres--;
            break;
        }
    }

    pthread_mutex_unlock(&mutex);

    printf("Carro %d usando cabine %d\n\n", id, cabine);
    sleep(1); // tempo de pagamento

    pthread_mutex_lock(&mutex);

    cabine_livre[cabine - 1] = 1;
    total_livres++;
    printf("Carro %d terminou pagamento e liberou cabine %d\n", id, cabine);
    pthread_cond_signal(&cond); // acorda um carro em espera

    pthread_mutex_unlock(&mutex);

    printf("Carro %d seguiu viagem\n\n", id);

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s <C> <T>\n", argv[0]);
        return 1;
    }

    C = atoi(argv[1]);
    int T = atoi(argv[2]);

    cabine_livre = malloc(C * sizeof(int));
    for (int i = 0; i < C; i++) {
        cabine_livre[i] = 1;
    }
    total_livres = C;

    pthread_t threads[T];
    for (int i = 0; i < T; i++) {
        int *id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&threads[i], NULL, carro, id);
    }

    for (int i = 0; i < T; i++) {
        pthread_join(threads[i], NULL);
    }

    free(cabine_livre);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}