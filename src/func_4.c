#include "../include/definicoes.h"
#include "../include/datamanager.h"

/**
 * @brief Funcionalidade [4]: Realiza a remoção lógica de registros.
 * Simula o comando SQL 'DELETE FROM WHERE'. Localiza registros através de filtros 
 * e os marca como removidos ('1'). Implementa uma lista encadeada (pilha) de espaços 
 * disponíveis, utilizando o campo 'topo' no cabeçalho para permitir reaproveitamento futuro.
 * @param arquivoBin Nome do arquivo binário onde ocorrerão as remoções.
 * @param n Quantidade de operações de remoção a serem processadas.
 * @return void
 */
void func_4(char* arquivoBin, int n){

    // ABRINDO O ARQUIVO:

    // abre o arquivo binário em modo de leitura e escrita
    FILE* filestream_bin = abre_binario(arquivoBin, "rb+");
    if(filestream_bin == NULL){
        printf("Falha no processamento do arquivo.\n");
        return;
    };

    REG_DADOS_STRUCT* registros_de_busca = (REG_DADOS_STRUCT* )malloc(n*sizeof(REG_DADOS_STRUCT));   // Cria um vetor com os registros chaves para busca dos registros a serem excluídos, 
    int* mask = (int* )calloc(n, sizeof(int));                                                        // e tambem um vetor com um bit mask dos campos utilizados para busca

    for(int i = 0; i < n; i++){  // lê as n entradas de argumentos para buscas
        ler_campos(&(registros_de_busca[i]), &(mask[i]));
    }

    int proxRRN, topoPilha;
    unsigned char removido = '1';

    fseek(filestream_bin, 1, SEEK_SET);
    fread(&topoPilha, 4, 1, filestream_bin);
    fread(&proxRRN, 4, 1, filestream_bin);

    for(int i = 0; i < n; i++){     // Começa uma busca sequencial no arquivo a partir do RRN = 0 para cada uma das n buscas

        for(int RRN = 0; RRN < proxRRN; RRN++){

            if(check_registro(&registros_de_busca[i], mask[i], RRN, filestream_bin)){
                // Se o registro bate com a busca, marca como removido e atualiza o campo 'proximo' para receber o topo da pilha
                fseek(filestream_bin, HEADER_S + RRN * REG_DADOS_S, SEEK_SET);
                fwrite(&removido, 1, 1, filestream_bin);
                fwrite(&topoPilha, 4, 1, filestream_bin);
                DEBUG("DEBUG: TOPO PILHA: %d\n", topoPilha);
                topoPilha = RRN; // atualiza o topo da pilha para o registro atual
            }
        }

        if (mask[i] & 64) free(registros_de_busca[i].nomeEstacao);
        if (mask[i] & 128) free(registros_de_busca[i].nomeLinha);
    }

    // ATUALIZANDO CABEÇALHO E FECHANDO ARQUIVO
    
    fecha_binario(filestream_bin);
    atualizar_cabecalho(arquivoBin, topoPilha, proxRRN);



    free(registros_de_busca);
    free(mask);

    BinarioNaTela(arquivoBin);
    #ifdef PRINT_ERROS
    ExibirBinario(arquivoBin);
    #endif
}