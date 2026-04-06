#include<stdio.h>
#include<string.h>	
#include<stdlib.h>
#include "../include/func.h"
#include "../include/datamanager.h"

int main(void){ // não receberemos argumentos da linha de comando

	char comando_usuario[500]; // buffer para armazenar o comando do usuario que invoca as funcionalidades
	fgets(comando_usuario, sizeof(comando_usuario), stdin); // fgets é melhor que scanf pois verifica o tamanho do buffer e não para quando encontra o espaço. Inclui \n.
	comando_usuario[strcspn(comando_usuario, "\n")] = 0;
	
	int funcionalidade; // armazena o código de 1 a 6 da funcionalidade que o usuário quer usar
	funcionalidade = atoi(strtok(comando_usuario, " ")); // começa a interpretar o comando: extrai o primeiro trecho delimitado por espaço, e transforma em inteiro
	
	char* token1 = strtok(NULL, " ");
	char* token2 = strtok(NULL, " ");
	
	switch(funcionalidade){
		case 1:
			if(token1 && token2)
				func_1(token1, token2);
			break;
		case 2:
			if(token1)
				func_2(token1);
			break;
		case 3:
			if(token1 && token2)
				func_3(token1, atoi(token2));
			break;
		case 4:
			if(token1 && token2)
				func_4(token1, atoi(token2));
			break;
		case 5:
			if(token1 && token2)
				func_5(token1, atoi(token2));
			break;
		case 6:
			if(token1 && token2)
				func_6(token1, atoi(token2));
	}

	return 0;
}
