/*----------------------------------*/
/* Autor: Elias Jose Moreira Torres */
/* <ejmt_ce@yahoo.com.br>           */
/* Este software esta disponivel    */
/* via licenca GPL                  */
/*----------------------------------*/

/*---------------------------------*/
/* anasin.h - analisador sintatico */
/*---------------------------------*/
#include "xodo.h"
#define TAM_MENSAGEM 30

/*-------------------------------------------------*/
/* associar um analisador sintatico com um arquivo */
/* fonte e um arquivo objeto                       */
/*-------------------------------------------------*/
PUBLIC void IniciarAnalisadorSintatico(char *NomeXodo, char *NomeAsm);
/*------------------------------------*/
/* desassociar o analisador sintatico */
/*------------------------------------*/
PUBLIC void TerminarAnalisadorSintatico();
