#include"../include/definicoes.h"

// Função fornecida por monitores da disciplina
void BinarioNaTela(char *arquivo) {
    FILE *fs;
    if (arquivo == NULL || !(fs = fopen(arquivo, "rb"))) {
        fprintf(stderr,
                "ERRO AO ESCREVER O BINARIO NA TELA (função binarioNaTela): "
                "não foi possível abrir o arquivo que me passou para leitura. "
                "Ele existe e você tá passando o nome certo? Você lembrou de "
                "fechar ele com fclose depois de usar?\n");
        return;
    }

    fseek(fs, 0, SEEK_END);
    size_t fl = ftell(fs);

    fseek(fs, 0, SEEK_SET);
    unsigned char *mb = (unsigned char *)malloc(fl);
    fread(mb, 1, fl, fs);

    unsigned long cs = 0;
    for (unsigned long i = 0; i < fl; i++) {
        cs += (unsigned long)mb[i];
    }

    printf("%lf\n", (cs / (double)100));

    free(mb);
    fclose(fs);
}

// função fornecida alterada para também tratar inteiros de forma correta
// Se o campo for nulo, retorna string vazia
void ScanQuoteString(char *str) { 
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


/**Objetivo: escrever o tamanho de uma string em um inteiro e retornar uma ćópia da string
 * 
 * Pré-condições:
 *      nenhuma
 * 
 * Pós-condições:
 *      Erro: tam recebe um != 0 mas o retorno é NULL
 *      Sucesso: escreve tam com o tamanho da string, e retorna uma cópia da string
 *      Chamador deve: apagar a cópia
 **/
char* processar_string(char* campo, int* tam){
    if(campo == NULL || strlen(campo) == 0){
        *tam = 0;
        return NULL;
    }else{
        *tam = strlen(campo);
        char* copia = strdup(campo);
        if(copia == NULL){
                DEBUG("DEBUG: ERRO AO COPIAR A STRING DO CAMPO LIDO");
        }
        return copia;
    }
}

/**Objetivo: receber uma string que descreve um inteiro e retornar
 * 
 * Pré-condições
 *      Parâmetro não pode ser nulo
 *      String deve ter tamanho maior que 0
 * 
 * Pós-condições
 *      Erro: retorna -1
 *      Sucesso: o inteiro descrito pela string
 **/
int processar_int(char* campo){
    if(campo == NULL || strlen(campo) == 0){
        return -1;
    }
    return atoi(campo);
}



/**Objetivo: ler um campo de tamanho variável de um arquivo binário e exibir seu conteúdo ou "NULO".
 * 
 * Pré-condições:
 *      arquivo deve estar aberto em modo de leitura binária.
 *      O cursor do arquivo deve estar posicionado no início do indicador de tamanho (int) do campo.
 * 
 * Pós-condições:
 *      Erro: se a leitura do tamanho falhar, a função retorna silenciosamente.
 *      Sucesso: exibe a string seguida de um espaço, ou a palavra "NULO " caso o tamanho seja 0.
 *      O cursor do arquivo é avançado para o final do campo lido.
 **/
void print_campo_string(FILE* filestream_bin) {
    int tam;
    // Lê o indicador de tamanho
    if(fread(&tam, 4, 1, filestream_bin) != 1) return;

    if(tam > 0){ // Se o tamanho do campo for maior que 0, o campo string não é NULO
        char *temp = (char* )malloc(tam + 1);
        if(temp){ // Se conseguiu alocar
            fread(temp, 1, tam, filestream_bin);
            temp[tam] = '\0'; // adiciona o '/0' no final da string para utilizar o 'printf'
            printf("%s ", temp);
            free(temp);
        }
    }else{ // Se o tamanho da string for 0, deve-se printar "NULO" ao invês de pular o campo
        printf("NULO ");
    }
}

/**Objetivo: ler e imprimir os dados de um registro de estação, tratando campos nulos e registros removidos.
 * 
 * Pré-condições:
 *      arquivo deve estar aberto em modo de leitura binária.
 *      O cursor deve estar posicionado no início de um registro de dados.
 * 
 * Pós-condições:
 *      Se o registro estiver removido, imprime nada. 
 *      Caso contrário, imprime os campos do registro no console, usando "NULO" para valores -1 ou strings vazias. 
 *      O cursor fica no início do próximo registro
 **/
void print_registro(FILE* filestream_bin){
    unsigned char removido;
    long pos_inicial = ftell(filestream_bin);

    fread(&removido, 1, 1, filestream_bin);
    if(removido == '1') return;

    // Pulamos o campo 'proximo' que não é impresso
    fseek(filestream_bin, 4, SEEK_CUR);

    // Lê os campos de inteiros
    int campos[6]; // 0-codEstacao, 1-codLinha, 2-codProxEstacao, 3-distProxEstacao, 4-codLinhaIntegra, 5-codEstIntegra
    fread(campos, 4, 6, filestream_bin);

    printf("%d ", campos[0]);

    // Nome estação
    print_campo_string(filestream_bin);

    printf("%d ", campos[1]);

    // Nome linha
    print_campo_string(filestream_bin);
    
    // Printa os demais campos inteiros
    for(int i = 2; i < 6; i++){
        if(campos[i] == -1){
            printf("NULO ");
        }else{
            printf("%d ", campos[i]);
        }
    }
    printf("\n");

    // Move o ponteiro para o início do próximo registro
    fseek(filestream_bin, pos_inicial + REG_DADOS_S, SEEK_SET);
}







/* Objetivo: ler uma linha do usuário com nomes de campos e valores de campos
    - Armazena quais campos foram lidos em mask
    - Armazena os valores dos campos no struct registro_busca
    - Campos cujo valor é NULO são marcados como -1
    - Campos cujo valor não foi especificado são marcados com -1
    - Para diferenciar um campo nulo e um campo não especificado, acesse mask
Mask:
    - 1: codEstacao
    - 2: codLinha
    - 4: codProxEstacao
    - 8: distProxEstacao
    - 16: codLinhaIntegra
    - 32: codEstIntegra
    - 64: nomeEstacao
    - 128: nomeLinha
*/
void ler_campos(REG_DADOS_STRUCT* registro_busca, int* mask){

    *mask = 0;
    char nomeCampo[64], valorCampo[64];
    int m; // quantidade de vezes que o par nomeCampo, valorCampo repete em uma linha de busca
    scanf(" %d", &m);

    // LENDO A LINHA
    for(int i = 0; i < m; i++){

        scanf(" %s", nomeCampo);
        ScanQuoteString(valorCampo);

        DEBUG("DEBUG: %s: %s\n", nomeCampo, valorCampo);

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
            registro_busca->nomeEstacao = processar_string(valorCampo, &registro_busca->tamNomeEstacao);
            *mask += 64;
        }else if(strcmp(nomeCampo, "nomeLinha") == 0){
            registro_busca->nomeLinha = processar_string(valorCampo, &registro_busca->tamNomeLinha);
            *mask += 128;
        }
    }
}

