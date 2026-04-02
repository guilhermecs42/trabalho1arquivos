#include<stdio.h>
#include<string.h>	
#include<stdlib.h>
#include "func.h"

int main(void){ // não receberemos argumentos da linha de comando

	char comando_usuario[500]; // buffer para armazenar o comando do usuario que invoca as funcionalidades
	fgets(comando_usuario, sizeof(comando_usuario), stdin); // fgets é melhor que scanf pois verifica o tamanho do buffer e não para quando encontra o espaço. Inclui \n.
	comando_usuario[strcspn(comando_usuario, "\n")] = 0;
	
	int funcionalidade; // armazena o código de 1 a 6 da funcionalidade que o usuário quer usar
	funcionalidade = atoi(strtok(comando_usuario, " ")); // começa a interpretar o comando: extrai o primeiro trecho delimitado por espaço, e transforma em inteiro
	
	printf("Você escolheu a funcionalidade %d\n", funcionalidade); // DEBUG!!!
	
	switch(funcionalidade){
		case 1:
			func_1(strtok(NULL, " "), strtok(NULL, " ")); // executa a funcionalidade 1, passando como argumento os nomes dos arquivos especificados pelo usuário. A ordem dos argumentos na função é o contrário da ordem digitada pelo usuário, pois o C executa primeiro o strtok mais à direita, e por último o mais à esquerda
			break;
		case 2:
			func_2("saida.bin");
			break;
		case 3:
			func_3("saida.bin", 1);
			break;
		case 5:
			func_5("saida.bin", 1);
			func_2("saida.bin");
	}
	
	return 0;
}
