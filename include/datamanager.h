/*
Operações que alteram o estado do binário. 
Gerencia as estruturas de dados relacionadas.
*/

#ifndef DATA_MANAGER_H
	#define DATA_MANAGER_H

	#include "definicoes.h"
	
	/**Objetivo: abrir um arquivo binário em modo especificado do Banco de Dados.  
	 * 
	 * Pré-condições: 
	 *      O caminho/nome do arquivo deve existir e ter as permissões necessárias
	 *      A necessidade de permissão de atualização (escrita) deve ser explicitada no parâmetro
	 *      O status do arquivo deve ser '1'
	 * 
	 * Pós condições: 
	 *      Erro: retorna nulo
	 *      Sucesso: retorna filestream do arquivo com o modo especificado, status '0' se for de atualização. Abrir em modo de leitura não afeta o status.
	 *      Chamador deve: fechar a filestream com a função fecha_binario() para impor status '1', se necessário.
	 */
	FILE* abre_binario(char* arquivoBin, bool escrita);
	
	/**Objetivo: fechar o arquivo binário mantendo o status como 'consistente'
	 * 
	 * Pré-condições:
	 *      filestream_bin deve estar aberta em modo que permita escrita, ou ser NULL
	 * 
	 * Pós-condições:
	 *      filestream estará fechada e não será mais possível acessá-la
	 */
	int fecha_binario(FILE* filestream_bin);

	/**Objetivo: atualizar o cabeçalho de um arquivo binário, recontando os registros e marcando-o como consistente
	 * 
	 * Pré-condições:
	 *      topo e proxRRN devem ser calculados corretamente pela função chamadora
	 * 
	 * Pós-condições:
	 *      arquivo fechado com status consistente 
	 **/
	void atualizar_cabecalho(char* arquivoBin, int topo, int proxRRN);

	/**Objetivo: extrair um registro de dados do disco e colocar na memória
	 * 
	 * Pré-condições:
	 *      Filestream aberta em modo que permita leitura
	 *      Cursor posicionada no começo de um registro de arquivos
	 *      Struct mem_destino alocado propriamente
	 * 
	 * Pós-condições:
	 *      Erro: retorna false
	 *      Sucesso: retorna true. O cursor aponta para o próximo registro de dados
	 *      Chamador deve: apagar o registro da memória quando terminar de usar, fechar a filestream com fecha_binario
	 **/
	bool load_registro(FILE* filestream_bin, REG_DADOS_STRUCT* mem_destino);

	/**Objetivo: verificar o RRN fornecido corresponde à busca realizada
	 * 
	 * Pré-condições:
	 *      Filestream binária em um modo que permita leitura
	 *      Struct chave e mask foram inicializados juntos
	 *      Mask só pode ser 0 ou 1
	 *      RRN dentro dos limites do binário
	 * 
	 * Pós-condições:
	 *      Erro: retorna false, com mensagem de DEBUG. Posição do cursor indefinida
	 *      Sucesso: retorna false se não corresponde ou se está removido, true se corresponde. Posição do cursor indefinida
	 *      Chamador deve: apagar a struct e possivelmente a mask, e fechar o filestream com fecha_binario
	 **/
	bool check_registro(REG_DADOS_STRUCT* chave, int mask, int RRN, FILE* bin);

	/**Objetivo: escrever um registro de dados no arquivo binário
	 * 
	 * Pré-condições:
	 *      Filestream binária em modo de atualização
	 *      Cursor em uma posição compatível com o início de um registro de dados
	 *      O registro a ser inserido deve ser válido
	 * 
	 * Pós-condições:
	 *      Erro: retorna false, sem alterar a posição do cursor
	 *      Sucesso: retorna true, com o cursor apontado para a posição do próximo registro de dados
	 * 		Chamador deve: apagar a struct e fechar o arquivo com fecha_binario
	 **/
	bool escreve_registro(REG_DADOS_STRUCT* registro_lido, FILE* filestream_bin);

	/**Objetivo: atualizar um registro no arquivo binário
	 * 
	 * Pré-condições:
	 *      Filestream binária em um modo que permita leitura e escrita
	 *      RRN dentro dos limites do binário
	 *      Struct campos_novos e mask foram inicializados juntos
	 * 
	 * Pós-condições:
	 *      Erro: retorna false, com posição do cursor indeterminada
	 *      Sucesso: retorna true, com o cursor apontado para a posição do próximo registro de dados
	 *      Chamador deve: apagar a struct e fechar o arquivo com fecha_binario
	 **/
	bool atualiza_registro(REG_DADOS_STRUCT* campos_novos, int mask, int RRN, FILE* filestream_bin);

#endif