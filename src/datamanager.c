#include "../include/datamanager.h"
#include "../include/tabelafuncoes.h"
#include "../include/arvore.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

/*
O uso de árvore binária foi a solução encontrada para o problema de calcular a
quantidade de elementos distintos, seja estações, sejam pares (codEstacao, codProxEstacao).
Os elementos são inseridos em suas respectivas árvores. Então é feito um percurso em ordem 
contabilizando a quantidade de vezes que o conteúdo do nó atual muda em relação ao anterior.
Como a inserção de um item em uma árvore balanceada é O(log k), em que k é a quantidade
atual de nós, a inserção de n nós é O(log n!), que é assintoticamente igual a O(n log n).

São definidas duas árvores visíveis para todo o arquivo, mas invisíveis para os outros. A única
forma de outros arquivos interagirem com essas árvores é a partir da função atualizar_cabecalho 

Cada árvore utiliza funções customizadas para ordenar, identificar e apagar o tipo de item 
para os quais cada nó aponta. Os tipos dessas funções são especificados em tabelafuncoes.h.

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

// A seguir seguem dois vetores de ponteiros para função. Cada árvore recebe a sua respectiva Tabela
// de Funções após serem criadas. 

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







// INÍCIO DAS FUNÇÕES QUE GERENCIAM DISCO E ESTRUTURA DE DADOS






/**Objetivo: abrir um arquivo binário em modo especificado do Banco de Dados.  
 * 
 * Pré-condições: 
 *      O caminho/nome do arquivo deve existir e ter as permissões necessárias
 *      A necessidade de permissão de atualização (escrita) deve ser explicitada no parâmetro
 *      O status do arquivo deve ser '1'
 * 
 * Pós condições: 
 *      Erro: retorna nulo
 *      Sucesso: retorna filestream do arquivo com o modo especificado, status '0' se for de atualização. Abrir em modo de leitura não afeta o status.
 *      Chamador deve: fechar a filestream com a função fecha_binario() para impor status '1', se necessário.
 */
FILE* abre_binario(char* arquivoBin, bool escrita){
    
    char* modo = escrita ? "rb+" : "rb";

    // abre o arquivo bin em modo leitura
    FILE* filestream_bin = fopen(arquivoBin, modo);
    if(filestream_bin == NULL){ // se falhou
        DEBUG("ERRO EM abre_binario: ERRO AO ABRIR O BINÁRIO %s. VERIFIQUE EXISTÊNCIA E PERMISSÕES.\n", arquivoBin);
        return NULL;
    }

    // Lê o status do arquivo
    unsigned char status;
    fread(&status, 1, 1, filestream_bin);
    if(status != '1'){
        DEBUG("DEBUG: ARQUIVO %s INCONSISTENTE. NÃO FOI POSSÍVEL ABRIR.\n", arquivoBin);
        fclose(filestream_bin);
        return NULL;
    }

    if(escrita){
        // Reescreve o status do arquivo
        fseek(filestream_bin, 0, SEEK_SET);
        status = '0';
        fwrite(&status, 1, 1, filestream_bin);
    }
    fseek(filestream_bin, 0, SEEK_SET); // Voltando ao começo

    return filestream_bin;
}

/**Objetivo: escrever um registro de dados no arquivo binário
 * 
 * Pré-condições:
 *      Filestream binária em modo de atualização
 *      Cursor em uma posição compatível com o início de um registro de dados
 *      O registro a ser inserido deve ser válido
 * 
 * Pós-condições:
 *      Erro: retorna false, sem alterar a posição do cursor
 *      Sucesso: retorna true, com o cursor apontado para a posição do próximo registro de dados
 *      Chamador deve: apagar a struct e fechar o arquivo com fecha_binario
 **/
bool escreve_registro(REG_DADOS_STRUCT* registro_lido, FILE* filestream_bin){
    if(registro_lido == NULL){
        DEBUG("ERRO EM escreve_registro: REGISTRO NULO.\n");
        return false;
    }
    if(filestream_bin == NULL){
        DEBUG("ERRO EM escreve_registro: FILESTREAM NULA.\n");
        return false;
    }

    long pos_inicial = ftell(filestream_bin);   
    if( (pos_inicial - HEADER_S)%REG_DADOS_S != 0 ){
        DEBUG("ERRO EM escreve_registro: CURSOR DE ARQUIVO NÃO ESTÁ NO INÍCIO DE UM REGISTRO DE DADOS.\n");
        return false;
    }

    // ESCREVENDO O STRUCT NO BINÁRIO

    if (fwrite(&(registro_lido->removido), 1, 1, filestream_bin) != 1){
        DEBUG("DEBUG: ESCRITA DO REGISTRO FALHOU. VERIFIQUE SE O ARQUIVO ESTÁ ABERTO EM MODO DE ESCRITA.\n");
        fseek(filestream_bin, pos_inicial, SEEK_SET);
        return false;
    }
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

    return true;
}

