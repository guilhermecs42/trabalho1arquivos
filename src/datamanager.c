#include "../include/datamanager.h"
#include "../include/tabelafuncoes.h"
#include "../include/arvore.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

// EXPLICAR POR QUE OPTAMOS POR ÁRVORE BINÁRIA

/*
Temos duas estruturas de dados para contabilizar eficientemente quantas estações
diferentes existem e quantos pares (codEstacao, proxEstacao) diferentes existem.

Cada estrutura de dados utiliza funções customizadas para ordenar, identificar 
e apagar o tipo de item para os quais cada nó aponta.

Esse arquivo define esses dois tipos de itens e as funções aplicáveis a cada um.
*/



static ABB* nroEstacoesTracker = NULL;
static ABB* nroParesEstacaoTracker = NULL;



typedef struct {
    int codEstacao; // Necessário para identificar o nó na hora de remover
    char* nomeEstacao; // Critério para ordenar os nós
} nroEstacoesItem;

// Deve apagar o item e modificar a referência a ele para NULL
void nroEstacoesItem_apagar(void** item){
    if(item == NULL || *item == NULL) return;

    nroEstacoesItem** ptr = (nroEstacoesItem**) item; // Casting para acessar o membro da struct
    free((*ptr)->nomeEstacao); // apagar a string do nome da estação
    free(*ptr);
    *ptr = NULL; // evitar use-after-free
}

// Deve retornar <0 se o item1 deve aparecer antes do item2, 0 se tanto faz, >0 se o item2 deve aparecer antes do item1.
int nroEstacoesItem_ordenar(void* item1, void* item2){
    if(item1 == NULL || item2 == NULL){
        DEBUG("DEBUG: Uma das estações a serem ordenadas é nula.\n");
        return 0;
    }

    nroEstacoesItem* ptr1 = (nroEstacoesItem*) item1;
    nroEstacoesItem* ptr2 = (nroEstacoesItem*) item2;
    return strcmp(ptr1->nomeEstacao, ptr2->nomeEstacao);
}

// Deve retornar true se os dois itens têm a mesma chave
bool nroEstacoesItem_identificar(void* item1, void* item2){
    if(item1 == NULL || item2 == NULL){
        return false;
    }

    nroEstacoesItem* ptr1 = (nroEstacoesItem*) item1;
    nroEstacoesItem* ptr2 = (nroEstacoesItem*) item2;

    return (ptr1->codEstacao == ptr2->codEstacao);
}



typedef struct {
    int codEstacao; // Necessário para identificar o nó na hora de remover
    int codProxEstacao;
    // O par (codEstacao, codProxEstacao) é o critério para ordenar os nós
} nroParesEstacaoItem;

// Deve apagar o item e modificar a referência a ele para NULL
void nroParesEstacaoItem_apagar(void** item) {
    if(item == NULL || *item == NULL) return;

    free(*item);
    *item = NULL; // evitar use-after-free
}

// Deve retornar <0 se o item1 deve aparecer antes do item2, 0 se tanto faz, >0 se o item2 deve aparecer antes do item1.
int nroParesEstacaoItem_ordenar(void* item1, void* item2){
    if(item1 == NULL || item2 == NULL){
        DEBUG("DEBUG: Um dos pares de estação a serem ordenados é nulo.");
        return 0;
    }

    nroParesEstacaoItem* ptr1 = (nroParesEstacaoItem*) item1;
    nroParesEstacaoItem* ptr2 = (nroParesEstacaoItem*) item2;

    int criterio1 = ptr1->codEstacao - ptr2->codEstacao;
    if(criterio1 != 0) return criterio1;

    int criterio2 = ptr1->codProxEstacao - ptr2->codProxEstacao;
    return criterio2;
}

