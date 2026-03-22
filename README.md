# trabalho1arquivos
Trabalho 1 da disciplina de Organização de Arquivos, que consiste em implementar um pequeno sistema de banco de dados que manuseia arquivos que especificam a malha metroviária da Grande São Paulo.

Nosso arquivo main.c receberá argumentos na linha de comando. O primeiro será o código da funcionalidade, e os argumentos seguintes dependem da funcionalidade:

1 arquivoEntrada.csv arquivoSaida.bin 
Interpreta o arquivo csv e escreve um arquivo binário com registro de cabeçalho e registros de dados conforme a especificação

2 arquivoEntrada.bin 
Imprime na tela todos os registros, com os campos ordenados da forma que estavam no csv

3 arquivoEntrada.bin n 
Realiza n buscas e imprime na tela os resultados das n buscas. Cada busca pode ter vários critérios de valor de campo, especificados nas n linhas abaixo

4 arquivoEntrada.bin n 
Realiza n buscas e remove os resultados

5 arquivoEntrada.bin n 
Insere no arquivo n resgistros, especificados na linha abaixo como listas de valores de todos os campos, segundo a ordem dos campos no csv

6 arquivoEntrada.bin n 
Faz n ciclos de busca&atualização, sendo que cada uma pede duas linhas de input: uma especifica os critérios de quais registros devem ser atualizados, seguindo o mesmo formato da funcionalidade 3, e a outra especifica quais campos devem mudar, junto com os novos valores

## Funções comuns a várias funcionalidades

Teoricamente iremos receber uma função que printa o binário de todos os registros na tela, de alguma forma que possamos ler.
Também receberemos uma função que lê uma string delimitada por aspas duplas.

Printar todos os campos (separados por espaço) de um registro, printando dados nulos como NULO, na ordem em que estão armazenados no csv, e não no binário
Ler e *interpretar* (será que dá?) m critérios de busca (campos que são string são delimitados por aspas duplas):
m nomeCampo valorCampo nomeCampo valorCampo ... nomeCampom valorCampom
