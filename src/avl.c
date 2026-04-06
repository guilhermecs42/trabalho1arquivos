// Cleyton Jose Rodrigues Macedo 16821725
// Guilherme Cavalcanti de Santana 15456556

/*
Essa árvore AVL foi escrita durante a disciplina de Algoritmos e Estruturas de Dados.
Portanto, o código aqui é genérico mas foi modificado para atender as necessidades do banco de dados.
As modificações são: ordenação da árvore (itens idênticos são aceitos, e são inseridos à esquerda 
de itens idênticos), existência duas funções para comparar nós ("ordenar" para inserir e 
"identificar" para remover), função "contar_distintos" (para atualizar contador do registro de cabeçalho).
*/

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../include/arvore.h"
#include "../include/definicoes.h"

typedef struct no NO;

struct no{
    void* item;
    NO* esquerda;
    NO* direita;
    unsigned char altura;
};

struct _abb{
    NO* raiz;
    int tam;
    const TABELA_FUNCOES* item_funcoes;
};


// FUNÇÕES AUXILIARES



static int max(int a, int b){
    return (a > b) ? a : b;
}

static int no_altura(NO* no){
    return (no == NULL) ? 0 : no->altura;
}

static int no_fator_balanceamento(NO* no){
    if(no == NULL) return 0;
    return no_altura(no->esquerda) - no_altura(no->direita);
}

static void no_atualizar_altura(NO* no){
    if(no != NULL){
        no->altura = 1 + max(no_altura(no->esquerda), no_altura(no->direita));
    }
}

// Rotações
static NO* rotacao_direita(NO* y){
    NO* x = y->esquerda;
    NO* T2 = x->direita;
    
    x->direita = y;
    y->esquerda = T2;
    
    no_atualizar_altura(y);
    no_atualizar_altura(x);
    
    return x;
}

static NO* rotacao_esquerda(NO* x){
    NO* y = x->direita;
    NO* T2 = y->esquerda;
    
    y->esquerda = x;
    x->direita = T2;
    
    no_atualizar_altura(x);
    no_atualizar_altura(y);
    
    return y;
}

// Balanceamento
static NO* balancear(NO* no){
    if(no == NULL) return NULL;
    
    no_atualizar_altura(no);
    int fb = no_fator_balanceamento(no);
    
    // Caso Esquerda-Esquerda
    if(fb > 1 && no_fator_balanceamento(no->esquerda) >= 0){
        return rotacao_direita(no);
    }
    
    // Caso Esquerda-Direita
    if(fb > 1 && no_fator_balanceamento(no->esquerda) < 0){
        no->esquerda = rotacao_esquerda(no->esquerda);
        return rotacao_direita(no);
    }
    
    // Caso Direita-Direita
    if(fb < -1 && no_fator_balanceamento(no->direita) <= 0){
        return rotacao_esquerda(no);
    }
    
    // Caso Direita-Esquerda
    if(fb < -1 && no_fator_balanceamento(no->direita) > 0){
        no->direita = rotacao_direita(no->direita);
        return rotacao_esquerda(no);
    }
    
    return no;
}

// Criar novo nó
static NO* no_criar(void* item){
    NO* no = (NO*)malloc(sizeof(NO));
    if(no != NULL){
        no->item = item;
        no->esquerda = NULL;
        no->direita = NULL;
        no->altura = 1;
    }
    return no;
}

// Inserção recursiva
static NO* no_inserir(NO* raiz, void* item, const TABELA_FUNCOES* funcoes){
    if(raiz == NULL){
        return no_criar(item);
    }
    
    if( funcoes->item_ordenar(item, raiz->item) <= 0 ){
        raiz->esquerda = no_inserir(raiz->esquerda, item, funcoes);
    }else{
        raiz->direita = no_inserir(raiz->direita, item, funcoes);
    }

    return balancear(raiz);
}

// Buscar nó com menor valor
static NO* no_minimo(NO* no){
    NO* atual = no;
    while(atual->esquerda != NULL){
        atual = atual->esquerda;
    }
    return atual;
}

