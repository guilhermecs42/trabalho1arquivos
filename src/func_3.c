#include "../include/definicoes.h"
#include "../include/datamanager.h"

/**
 * @brief Funcionalidade [3]: Recupera registros com base em critérios de busca.
 * Simula o comando SQL 'SELECT WHERE'. Permite a busca por um ou mais campos 
 * (inteiros ou strings). Realiza uma busca sequencial no arquivo e exibe todos 
 * os registros que satisfazem os filtros informados.
 * @param arquivoBin Nome do arquivo binário para consulta.
 * @param n Quantidade de buscas independentes a serem realizadas.
 * @return void
 */
void func_3(char* arquivoBin, int n){

    REG_DADOS_STRUCT* registros_de_busca = NULL;
    int* mask = NULL;

    // ABRINDO ARQUIVO
 
    FILE* filestream_bin = abre_binario(arquivoBin, false); // Abre o arquivo binário em modo de leitura
    if(filestream_bin == NULL){
        DEBUG("ERRO EM func_3: ERRO AO ABRIR O BINÁRIO %s.\n", arquivoBin);
        goto erro;
    }

    // COLETANDO CRITÉRIOS DE BUSCA

    // Cria um vetor com os registros chaves de busca, e tambem um vetor com um bit mask dos campos utilizados para busca
    registros_de_busca = (REG_DADOS_STRUCT* )malloc(n*sizeof(REG_DADOS_STRUCT));
    mask = (int* )calloc(n, sizeof(int));
    if(registros_de_busca == NULL || mask == NULL){DEBUG("ERRO EM func_3: ALOCAÇÃO DE MEMÓRIA.\n"); goto erro;}

    for(int i = 0; i < n; i++){ // lê as n entradas de argumentos para buscas
        ler_campos(&registros_de_busca[i], &mask[i]);
    }

    // BUSCANDO REGISTROS DE DADOS

    int topoPilha, proxRRN;
    fseek(filestream_bin, 1, SEEK_SET);
    fread(&topoPilha, 4, 1, filestream_bin);
    fread(&proxRRN, 4, 1, filestream_bin);
    // Começar busca sequencial no arquivo a partir do RRN = 0 para cada uma das n buscas
    for(int i = 0; i < n; i++){

        bool flag_encontrou = false;

        for(int RRN = 0; RRN < proxRRN; RRN++){
            if(check_registro(&registros_de_busca[i], mask[i], RRN, filestream_bin)){
                // Se o registro bate com a busca, posiciona e imprime
                fseek(filestream_bin, RRN * REG_DADOS_S + HEADER_S, SEEK_SET);
                print_registro(filestream_bin); 
                flag_encontrou = true;
            }
        }

        if(!flag_encontrou) printf("Registro inexistente.\n");
        printf("\n");

        if (mask[i] & 64) free(registros_de_busca[i].nomeEstacao);
        if (mask[i] & 128) free(registros_de_busca[i].nomeLinha);
    }

    // FECHANDO ARQUIVO

    if(fecha_binario(filestream_bin) != 0){
        DEBUG("DEBUG: ERRO AO FECHAR BIN %s\n", arquivoBin);
        goto erro;
    }

    // LIMPANDO E RETORNANDO

    free(registros_de_busca);
    free(mask);

    return;

    erro:

    free(registros_de_busca);
    free(mask);
    if(fecha_binario(filestream_bin) != 0){
        DEBUG("DEBUG: ERRO AO FECHAR BIN %s\n", arquivoBin);
    }
    printf("Falha no processamento do arquivo.\n");

    return;
}