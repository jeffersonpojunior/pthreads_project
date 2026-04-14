#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// Struct do NO
struct node {
    int          key;
    int          value;
    struct node* next;
};
// Definindo o NO
void setNode(struct node* n, int key, int value) {
    n->key   = key;
    n->value = value;
    n->next  = NULL;
}

// Struct do HashMap 
struct hashMap {
    int              numOfElements;
    int              capacity;
    struct node**    arr;
    pthread_mutex_t* locks;
};
// Inicialização do HashMap
void initializeHashMap(struct hashMap* mp) {
    mp->capacity      = 10;
    mp->numOfElements = 0;

    mp->arr   = (struct node**)calloc(mp->capacity, sizeof(struct node*));
    mp->locks = (pthread_mutex_t*)malloc(mp->capacity * sizeof(pthread_mutex_t)); // Cada bucket inicializa com um mutex

    for (int i = 0; i < mp->capacity; i++)
        pthread_mutex_init(&mp->locks[i], NULL);
}

// Função Hash Modular
int hashFunction(struct hashMap* mp, int key) {
    return key % mp->capacity;
}

/* -------Operações------*/
void insert(struct hashMap* mp, int key, int value) {
    int bucketIndex = hashFunction(mp, key); // Calcula qual Bucket inserir
    // Região Crítica
    // Trava o bucket
    pthread_mutex_lock(&mp->locks[bucketIndex]);

    struct node* newNode = (struct node*)malloc(sizeof(struct node));
    setNode(newNode, key, value);

    if (mp->arr[bucketIndex] == NULL) {
        mp->arr[bucketIndex] = newNode; // Insere em 1 Bucket vazio
    } else {
        newNode->next = mp->arr[bucketIndex];
        mp->arr[bucketIndex] = newNode; // Insere no Topo do Bucket
    }
    //Destrava o Bucket
    pthread_mutex_unlock(&mp->locks[bucketIndex]);
}

void deleteKey(struct hashMap* mp, int key) {
    int bucketIndex = hashFunction(mp, key);
    // Região Crítica
    // Trava o bucket
    pthread_mutex_lock(&mp->locks[bucketIndex]);

    struct node* PreviousNode = NULL;
    struct node* currentNode = mp->arr[bucketIndex];

    while (currentNode != NULL) {
        if (key == currentNode->key) {
            if (currentNode == mp->arr[bucketIndex])
                mp->arr[bucketIndex] = currentNode->next; // Remove O topo 
            else
                PreviousNode->next = currentNode->next; // Remove o meio ou fim
            free(currentNode);
            break;
        }
        PreviousNode = currentNode;
        currentNode = currentNode->next;
    }
    //Destrava o Bucket
    pthread_mutex_unlock(&mp->locks[bucketIndex]);
}

int search(struct hashMap* mp, int key) {
    int bucketIndex = hashFunction(mp, key);
    // Região Crítica
    // Trava o bucket
    pthread_mutex_lock(&mp->locks[bucketIndex]);

    struct node* current = mp->arr[bucketIndex];
    while (current != NULL) {
        if (current->key == key) {
            int result = current->value;
            pthread_mutex_unlock(&mp->locks[bucketIndex]);
            return result;
        }
        current = current->next;
    }
    //Destrava o Bucket
    pthread_mutex_unlock(&mp->locks[bucketIndex]);
    return -1; 
}

// Struct para Thread
struct thread_data {

    struct hashMap* mp;
    int op,key,value;     

};

void* executar_operacao(void* arg) {
    struct thread_data* data = (struct thread_data*)arg;

    if (data->op == 1) {
        insert(data->mp, data->key, data->value);
        printf("[INSERIR] chave = %d valor = %d\n", data->key, data->value);
    } else if (data->op == 2) {
        deleteKey(data->mp, data->key);
        printf("[DELETAR] chave = %d\n", data->key);
    } else if (data->op == 3) {
        int result = search(data->mp, data->key);
        if (result == -1)
            printf("[BUSCAR] chave = %d → nao existe\n", data->key);
        else
            printf("[BUSCAR] chave = %d → valor = %d\n", data->key, result);
    }

    free(data);
    pthread_exit(NULL);
}

int main() {
    struct hashMap* mp = (struct hashMap*)malloc(sizeof(struct hashMap));
    initializeHashMap(mp);
    // Entrada do Arquivo
    FILE* arquivo = fopen("Questao3_Entrada.txt", "r");
    if (!arquivo) { perror("Erro ao abrir Questao3_Entrada.txt"); return 1; }

    pthread_t threads[2000]; // // Suporta até 2000 operações simultâneas
    int       t_count = 0;
    char      linha[100];

    // Operações do Arquivo
    while (fgets(linha, sizeof(linha), arquivo)) {
        struct thread_data* data = malloc(sizeof(struct thread_data));
        data->mp = mp;

        if (strstr(linha, "inserir")) {
            data->op = 1;
            sscanf(linha, "inserir(%d, %d)", &data->key, &data->value);

        } else if (strstr(linha, "deletar")) {
            data->op = 2;
            sscanf(linha, "deletar(%d)", &data->key);

        } else if (strstr(linha, "buscar")) {
            data->op = 3;
            sscanf(linha, "buscar(%d)", &data->key);

        } else {
            free(data);
            continue;
        }
        // Criação de cada Thread de acordo com a operação
        pthread_create(&threads[t_count++], NULL, executar_operacao, data);
    }
    fclose(arquivo);
        // Aguarda todas as threads terminarem antes de liberar memória
    for (int i = 0; i < t_count; i++)
        pthread_join(threads[i], NULL);

    printf("\nTodas as operacoes concluidas.\n");

    // Destrói os mutexes 
    for (int i = 0; i < mp->capacity; i++)
        pthread_mutex_destroy(&mp->locks[i]);

    free(mp->locks);
    free(mp->arr);
    free(mp);

    return 0;
}