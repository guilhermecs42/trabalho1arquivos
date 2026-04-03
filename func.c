#include "func.h"
#include<stdlib.h>
#include<stdbool.h>
#include<stdio.h>
#include<string.h>
#include<ctype.h>

#define HEADER_S 17 // O tamanho do cabeçalho do registro
#define REG_DADOS_S 80 // O tamanho de cada registro
#define BYTES_FIXOS_S 37 // Número de bytes em campos fixos no registro

#define LIXO 0x24 // O conteúdo de bytes não utilizados é 0x24, $ em ascii, apelidado de LIXO nesse código
#define LIXO_INT 0x24242424 // Um campo de tipo inteiro não utilizado teria valor $$$$, ou 0x24242424, apelidado de LIXO_INT nesse código
typedef struct reg_dados_struct REG_DADOS_STRUCT; // Essa struct é uma representação do registro de dados na memória. Não é exatamente a mesma estrutura, pois as strings foram substituídas por ponteiros, e elas terminam em \0, diferentemente da versão em disco.

struct reg_dados_struct {
	unsigned char removido;
	int proximo;
	int codEstacao;
	int codLinha;
	int codProxEstacao;
	int distProxEstacao;
	int codLinhaIntegra;
	int codEstIntegra;
	int tamNomeEstacao; // até aqui são 1 + 8*4 = 33 bytes
	char* nomeEstacao; // ponteiro para uma string de tamanho tamNomeEstacao + 1, terminada em \0
	int tamNomeLinha;
	char* nomeLinha; // ponteiro para uma string de tamanho tamNomeLinha + 1, terminada em \0
};
//
//
//
//


// -----------------------FUNCOES AUXILIARES --------------------------- //

void ScanQuoteString(char *str) { // função fornecida alterada para também tratar inteiros de forma correta
    char R;

    while ((R = getchar()) != EOF && isspace(R))
        ; // ignorar espaços, \r, \n...

    if (R == 'N' || R == 'n') { // campo NULO
        getchar();
        getchar();
        getchar();       // ignorar o "ULO" de NULO.
        strcpy(str, ""); // copia string vazia
    } else if (R == '\"') {
        if (scanf("%[^\"]", str) != 1) { // ler até o fechamento das aspas
            strcpy(str, "");
        }
        getchar();         // ignorar aspas fechando
	} else if (R != EOF) { 
        int i = 0;
        str[i++] = R;
        // Lê caractere por caractere até encontrar um delimitador (espaço, tab, newline ou aspas)
        while ((R = getchar()) != EOF && !isspace(R) && R != '\"') {
            str[i++] = R;
        }
        str[i] = '\0';
        
        // Se parou por causa de uma aspa, devolve ela para o buffer (ungetc) 
        // para que a próxima chamada trate como início de string se necessário
        if (R == '\"') ungetc(R, stdin);
    }
}

// Função auxiliar para processar strings
void processar_string(char* campo, int* tam, char** destino){
    if(campo == NULL || strlen(campo) == 0){
        *tam = 0;
        *destino = NULL;
    }else{
        *destino = strdup(campo);
        *tam = strlen(*destino);
    }
}

// Função auxiliar para processar inteiros
int processar_int(char* campo){
    if(campo == NULL || strlen(campo) == 0){
        return -1;
    }
    return atoi(campo);
}

// essa função recebe uma linha do arquivo csv e retorna um struct REG_DADOS_STRUCT contendo as informações naquela linha
REG_DADOS_STRUCT* ler_linha_csv(char* linha){
    if(linha == NULL) return NULL;

    REG_DADOS_STRUCT* registro_lido = (REG_DADOS_STRUCT*) malloc(sizeof(REG_DADOS_STRUCT));
    registro_lido->removido = '0'; // Seguindo a lógica de registro ativo
    registro_lido->proximo = -1;

    char *campo;
    char *ptr = linha; // strsep modifica o ponteiro, usamos um auxiliar

    for(int i = 0; i < 8; i++){
        // strsep extrai o texto até a próxima vírgula e coloca \0 no lugar dela
        campo = strsep(&ptr, ",");

        // Se strsep retornar NULL antes do 8º campo, a linha está incompleta
        if(campo == NULL){
            printf("Falha no processamento: linha incompleta.\n");
            free(registro_lido);
            exit(1);
        }

        switch(i){
            case 0: registro_lido->codEstacao = processar_int(campo); break;
            case 1: processar_string(campo, &registro_lido->tamNomeEstacao, &registro_lido->nomeEstacao); break;
            case 2: registro_lido->codLinha = processar_int(campo); break;
            case 3: processar_string(campo, &registro_lido->tamNomeLinha, &registro_lido->nomeLinha); break;
            case 4: registro_lido->codProxEstacao = processar_int(campo); break;
            case 5: registro_lido->distProxEstacao = processar_int(campo); break;
            case 6: registro_lido->codLinhaIntegra = processar_int(campo); break;
            case 7: registro_lido->codEstIntegra = processar_int(campo); break;
        }
    }

    // Se após os 8 campos o ponteiro 'ptr' não for NULL, existem campos extras
    if(ptr != NULL){
        printf("Falha no processamento: linha com campos extras.\n");
	    free(registro_lido);
		exit(1);
    }

    return registro_lido;
}

