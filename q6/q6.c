// Compilar: gcc -pthread q6.c -o q6
// Executar: ./q6 <profundidade> <elem1> <elem2> ...
// profundidade 0 = sequencial, P = paralelo ate nivel P

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int *arr; // vetor global compartilhado pelas threads

typedef struct {
    int l;
    int r;
    int prof;
} Args;

// declaracao antecipada (sort e sort_thread se chamam mutuamente)
void *sort_thread(void *arg);

void merge(int l, int m, int r) {
    int n = r - l + 1;
    int *tmp = malloc(n * sizeof(int));

    int i = l, j = m + 1, k = 0;
    while (i <= m && j <= r) {
        tmp[k++] = (arr[i] <= arr[j]) ? arr[i++] : arr[j++];
    }
    while (i <= m) {
        tmp[k++] = arr[i++];
    }
    while (j <= r) {
        tmp[k++] = arr[j++];
    }

    memcpy(arr + l, tmp, n * sizeof(int));
    free(tmp);
}

void sort(int l, int r, int prof) {
    printf("Ordenando [%d, %d]\n", l, r);
    if (l == r) {
        return;
    }

    int m = (l + r) / 2;

    if (prof > 0) {
        // cria uma thread para cada metade (intervalos disjuntos, por isso não há conflito)
        Args arg_esq = {l,     m, prof - 1};
        Args arg_dir = {m + 1, r, prof - 1};

        pthread_t t_esq, t_dir;
        pthread_create(&t_esq, NULL, sort_thread, &arg_esq);
        pthread_create(&t_dir, NULL, sort_thread, &arg_dir);

        // so faz o merge depois que as duas metades terminarem
        pthread_join(t_esq, NULL);
        pthread_join(t_dir, NULL);
    } 
    else {
        // profundidade esgotada, continua sequencial
        sort(l,     m, 0);
        sort(m + 1, r, 0);
    }

    merge(l, m, r);
    printf("Ordenado  [%d, %d]\n", l, r);
}

void *sort_thread(void *arg) {
    Args *a = (Args *)arg;
    sort(a->l, a->r, a->prof);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <profundidade> <elem1> <elem2> ...\n", argv[0]);
        return 1;
    }

    int prof = atoi(argv[1]);
    int n = argc - 2;

    arr = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        arr[i] = atoi(argv[2 + i]);
    }

    printf("Vetor inicial:  ");
    for (int i = 0; i < n; i++) printf("%d ", arr[i]);
    printf("\n\n");

    sort(0, n - 1, prof);

    printf("\nVetor ordenado: ");
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    free(arr);
    return 0;
}