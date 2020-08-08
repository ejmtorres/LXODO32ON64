/*----------------------------------*/
/* Autor: Elias Jose Moreira Torres */
/* <ejmt_ce@yahoo.com.br>           */
/* Este software esta disponivel    */
/* via licenca GPL                  */
/*----------------------------------*/
#include "xodo.h"

/*------------------------------*/
/* analex.h - analisador lexico */
/*------------------------------*/
/* constantes para ... */
#define TAM_BUFFER 2048 /* tamanho do buffer */
#define TAM_IDENT  30   /* tamanho maximo do identificador */
#define TAM_NUMERO 10   /* numero de digitos em numero */
#define TAM_CADEIA 128  /* tamanho maximo de uma cadeia */
#define TAM_LEXEMA 128  /* tamanho maximo de um item lexico */
#define TAM_LINHA  128  /* tamanho da linha do arquivo fonte .xod */
#define FIM_LIN    10   /* caractere de fim de linha */
#define FIM_CAD    '\0' /* delimitador de final de cadeia */
/* categorias para ... */
#define FIM_ARQ 300     /* fim de arquivo */
#define IDENTIF 310     /* identificador */
#define NUMERO  320     /* numero */
#define STRING  330     /* cadeia de caracteres */
#define ERROLEX 340     /* indica uma mal formacao de token */
/* categoria para simbolos de dois caracteres */
#define MAIORIG 400     /* maior ou igual (>=) */
#define MENORIG 401     /* menor ou igual (<=) */
#define ATRIBUI 402     /* atribuicao (<-) */
#define DIFERE  403     /* diferente (<>) */
/* categorias de palavras reservadas */
#define PROG 500
#define PROC PROG + 1
#define VAR  PROC + 1
#define INTEIRO VAR + 1
#define CADEIA INTEIRO + 1
#define INICIO CADEIA + 1
#define FIM INICIO + 1
#define SE FIM + 1
#define ENTAO SE + 1
#define SENAO ENTAO + 1
#define FIMSE SENAO + 1
#define ENQUANTO FIMSE + 1
#define FACA ENQUANTO + 1
#define FIMENQ FACA + 1
#define REPITA FIMENQ + 1
#define ATE REPITA + 1
#define E ATE + 1
#define OU E + 1
#define NAO OU + 1
#define ENDER NAO + 1
#define PONTEIRO ENDER + 1
#define PAR PONTEIRO + 1
#define VALOR PAR + 1
#define FUNC VALOR + 1
#define RETORNE FUNC + 1
#define TIPO RETORNE + 1
#define VETOR TIPO + 1
#define DE VETOR + 1
#define PUBLICO DE + 1
#define PRIVADO PUBLICO + 1
#define EXTERNO PRIVADO + 1
#define ROTINA EXTERNO + 1

/*-----------------------------------*/
/* associar um arquivo com um buffer */
/*-----------------------------------*/
PUBLIC void IniciarAnalisadorLexico(char *NomeXodo);
/*-------------------------------------------*/
/* obter proximo token = (categoria, lexema) */
/*-------------------------------------------*/
PUBLIC int ObterToken(char *Lexema);
/*----------------------*/
/* obter linha corrente */
/*----------------------*/
PUBLIC int ObterLinha();
/*-----------------------------------*/
/* desassocia o arquivo com o buffer */
/*-----------------------------------*/
PUBLIC void TerminarAnalisadorLexico();