// Deve retornar true se os dois itens têm a mesma chave
bool nroParesEstacaoItem_identificar(void* item1, void* item2){
    if(item1 == NULL || item2 == NULL){
        return false;
    }

    nroParesEstacaoItem* ptr1 = (nroParesEstacaoItem*) item1;
    nroParesEstacaoItem* ptr2 = (nroParesEstacaoItem*) item2;
    
    return (ptr1->codEstacao == ptr2->codEstacao) && (ptr1->codProxEstacao == ptr2->codProxEstacao);
}



static const TABELA_FUNCOES nroEstacoesItemFuncoes = {
    nroEstacoesItem_apagar,
    nroEstacoesItem_ordenar,
    nroEstacoesItem_identificar,
};

static const TABELA_FUNCOES nroParesEstacaoItemFuncoes = {
    nroParesEstacaoItem_apagar,
    nroParesEstacaoItem_ordenar,
    nroParesEstacaoItem_identificar,
};





// FIM DAS DEFINIÇÕES DAS ESTRUTURAS DE DADOS







// INÍCIO DAS FUNÇÕES QUE GERENCIAM DISCO E ESTRUTURA DE DADOS

FILE* abre_binario(char* arquivoBin, char* modo){
    
    // Testando se o modo de abrir o arquivo é válido:
    if( strcmp(modo, "rb") != 0 && 
        strcmp(modo, "wb") != 0 &&
        strcmp(modo, "ab") != 0 &&
        strcmp(modo, "rb+") != 0 &&
        strcmp(modo, "wb+") != 0 &&
        strcmp(modo, "ab+"))
    {
        DEBUG("DEBUG: %s NÃO É UM MODO VÁLIDO. NÃO PÔDE ABRIR O ARQUIVO %s.\n", modo, arquivoBin);
    }

    // abre o arquivo bin em modo leitura
    FILE* filestream_bin = fopen(arquivoBin, modo);
    if(filestream_bin == NULL){ // se falhou
        DEBUG("DEBUG: ERRO AO ABRIR O BINÁRIO %s\n", arquivoBin);
        return NULL;
    }
    fseek(filestream_bin, 0, SEEK_SET);

    // Lê o status do arquivo
    unsigned char status;
    fread(&status, 1, 1, filestream_bin);
    if(status != '1'){
        DEBUG("DEBUG: ARQUIVO %s INCONSISTENTE. NÃO FOI POSSÍVEL ABRIR.\n", arquivoBin);
        fclose(filestream_bin);
        return NULL;
    }

    // Reescreve o status do arquivo
    fseek(filestream_bin, 0, SEEK_SET);
    status = '0';
    fwrite(&status, 1, 1, filestream_bin);
    fseek(filestream_bin, 0, SEEK_SET); // Voltando ao começo

    return filestream_bin;
}

void escreve_registro(REG_DADOS_STRUCT* registro_lido, FILE* filestream_bin){

    // ESCREVENDO O STRUCT NO BINÁRIO

    fwrite(&(registro_lido->removido), 1, 1, filestream_bin);
    fwrite(&(registro_lido->proximo), 1, 4, filestream_bin);
    fwrite(&(registro_lido->codEstacao), 1, 4, filestream_bin);
    fwrite(&(registro_lido->codLinha), 1, 4, filestream_bin);
    fwrite(&(registro_lido->codProxEstacao), 1, 4, filestream_bin);
    fwrite(&(registro_lido->distProxEstacao), 1, 4, filestream_bin);
    fwrite(&(registro_lido->codLinhaIntegra), 1, 4, filestream_bin);
    fwrite(&(registro_lido->codEstIntegra), 1, 4, filestream_bin);

    fwrite(&(registro_lido->tamNomeEstacao), 1, 4, filestream_bin); // armazena o tamanho do nome da estação
    if(registro_lido->tamNomeEstacao != 0){
        fwrite(registro_lido->nomeEstacao, 1, registro_lido->tamNomeEstacao, filestream_bin); // como tamNomeEstacao foi inicializado por strlen, o \0 no final não será escrito
    }

    fwrite(&(registro_lido->tamNomeLinha), 1, 4, filestream_bin); // armazena o tamanho do nome da linha
    if(registro_lido->tamNomeLinha != 0){
        fwrite(registro_lido->nomeLinha, 1, registro_lido->tamNomeLinha, filestream_bin); // como tamNomeLinha foi inicializado por strlen, o \0 no final não será escrito
    }

    int num_bytes_lixo = REG_DADOS_S - BYTES_FIXOS_S - registro_lido->tamNomeEstacao - registro_lido->tamNomeLinha; // Calcula o número de bytes a serem preenchidos com lixo
    char lixo = LIXO;
    for(int i = 0; i < num_bytes_lixo; i++){
        fwrite(&lixo, 1, 1, filestream_bin);
    }

    return;
}

