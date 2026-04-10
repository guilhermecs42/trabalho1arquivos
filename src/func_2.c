#include "../include/definicoes.h"
#include "../include/datamanager.h"

/**
 * @brief Funcionalidade [2]: Recupera e exibe todos os registros do arquivo binário.
 * Simula o comando SQL 'SELECT FROM'. Percorre sequencialmente o arquivo binário 
 * e imprime todos os registros que não estão marcados como logicamente removidos. 
 * Campos nulos são exibidos como 'NULO'.
 * @param arquivoBin Nome do arquivo binário de onde os dados serão lidos.
 * @return void
 */
void func_2(char* arquivoBin){

	// abre o arquivo bin em modo leitura
	FILE* filestream_bin = abre_binario(arquivoBin, false);
	if(filestream_bin == NULL){
		DEBUG("ERRO EM func_2: FALHA EM ABRIR O ARQUIVO BIN %s.\n", arquivoBin);
		goto erro;
	}

	fseek(filestream_bin, HEADER_S, SEEK_SET);

	unsigned char removido;
	bool reg_existe = false;

	while(fread(&removido, 1, 1, filestream_bin) == 1){
		if(removido == '1'){ // O registro foi removido, salta o registro ao invés de ler
			fseek(filestream_bin, REG_DADOS_S - 1, SEEK_CUR); // -1 pois o cursor está no segundo byte
		}else{
            fseek(filestream_bin, -1, SEEK_CUR);
			print_registro(filestream_bin);
			reg_existe = true;	
		}
	}

    if (!reg_existe) {
        printf("Registro inexistente.\n");
    }

    // FECHANDO ARQUIVO

	if(fecha_binario(filestream_bin) != 0){
		DEBUG("DEBUG: ERRO AO FECHAR BIN %s\n", arquivoBin);
		goto erro;
	}

	return;

	erro:

	if(fecha_binario(filestream_bin) != 0){
		DEBUG("DEBUG: ERRO AO FECHAR BIN %s\n", arquivoBin);
	}
	printf("Falha no processamento do arquivo.\n");

	return;
}