#include "../include/definicoes.h"
#include "../include/datamanager.h"


REG_DADOS_STRUCT* ler_input_reg(){

    REG_DADOS_STRUCT* registro_lido = (REG_DADOS_STRUCT*)malloc(sizeof(REG_DADOS_STRUCT));
    if(registro_lido == NULL) return NULL;

    // Inicialização de campos de controle
    registro_lido->removido = '0';
    registro_lido->proximo = -1;

    char buffer[100];

    // Lê os campos de input, tanto inteiros quanto strings, utilizando a função scanQuoteString, e os salva num registro

    // codEstacao
    ScanQuoteString(buffer);
    registro_lido->codEstacao = (strlen(buffer) == 0) ? -1 : atoi(buffer);

    // nomeEstacao
    ScanQuoteString(buffer); 
    registro_lido->nomeEstacao = processar_string(buffer, &registro_lido->tamNomeEstacao);

    // codLinha
    ScanQuoteString(buffer);
    registro_lido->codLinha = (strlen(buffer) == 0) ? -1 : atoi(buffer);

    // nomeLinha
    ScanQuoteString(buffer);
    registro_lido->nomeLinha = processar_string(buffer, &registro_lido->tamNomeLinha);

    // codProxEstacao
    ScanQuoteString(buffer);
    registro_lido->codProxEstacao = (strlen(buffer) == 0) ? -1 : atoi(buffer);

    // distProxEstacao
    ScanQuoteString(buffer);
    registro_lido->distProxEstacao = (strlen(buffer) == 0) ? -1 : atoi(buffer);

    // codLinhaIntegra
    ScanQuoteString(buffer);
    registro_lido->codLinhaIntegra = (strlen(buffer) == 0) ? -1 : atoi(buffer);

    // codEstIntegra
    ScanQuoteString(buffer);
    registro_lido->codEstIntegra = (strlen(buffer) == 0) ? -1 : atoi(buffer);

    return registro_lido;
}

/**
 * @brief Funcionalidade [5]: Insere novos registros reaproveitando espaços removidos.
 * Simula o comando SQL 'INSERT INTO'. Tenta inserir o novo registro no RRN indicado 
 * pelo 'topo' da pilha de removidos. Caso a pilha esteja vazia (topo == -1), a inserção 
 * ocorre no final do arquivo (proxRRN).
 * @param arquivoBin Nome do arquivo binário para inserção.
 * @param n Quantidade de novos registros a serem inseridos.
 * @return void
 */
void func_5(char* arquivoBin, int n){
    
    FILE* filestream_bin = fopen(arquivoBin, "rb+");
    if(filestream_bin == NULL){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    int topo, proxRRN;
    // Lê o topo da pilha de removidos
    fseek(filestream_bin, 1, SEEK_SET);
    fread(&topo, 4, 1, filestream_bin);
    // Lê o próximo RRN disponível para o fim do arquivo
    fread(&proxRRN, 4, 1, filestream_bin);

    for(int i = 0; i < n; i++){
        REG_DADOS_STRUCT* registro_lido = ler_input_reg();

        if(topo != -1){
            // Reutilização do espaço na pilha de removidos
            long offset = (long)topo * REG_DADOS_S + HEADER_S;
            fseek(filestream_bin, offset + 1, SEEK_SET); // Pula o byte 'removido' para ler o próximo da pilha
            
            int proximo_na_pilha;
            fread(&proximo_na_pilha, 4, 1, filestream_bin);

            // Volta para o início do registro para escrever os novos dados
            fseek(filestream_bin, offset, SEEK_SET);
            escreve_registro(registro_lido, filestream_bin);

            // Atualiza o topo da pilha
            topo = proximo_na_pilha;
        }else{
            // Usa o proxRRN para inserir no fim
            long offset = (long)proxRRN * REG_DADOS_S + HEADER_S;
            fseek(filestream_bin, offset, SEEK_SET);
            escreve_registro(registro_lido, filestream_bin);
            
            proxRRN++; // Incrementa o contador de registros do arquivo
        }

        // Limpeza de memória do registro lido
        if(registro_lido->nomeEstacao) free(registro_lido->nomeEstacao);
        if(registro_lido->nomeLinha) free(registro_lido->nomeLinha);
        free(registro_lido);
    }

    // ATUALIZANDO CABEÇALHO E FECHANDO ARQUIVO
    fecha_binario(filestream_bin);
    atualizar_cabecalho(arquivoBin, topo, proxRRN);

    BinarioNaTela(arquivoBin);

    #ifdef PRINT_ERROS
        ExibirBinario(arquivoBin);
    #endif
}