void atualiza_registro(REG_DADOS_STRUCT *campos_novos, int mask, int RRN, FILE *filestream_bin){
    
    long int pos_reg = RRN * REG_DADOS_S + HEADER_S; // byte offset do registro atual
    
    // ATUALIZANDO CAMPOS INTEIROS

    /*
    - 1: codEstacao
    - 2: codLinha
    - 4: codProxEstacao
    - 8: distProxEstacao
    - 16: codLinhaIntegra
    - 32: codEstIntegra
    */

    if(mask & 1){
        fseek(filestream_bin, pos_reg + 5, SEEK_SET);
        fwrite(&(campos_novos->codEstacao), 4, 1, filestream_bin);
    }
    if(mask & 2){
        fseek(filestream_bin, pos_reg + 9, SEEK_SET);
        fwrite(&(campos_novos->codLinha), 4, 1, filestream_bin);
    }
    if(mask & 4){
        fseek(filestream_bin, pos_reg + 13, SEEK_SET);
        fwrite(&(campos_novos->codProxEstacao), 4, 1, filestream_bin);
    }
    if(mask & 8){
        fseek(filestream_bin, pos_reg + 17, SEEK_SET);
        fwrite(&(campos_novos->distProxEstacao), 4, 1, filestream_bin);
    }
    if(mask & 16){
        fseek(filestream_bin, pos_reg + 21, SEEK_SET);
        fwrite(&(campos_novos->codLinhaIntegra), 4, 1, filestream_bin);
    }
    if(mask & 32){
        fseek(filestream_bin, pos_reg + 25, SEEK_SET);
        fwrite(&(campos_novos->codEstIntegra), 4, 1, filestream_bin);
    }

    // ATUALIZANDO CAMPOS STRING

    /*
    - 64: nomeEst
    - 128: nomeLinha
    */

    int tamNomeEst, tamNomeLinha;
    char nomeEst[64], nomeLinha[64];

    // Transferir do disco para a memória os valores atuais:
    fseek(filestream_bin, pos_reg + 29, SEEK_SET);
    fread(&tamNomeEst, 4, 1, filestream_bin);
    fread(nomeEst, 1, tamNomeEst, filestream_bin);
    fread(&tamNomeLinha, 4, 1, filestream_bin);
    fread(nomeLinha, 1, tamNomeLinha, filestream_bin);

    DEBUG("DEBUG: LENDO NOME E LINHA ANTIGOS: %s\n%s\n", nomeEst, nomeLinha);

    // Atualizando os valores na memória
    if(mask & 64){
        strcpy(nomeEst, campos_novos->nomeEstacao);
        tamNomeEst = strlen(nomeEst);
    }
    if(mask & 128){
        strcpy(nomeLinha, campos_novos->nomeLinha);
        tamNomeLinha = strlen(nomeLinha);
    }

    // Transferindo os valores da memória para o disco
    fseek(filestream_bin, pos_reg + 29, SEEK_SET);
    fwrite(&tamNomeEst, 4, 1, filestream_bin);
    fwrite(nomeEst, 1, tamNomeEst, filestream_bin);
    fwrite(&tamNomeLinha, 4, 1, filestream_bin);
    fwrite(nomeLinha, 1, tamNomeLinha, filestream_bin);

    DEBUG("DEBUG: LENDO NOVO NOME E LINHA: %s\n%s\n", nomeEst, nomeLinha);

    // Preenchendo com lixo até o final
    int quantidade = REG_DADOS_S - 1 - 4*(CAMPOS_INT + CAMPOS_STRINGS) - tamNomeEst - tamNomeLinha;
    char *buffer_lixo = malloc(quantidade);
    memset(buffer_lixo, LIXO, quantidade); // Enche o buffer com a quantidade necessária de '$'
    DEBUG("DEBUG: FORAM ESCRITOS %d $.\n", quantidade);
    fwrite(buffer_lixo, 1, quantidade, filestream_bin);
    
    free(buffer_lixo);
    return;
}

