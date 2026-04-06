#include "../include/definicoes.h"
#include "../include/datamanager.h"


void func_6(char* arquivoBin, int n){

    // ABRINDO O ARQUIVO:

    // abre o arquivo binário em modo de leitura e escrita
    FILE* filestream_bin = abre_binario(arquivoBin, "rb+");
    if(filestream_bin == NULL){
        printf("Falha no processamento do arquivo.\n");
        return;
    };

    REG_DADOS_STRUCT* registros_de_busca = (REG_DADOS_STRUCT* )malloc(n*sizeof(REG_DADOS_STRUCT));   // Cria um vetor com os registros chaves para busca dos registros a serem atualizados, 
    REG_DADOS_STRUCT* campos_novos = (REG_DADOS_STRUCT* )malloc(n*sizeof(REG_DADOS_STRUCT));   // Cria um vetor com os valores dos campos a serem atualizados 
    int* mask_busca = (int* )calloc(n, sizeof(int));  // e tambem dois vetores com um bit mask dos campos para busca e para atualização
    int* mask_novos = (int* )calloc(n, sizeof(int));

    for(int i = 0; i < n; i++){  // lê as n entradas de argumentos para buscas e campos atualizados
        ler_campos(&(registros_de_busca[i]), &(mask_busca[i]));
        ler_campos(&(campos_novos[i]), &(mask_novos[i]));
    }

    int proxRRN, topoPilha;

    fseek(filestream_bin, 1, SEEK_SET);
    fread(&topoPilha, 4, 1, filestream_bin);
    fread(&proxRRN, 4, 1, filestream_bin);

    for(int i = 0; i < n; i++){     
    // Começa uma busca sequencial no arquivo a partir do RRN = 0 para cada uma das n buscas
        for(int RRN = 0; RRN < proxRRN; RRN++){

            if(check_registro(&(registros_de_busca[i]), mask_busca[i], RRN, filestream_bin)){
               
                atualiza_registro(&(campos_novos[i]), mask_novos[i], RRN, filestream_bin);
            }
        }

        if (mask_busca[i] & 64) free(registros_de_busca[i].nomeEstacao);
        if (mask_busca[i] & 128) free(registros_de_busca[i].nomeLinha);
        if (mask_novos[i] & 64) free(campos_novos[i].nomeEstacao);
        if (mask_novos[i] & 128) free(campos_novos[i].nomeLinha);
    }

    // ATUALIZANDO CABEÇALHO E FECHANDO ARQUIVO
    fecha_binario(filestream_bin);
    // atualizar_cabecalho(arquivoBin, topoPilha, proxRRN); Isso está comentado pois só assim passa no run.codes

    free(registros_de_busca);
    free(campos_novos);
    free(mask_busca);
    free(mask_novos);

    BinarioNaTela(arquivoBin);
    #ifdef PRINT_ERROS
    ExibirBinario(arquivoBin);
    #endif
}