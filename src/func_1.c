#include "../include/definicoes.h"
#include "../include/datamanager.h"

/**Objetivo: ler uma linha do csv e converter em um struct na memória
 * Pré-condições:
 * 		linha deve ser uma string formada por 8 valores separados por vírgula, valores nulos como vírgulas adjacentes e \0 no final.
 * Pós condições:
 * 		Erro: retorna NULL
 * 		Sucesso: retorna um struct com todos os campos válidos
 * 		Chamador deve: apagar o struct.
 **/
static REG_DADOS_STRUCT* ler_linha_csv(char* linha){
    if(linha == NULL){
    	DEBUG("ERRO EM ler_linha_csv: LINHA NULA.\n");
    	return NULL;
    }

    // ALOCANDO E INICIALIZANDO STRUCT

    REG_DADOS_STRUCT* registro_lido = (REG_DADOS_STRUCT*) malloc (sizeof(REG_DADOS_STRUCT));
    if(registro_lido == NULL){
    	DEBUG("ERRO EM ler_linha_csv: FALAH DE ALOCAÇÃO DE MEMÓRIA.\n");
    	return NULL;
    }
    registro_lido->nomeEstacao = NULL; registro_lido->nomeLinha = NULL; // inicializando ponteiros como nulo para liberar memória com segurança
    registro_lido->removido = '0'; // Seguindo a lógica de registro ativo
    registro_lido->proximo = -1; // Conforme a especificação

    char *campo;
    char *ptr = linha; // strsep modifica o ponteiro, usamos um auxiliar

    for(int i = 0; i < CAMPOS_INT + CAMPOS_STRINGS - 1; i++){ // -1 pois o campo "proximo" não é lido
        // strsep extrai o texto até a próxima vírgula e coloca \0 no lugar dela
        campo = strsep(&ptr, ",");

        // Se strsep retornar NULL antes do 8º campo, a linha está incompleta
        if(campo == NULL){
            DEBUG("ERRO EM ler_linha_csv: LINHA INCOMPLETA.\n");
            free(registro_lido->nomeEstacao);
            free(registro_lido->nomeLinha);
            free(registro_lido);
            return NULL;
        }

        switch(i){
            case 0: registro_lido->codEstacao = processar_int(campo); break;
            case 1: registro_lido->nomeEstacao = processar_string(campo, &registro_lido->tamNomeEstacao); break;
            case 2: registro_lido->codLinha = processar_int(campo); break;
            case 3: registro_lido->nomeLinha = processar_string(campo, &registro_lido->tamNomeLinha); break;
            case 4: registro_lido->codProxEstacao = processar_int(campo); break;
            case 5: registro_lido->distProxEstacao = processar_int(campo); break;
            case 6: registro_lido->codLinhaIntegra = processar_int(campo); break;
            case 7: registro_lido->codEstIntegra = processar_int(campo); break;
        }
    }

    // Se após os 8 campos o ponteiro 'ptr' não for NULL, existem campos extras
    if(ptr != NULL){
		DEBUG("ERRO EM ler_linha_csv: LINHA COM CAMPOS A MAIS.\n");
        free(registro_lido->nomeEstacao);
        free(registro_lido->nomeLinha);
		free(registro_lido);
        return NULL;
    }

    return registro_lido;
}

/**
	 * @brief Funcionalidade [1]: Converte um arquivo csv para binário, de acordo com a especificação.
	 * Simula o comando SQL 'CREATE TABLE'. Percorre sequencialmente o arquivo csv e 
	 * e gera os registros de dados correspondentes a cada linha. 
	 * Nenhum registro é marcado como logicamente removido
	 * O cabeçalho é gerado com a contagem correta de estações e de pares (codEstacao, codProxEstacao)
	 * @param arquivoEntrada Nome do arquivo csv de onde os dados serão lidos
	 * @param arquivoEntrada Nome do arquivo binário que será produzido
	 * @return void
	 */