// Remoção recursiva
static NO* no_remover(NO* raiz, void* item, const TABELA_FUNCOES* funcoes, bool* sucesso, int* cont_remocoes){
    if(raiz == NULL){
        *sucesso = false;
        return NULL;
    }
    
    if(funcoes->item_ordenar(item, raiz->item) > 0){
        raiz->direita = no_remover(raiz->direita, item, funcoes, sucesso, cont_remocoes);
    }else{
        if(funcoes->item_identificar(item, raiz->item) == false){
            raiz->esquerda = no_remover(raiz->esquerda, item, funcoes, sucesso, cont_remocoes);
        }else{
            *sucesso = true;
            // Nó encontrado
            
            // Caso 1: Nó folha ou com um filho
            if(raiz->esquerda == NULL || raiz->direita == NULL){
                NO* temp = raiz->esquerda ? raiz->esquerda : raiz->direita;
                
                if(temp == NULL){ // Nó folha
                    funcoes->item_apagar(&(raiz->item));
                	free(raiz);
                	return NULL;
                }else{ // Um filho
                    NO* filho = temp;
                
    		        // Apaga o PAI (o item atual)
    		        funcoes->item_apagar(&(raiz->item));
    		        free(raiz);
                
              	    // Retorna o FILHO para assumir o lugar do pai
             		return filho;
                }
            }else{
                // Caso 2: Nó com dois filhos
                NO* temp = no_minimo(raiz->direita); // esse é o menor nó maior que o atual
                
                // Troca o item (mas não apaga ainda)
                void* troca_item = raiz->item;
                raiz->item = temp->item;
                temp->item = troca_item;
                
                // Remove o nó sucessor
                raiz->direita = no_remover(raiz->direita, temp->item, funcoes, sucesso, cont_remocoes);
            }

            *cont_remocoes += 1;
        }
    }
    
    if(raiz == NULL) return NULL; // acho que essa linha é redundante, mas não apago por nada
    
    return balancear(raiz);
}

// Apagar todos os nós
static void no_apagar_recursivo(NO* raiz, const TABELA_FUNCOES* funcoes){
    if(raiz != NULL){
        no_apagar_recursivo(raiz->esquerda, funcoes);
        no_apagar_recursivo(raiz->direita, funcoes);
        funcoes->item_apagar(&(raiz->item));
        free(raiz);
    }
}

static void no_contar_distintos(NO* no, void** anterior, const TABELA_FUNCOES* funcoes, int* contador){
    if(no == NULL) return;

    no_contar_distintos(no->esquerda, anterior, funcoes, contador);

    int ordem = funcoes->item_ordenar(*anterior, no->item); // retorna 0 quando executa pela primeira vez, pois anterior é NULO
    *anterior = no->item;

    // usa-se item_ordenar pois o propósito de termos construído 
    // a AVL dessa forma é que podemos percorrer ela em ordem 
    // e comparar cada nó com o anterior para contarmos os distintos

    if(ordem < 0){ // Se o item atual e o anterior são distintos
   
        (*contador)++; // Incrementa o contador

    }else if(ordem > 0){

        printf("ERRO FATAL: A ESTRUTURA DA ÁRVORE ESTÁ CORROMPIDA.\n");
        exit(2);

    }

    no_contar_distintos(no->direita, anterior, funcoes, contador);

    return;
}

static bool abb_vazia(ABB* avl){
    if(avl == NULL) return true;
    return (avl->tam == 0);
}






// FUNÇÕES PÚBLICAS






ABB* abb_criar(const TABELA_FUNCOES* funcoes){
    if(funcoes == NULL) return NULL;
    
    ABB* avl = (ABB*)malloc(sizeof(ABB));
    if(avl != NULL){
        avl->raiz = NULL;
        avl->tam = 0;
        avl->item_funcoes = funcoes;
    }
    return avl;
}

void abb_apagar(ABB** avl){
    if(avl != NULL && *avl != NULL){
        no_apagar_recursivo((*avl)->raiz, (*avl)->item_funcoes);
        free(*avl);
        *avl = NULL;
    }
}

void abb_inserir(ABB* avl, void* item){
    if(avl == NULL || item == NULL) return;
    
    avl->raiz = no_inserir(avl->raiz, item, avl->item_funcoes);
    
    avl->tam++;
    
    return;
}

bool abb_remover(ABB* avl, void* chave){
    if(avl == NULL || chave == NULL) return false;

    bool sucesso;
    int cont_remocoes=0; 
    avl->raiz = no_remover(avl->raiz, chave, avl->item_funcoes, &sucesso, &cont_remocoes);
    
    avl->tam -= cont_remocoes;
    
    return sucesso;
}

int abb_contar_distintos(ABB* avl){
    if(avl == NULL || abb_vazia(avl)){
        DEBUG("DEBUG: A ÁRVORE É NULA OU VAZIA.\n");
        return 0;
    }

    int contador = 1; // Como a ABB tem pelo menos um elemento, pelo menos um é distinto
    void* anterior = NULL; // a função recursiva acessa essa variável "global".

    no_contar_distintos(avl->raiz, &anterior, avl->item_funcoes, &contador);

    return contador;
}