/* Objetivo: verificar o RRN fornecido corresponde à busca realizada
    - Os valores dos campos buscados, as chaves da busca, estão no struct chave
    - Quais campos do struct são chaves está especificado em mask 
    - Retorna true se todos os campos, especificados por mask, do RRN fornecido são iguais aos campos equivalentes do struct chave
*/
bool check_registro(REG_DADOS_STRUCT* chave, int mask, int RRN, FILE* bin){
    fseek(bin, RRN * REG_DADOS_S + HEADER_S, SEEK_SET);
    
    // Verificando se o registro está removido e decidindo se deve continuar:
    unsigned char removido;
    fread(&removido, 1, 1, bin);
    if(removido == '1') return false;

    // Pular o campo 'proximo'
    fseek(bin, 4, SEEK_CUR);

    // Ler todos os campos em ordem
    int cEst, cLin, cProx, dist, cLinInt, cEstInt, tNomeE, tNomeL;
    
    fread(&cEst, 4, 1, bin);
    fread(&cLin, 4, 1, bin);
    fread(&cProx, 4, 1, bin);
    fread(&dist, 4, 1, bin);
    fread(&cLinInt, 4, 1, bin);
    fread(&cEstInt, 4, 1, bin);

    // Verificação de inteiros usando bitwise AND
    if((mask & 1) && chave->codEstacao != cEst) return false;
    if((mask & 2) && chave->codLinha != cLin) return false;
    if((mask & 4) && chave->codProxEstacao != cProx) return false;
    if((mask & 8) && chave->distProxEstacao != dist) return false;
    if((mask & 16) && chave->codLinhaIntegra != cLinInt) return false;
    if((mask & 32) && chave->codEstIntegra != cEstInt) return false;

    // Verificação de Strings
    // Nome Estação
    fread(&tNomeE, 4, 1, bin);
    if(mask & 64){
        if(tNomeE > 0){
            char temp[100];
            fread(temp, 1, tNomeE, bin);
            temp[tNomeE] = '\0';
            // Se a busca não é nula e o arquivo tem dado, compara
            if(chave->nomeEstacao != NULL){
                if(strcmp(chave->nomeEstacao, temp) != 0) return false;
            }else{
                // Buscando por NULO mas encontrou dado no arquivo
                return false;
            }
        }else{
            // Campo no arquivo é NULO
            if(chave->nomeEstacao != NULL) return false;
        }
    }else{
        fseek(bin, tNomeE, SEEK_CUR);
    }

    // Nome Linha
    fread(&tNomeL, 4, 1, bin);
    if(mask & 128){
        if(tNomeL > 0){
            char temp[100];
            fread(temp, 1, tNomeL, bin);
            temp[tNomeL] = '\0';
            // Se a busca não é nula e o arquivo tem dado, compara
            if(chave->nomeLinha != NULL){
                if(strcmp(chave->nomeLinha, temp) != 0) return false;
            }else{
                // Buscando por NULO mas encontrou dado no arquivo
                return false;
            }
        }else{
            // Campo no arquivo é NULO
            if(chave->nomeLinha != NULL) return false;
        }
    }else{
        fseek(bin, tNomeL, SEEK_CUR);
    }
    return true;
}