/**Objetivo: atualizar um registro no arquivo binário
 * 
 * Pré-condições:
 *      Filestream binária em um modo que permita leitura e escrita
 *      RRN dentro dos limites do binário
 *      Struct campos_novos e mask foram inicializados juntos
 * 
 * Pós-condições:
 *      Erro: retorna false, com posição do cursor indeterminada
 *      Sucesso: retorna true, com o cursor apontado para a posição do próximo registro de dados
 *      Chamador deve: apagar a struct e fechar o arquivo com fecha_binario
 **/
bool atualiza_registro(REG_DADOS_STRUCT *campos_novos, int mask, int RRN, FILE *filestream_bin){
    if(campos_novos == NULL){
        DEBUG("ERRO EM atualiza_registro: NÃO FOI FORNECIDO O VALOR DOS NOVOS CAMPOS.\n");
        return false;
    }

    int proxRRN;
    fseek(filestream_bin, 5, SEEK_SET);
    if (fread(&proxRRN, 4, 1, filestream_bin) != 1){
        DEBUG("ERRO EM atualiza_registro: LEITURA FALHOU. VERIFIQUE SE O ARQUIVO ESTÁ ABERTO EM MODO DE LEITURA.\n");
        return false;
    }
    if(RRN < 0 || RRN >= proxRRN){
        if(RRN < 0){ DEBUG("ERRO EM atualiza_registro: RRN < 0.\n");
        }else{DEBUG("ERRO EM atualiza_registro: RRN MAIOR QUE O TAMANHO DO ARQUIVO.\n");}
        return false;
    }

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
        if (campos_novos->nomeEstacao != NULL) {
            strcpy(nomeEst, campos_novos->nomeEstacao);
            tamNomeEst = strlen(nomeEst);
        }else{
            tamNomeEst = 0; // Se não tem nome, tamanho é 0
        }
    }

    if(mask & 128){
        if (campos_novos->nomeLinha != NULL) {
            strcpy(nomeLinha, campos_novos->nomeLinha);
            tamNomeLinha = strlen(nomeLinha);
        }else{
            tamNomeLinha = 0;
        }
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
    return true;
}

/**Objetivo: verificar o RRN fornecido corresponde à busca realizada
 * 
 * Pré-condições:
 *      Filestream binária em um modo que permita leitura
 *      Struct chave e mask foram inicializados juntos
 *      Mask só pode ser 0 ou 1
 *      RRN dentro dos limites do binário
 * 
 * Pós-condições:
 *      Erro: retorna false, com mensagem de DEBUG. Posição do cursor indefinida
 *      Sucesso: retorna false se não corresponde ou se está removido, true se corresponde. Posição do cursor indefinida
 *      Chamador deve: apagar a struct e possivelmente a mask, e fechar o filestream com fecha_binario
 **/
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
            if(tNomeE >= 100) return false; // caso o arquivo esteja corrompido, devemos impedir um tamanho grande de ser lido e causar buffer overflow

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

/**Objetivo: extrair um registro de dados do disco e colocar na memória
 * 
 * Pré-condições:
 *      Filestream aberta em modo que permita leitura
 *      Cursor posicionada no começo de um registro de arquivos
 *      Struct mem_destino alocado propriamente
 * 
 * Pós-condições:
 *      Erro: retorna false
 *      Sucesso: retorna true. O cursor aponta para o próximo registro de dados
 *      Chamador deve: apagar o registro da memória quando terminar de usar, fechar a filestream com fecha_binario
 **/
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

/** Objetivo: povoar as árvores binárias com os elementos necessários para a contagem
 * 
 *  Pré-condições:
 *      Filestream aberta em modo de leitura
 * 
 *  Pós condições:
 *      Duas árvores criadas e preenchidas com elementos relevantes para contagem
 *      Chamador deve: apagar as árvores por e fechar o filestream
 */
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

/**Objetivo: atualizar o cabeçalho de um arquivo binário, recontando os registros e marcando-o como consistente
 * 
 * Pré-condições:
 *      topo e proxRRN devem ser calculados corretamente pela função chamadora
 * 
 * Pós-condições:
 *      arquivo fechado com status consistente 
**/
void atualizar_cabecalho(char* arquivoBin, int topo, int proxRRN){

    FILE* filestream_bin = fopen(arquivoBin, "rb+");
    if(filestream_bin == NULL){
        DEBUG("ERRO EM atualizar_cabecalho: FALHA EM ABRIR O ARQUIVO. ELE EXISTE?\n");
    }
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

/**Objetivo: fechar o arquivo binário mantendo o status como 'consistente'
 * 
 * Pré-condições:
 *      filestream_bin deve estar aberta em modo que permita escrita, ou ser NULL
 * 
 * Pós-condições:
 *      filestream estará fechada e não será mais possível acessá-la
 */
int fecha_binario(FILE* filestream_bin){
    if(filestream_bin == NULL) return 0;

    unsigned char status_consistente = '1';
    // ATUALIZANDO STATUS PARA CONSISTENTE
    fseek(filestream_bin, 0, SEEK_SET);
    fwrite(&status_consistente, 1, 1, filestream_bin);

    return fclose(filestream_bin);
}