/*----------------------------------*/
/* Autor: Elias Jose Moreira Torres */
/* <ejmt_ce@yahoo.com.br>           */
/* Este software esta disponivel    */
/* via licenca GPL                  */
/*----------------------------------*/

/*---------------------------------*/
/* anasin.c - analisador sintatico */
/*---------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "analex.h"
#include "anasin.h"
#include "anasem.h"

PRIVATE int  Token;
PRIVATE char Lexema[TAM_LEXEMA+1];
PRIVATE void Expressao(int *T);
PRIVATE void Comandos();

/*----------------------------------*/
/* mostrar erro sintatico e abortar */
/*----------------------------------*/
PRIVATE void Abortar(int N)
{
  char Mensagem[TAM_MENSAGEM];

  switch(N)
  {
    case  1 : strcpy(Mensagem, "identificador");         break;
    case  2 : strcpy(Mensagem, ")");                     break;
    case  3 : strcpy(Mensagem, "var ou rotina ou prog"); break;
    case  4 : strcpy(Mensagem, ";");                     break;
    case  5 : strcpy(Mensagem, ".");                     break;
    case  6 : strcpy(Mensagem, "fim de arquivo");        break;
    case  7 : strcpy(Mensagem, "inicio");                break;
    case  8 : strcpy(Mensagem, "fim");                   break;
    case  9 : strcpy(Mensagem, "<-");                    break;
    case 10 : strcpy(Mensagem, "expressao");             break;
    case 11 : strcpy(Mensagem, ":");                     break;
    case 12 : strcpy(Mensagem, "tipo");                  break;
    case 13 : strcpy(Mensagem, "entao");                 break;
    case 14 : strcpy(Mensagem, "fimse");                 break;
    case 15 : strcpy(Mensagem, "faca");                  break;
    case 16 : strcpy(Mensagem, "fimenq");                break;
    case 17 : strcpy(Mensagem, "ate");                   break;
    case 18 : strcpy(Mensagem, "(");                     break;
    case 19 : strcpy(Mensagem, "vetor");                 break;
    case 20 : strcpy(Mensagem, "[");                     break;
    case 21 : strcpy(Mensagem, "]");                     break;
    case 22 : strcpy(Mensagem, "de");                    break;
    case 23 : strcpy(Mensagem, "numero");                break;
    case 24 : strcpy(Mensagem, "=");                     break;
    case 25 : strcpy(Mensagem, "tipo basico");           break;
    case 26 : strcpy(Mensagem, "proc ou func");          break;
  }
  printf("Linha %d ==> %s esperado(a)!\n", ObterLinha(), Mensagem);
  exit(N);
}
/*---------------------*/
/* <, <=, >, >=, =, <> */
/*---------------------*/
PRIVATE int Relacao(char *Op)
{
  char *OpRel[] = {"<", "<=", "<>", ">", ">=", "=", 0};
  int   Ind;

  Ind = 0;
  while (OpRel[Ind])
    if (! strcmp(OpRel[Ind++], Op)) return 1;
  return 0;
}
/*---------------------*/
/* lista de expressoes */
/*---------------------*/
PRIVATE void ListaDeExpressoes(int *NA, int *NB)
{
  int T;    /* tipo da expressao */

  Expressao(&T);
  EmpilharExpressao();
  *NA = *NA + 1;
  switch(T)
  {
    case INTEIRO:  *NB = *NB + NB_INTEIRO;  break;
    case PONTEIRO: *NB = *NB + NB_PONTEIRO; break;
    case CADEIA:   *NB = *NB + NB_PONTEIRO; break;
  }

  while (Token == ',')
  {
    Token = ObterToken(Lexema);
    Expressao(&T);
    EmpilharExpressao();
    *NA = *NA + 1;
    switch(T)
    {
      case INTEIRO:  *NB = *NB + NB_INTEIRO;  break;
      case PONTEIRO: *NB = *NB + NB_PONTEIRO; break;
      case CADEIA:   *NB = *NB + NB_PONTEIRO; break;
    }
  }
}
/*------------------*/
/* lista de indices */
/*------------------*/
PRIVATE void ListaDeIndices(int *NI, int D)
{
  int T;    /* tipo da expressao */

  Expressao(&T);
  GerarIndice1();
  *NI = *NI + 1;     /* mais um indice */
  while (Token == ',')
  {
    D++; /* local das proximas dimensoes */
    Token = ObterToken(Lexema);
    Expressao(&T);
    GerarIndiceN(D);
    *NI = *NI + 1;     /* mais um indice */
  }
}
/*-------*/
/* fator */
/*-------*/
PRIVATE void Fator(int *R, int *T)
{
  auto     int TR;      /* topo dos registradores */
  register int Cat;     /* categoria */
  auto     int NA;      /* numero de argumentos ou indices */
  auto     int NB;      /* numero de bytes empilhados */
  auto     int TA;      /* topo auxilar dos registradores */
  auto     int P;       /* entrada para tipo estruturado */
  char Alvo[TAM_NOME];  /* nome da funcao ou vetor */

  switch(Token)
  {
    case IDENTIF : Cat = ObterCategoria(Lexema);
                   /* funcao declarada ou externa */
                   if ((Cat == CFUNC)||(Cat == NADA))
                   {
                     strcpy(Alvo, Lexema);
                     Token = ObterToken(Lexema);
                     IniciarFuncao(&NA, &NB, &TA);
                     if (Token == '(')
                     {
                        Token = ObterToken(Lexema);
                        if (Token == ')') Token = ObterToken(Lexema);
                        else
                        {
                          ListaDeExpressoes(&NA, &NB);
                          if (Token == ')') Token = ObterToken(Lexema);
                          else Abortar(2);
                        }
                     }
                     TerminarFuncao(R, T, NA, NB, TA, Alvo);
                   }
                   else
                   /* variavel vetor */
                   if (Cat == CVET)
                   {
                     strcpy(Alvo, Lexema);
                     Token = ObterToken(Lexema);
                     IniciarFatorVetor(&NA, &TA, R, T, &P, Alvo);
                     if (Token == '[')
                     {
                       Token = ObterToken(Lexema);
                       ListaDeIndices(&NA, P);
                       if (Token == ']') Token = ObterToken(Lexema);
                       else Abortar(21);
                     }
                     else  Abortar(20);
                     TerminarFatorVetor(NA, TA, *R, Alvo);
                   }
                   /* variavel simples */
                   else
                   {
                     GerarFator(R, T, Token, Lexema);
                     Token = ObterToken(Lexema);
                   }
                   break;
    case NUMERO  : GerarFator(R, T, Token, Lexema);
                   Token = ObterToken(Lexema);
                   break;
    case VALOR   : Token = ObterToken(Lexema);
                   if (Token == '(') Token = ObterToken(Lexema);
                   else Abortar(18);
                   if (Token == IDENTIF)
                   {
                     GerarFatorValor(R, T, Lexema);
                     Token = ObterToken(Lexema);
                   }
                   else Abortar(1);
                   if (Token == ')') Token = ObterToken(Lexema);
                   else Abortar(2);
                   break;
    case '('     : SalvarRegistradores(&TR);
                   Token = ObterToken(Lexema);
                   Expressao(T);
                   if (Token == ')') Token = ObterToken(Lexema);
                   else Abortar(2);
                   RecuperarRegistradores(R, TR);
                   break;
    default      : Abortar(10);
  }
}
/*-------*/
/* termo */
/*-------*/
PRIVATE void Termo(int *R, int *T)
{
  int R1, R2; /* registradores para os fatores */
  int T1, T2; /* tipos dos fatores */
  int Operador;

  Fator(&R1, &T1);
  while ((Token == '*')||(Token == '/'))
  {
    Operador = Token;
    Token    = ObterToken(Lexema);
    Fator(&R2, &T2);
    Operar(Operador, R1, R2, T1, T2);
  }
  *R = R1;
  *T = T1;
}
/*--------------------*/
/* expressoes simples */
/*--------------------*/
PRIVATE void ExpressaoSimples(int *R, int *T)
{
  int R1, R2; /* registradores de cada termo */
  int T1, T2; /* tipos de cada termo */
  int Operador;
  int Menos; /* setado se tem menos unario */

  Menos = 0;
  if (Token == '-')
  {
    Menos = 1;
    Token = ObterToken(Lexema);
  }
  Termo(&R1, &T1);
  GerarMenos(Menos, R1);
  while ((Token == '+')||(Token == '-'))
  {
    Operador = Token;
    Token    = ObterToken(Lexema);
    Termo(&R2, &T2);
    Operar(Operador, R1, R2, T1, T2);
  }
  *R = R1;
  *T = T1;
}
/*-----------*/
/* expressao */
/*-----------*/
PRIVATE void Expressao(int *T)
{
  int R1, R2; /* registradores das expressoes simples */
  int T1, T2; /* tipos das expressoes simples */
  char Op[3]; /* operador das expressoes simples */

  if (Token == STRING)
  {
    GerarCadeia(Lexema, T);
    Token = ObterToken(Lexema);
  }
  else
  if (Token == ENDER)
  {
    Token = ObterToken(Lexema);
    if (Token == '(') Token = ObterToken(Lexema);
    else Abortar(18);
    if (Token == IDENTIF)
    {
      GerarEndereco(Lexema, T);
      Token = ObterToken(Lexema);
    }
    else Abortar(1);
    if (Token == ')') Token = ObterToken(Lexema);
    else Abortar(2);
  }
  else
  {
    ExpressaoSimples(&R1, &T1);
    if (Relacao(Lexema))
    {
      strcpy(Op, Lexema);
      Token = ObterToken(Lexema);
      ExpressaoSimples(&R2, &T2);
      OperarRelacao(Op, R1, R2, T1, T2);
    }
    *T = T1;
  }
}
/*-------------------*/
/* lista de inteiros */
/*-------------------*/
PRIVATE void ListaDeInteiros(int *NDim)
{
  *NDim = 0;
  if (Token == NUMERO)
  {
    DefinirN_ezimaDimensao(ObterNomeDoTipo(), Lexema);   /* def 1a dimensao */
    Token = ObterToken(Lexema);
    *NDim += 1;
  }
  else Abortar(23);
  while (Token == ',')
  {
    Token = ObterToken(Lexema);
    if (Token == NUMERO)
    {
      DefinirN_ezimaDimensao(ObterNomeDoTipo(), Lexema); /* def N-esima dim */
      Token = ObterToken(Lexema);
      *NDim += 1;
    }
    else Abortar(23);
  }
}
/*------------------------------------------*/
/* construtor = vetor [ lista de indices ] */
/*------------------------------------------*/
PRIVATE void Construtor()
{
  int NDim;

  if (Token == VETOR) Token = ObterToken(Lexema);
  else Abortar(19);
  if (Token == '[') Token = ObterToken(Lexema);
  else Abortar(20);
  ListaDeInteiros(&NDim);
  if (Token == ']') Token = ObterToken(Lexema);
  else Abortar(21);
  if (Token == DE) Token = ObterToken(Lexema);
  else Abortar(22);
  switch(Token)
  {
    case INTEIRO  :
    case CADEIA   :
    case PONTEIRO : break;
    default       : Abortar(12);
  }
  DefinirTipoBase(Token, NDim);
  Token = ObterToken(Lexema);
}
/*---------------------*/
/* declaracao de tipos */
/*---------------------*/
PRIVATE void DeclaracaoDeTipos()
{
  if (Token == IDENTIF)
  {
    DefinirNomeDoTipo(Lexema);
    Token = ObterToken(Lexema);
    if (Token == '=') Token = ObterToken(Lexema);
    else Abortar(24);
    Construtor();
    if (Token == ';') Token = ObterToken(Lexema);
    else Abortar(4);
  }
  else Abortar(1);
  while (Token == IDENTIF)
  {
    DefinirNomeDoTipo(Lexema);
    Token = ObterToken(Lexema);
    if (Token == '=') Token = ObterToken(Lexema);
    else Abortar(24);
    Construtor();
    if (Token == ';') Token = ObterToken(Lexema);
    else Abortar(4);
  }
}
/*------*/
/* tipo */
/*------*/
PRIVATE void Tipo(int *T)
{
  switch(Token)
  {
    case INTEIRO  : *T = INTEIRO;         break;
    case CADEIA   : *T = CADEIA;          break;
    case PONTEIRO : *T = PONTEIRO;        break;
    case IDENTIF  : *T = ObterTipo(Lexema); break; /* tipo estruturado */
    default       : Abortar(12);
  }
  Token = ObterToken(Lexema);
}
/*--------------------*/
/* lista de variaveis */
/*--------------------*/
PRIVATE void ListaDeVariaveis(int Escopo, int T)
{
  long N; /* numero de bytes locais */

  N = 0;
  if (Token == IDENTIF)
  {
    DefinirVariavel(Escopo, Lexema, T, &N);
    Token = ObterToken(Lexema);
  }
  else Abortar(1);
  while (Token == ',')
  {
    Token = ObterToken(Lexema);
    if (Token == IDENTIF)
    {
      DefinirVariavel(Escopo, Lexema, T, &N);
      Token = ObterToken(Lexema);
    }
    else Abortar(1);
  }
  DefinirVariavelLocal(N);
}
/*--------------------------------------*/
/* decvar = tipo : lista de variaveis ; */
/*--------------------------------------*/
PRIVATE void DeclaracaoDeVariavel()
{
  int T;
  int Escopo;

  Escopo = PUBLICO;
  switch(Token)
  {
    case PUBLICO :
    case PRIVADO :
    case EXTERNO : Escopo = Token;
                   Token  = ObterToken(Lexema);
  }
  Tipo(&T);
  if (Token == ':') Token = ObterToken(Lexema);
  else Abortar(11);
  ListaDeVariaveis(Escopo, T);
  if (Token == ';') Token = ObterToken(Lexema);
  else Abortar(4);
}
/*-------------------------*/
/* declaracao de variaveis */
/*-------------------------*/
PRIVATE void DeclaracaoDeVariaveis()
{
  DeclaracaoDeVariavel();
  while ((Token == INTEIRO)  ||
         (Token == CADEIA)   ||
         (Token == PONTEIRO) ||
         (Token == IDENTIF)  ||
         (Token == PUBLICO)  ||
         (Token == PRIVADO)  ||
         (Token == EXTERNO)) DeclaracaoDeVariavel();
}
/*------------*/
/* atribuicao */
/*------------*/
PRIVATE void ComandoDeAtribuicao()
{
  char Alvo[TAM_NOME];
  auto     int T;       /* tipo base do alvo */
  auto     int P;       /* tipo estruturado do alvo */
  register int Cat;     /* categoria do alvo */
  auto     int NI;      /* numero de indices */

  strcpy(Alvo, Lexema);
  Cat   = ObterCategoria(Alvo);
  Token = ObterToken(Lexema);
  if (Cat == CVET) /* variavel vetor */
  {
    IniciarAtribuicaoVetor(&NI, &T, &P, Alvo);
    if (Token == '[')
    {
      Token = ObterToken(Lexema);
      ListaDeIndices(&NI, P);
      if (Token == ']') Token = ObterToken(Lexema);
      else Abortar(21);
    }
    else  Abortar(20);
    TerminarAtribuicaoVetor(NI, Alvo);
    if (Token == ATRIBUI) Token = ObterToken(Lexema);
    else Abortar(9);
    Expressao(&T);
    GerarAtribuicaoVetor(Alvo, T);
  }
  else             /* variavel simples */
  {
    if (Token == ATRIBUI) Token = ObterToken(Lexema);
    else Abortar(9);
    Expressao(&T);
    GerarAtribuicao(Alvo, T);
  }
}
/*-------------------------*/
/* chamada de procedimento */
/*-------------------------*/
PRIVATE void ComandoDeChamada()
{
  char Alvo[TAM_NOME]; /* nome do procedimento */
  int  NA;             /* numero de argumentos */
  int  NB;             /* numero de bytes empilhados */

  strcpy(Alvo, Lexema);
  NA = 0;
  NB = 0;
  Token = ObterToken(Lexema);
  if (Token == '(')
  {
    Token = ObterToken(Lexema);
    if (Token == ')') Token = ObterToken(Lexema);
    else
    {
      ListaDeExpressoes(&NA, &NB);
      if (Token == ')') Token = ObterToken(Lexema);
      else Abortar(2);
    }
  }
  GerarChamada(Alvo, NA, NB);
}
/*----------------------------*/
/* comando retorne Expressao */
/*----------------------------*/
PRIVATE void ComandoRetorne()
{
  int T; /* tipo da expressao */

  Token = ObterToken(Lexema);
  Expressao(&T);
  GerarRetorne();
}
/*---------------------------------------*/
/* comando repita Comandos ate Expressao */
/*---------------------------------------*/
PRIVATE void ComandoRepita()
{
  int T;    /* tipo da expressao */
  int Rot1; /* rotulo do comando repita */

  Token = ObterToken(Lexema);
  GerarRepeticao1(&Rot1);
  Comandos();
  if (Token == ATE) Token = ObterToken(Lexema);
  else Abortar(17);
  Expressao(&T);
  GerarRepeticao2(Rot1);
}
/*-------------------------------------------------*/
/* comando enquanto Expressao faca Comandos fimenq */
/*-------------------------------------------------*/
PRIVATE void ComandoEnquanto()
{
  int T;    /* tipo da expressao */
  int Rot1; /* rotulo do comando enquanto parte 1 e 3 */
  int Rot3; /* rotulo do comando enquanto parte 2 e 3 */

  Token = ObterToken(Lexema);
  GerarEnquanto1(&Rot1);
  Expressao(&T);
  if (Token == FACA) Token = ObterToken(Lexema);
  else Abortar(15);
  GerarEnquanto2(&Rot3);
  Comandos();
  if (Token == FIMENQ) Token = ObterToken(Lexema);
  else Abortar(16);
  GerarEnquanto3(Rot1, Rot3);
}
/*----------------------------------------------------------*/
/* comando se Expressao entao Comandos senao Comandos fimse */
/*----------------------------------------------------------*/
PRIVATE void ComandoSe()
{
  int T;    /* tipo da expressao */
  int Rot1; /* rotulo do comando se parte 1 */
  int Rot2; /* rotulo do comando se parte 2 */
  int Rot3; /* rotulo do comando se parte 3 */

  Token = ObterToken(Lexema);
  Expressao(&T);
  if (Token == ENTAO) Token = ObterToken(Lexema);
  else Abortar(13);
  GerarSe1(&Rot1, &Rot2);
  Comandos();
  if (Token == SENAO)
  {
    Token = ObterToken(Lexema);
    GerarSe2(Rot2, &Rot3);
    Comandos();
  }
  if (Token == FIMSE) Token = ObterToken(Lexema);
  else Abortar(14);
  if (Rot3 == Rot2+1) GerarSe3(Rot3);
  else                GerarSe3(Rot2);
}
/*------------------------*/
/* valor(id) <- Expressao */
/*------------------------*/
PRIVATE void ComandoValor()
{
  int T;   /* tipo da expressao */
  char Alvo[TAM_NOME];

  Token = ObterToken(Lexema);
  if (Token == '(') Token = ObterToken(Lexema);
  else Abortar(18);
  if (Token == IDENTIF)
  {
    strcpy(Alvo, Lexema);
    Token = ObterToken(Lexema);
  }
  else Abortar(1);
  if (Token == ')') Token = ObterToken(Lexema);
  else Abortar(2);
  if (Token == ATRIBUI) Token = ObterToken(Lexema);
  else Abortar(9);
  Expressao(&T);
  GerarValor(T, Alvo);
}
/*---------*/
/* comando */
/*---------*/
PRIVATE void Comando()
{
  int Categ; /* categoria */

  Categ = ObterCategoria(Lexema);
  switch(Token)
  {
    case IDENTIF  : if ((Categ == CVAR) ||
                        (Categ == CVET) ||
                        (Categ == CPAR)) ComandoDeAtribuicao();
                    else                 ComandoDeChamada();
                    break;
    case VALOR    : ComandoValor();    break;
    case SE       : ComandoSe();       break;
    case ENQUANTO : ComandoEnquanto(); break;
    case REPITA   : ComandoRepita();   break;
    case RETORNE :  ComandoRetorne();  break;
  }
}
/*----------*/
/* comandos */
/*----------*/
PRIVATE void Comandos()
{
  while ((Token == IDENTIF)  || (Token == SE)     ||
         (Token == ENQUANTO) || (Token == REPITA) ||
         (Token == VALOR)    || (Token == RETORNE))
  {
    Comando();
    if (Token == ';') Token = ObterToken(Lexema);
    else Abortar(4);
  }
}
/*-------*/
/* corpo */
/*-------*/
PRIVATE void Corpo()
{
  if (Token == INICIO) Token = ObterToken(Lexema);
  else Abortar(7);
  Comandos();
  if (Token == FIM) Token = ObterToken(Lexema);
  else Abortar(8);
}
/*---------------------*/
/* lista de parametros */
/*---------------------*/
PRIVATE void ListaDeParametros(int T, int *NP, int *NB)
{
  if (Token == IDENTIF)
  {
    DefinirParametro(Lexema, T);
    *NP = *NP + 1;
    switch(T)
    {
      case INTEIRO  : *NB += NB_INTEIRO;  break;
      case CADEIA   : *NB += NB_PONTEIRO; break;
      case PONTEIRO : *NB += NB_PONTEIRO; break;
    }
    Token = ObterToken(Lexema);
  }
  else Abortar(1);
  while (Token == ',')
  {
    Token = ObterToken(Lexema);
    if (Token == IDENTIF)
    {
      DefinirParametro(Lexema, T);
      *NP = *NP + 1;
      switch(T)
      {
        case INTEIRO  : *NB += NB_INTEIRO;  break;
        case CADEIA   : *NB += NB_PONTEIRO; break;
        case PONTEIRO : *NB += NB_PONTEIRO; break;
      }
      Token = ObterToken(Lexema);
    }
    else Abortar(1);
  }
}
/*---------------------------------------*/
/* decpar = tipo : lista de parametros ; */
/*---------------------------------------*/
PRIVATE void DeclaracaoDeParametro(int *NP, int *NB)
{
  int T;

  Tipo(&T);
  if (Token == ':') Token = ObterToken(Lexema);
  else Abortar(11);
  ListaDeParametros(T, NP, NB);
}
/*--------------------------*/
/* declaracao de parametros */
/*--------------------------*/
PRIVATE void DeclaracaoDeParametros(int *NP, int *NB)
{
  DeclaracaoDeParametro(NP, NB);
  while (Token == ';')
  {
    Token = ObterToken(Lexema);
    DeclaracaoDeParametro(NP, NB);
  }
}
/*-------*/
/* bloco */
/*-------*/
PRIVATE void Bloco(int Escopo, int TipoSub)
{
  int NP;      /* numero de parametros */
  int NB;      /* numero de bytes */
  int TipoRet; /* tipo de retorno se funcao */

  IniciarBloco(ObterNomeDaSubRotina(), TipoSub);
  IniciarSubRotina(Escopo, ObterNomeDaSubRotina());
  NP = NB = 0;
  if (Token == '(')
  {
    Token = ObterToken(Lexema);
    if ((Token == INTEIRO) ||
        (Token == CADEIA)  ||
        (Token == PONTEIRO) ||
        (Token == IDENTIF))
    {
      DeclaracaoDeParametros(&NP, &NB);
      AjustarParametros(NP);
    }
    if (Token == ')')
    {
      Token = ObterToken(Lexema);
      if (TipoSub == FUNC)             /* funcao */
      {
        if (Token == ':') Token = ObterToken(Lexema);
        else Abortar(11);
        Tipo(&TipoRet);
        switch(TipoRet)
        {
          case INTEIRO :
          case CADEIA  :
          case PONTEIRO: break;
          default      : Abortar(12);
        }
        AjustarTipoDaFuncao(ObterNomeDaSubRotina(), TipoRet);
      }
                                       /* procedimento */
    }
    else Abortar(2);
  }
  else Abortar(18);

  if (Token == TIPO)
  {
    Token = ObterToken(Lexema);
    DeclaracaoDeTipos();
  }
  if (Token == VAR)
  {
    Token = ObterToken(Lexema);
    DeclaracaoDeVariaveis();
  }
  Corpo();
  TerminarSubRotina(ObterNomeDaSubRotina());
  TerminarBloco();
}
/*----------------------------*/
/* declaracao de sub-rotina   */
/*----------------------------*/
PRIVATE void DeclaracaoDeSubRotina()
{
  int TipoSub; /* proc ou func */
  int Escopo;  /* privado, publico ou externo */

  Escopo = PUBLICO;
  switch(Token)
  {
    case PUBLICO :
    case PRIVADO : Escopo = Token;
                   Token  = ObterToken(Lexema);
                   if ((Token != PROC) && (Token != FUNC))
                     Abortar(26);
  }
  TipoSub = Token;
  Token   = ObterToken(Lexema);
  if (Token == IDENTIF)
  {
    DefinirNomeDaSubRotina(Lexema);
    Token = ObterToken(Lexema);
  }
  else Abortar(1);
  Bloco(Escopo, TipoSub);
  if (Token == ';') Token = ObterToken(Lexema);
  else Abortar(4);
}
/*----------*/
/* programa */
/*----------*/
PRIVATE void Programa()
{
  IniciarMontagem();
  DefinirNivel(NGLOBAL);
  if (Token == TIPO)
  {
    Token = ObterToken(Lexema);
    DeclaracaoDeTipos();
  }
  if (Token == VAR)
  {
    IniciarSegmentoDeDados();
    IniciarEspacoGlobal();
    Token = ObterToken(Lexema);
    DeclaracaoDeVariaveis();
    TerminarSegmentoDeDados();
    if (Token == ROTINA)
    {
      IniciarSegmentoDeCodigo();
      Token = ObterToken(Lexema);
      if ((Token == PUBLICO) ||
          (Token == PRIVADO) ||
          (Token == PROC)    ||
          (Token == FUNC))
      {
        DeclaracaoDeSubRotina();
        while ((Token == PUBLICO) ||
               (Token == PRIVADO) ||
               (Token == PROC)    ||
               (Token == FUNC))
          DeclaracaoDeSubRotina();
        TerminarSegmentoDeCodigo();
      }
      else Abortar(26);
      if (Token == PROG)
      {
        DefinirSegmentoDePilha();
        IniciarSegmentoDeCodigo();
        Token = ObterToken(Lexema);
        if (Token == IDENTIF)
        {
          DefinirNomeDoPrograma(Lexema);
          Token = ObterToken(Lexema);
        }
        else Abortar(1);
        if (Token == ';') Token = ObterToken(Lexema);
        else Abortar(4);
        IniciarPrograma(ObterNomeDoPrograma());
        Corpo();
        if (Token == '.') Token = ObterToken(Lexema);
        else Abortar(5);
        if (Token == FIM_ARQ) ;
        else Abortar(6);
        TerminarPrograma(ObterNomeDoPrograma());
        TerminarSegmentoDeCodigo();
      }
      else
      if (Token == FIM_ARQ) ;
      else Abortar(6);
    }
    else
    if (Token == PROG)
    {
      DefinirSegmentoDePilha();
      IniciarSegmentoDeCodigo();
      Token = ObterToken(Lexema);
      if (Token == IDENTIF)
      {
        DefinirNomeDoPrograma(Lexema);
        Token = ObterToken(Lexema);
      }
      else Abortar(1);
      if (Token == ';') Token = ObterToken(Lexema);
      else Abortar(4);
      IniciarPrograma(ObterNomeDoPrograma());
      Corpo();
      if (Token == '.') Token = ObterToken(Lexema);
      else Abortar(5);
      if (Token == FIM_ARQ) ;
      else Abortar(6);
      TerminarPrograma(ObterNomeDoPrograma());
      TerminarSegmentoDeCodigo();
    }
    else
    if (Token == FIM_ARQ) ;
    else Abortar(6);
  }
  else
  if (Token == ROTINA)
  {
    IniciarSegmentoDeCodigo();
    Token = ObterToken(Lexema);
    if ((Token == PUBLICO) ||
        (Token == PRIVADO) ||
        (Token == PROC)    ||
        (Token == FUNC))
    {
      DeclaracaoDeSubRotina();
      while ((Token == PUBLICO) ||
             (Token == PRIVADO) ||
             (Token == PROC)    ||
             (Token == FUNC))
        DeclaracaoDeSubRotina();
      TerminarSegmentoDeCodigo();
    }
    else Abortar(26);
    if (Token == PROG)
    {
      DefinirSegmentoDePilha();
      IniciarSegmentoDeCodigo();
      Token = ObterToken(Lexema);
      if (Token == IDENTIF)
      {
        DefinirNomeDoPrograma(Lexema);
        Token = ObterToken(Lexema);
      }
      else Abortar(1);
      if (Token == ';') Token = ObterToken(Lexema);
      else Abortar(4);
      IniciarPrograma(ObterNomeDoPrograma());
      Corpo();
      if (Token == '.') Token = ObterToken(Lexema);
      else Abortar(5);
      if (Token == FIM_ARQ) ;
      else Abortar(6);
      TerminarPrograma(ObterNomeDoPrograma());
      TerminarSegmentoDeCodigo();
    }
    else
    if (Token == FIM_ARQ) ;
    else Abortar(6);
  }
  else
  if (Token == PROG)
  {
    DefinirSegmentoDePilha();
    IniciarSegmentoDeCodigo();
    Token = ObterToken(Lexema);
    if (Token == IDENTIF)
    {
      DefinirNomeDoPrograma(Lexema);
      Token = ObterToken(Lexema);
    }
    else Abortar(1);
    if (Token == ';') Token = ObterToken(Lexema);
    else Abortar(4);
    IniciarPrograma(ObterNomeDoPrograma());
    Corpo();
    if (Token == '.') Token = ObterToken(Lexema);
    else Abortar(5);
    if (Token == FIM_ARQ) ;
    else Abortar(6);
    TerminarPrograma(ObterNomeDoPrograma());
    TerminarSegmentoDeCodigo();
  }
  else Abortar(3);
  TerminarMontagem(ObterNomeDoPrograma());
}
/*-------------------------------------------------*/
/* associar um analisador sintatico com um arquivo */
/* fonte e um arquivo objeto                       */
/*-------------------------------------------------*/
PUBLIC void IniciarAnalisadorSintatico(char *NomeXodo, char *NomeAsm)
{
  IniciarAnalisadorLexico(NomeXodo);
  IniciarAnalisadorSemantico(NomeAsm);
  Token = ObterToken(Lexema);
  Programa();
}
/*------------------------------------*/
/* desassociar o analisador sintatico */
/*------------------------------------*/
PUBLIC void TerminarAnalisadorSintatico()
{
  TerminarAnalisadorSemantico();
  TerminarAnalisadorLexico();
}
