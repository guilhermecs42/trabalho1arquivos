#include "../include/definicoes.h"
#include "../include/datamanager.h"

// Essa função recebe uma linha do arquivo csv e retorna um struct REG_DADOS_STRUCT contendo as informações naquela linha
static REG_DADOS_STRUCT* ler_linha_csv(char* linha){
    if(linha == NULL){
    	DEBUG("Falha no processamento: linha nula.\n");
    	printf("Falha no processamento do arquivo.\n");
      	exit(1);
    }

    REG_DADOS_STRUCT* registro_lido = (REG_DADOS_STRUCT*) malloc (sizeof(REG_DADOS_STRUCT));
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
            DEBUG("Falha no processamento: linha incompleta.\n");
            printf("Falha no processamento do arquivo.\n");
            free(registro_lido);
            exit(1);
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
        DEBUG("Falha no processamento: linha com campos extras.\n");
        printf("Falha no processamento do arquivo.\n");
	    free(registro_lido);
		exit(1);
    }

    return registro_lido;
}

void func_1(char* arquivoEntrada, char* arquivoSaida){ // a ordem dos argumentos é o contrário da ordem digitada pelo usuário, devido à ordem de empilhamento dos argumentos na memória, lembrando que cada argumento é o retorno de uma chamada de strtok
	
	FILE* filestream_csv = NULL;
	FILE* filestream_bin = NULL;
	
	// ABRIR CSV EM MODO LEITURA
	filestream_csv = fopen(arquivoEntrada, "r"); // abre o arquivo csv em modo leitura e como texto
	if(filestream_csv == NULL){ // se falhou
		printf("Falha no processamento do arquivo.\n");
		DEBUG("DEBUG: ERRO AO ABRIR CSV\n");
		goto fechar;
	}
	
	// CRIAR ARQUIVO BINÁRIO EM MODO ESCRITA
	filestream_bin = fopen(arquivoSaida, "wb"); // abre o arquivo de saída em modo escrita e como binário
	if(filestream_bin == NULL){ // se falhou
		printf("Falha no processamento do arquivo.\n");
		goto fechar;
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
		printf("Falha no processamento do arquivo.\n");
		DEBUG("DEBUG: PRIMEIRA LINHA NÃO CORRESPONDE AO ESPERADO\n");
		goto fechar;
	}
	
	while(fgets(linha, sizeof(linha), filestream_csv)){ // lê novas linhas até chegar ao fim do arquivo, quando fgets retorna NULL

		//DEBUG("DEBUG: LENDO REGISTRO DE DADOS\n");
	
		linha[strcspn(linha, "\r\n")] = '\0'; // faz a linha terminar em \0
		REG_DADOS_STRUCT* registro_lido = ler_linha_csv(linha);
		
		// ESCREVER O REGISTRO NO BINÁRIO E NAS ESTRUTURAS

		escreve_registro(registro_lido, filestream_bin); 
		proxRRN++;

		free(registro_lido->nomeEstacao);
		free(registro_lido->nomeLinha);
		free(registro_lido);

	}

	fclose(filestream_csv);
	fclose(filestream_bin);
	atualizar_cabecalho(arquivoSaida, -1, proxRRN);
	
	// EXIBINDO ARQUIVO

	BinarioNaTela(arquivoSaida);
	#ifdef PRINT_ERROS
	ExibirBinario(arquivoSaida);
	#endif 

	return;
	
	// FECHAR ARQUIVOS
	fechar:
	
	if(filestream_csv != NULL){
		if(fclose(filestream_csv) != 0){
			printf("Falha no processamento do arquivo.\n");
			DEBUG("DEBUG: ERRO AO FECHAR CSV\n");
			exit(1);
		}
	}
	
	if(filestream_bin != NULL){
		if(fecha_binario(filestream_bin) != 0){
			printf("Falha no processamento do arquivo.\n");
			DEBUG("DEBUG: ERRO AO FECHAR BIN\n");
			exit(1);
		}
	}

	return;
}