bool load_registro(FILE* filestream_bin, REG_DADOS_STRUCT* mem_destino){

    long pos_inicial = ftell(filestream_bin); // Armazena a posição inicial de leitura
    if((pos_inicial - HEADER_S)%REG_DADOS_S != 0){
        DEBUG("DEBUG: CURSOR NÃO ESTÁ POSICIONADO NO COMEÇO DE UM REGISTRO");
        return false;
    }
    
    DEBUG("DEBUG: load_registro SENDO EXECUTADA.\n");

    unsigned char removido;
    int campos_inteiros[CAMPOS_INT]; // 0-Proximo, 1-codEstacao, 2-codLinha, 3-codProxEstacao, 4-distProxEstacao, 5-codLinhaIntegra, 6-codEstIntegra

    // LENDO 'REMOVIDO' E CAMPOS INTEIROS
    if(fread(&removido, 1, 1, filestream_bin) != 1){
        DEBUG("ERRO AO LER CAMPO removido.\n");
        return false;
    }
    if(fread(campos_inteiros, 4, CAMPOS_INT, filestream_bin) != CAMPOS_INT){
        DEBUG("ERRO AO LER CAMPOS INTEIROS.\n");
        return false;
    }

    DEBUG("CAMPOS INTEIROS FORAM LIDOS. codEstacao = %d\n", campos_inteiros[1]);

    // LENDO CAMPOS STRING
    char* campos_strings[CAMPOS_STRINGS] = {NULL}; // 0-nomeEstacao, 1-nomeLinha
    // Sempre inicialize ponteiros como NULL
    int indicadores_tamanhos[CAMPOS_STRINGS]; // 0-nomeEstacao, 1-nomeLinha
    
    for(int i=0; i<CAMPOS_STRINGS; i++){
        int tam;
        // Lê o indicador de tamanho
        fread(&tam, 4, 1, filestream_bin);

        indicadores_tamanhos[i] = tam;
        if(tam > 0){ // Se o tamanho do campo for maior que 0, o campo string não é NULO
            campos_strings[i] = (char*)malloc(tam + 1);
            if(campos_strings[i]){
                fread(campos_strings[i], 1, tam, filestream_bin);
                campos_strings[i][tam] = '\0'; // adiciona o '/0' no final da string
            }else{
                DEBUG("DEBUG: ERRO AO ALOCAR MEMÓRIA PARA NOME DO REGISTRO.\n");
                return false;
            }
        }else{ // Se o tamanho da string for 0, deve-se printar "NULO" ao invês de pular o campo
            campos_strings[i] = NULL;
        }
    }

    // TRANSFERINDO OS DADOS PARA O STRUCT

    mem_destino->removido = removido;
    mem_destino->proximo = campos_inteiros[0];
    mem_destino->codEstacao = campos_inteiros[1];
    mem_destino->codLinha = campos_inteiros[2];
    mem_destino->codProxEstacao = campos_inteiros[3];
    mem_destino->distProxEstacao = campos_inteiros[4];
    mem_destino->codLinhaIntegra = campos_inteiros[5];
    mem_destino->codEstIntegra = campos_inteiros[6];
    mem_destino->tamNomeEstacao = indicadores_tamanhos[0];
    mem_destino->nomeEstacao = campos_strings[0];
    mem_destino->tamNomeLinha = indicadores_tamanhos[1];
    mem_destino->nomeLinha = campos_strings[1];

    DEBUG("TODOS OS CAMPOS FORAM LIDOS. nomeEstacao = %s\n", mem_destino->nomeEstacao);

    // Move o cursor para o início do próximo registro
    fseek(filestream_bin, pos_inicial + REG_DADOS_S, SEEK_SET);

    return true;
}