void escreve_registro(REG_DADOS_STRUCT* registro_lido, FILE* filestream_bin){

	fwrite(&(registro_lido->removido), 1, 1, filestream_bin);
	fwrite(&(registro_lido->proximo), 1, 4, filestream_bin);
	fwrite(&(registro_lido->codEstacao), 1, 4, filestream_bin);
	fwrite(&(registro_lido->codLinha), 1, 4, filestream_bin);
	fwrite(&(registro_lido->codProxEstacao), 1, 4, filestream_bin);
	fwrite(&(registro_lido->distProxEstacao), 1, 4, filestream_bin);
	fwrite(&(registro_lido->codLinhaIntegra), 1, 4, filestream_bin);
	fwrite(&(registro_lido->codEstIntegra), 1, 4, filestream_bin);
	fwrite(&(registro_lido->tamNomeEstacao), 1, 4, filestream_bin);

	if(registro_lido->tamNomeEstacao != 0){
		fwrite(registro_lido->nomeEstacao, 1, registro_lido->tamNomeEstacao, filestream_bin);
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

}

void print_campo_string(FILE* filestream_bin) {
    int tam;
    // Lê o indicador de tamanho
    if(fread(&tam, 4, 1, filestream_bin) != 1) return;

    if(tam > 0){ // Se o tamanho do campo for maior que 0, o campo string não é NULO
        char *temp = (char* )malloc(tam + 1);
        if(temp){
            fread(temp, 1, tam, filestream_bin);
            temp[tam] = '\0'; // adiciona o '/0' no final da string para utilizar o 'printf'
            printf("%s ", temp);
            free(temp);
        }
    }else{ // Se o tamanho da string for 0, deve-se printar "NULO" ao invês de pular o campo
        printf("NULO ");
    }
}

void print_registro(FILE* filestream_bin){
    unsigned char removido;
    long pos_inicial = ftell(filestream_bin);

    fread(&removido, 1, 1, filestream_bin);
    if(removido == '1') return;

    // Pulamos o campo 'proximo' que não é impresso
    fseek(filestream_bin, 4, SEEK_CUR);

    // Lê os campos de inteiros
    int campos[6]; // 0-codEstacao, 1-codLinha, 2-codProxEstacao, 3-distProxEstacao, 4-codLinhaIntegra, 5-codEstIntegra
    fread(campos, 4, 4, filestream_bin);

    printf("%d ", campos[0]);

    // Nome estação
    print_campo_string(filestream_bin);

    printf("%d ", campos[1]);

    // Nome linha
    print_campo_string(filestream_bin);
    
	// Printa os demais campos inteiros
    for(int i = 2; i < 6; i++){
        if(campos[i] != -1){
			printf("%d ", campos[i]);
		}else{
			printf("NULO ");
		}
    }
    printf("\n");

    // Move o ponteiro para o início do próximo registro
    fseek(filestream_bin, pos_inicial + REG_DADOS_S, SEEK_SET);
}

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
    processar_string(buffer, &registro_lido->tamNomeEstacao, &registro_lido->nomeEstacao);

    // codLinha
    ScanQuoteString(buffer);
    registro_lido->codLinha = (strlen(buffer) == 0) ? -1 : atoi(buffer);

    // nomeLinha
    ScanQuoteString(buffer);
    processar_string(buffer, &registro_lido->tamNomeLinha, &registro_lido->nomeLinha);

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

bool check_registro(REG_DADOS_STRUCT* busca, int mask, int RRN, FILE* bin){
    fseek(bin, RRN * REG_DADOS_S + HEADER_S, SEEK_SET);
    
    unsigned char removido;
    fread(&removido, 1, 1, bin);
    if(removido == '1') return false; // Registro removido

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
    if((mask & 1) && busca->codEstacao != cEst) return false;
    if((mask & 2) && busca->codLinha != cLin) return false;
    if((mask & 4) && busca->codProxEstacao != cProx) return false;
    if((mask & 8) && busca->distProxEstacao != dist) return false;
    if((mask & 16) && busca->codLinhaIntegra != cLinInt) return false;
    if((mask & 32) && busca->codEstIntegra != cEstInt) return false;

    // Verificação de Strings
    // Nome Estação
    fread(&tNomeE, 4, 1, bin);
    if(mask & 64){
        if(tNomeE > 0){
            char temp[100];
            fread(temp, 1, tNomeE, bin);
            temp[tNomeE] = '\0';
            // Se a busca não é nula e o arquivo tem dado, compara
            if(busca->nomeEstacao != NULL){
                if(strcmp(busca->nomeEstacao, temp) != 0) return false;
            }else{
                // Buscando por NULO mas encontrou dado no arquivo
                return false;
            }
        }else{
            // Campo no arquivo é NULO
            if(busca->nomeEstacao != NULL) return false;
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
            if(busca->nomeLinha != NULL){
                if(strcmp(busca->nomeLinha, temp) != 0) return false;
            }else{
                // Buscando por NULO mas encontrou dado no arquivo
                return false;
            }
        }else{
            // Campo no arquivo é NULO
            if(busca->nomeLinha != NULL) return false;
        }
    }else{
        fseek(bin, tNomeL, SEEK_CUR);
    }
    return true;
}

// lê e adiciona o campo lido ao registro chave de busca
void ler_campos_busca(REG_DADOS_STRUCT* registro_busca, int* mask){

    *mask = 0;
    char nomeCampo[64], valorCampo[64];
    int m;
    scanf("%d", &m);

    for(int i = 0; i < m; i++){

        scanf("%s", nomeCampo);
        ScanQuoteString(valorCampo);

        if(strcmp(nomeCampo, "codEstacao") == 0){
            registro_busca->codEstacao = (strlen(valorCampo) == 0) ? -1 : atoi(valorCampo);
            *mask += 1;
        }else if(strcmp(nomeCampo, "codLinha") == 0){
            registro_busca->codLinha = (strlen(valorCampo) == 0) ? -1 : atoi(valorCampo);
            *mask += 2;
        }else if(strcmp(nomeCampo, "codProxEstacao") == 0){
            registro_busca->codProxEstacao = (strlen(valorCampo) == 0) ? -1 : atoi(valorCampo);
            *mask += 4;
        }else if(strcmp(nomeCampo, "distProxEstacao") == 0){
            registro_busca->distProxEstacao = (strlen(valorCampo) == 0) ? -1 : atoi(valorCampo);
            *mask += 8;    
        }else if(strcmp(nomeCampo, "codLinhaIntegra") == 0){
            registro_busca->codLinhaIntegra = (strlen(valorCampo) == 0) ? -1 : atoi(valorCampo);
            *mask += 16;
        }else if(strcmp(nomeCampo, "codEstIntegra") == 0){
            registro_busca->codEstIntegra = (strlen(valorCampo) == 0) ? -1 : atoi(valorCampo);
            *mask += 32;
        }else if(strcmp(nomeCampo, "nomeEstacao") == 0){
            processar_string(valorCampo, &registro_busca->tamNomeEstacao, &registro_busca->nomeEstacao);
            *mask += 64;
        }else if(strcmp(nomeCampo, "nomeLinha") == 0){
            processar_string(valorCampo, &registro_busca->tamNomeLinha, &registro_busca->nomeLinha);
            *mask += 128;
        }
    }
}
//
//
//
//

// -----------------------FUNCOES PRINCIPAIS --------------------------- //

/**
 * @brief Funcionalidade 1: Importa registros de um arquivo CSV para um arquivo binário.
 * Simula o comando SQL 'CREATE TABLE'. Lê os dados de um arquivo .csv de entrada e os 
 * armazena em um arquivo binário estruturado com registro de cabeçalho e registros 
 * de dados de tamanho fixo (80 bytes).
 * @param arquivoSaida Nome do arquivo binário (.bin) a ser gerado.
 * @param arquivoEntrada Nome do arquivo de texto (.csv) contendo os dados originais.
 * @return void
 */
void func_1(char* arquivoSaida, char* arquivoEntrada){ 
	
	FILE* filestream_csv = NULL;
	FILE* filestream_bin = NULL;
	
	// ABRIR CSV EM MODO LEITURA
	filestream_csv = fopen(arquivoEntrada, "r"); // abre o arquivo csv em modo leitura e como texto
	if(filestream_csv == NULL){ // se falhou
		printf("Falha no processamento do arquivo.\n");
		printf("DEBUG: ERRO AO ABRIR CSV\n");
		goto fechar;
	}
	
	// CRIAR ARQUIVO BINÁRIO EM MODO ESCRITA
	filestream_bin = fopen(arquivoSaida, "wb"); // abre o arquivo de saída em modo escrita e como binário
	if(filestream_bin == NULL){ // se falhou
		printf("Falha no processamento do arquivo.\n");
		printf("DEBUG: ERRO AO ABRIR BIN\n");
		goto fechar;
	}
	
	printf("DEBUG: ARQUIVOS ABERTOS COM SUCESSO\n");
	
	// ESCREVER REGISTRO DE CABEÇALHO
	
	unsigned char cabecalho[] = { // a variável cabecalho é o endereço de memória de uma sequência de bytes, especificados abaixo. Esses são valores iniciais para o registro de cabeçalho, que deverá ser atualizado quando terminarmos a leitura.
		0x00,                   // status 
		0xff, 0xff, 0xff, 0xff, // topo
		0x00, 0x00, 0x00, 0x00, // proxRRN
		0x24, 0x24, 0x24, 0x24, // nroEstacoes, será atualizado depois
		0x24, 0x24, 0x24, 0x24  // nroParesEstacao 
	};
	
	fwrite(cabecalho, 1, sizeof(cabecalho), filestream_bin);
	
	// ESCREVER REGISTRO DE DADOS
	
	char linha[103]; // uma linha do arquivo csv. O cabeçalho tem caracteres, mais \n, mais \0
	fgets(linha, 103, filestream_csv);
	linha[strcspn(linha, "\r\n")] = '\0'; // faz a linha terminar em \0
	 
	if(strcmp(linha, "CodEstacao,NomeEstacao,CodLinha,NomeLinha,CodProxEst,DistanciaProxEst,CodLinhaInteg,CodEstacaoInteg") != 0){
		printf("Falha no processamento do arquivo.\n");
		printf("DEBUG: PRIMEIRA LINHA NÃO CORRESPONDE AO ESPERADO\n");
		goto fechar;
	}
	
	int proxRNN = 0;

	while(fgets(linha, sizeof(linha), filestream_csv)){
	
		printf("DEBUG: LENDO REGISTRO DE DADOS\n");
	
		linha[strcspn(linha, "\r\n")] = '\0'; // faz a linha terminar em \0
		REG_DADOS_STRUCT* registro_lido = ler_linha_csv(linha);
		
		// ESCREVER O REGISTRO NO BINÁRIO

		escreve_registro(registro_lido, filestream_bin);
		proxRNN++;

		free(registro_lido->nomeEstacao);
		free(registro_lido->nomeLinha);
		free(registro_lido);

		
		printf("DEBUG: REGISTRO DE DADOS ESCRITO COM SUCESSO\n");
	}

	unsigned char status = 0x01;
	fseek(filestream_bin, 0, SEEK_SET);
	fwrite(&status, 1, 1, filestream_bin);
	fseek(filestream_bin, 4, SEEK_CUR);
	fwrite(&proxRNN, 4, 1, filestream_bin);
	
	// FECHAR ARQUIVOS
	
	fechar:
	
	if(filestream_csv != NULL){
		if(fclose(filestream_csv) != 0){
			printf("Falha no processamento do arquivo.\n");
			printf("DEBUG: ERRO AO FECHAR CSV\n");
			exit(1);
		}
	}
	
	if(filestream_bin != NULL){
		if(fclose(filestream_bin) != 0){
			printf("Falha no processamento do arquivo.\n");
			printf("DEBUG: ERRO AO FECHAR BIN\n");
			exit(1);
		}
	}
	
	return;
}

/**
 * @brief Funcionalidade [2]: Recupera e exibe todos os registros do arquivo binário.
 * Simula o comando SQL 'SELECT FROM'. Percorre sequencialmente o arquivo binário 
 * e imprime todos os registros que não estão marcados como logicamente removidos. 
 * Campos nulos são exibidos como 'NULO'.
 * @param arquivoLeitura Nome do arquivo binário de onde os dados serão lidos.
 * @return void
 */
void func_2(char* arquivoLeitura){

	// abre o arquivo bin em modo leitura
	FILE* filestream_bin = fopen(arquivoLeitura, "rb");
	if(filestream_bin == NULL){ // se falhou
		printf("Falha no processamento do arquivo.\n");
		return;
	}

	// Lê o status do cabeçalho do arquivo
	unsigned char status;
	fread(&status, 1, 1, filestream_bin);
	if(status != 1){
		printf("Falha no processamento do arquivo.\n");
		fclose(filestream_bin);
		return;
	}

	fseek(filestream_bin, HEADER_S, SEEK_SET);

	unsigned char removido;
	bool reg_existe = false;

	while(fread(&removido, 1, 1, filestream_bin) == 1){
		if(removido == 1){ // O registro foi removido, salta o registro ao invés de ler
			fseek(filestream_bin, REG_DADOS_S - 1, SEEK_CUR);
		}else{
            fseek(filestream_bin, -1, SEEK_CUR);
			print_registro(filestream_bin);
			reg_existe = true;	
		}
	}

    if (!reg_existe) {
        printf("Registro inexistente.\n");
    }

	fclose(filestream_bin);
}

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

    // abre o arquivo binário em modo leitura
    FILE* filestream_bin = fopen(arquivoBin, "rb");
    if(filestream_bin == NULL){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    REG_DADOS_STRUCT* registros_de_busca = (REG_DADOS_STRUCT* )malloc(n*sizeof(REG_DADOS_STRUCT));   // Cria um vetor com os registros chaves para busca, 
    int* mask = (int* )malloc(n*sizeof(int));                                                        // e tambem um vetor com um bit mask dos campos utilizados para busca

    for(int i = 0; i < n; i++){ // lê as n entradas de argumentos para buscas
        ler_campos_busca(&registros_de_busca[i], &mask[i]);
    }

    int proxRRN;
    fseek(filestream_bin, 5, SEEK_SET);
    fread(&proxRRN, 4, 1, filestream_bin);

    for(int i = 0; i < n; i++){     // Começa uma busca sequencial no arquivo a partir do RRN = 0 para cada uma das n buscas

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

        if(mask[i] & 64) free(registros_de_busca[i].nomeEstacao);
        if(mask[i] & 128) free(registros_de_busca[i].nomeLinha);
    }

    free(registros_de_busca);
    free(mask);
}
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

    // abre o arquivo binério em modo de leitura e escrita
    FILE* filestream_bin = fopen(arquivoBin, "rb+");
    if(filestream_bin == NULL){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // Marca arquivo como inconsistente no início da operação
    unsigned char status_inconsistente = 0x00;
    fseek(filestream_bin, 0, SEEK_SET);
    fwrite(&status_inconsistente, 1, 1, filestream_bin);

    REG_DADOS_STRUCT* registros_de_busca = (REG_DADOS_STRUCT* )malloc(n*sizeof(REG_DADOS_STRUCT));   // Cria um vetor com os registros chaves para busca dos registros a serem excluídos, 
    int* mask = (int* )malloc(n*sizeof(int));                                                        // e tambem um vetor com um bit mask dos campos utilizados para busca

    for(int i = 0; i < n; i++){  // lê as n entradas de argumentos para buscas
        ler_campos_busca(&registros_de_busca[i], &mask[i]);
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
                fseek(filestream_bin, RRN * REG_DADOS_S + HEADER_S, SEEK_SET);
                fwrite(&removido, 1, 1, filestream_bin);
                fwrite(&topoPilha, 4, 1, filestream_bin);
                topoPilha = RRN; // atualiza o topo da pilha para o registro atual
            }
        }

        if (mask[i] & 64) free(registros_de_busca[i].nomeEstacao);
        if (mask[i] & 128) free(registros_de_busca[i].nomeLinha);
    }

    fseek(filestream_bin, 1, SEEK_SET);
    fwrite(&topoPilha, 4, 1, filestream_bin); // O topo da pilha no cabeçalho recebe o RRN do último registro a ser removido

    free(registros_de_busca);
    free(mask);

    // Marca como consistente e fecha o arquivo
    unsigned char status_consistente = 1;
    fseek(filestream_bin, 0, SEEK_SET);
    fwrite(&status_consistente, 1, 1, filestream_bin);

    fclose(filestream_bin);
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

    // Marca arquivo como inconsistente no início da operação
    unsigned char status_inconsistente = 0x00;
    fseek(filestream_bin, 0, SEEK_SET);
    fwrite(&status_inconsistente, 1, 1, filestream_bin);

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

    // Atualiza o cabeçalho final
    fseek(filestream_bin, 1, SEEK_SET);
    fwrite(&topo, 4, 1, filestream_bin);    // Novo topo da pilha
    fwrite(&proxRRN, 4, 1, filestream_bin); // Novo próximo RRN

    // Marca como consistente e fecha o arquivo
    unsigned char status_consistente = 1;
    fseek(filestream_bin, 0, SEEK_SET);
    fwrite(&status_consistente, 1, 1, filestream_bin);

    fclose(filestream_bin);
}