#ifdef DEBUG
// Função que de fato printa cada byte do binário na tela
void ExibirBinario(char *arquivo){
    FILE *filestream_bin;
    if (arquivo == NULL || !(filestream_bin = fopen(arquivo, "rb"))) {
        printf("Erro ao exibir o binário.\n");
        return;
    }

    unsigned char cabecalho[HEADER_S];
    unsigned char reg_dados[REG_DADOS_S];
    unsigned int byte_offset = 0;

    fseek(filestream_bin, 0, SEEK_SET);

    fread(cabecalho, HEADER_S, 1, filestream_bin);

    // Imprime o cabeçalho:

    printf("%5u: ", byte_offset);
    for(int i=0;i<HEADER_S; i++){
        printf("%0x ", cabecalho[i]);
        byte_offset++;
    }
    printf("\n");

    while(fread(reg_dados, REG_DADOS_S, 1, filestream_bin) == 1){
        printf("%5u: ", byte_offset);
        for(int i=0;i<REG_DADOS_S; i++){
            if(i < BYTES_FIXOS_S - CAMPOS_STRINGS*4){ // Se estiver printando um campo fixo, que não é tamanho de nome
                printf("%2x ", reg_dados[i]); // Para printar os inteiros
            }else{
                if(reg_dados[i] >= '!' && reg_dados[i] <= 'z'){
                    printf("%2c ", reg_dados[i]); // Para printar os nomes
                }else{
                    printf("%2x ", reg_dados[i]); // Para printar os tamanhos dos nomes
                }
            }
            byte_offset++;
        }
        printf("\n");
    }


}
#endif