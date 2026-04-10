#include "../include/definicoes.h"
#include "../include/datamanager.h"

/**
  * @brief Funcionalidade [6]: Busca por registros e atualiza os campos com novos valores.
  * Simula o comando SQL 'UPDATE'. Localiza registros através de filtros e atualiza seus campos.
  * Para cada busca realizada, os registros correspondentes devem ser atualizados 
  * com os novos valores especificados. Vários ciclos de busca&atualização podem ser realizados.
  * @param arquivoBin Nome do arquivo binário para atualização.
  * @param n Quantidade de ciclos de busca&atualização a serem realizados.
  * @return void
  */
void func_6(char* arquivoBin, int n){

    REG_DADOS_STRUCT* registros_de_busca = NULL;
    REG_DADOS_STRUCT* campos_novos = NULL;
    int* mask_busca = NULL;
    int* mask_novos = NULL;

    // ABRINDO O ARQUIVO:

    FILE* filestream_bin = abre_binario(arquivoBin, true); // abre o arquivo binário em modo de leitura e escrita
    if(filestream_bin == NULL){
        printf("Falha no processamento do arquivo.\n");
        return;
    };

    // LENDO CAMPOS DE BUSCA E OS VALORES DE ATUALIZAÇÃO:

    registros_de_busca = (REG_DADOS_STRUCT* )malloc(n*sizeof(REG_DADOS_STRUCT));   // Cria um vetor com os registros chaves para busca dos registros a serem atualizados, 
    campos_novos = (REG_DADOS_STRUCT* )malloc(n*sizeof(REG_DADOS_STRUCT));   // Cria um vetor com os valores dos campos a serem atualizados 
    mask_busca = (int* )calloc(n, sizeof(int));  // e tambem dois vetores com um bit mask dos campos para busca e para atualização
    mask_novos = (int* )calloc(n, sizeof(int));
    if(registros_de_busca == NULL || campos_novos == NULL || mask_busca == NULL || mask_novos == NULL){
        DEBUG("ERRO EM func_6: ERRO DE ALOCAÇÃO.\n"); goto erro;
    }

    for(int i = 0; i < n; i++){  // lê as n entradas de argumentos para buscas e campos atualizados
        ler_campos(&(registros_de_busca[i]), &(mask_busca[i]));
        ler_campos(&(campos_novos[i]), &(mask_novos[i]));
    }

    // BUSCANDO OS REGISTROS NO ARQUIVO PARA ATUALIZAR

    int proxRRN, topoPilha;

    fseek(filestream_bin, 1, SEEK_SET);
    fread(&topoPilha, 4, 1, filestream_bin);
    fread(&proxRRN, 4, 1, filestream_bin);

    for(int i = 0; i < n; i++){     
    // Começa uma busca sequencial no arquivo a partir do RRN = 0 para cada uma das n buscas
        for(int RRN = 0; RRN < proxRRN; RRN++){

            if(check_registro(&(registros_de_busca[i]), mask_busca[i], RRN, filestream_bin)){
               
                if(atualiza_registro(&(campos_novos[i]), mask_novos[i], RRN, filestream_bin) == false){
                    DEBUG("ERRO EM func_6: FALHA AO ATUALIZAR REGISTRO.\n");
                }
            }
        }

        if (mask_busca[i] & 64) free(registros_de_busca[i].nomeEstacao);
        if (mask_busca[i] & 128) free(registros_de_busca[i].nomeLinha);
        if (mask_novos[i] & 64) free(campos_novos[i].nomeEstacao);
        if (mask_novos[i] & 128) free(campos_novos[i].nomeLinha);
    }

    // FECHANDO ARQUIVO
    if(fecha_binario(filestream_bin) != 0){
        DEBUG("DEBUG: ERRO AO FECHAR BIN %s\n", arquivoBin);
        goto erro;
    }

    // RETORNANDO

    free(registros_de_busca);
    free(campos_novos);
    free(mask_busca);
    free(mask_novos);

    BinarioNaTela(arquivoBin);
    #ifdef PRINT_ERROS
    ExibirBinario(arquivoBin);
    #endif

    return;

    erro:

    free(registros_de_busca);
    free(campos_novos);
    free(mask_busca);
    free(mask_novos);
    if(fecha_binario(filestream_bin) != 0){
        DEBUG("DEBUG: ERRO AO FECHAR BIN %s\n", arquivoBin);
    }

    return;
}