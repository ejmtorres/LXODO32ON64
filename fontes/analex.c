/*----------------------------------*/
/* Autor: Elias Jose Moreira Torres */
/* <ejmt_ce@yahoo.com.br>           */
/* Este software esta disponivel    */
/* via licenca GPL                  */
/*----------------------------------*/

/*------------------------------*/
/* analex.c - analisador lexico */
/*------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "analex.h"

PRIVATE FILE *ArqXod;     /* fluxo do arquivo fonte */
PRIVATE char *Buffer;     /* buffer para leitura do arquivo fonte */
PRIVATE int   Indice;     /* indice dentro do buffer */
PRIVATE int   Linha;      /* linha atual */
PRIVATE int   ItensLidos; /* numero de caracteres lidos de cada vez */
PRIVATE int   Caract;     /* cada caractere lido do arquivo fonte */
PRIVATE char *PalRes[]    /* lista de palavras reservadas */
                      = {"prog",     "proc",     "var",     "inteiro",
                         "cadeia",   "se",       "entao",   "senao",
                         "fimse",    "enquanto", "faca",    "fimenq",
                         "repita",   "ate",      "e",       "nao",
                         "ou",       "inicio",   "fim",     "ender",
                         "ponteiro", "par",     "valor",    "func",
                         "retorne",  "tipo",    "vetor",    "de",
                         "publico",  "privado", "externo",  "rotina",
                         0};
PRIVATE int   TknRes[]    /* lista dos respectivos tokens reservados */
                      = { PROG,       PROC,     VAR,      INTEIRO,
                          CADEIA,     SE,       ENTAO,    SENAO,
                          FIMSE,      ENQUANTO, FACA,     FIMENQ,
                          REPITA,     ATE,      E,        NAO,
                          OU,         INICIO,   FIM,      ENDER,
                          PONTEIRO,   PAR,      VALOR,    FUNC,
                          RETORNE,    TIPO,     VETOR,    DE,
                          PUBLICO,    PRIVADO,  EXTERNO,  ROTINA,
                          0};
