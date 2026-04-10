/*
Inclusão de bibliotecas, definição de constantes, structs
e funções auxiliares que podem ser usadas por todo o código.
*/

#ifndef DEFINICOES_H
	#define DEFINICOES_H

	#include<stdlib.h>
	#include<stdbool.h>
	#include<stdio.h>
	#include<string.h>
	#include<ctype.h>
	

	#ifdef PRINT_ERROS // se esse símbolo estiver definido
	#	define DEBUG(...) printf(__VA_ARGS__); // substitua DEBUG por printf e copie os argumentos
	#else // se não estiver definido
	# 	define DEBUG(...) // substitua DEBUG por nada, i.e, apague
	#endif

	#define HEADER_S 17 // O tamanho do registro de cabeçalho
	#define REG_DADOS_S 80 // O tamanho de cada registro de dados
	#define BYTES_FIXOS_S 37 // Quantidade de bytes usada para campos fixos no registro de dados, incluindo campos que armazenam tamanhos de nomes
	#define CAMPOS_STRINGS 2 // Quantidade de campos strings em um registro
	#define CAMPOS_INT 7 // Quantidade de campos inteiros em um registro (sem contar indicadores de tamanho)

	#define LIXO 0x24 // O conteúdo de bytes não utilizados é 0x24, $ em ascii, apelidado de LIXO nesse código
	#define LIXO_INT 0x24242424
	typedef struct reg_dados_struct REG_DADOS_STRUCT; // Representação do registro de dados na memória para ajudar na organização do código. Não é a mesma estrutura byte a byte.

	// Todos os scripts devem ver a organização interna do struct para que ele ajude a organizar as variáveis
	struct reg_dados_struct { // ESSE STRUCT SERVE APENAS PARA ORGANIZAR O CÓDIGO E ELE NÃO É ESCRITO DIRETAMENTE NO ARQUIVO!
		unsigned char removido;
		int proximo;
		int codEstacao;
		int codLinha;
		int codProxEstacao;
		int distProxEstacao;
		int codLinhaIntegra;
		int codEstIntegra;
		int tamNomeEstacao;
		char* nomeEstacao; // ponteiro para uma string de tamanho tamNomeEstacao + 1, terminada em \0
		int tamNomeLinha;
		char* nomeLinha; // ponteiro para uma string de tamanho tamNomeLinha + 1, terminada em \0
	};

	void ScanQuoteString(char *str);
	void BinarioNaTela(char *arquivo);

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
	char* processar_string(char* campo, int* tam);

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
	int processar_int(char* campo);

	/**Objetivo: ler e imprimir os dados de um registro de estação, tratando campos nulos e registros removidos.
	 * 
	 * Pré-condições:
	 *      arquivo deve estar aberto em modo de leitura binária
	 *      O cursor deve estar posicionado no início de um registro de dados
	 * 
	 * Pós-condições:
	 *      Se o registro estiver removido, imprime nada
	 *      Caso contrário, imprime os campos do registro no console, usando "NULO" para valores -1 ou strings vazias
	 *      O cursor fica no início do próximo registro
	 **/
	void print_registro(FILE* filestream_bin);

	/* Objetivo: ler uma linha do usuário com nomes de campos e valores de campos:
	 *  	Armazena quais campos foram lidos em mask
	 *   	Armazena os valores dos campos no struct registro_busca
	 *   	Campos cujo valor é NULO são marcados como -1
	 *   	Campos cujo valor não foi especificado são marcados com -1
	 *   	Para diferenciar um campo nulo e um campo não especificado, acesse mask
	 * Mask:
	 *   - 1: codEstacao
	 *   - 2: codLinha
	 *   - 4: codProxEstacao
	 *   - 8: distProxEstacao
	 *   - 16: codLinhaIntegra
	 *   - 32: codEstIntegra
	 *   - 64: nomeEstacao
	 *   - 128: nomeLinha
	 **/
	void ler_campos(REG_DADOS_STRUCT* registro_busca, int* mask);

	// Função para debug
	void ExibirBinario(char *arquivo);

#endif