void func_1(char* arquivoEntrada, char* arquivoSaida){ // a ordem dos argumentos é o contrário da ordem digitada pelo usuário, devido à ordem de empilhamento dos argumentos na memória, lembrando que cada argumento é o retorno de uma chamada de strtok
	
	FILE* filestream_csv = NULL;
	FILE* filestream_bin = NULL;
	REG_DADOS_STRUCT* registro_lido = NULL;
	
	// ABRIR CSV EM MODO LEITURA
	filestream_csv = fopen(arquivoEntrada, "r"); // abre o arquivo csv em modo leitura e como texto
	if(filestream_csv == NULL){ // se falhou
		DEBUG("ERRO EM func_1: ERRO AO ABRIR CSV\n");
		goto erro;
	}
	
	// CRIAR ARQUIVO BINÁRIO EM MODO ESCRITA
	filestream_bin = fopen(arquivoSaida, "wb"); // abre o arquivo de saída em modo escrita e como binário
	if(filestream_bin == NULL){ // se falhou
		DEBUG("ERRO EM func_1: ERRO AO ABRIR BINÁRIO\n");
		goto erro;
	}
	
	// ESCREVER REGISTRO DE CABEÇALHO DUMMY

	int proxRRN = 0;
	
	unsigned char cabecalho[] = { // a variável cabecalho é o endereço de memória de uma sequência de bytes, especificados abaixo. Esses são valores iniciais para o registro de cabeçalho, que deverá ser atualizado quando terminarmos a leitura.
		'0',                   // status, inicializado como '0' pois o registro está inconsistente
		0xff, 0xff, 0xff, 0xff, // topo, inicializado como -1.
		0x00, 0x00, 0x00, 0x00, // proxRRN, será atualizado depois.
		LIXO, LIXO, LIXO, LIXO, // nroEstacoes, será atualizado depois
		LIXO, LIXO, LIXO, LIXO  // nroParesEstacao, será atualizado depois
	};
	
	fwrite(cabecalho, 1, sizeof(cabecalho), filestream_bin);
	
	// ESCREVER REGISTRO DE DADOS
	
	char linha[103]; // uma linha do arquivo csv. O cabeçalho tem caracteres, mais \n, mais \0
	fgets(linha, 103, filestream_csv);
	linha[strcspn(linha, "\r\n")] = '\0'; // faz a linha terminar em \0
	 
	if(strcmp(linha, "CodEstacao,NomeEstacao,CodLinha,NomeLinha,CodProxEst,DistanciaProxEst,CodLinhaInteg,CodEstacaoInteg") != 0){
		DEBUG("ERRO EM func_1: PRIMEIRA LINHA NÃO CORRESPONDE AO ESPERADO\n");
		goto erro;
	}
	
	while(fgets(linha, sizeof(linha), filestream_csv)){ // lê novas linhas até chegar ao fim do arquivo, quando fgets retorna NULL

		//DEBUG("DEBUG: LENDO REGISTRO DE DADOS\n");
	
		linha[strcspn(linha, "\r\n")] = '\0'; // faz a linha terminar em \0
		registro_lido = ler_linha_csv(linha);
		if(registro_lido == NULL){
			DEBUG("ERRO EM func_1: FALHA EM LER LINHA DO CSV %s\n", arquivoEntrada);
			goto erro;
		}
		
		// ESCREVER O REGISTRO NO BINÁRIO E NAS ESTRUTURAS

		if(escreve_registro(registro_lido, filestream_bin) == false){
			DEBUG("ERRO EM func_1: FALHA EM ESCREVER REGISTRO NO BINÁRIO %s\n", arquivoSaida);
			goto erro;
		} 
		proxRRN++;

		free(registro_lido->nomeEstacao);
		free(registro_lido->nomeLinha);
	}

	// FECHANDO ARQUIVOS E ATUALIZANDO CABEÇALHO
	
	if(filestream_csv != NULL){
		if(fclose(filestream_csv) != 0){
			DEBUG("DEBUG: ERRO AO FECHAR CSV %s\n", arquivoEntrada);
			goto erro;
		}
	}
	if(fecha_binario(filestream_bin) != 0){
		DEBUG("DEBUG: ERRO AO FECHAR BIN %s\n", arquivoSaida);
		goto erro;
	}

	atualizar_cabecalho(arquivoSaida, -1, proxRRN);
	
	// EXIBINDO ARQUIVO

	BinarioNaTela(arquivoSaida);
	#ifdef PRINT_ERROS
	ExibirBinario(arquivoSaida);
	#endif

	return;
	
	// TRATAMENTO DE ERRO:

	erro: 

	if(registro_lido != NULL){
		free(registro_lido->nomeEstacao);
		free(registro_lido->nomeLinha);
	}
	free(registro_lido);

	if(filestream_csv != NULL){
		if(fclose(filestream_csv) != 0){
			DEBUG("DEBUG: ERRO AO FECHAR CSV %s\n", arquivoEntrada);
		}
	}
	
	if(filestream_bin != NULL){
		if(fecha_binario(filestream_bin) != 0){
			DEBUG("DEBUG: ERRO AO FECHAR BIN %s\n", arquivoSaida);
		}
	}

	printf("Falha no processamento do arquivo.\n");

	return;
}