/*---------------------------------------------------*/
/* ler no maximo TAM_BUFFER caracteres para o buffer */
/*---------------------------------------------------*/
PRIVATE int LerBuffer()
{
  return fread(Buffer, 1, TAM_BUFFER, ArqXod);
}
/*------------------------------------*/
/* obter proximo caractere do arquivo */
/*------------------------------------*/
PRIVATE int ObterCaractere()
{
  if (ItensLidos == TAM_BUFFER) /* se faltam itens */
  {
    if (Indice == TAM_BUFFER) /* se fim do buffer */
    {
      ItensLidos = LerBuffer();
      Indice     = 0;
    }
  }
  else                          /* se nao faltam mais itens */
  {
    if (Indice == ItensLidos) /* se fim de arquivo */
      return FIM_ARQ;
  }
  if (Buffer[Indice] == FIM_LIN) Linha++;
  return Buffer[Indice++];
}
/*-----------------------------------*/
/* associar um arquivo com um buffer */
/*-----------------------------------*/
PUBLIC void IniciarAnalisadorLexico(char *NomeXodo)
{
  if ((ArqXod = fopen(NomeXodo, "r")) == (FILE *)0)
  {
    printf("Erro de abertura do arquivo %s\n", NomeXodo);
    exit(1);
  }
  Buffer = malloc(TAM_BUFFER);
  if (! Buffer)
  {
    puts("Sem memoria para alocar buffer...");
    exit(1);
  }
  ItensLidos = LerBuffer();
  Indice     = 0;
  Linha      = 1;
  Caract     = ObterCaractere();
}
/*--------------------------------------------------*/
/* retornar a categoria se for palavra reservada ou */
/* 0 para indicar que o lexema eh um identificador  */
/*--------------------------------------------------*/
PRIVATE int PalavraReservada(char *Lexema)
{
  register int Indice;

  Indice = 0;
  while (PalRes[Indice])
  {
    if (! strcmp(Lexema, PalRes[Indice])) return TknRes[Indice];
    Indice++;
  }
  return 0;
}
/*--------------------------------------------------------*/
/* retornar a categoria se for simbolo de dois caracteres */
/* ou 0 em caso contrario                                 */
/*--------------------------------------------------------*/
PRIVATE int SimboloDuplo(char *Lexema)
{
  register int   Indice;
  auto     char *SmbDup[] = {"<=",    "<-",    ">=",    "<>",   0};
  auto     int   TknDup[] = {MENORIG, ATRIBUI, MAIORIG, DIFERE, 0};

  Indice = 0;
  while (SmbDup[Indice])
  {
    if (! strcmp(Lexema, SmbDup[Indice])) return TknDup[Indice];
    Indice++;
  }
  return 0;
}
/*-------------------------------------------*/
/* obter proximo token = (categoria, lexema) */
/*-------------------------------------------*/
PUBLIC int ObterToken(char *Lexema)
{
  int Estado; /* estado do automato finito usado para analise lexica */
  int IndLex; /* indice do lexema */
  int IndCad; /* indice da cadeia */
  int Token;  /* token encontrado */

  Estado    = 1;
  IndLex    = 0;
  IndCad    = 0;
  Lexema[0] = '\0';

  while (1)
  {
    switch(Estado)
    {
      case 1 : if (Caract == FIM_ARQ) return FIM_ARQ;
               if (isspace(Caract)) Caract = ObterCaractere();
               else
               if (Caract == '"')
               {
                 Caract = ObterCaractere();
                 Estado = 5;
               }
               else
               if (Caract == '#')
               {
                 Caract = ObterCaractere();
                 Estado = 6;
               }
               else
               if (isalpha(Caract))
               {
                 Lexema[IndLex++] = Caract;
                 Caract           = ObterCaractere();
                 Estado           = 2;
               }
               else
               if (isdigit(Caract))
               {
                 Lexema[IndLex++] = Caract;
                 Caract           = ObterCaractere();
                 Estado           = 4;
               }
               else
               {
                 Lexema[IndLex++] = Caract;
                 Caract           = ObterCaractere();
                 Estado           = 3;
               }
               break;
      case 2 : if (isalnum(Caract)||(Caract == '_'))
               {
                 Lexema[IndLex++] = Caract;
                 Caract           = ObterCaractere();
               }
               else
               {
                 Lexema[IndLex] = '\0';
                 Token          = PalavraReservada(Lexema);
                 if (Token) return Token;
                 else
                 {
                   Lexema[TAM_IDENT] = '\0';
                   return IDENTIF;
                 }
               }
               break;
      case 3 : Lexema[IndLex++] = Caract;
               Lexema[2]        = '\0';
               Token            = SimboloDuplo(Lexema);
               if (Token)
               {
                 Caract = ObterCaractere();
                 return Token;
               }
               Lexema[1] = '\0';
               return Lexema[0];
      case 4 : if (isdigit(Caract))
               {
                 Lexema[IndLex++] = Caract;
                 Caract           = ObterCaractere();
               }
               else
               {
                 Lexema[IndLex] = '\0';
                 if (IndLex <= TAM_NUMERO) return NUMERO;
                 else                      return ERROLEX;
               }
               break;
      case 5 : if (Caract == '"')
               {
                 Caract         = ObterCaractere();
                 Lexema[IndCad] = '\0';
                 if (IndCad <= TAM_CADEIA) return STRING;
                 else                      return ERROLEX;
               }
               else
               if (Caract == FIM_ARQ) return ERROLEX;
               else
               {
                 Lexema[IndCad++] = Caract;
                 Caract           = ObterCaractere();
               }
               break;
      case 6 : if (Caract == FIM_ARQ) return ERROLEX;
               if (Caract == '#')
               {
                 Caract = ObterCaractere();
                 Estado = 1;
               }
               else Caract = ObterCaractere();
               break;
    }
  }
}
/*----------------------*/
/* obter linha corrente */
/*----------------------*/
PUBLIC int ObterLinha()
{
  return Linha;
}
/*-----------------------------------*/
/* desassocia o arquivo com o buffer */
/*-----------------------------------*/
PUBLIC void TerminarAnalisadorLexico()
{
  fclose(ArqXod);
}