static void carregar_dados(FILE* filestream_bin){

    // APAGANDO POSSÍVEIS ESTRUTURAS DE DADOS
    abb_apagar(&nroEstacoesTracker);
    abb_apagar(&nroParesEstacaoTracker);

    // CRIANDO NOVAS ESTRUTURAS DE DADOS CONFIGURADAS PARA CONTER OS ITENS QUE QUEREMOS CONTAR
    nroEstacoesTracker = abb_criar(&nroEstacoesItemFuncoes);
    nroParesEstacaoTracker = abb_criar(&nroParesEstacaoItemFuncoes);

    // LENDO OS REGISTROS DE DADOS DO DISCO PARA A MEMÓRIA, E INSERINDO NA ESTRUTURA DE DADOS

    fseek(filestream_bin, HEADER_S, SEEK_SET); // Move o cursor para o primeiro registro
    REG_DADOS_STRUCT registro_lido;

    while(load_registro(filestream_bin, &registro_lido)) {
        if(registro_lido.removido == '0'){
            if(registro_lido.codEstacao != -1 && registro_lido.nomeEstacao != NULL){
                
                nroEstacoesItem* item1 = (nroEstacoesItem*) malloc (sizeof(nroEstacoesItem));
                item1->codEstacao = registro_lido.codEstacao;
                item1->nomeEstacao = strdup(registro_lido.nomeEstacao);

                abb_inserir(nroEstacoesTracker, item1); // Atualiza o tracker de nroEstacoes apenas se o novo registro contiver um código e nome válidos
            }
            if(registro_lido.codEstacao != -1 && registro_lido.codProxEstacao != -1){

                nroParesEstacaoItem* item2 = (nroParesEstacaoItem*) malloc (sizeof(nroParesEstacaoItem));
                item2->codEstacao = registro_lido.codEstacao;
                item2->codProxEstacao = registro_lido.codProxEstacao;

                abb_inserir(nroParesEstacaoTracker, item2); // Atualiza o tracker de nroParesEstacao apenas se o novo registro contiver um par válido
            }
        }

        free(registro_lido.nomeEstacao);
        free(registro_lido.nomeLinha);
    }
}

/* 
 - Recebe uma filestream aberta com permissão de escrita

*/
void atualizar_cabecalho(char* arquivoBin, int topo, int proxRRN){

    FILE* filestream_bin = fopen(arquivoBin, "rb+");
    carregar_dados(filestream_bin); // CRIANDO E POPULANDO AS ESTRUTURAS DE DADOS A PARTIR DA INFORMAÇÃO NO DISCO

    
    // ATUALIZANDO TOPO DA PILHA E PROX RRN
    DEBUG("DEBUG CABECALHO: topo: %d, proxRRN: %d\n", topo, proxRRN);

    unsigned char status_consistente = '1';
    fseek(filestream_bin, 0, SEEK_SET);
    fwrite(&status_consistente, 1, 1, filestream_bin);
    fwrite(&topo, 4, 1, filestream_bin);    // Novo topo da pilha
    fwrite(&proxRRN, 4, 1, filestream_bin); // Novo próximo RRN

    // CALCULANDO E ATUALIZANDO CONTADORES
    int nroEstacoes = abb_contar_distintos(nroEstacoesTracker);
    int nroParesEstacao = abb_contar_distintos(nroParesEstacaoTracker);

    DEBUG("DEBUG: nroEstacoes = %d\n", nroEstacoes);
    DEBUG("DEBUG: nroParesEstacao = %d\n", nroParesEstacao);

    fwrite(&nroEstacoes, 4, 1, filestream_bin);
    fwrite(&nroParesEstacao, 4, 1, filestream_bin);

    abb_apagar(&nroEstacoesTracker);
    abb_apagar(&nroParesEstacaoTracker);

    fclose(filestream_bin);

    return;
}

int fecha_binario(FILE* filestream_bin){

    unsigned char status_consistente = '1';
    // ATUALIZANDO STATUS PARA CONSISTENTE
    fseek(filestream_bin, 0, SEEK_SET);
    fwrite(&status_consistente, 1, 1, filestream_bin);

    return fclose(filestream_bin);
}