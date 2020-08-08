/*----------------------------------*/
/* Autor: Elias Jose Moreira Torres */
/* <ejmt_ce@yahoo.com.br>           */
/* Este software esta disponivel    */
/* via licenca GPL                  */
/*----------------------------------*/

/*-------------------------------*/
/* xodo.cpp - programa principal */
/*-------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "anasin.h"
#include "xodo.h"

#define TAM_NOME_ARQ 80

PRIVATE char NomeXodo[TAM_NOME_ARQ];   /* nome do arquivo fonte */
PRIVATE char NomeAsm [TAM_NOME_ARQ];   /* nome do arquivo .asm */

/*----------------------------------*/
/* mostrar sintaxe de uso e abortar */
/*----------------------------------*/
void Abortar()
{
	puts("Sintaxe de uso:\n");
	puts("  xodo 'prog.xod'");
	puts("  onde 'prog.xod' eh o programa em xodo a ser compilado.\n");
	exit(1);
}

/*------------------------------------------------*/
/* obter nome do arquivo com a extensao fornecida */
/*------------------------------------------------*/
PRIVATE void ObterNome(char *NomeArq, char *Extensao)
{
  char *PtrArq = NomeArq;
  char *PtrExt = Extensao;
  int   TemPonto;  /* indica se tem ponto no nome do arquivo */

  TemPonto = 0;
  while (*PtrArq) /* pesquisa todo o nome do arquivo */
    if (*PtrArq++ == '.') /* se tiver ponto sai do laco */
    {
      TemPonto = 1;
      break;
    }
  if (! TemPonto) /* se nao tinha ponto*/
    *PtrArq++ = '.'; /* acrescenta o ponto */
  *PtrArq++ = *PtrExt++;
  *PtrArq++ = *PtrExt++;
  *PtrArq   = *PtrExt;
}

/*--------------------*/
/* programa principal */
/*--------------------*/
PUBLIC int main(int NumArg, char *Arg[])
{
  puts("************************************");
  puts("* XODO - O Compilador              *");
  puts("* (32 bits - implementado em C)    *");
  puts("* Autor: Elias Jose Moreira Torres *");
  puts("* <ejmt_ce@yahoo.com.br>           *");
  puts("************************************");
  puts("\n");
  switch(NumArg)
  {
    case  1 : Abortar();
              break;
    case  2 : strcpy(NomeXodo, Arg[1]);
              break;
  }
  strcpy(NomeAsm, NomeXodo);
  ObterNome(NomeAsm, "s");
  printf("Gerando %s ...\n", NomeAsm);
  IniciarAnalisadorSintatico(NomeXodo, NomeAsm);
  TerminarAnalisadorSintatico();
  return 0;
}
