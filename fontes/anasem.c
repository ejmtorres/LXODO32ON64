/* <ejmt_ce@yahoo.com.br>           */
/* Este software esta disponivel    */
/* via licenca GPL                  */
/*----------------------------------*/

/*---------------------------------*/
/* anasem.c - analisador semantico */
/*---------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "analex.h"
#include "anasin.h"
#include "anasem.h"

PRIVATE FILE          *ArqAsm;             /* fluxo do arquivo destino */
PRIVATE TSimbolo      *Tabela[TAM_TABELA]; /* tabela de simbolos */
PRIVATE int           TopoTab;             /* topo da tab. de simb. */
PRIVATE char          *Reg32[] = {"%ebx", 
	                          "%ecx", 
			          "%esi", 
			          "%edi"}; /* regs de 32 bits */
PRIVATE int           TopoReg;             /* topo dos registradores */
PRIVATE int           Rotulo;              /* numero do rotulo atual */
PRIVATE int           Nivel;               /* nivel (global/local) */
PRIVATE unsigned long EspacoGlobal;        /* espaco global */
PRIVATE unsigned long EspacoLocal;         /* espaco na pilha */
PRIVATE long          DeslocLocal;         /* deslocamento na pilha */
PRIVATE char          NomeProg[TAM_NOME];  /* nome do programa compilado */
PRIVATE char          NomeSubRot[TAM_NOME];/* nome da sub rotina */
PRIVATE char          NomeTipo[TAM_NOME];  /* nome do tipo estruturado */

/*-------------------------*/
/* mostrar erro e terminar */
/*-------------------------*/
PRIVATE void Abortar(int N)
{
  char Mensagem[TAM_MENSAGEM];

  switch(N)
  {
    case  1 : strcpy(Mensagem, "Tabela Cheia");                break;
    case  2 : strcpy(Mensagem, "Identificador Redeclarado");   break;
    case  3 : strcpy(Mensagem, "Identificador Nao Declarado"); break;
    case  4 : strcpy(Mensagem, "Mistura De Tipos");            break;
    case  5 : strcpy(Mensagem, "Operacao Invalida");           break;
    case  6 : strcpy(Mensagem, "Categoria Invalida");          break;
    case  7 : strcpy(Mensagem, "Segmento Cheio");              break;
    case  8 : strcpy(Mensagem, "Muitos Argumentos");           break;
    case  9 : strcpy(Mensagem, "Poucos Argumentos");           break;
    case 10 : strcpy(Mensagem, "Muitos Indices");              break;
    case 11 : strcpy(Mensagem, "Poucos Indices");              break;
    case 12 : strcpy(Mensagem, "Tipo de Retorno Invalido");    break;
    case 13 : strcpy(Mensagem, "Tipo do Parametro Invalido");  break;
    case 14 : strcpy(Mensagem, "Sem Memoria");                 break;
  }
  printf("Linha %d ==> %s!\n", ObterLinha(), Mensagem);
  exit(N);
}
/*-----------------------------------------------*/
/* iniciar o espaco alocado no segmento de dados */
/*-----------------------------------------------*/
PUBLIC void IniciarEspacoGlobal()
{
  EspacoGlobal = 0;
}
/*--------------------------------*/
/* definir nivel lexico igual a N */
/*--------------------------------*/
PUBLIC void DefinirNivel(int N)
{
  Nivel = N;
}
/*----------------------------------*/
/* definir nome do tipo estruturado */
/*----------------------------------*/
PUBLIC void DefinirNomeDoTipo(char *Id)
{
  strcpy(NomeTipo, Id);
}
/*--------------------------------*/
/* obter nome do tipo estruturado */
/*--------------------------------*/
PUBLIC char *ObterNomeDoTipo()
{
  return NomeTipo;
}
/*---------------------------*/
/* procurar um identificador */
/*---------------------------*/
PRIVATE int Procurar(char *Id)
{
  register int  Ind;

  for (Ind=TopoTab-1; Ind>=0; Ind--)
    if (! strcmp(Tabela[Ind]->Nome, Id)) return Ind;
  return -1;
}
/*-------------------------------------*/
/* inserir um item na tab. de simbolos */
/*-------------------------------------*/
PRIVATE void Inserir(char *Id, int Cat, int Niv, long Desl, int Tipo)
{
  register TSimbolo *PtrTopoTab;
  register int       Indice;

  Indice = Procurar(Id);
  if (Indice != -1)
    if (Tabela[Indice]->Nivel == Niv) Abortar(2);
  
  if (TopoTab < TAM_TABELA)
  {
    PtrTopoTab = malloc(sizeof(TSimbolo));
    if (PtrTopoTab == NULL) Abortar(14);
    strcpy(PtrTopoTab->Nome, Id);
    PtrTopoTab->Categ = Cat;
    PtrTopoTab->Nivel = Niv;
    PtrTopoTab->Desl  = Desl;
    PtrTopoTab->Tipo  = Tipo;
    Tabela[TopoTab] = PtrTopoTab;
    TopoTab++;
  }
  else Abortar(1);
}
/*--------------------------*/
/* definir nome do programa */
/*--------------------------*/
PUBLIC void DefinirNomeDoPrograma(char *Id)
{
  strcpy(NomeProg, Id);
  Inserir(Id, CPROG, NGLOBAL, NADA, NADA);
}
/*------------------------*/
/* obter nome do programa */
/*------------------------*/
PUBLIC char *ObterNomeDoPrograma()
{
  return NomeProg;
}
/*----------------------------*/
/* definir nome da sub rotina */
/*----------------------------*/
PUBLIC void DefinirNomeDaSubRotina(char *Id)
{
  strcpy(NomeSubRot, Id);
}
/*--------------------------*/
/* obter nome da sub rotina */
/*--------------------------*/
PUBLIC char *ObterNomeDaSubRotina()
{
  return NomeSubRot;
}
/*--------------------*/
/* alocar registrador */
/*--------------------*/
PRIVATE int AlocarRegistrador()
{
  return TopoReg++;
}
/*-----------------------*/
/* desalocar registrador */
/*-----------------------*/
PRIVATE void DesalocarRegistrador()
{
  --TopoReg;
}
/*----------------------*/
/* obter proximo rotulo */
/*----------------------*/
PRIVATE int ObterRotulo()
{
  return Rotulo++;
}
/*------------------------------------------------*/
/* salvar registradores antes do abre parenteses  */
/*------------------------------------------------*/
PUBLIC void SalvarRegistradores(int *T)
{
  register int I;

  *T = TopoReg;
  for (I=0; I<TopoReg; I++)
    fprintf(ArqAsm, " %30s pushl %s\n", BRANCO, Reg32[I]);
  TopoReg = 0;
}
/*----------------------------------------------------*/
/* recuperar registradores depois do fecha parenteses */
/*----------------------------------------------------*/
PUBLIC void RecuperarRegistradores(int *R, int T)
{
  register int I;

  TopoReg = T;
  *R      = AlocarRegistrador();
  if (*R > 0) /* se Reg32[*R] > bx */
    fprintf(ArqAsm, " %30s movl  %%ebx, %s\n", BRANCO, Reg32[*R]);
  for (I=T-1; I>=0; I--)
    fprintf(ArqAsm, " %30s popl  %s\n", BRANCO, Reg32[I]);
}
/*-----------------------------------------------*/
/* associar um arquivo a um analisador semantico */
/*-----------------------------------------------*/
PUBLIC void IniciarAnalisadorSemantico(char *NomeAsm)
{
  register int Ind;

  if ((ArqAsm = fopen(NomeAsm, "w")) == (FILE *)0)
  {
    printf("Erro de criacao do arquivo %s\n", NomeAsm);
    exit(1);
  }
  Rotulo = TopoReg = TopoTab = 0;
  for (Ind=0; Ind<TAM_TABELA; Ind++) Tabela[Ind] = (TSimbolo *)0;
}
/*----------------------------------------------*/
/* remover todos simbolos do nivel especificado */
/*----------------------------------------------*/
PRIVATE void Remover(int Niv)
{
  TopoTab--;
  while ((TopoTab >= 0) &&
         (Tabela[TopoTab]->Nivel == Niv))
  {
    free(Tabela[TopoTab]);
    TopoTab--;
  }
  TopoTab++;
}
/*----------------------------------------*/
/* definir N-esima dimensao do tipo vetor */
/*----------------------------------------*/
PUBLIC void DefinirN_ezimaDimensao(char *Id, char *D)
{
  /* D = tamanho da N-esima dimensao */
  register TSimbolo *PtrTopoTab;

  if (TopoTab < TAM_TABELA)
  {
    PtrTopoTab = malloc(sizeof(TSimbolo));
    if (PtrTopoTab == NULL) Abortar(14);
    strcpy(PtrTopoTab->Nome, Id);
    PtrTopoTab->Categ = CTIPO;
    PtrTopoTab->Nivel = Nivel;
    PtrTopoTab->Desl  = atol(D);
    PtrTopoTab->Tipo  = NADA;
    Tabela[TopoTab] = PtrTopoTab;
    TopoTab++;
  }
  else Abortar(1);
}
/*---------------------------------------*/
/* definir tipo base do tipo estruturado */
/*---------------------------------------*/
PUBLIC void DefinirTipoBase(int T, int NDim)
{
  while (NDim)
  {
    Tabela[TopoTab-NDim]->Tipo = T;
    NDim--;
  }
}
/*-----------------------------------*/
/* obter entrada da tabela para tipo */
/*-----------------------------------*/
PUBLIC int ObterTipo(char *Id)
{
  register int T; /* entrada de Id */
  register int C; /* categoria de Id */

  T = Procurar(Id);
  if (T == -1) Abortar(3);    /* nao foi declarado */
  C = Tabela[T]->Categ;
  if (C != CTIPO) Abortar(6); /* nao e um id. de tipo */
  while ((T-1 >= 0) &&
         (! strcmp(Tabela[T]->Nome, Tabela[T-1]->Nome))) T--;
  return T;
}
/*---------------------------------------*/
/* obter numero de dimensoes de um vetor */
/*---------------------------------------*/
PRIVATE int ObterNumeroDeDimensoes(char *Id)
{
  register int Pos;
  register int NumDim;

  Pos = Procurar(Id);
  Pos = Tabela[Pos]->Tipo;
  NumDim = 1;
  while ((Pos+1 < TopoTab) &&
         (! strcmp(Tabela[Pos]->Nome, Tabela[Pos+1]->Nome)))
  {
    Pos++;
    NumDim++;
  }
  return NumDim;
}
/*-------------------------------------*/
/* obter categoria de um identificador */
/*-------------------------------------*/
PUBLIC int ObterCategoria(char *Id)
{
  register int Ind;

  if ((Ind = Procurar(Id)) == -1) return NADA;
  else                            return Tabela[Ind]->Categ;
}
/*----------------------------*/
/* iniciar segmento de codigo */
/*----------------------------*/
PUBLIC void IniciarSegmentoDeCodigo()
{
  fprintf(ArqAsm, "\n.text\n");
}
/*-----------------------------*/
/* terminar segmento de codigo */
/*-----------------------------*/
PUBLIC void TerminarSegmentoDeCodigo()
{
}
/*---------------------------*/
/* definir segmento de pilha */
/*---------------------------*/
PUBLIC void DefinirSegmentoDePilha()
{
}
/*---------------------------*/
/* iniciar segmento de dados */
/*---------------------------*/
PUBLIC void IniciarSegmentoDeDados()
{
  fprintf(ArqAsm, ".data\n");
}
/*----------------------------*/
/* terminar segmento de dados */
/*----------------------------*/
PUBLIC void TerminarSegmentoDeDados()
{
}
/*--------------------------*/
/* gerar diretivas iniciais */
/*--------------------------*/
PUBLIC void IniciarMontagem()
{
}
/*--------------------------------------*/
/* gerar ponto de entrada se for o caso */
/*--------------------------------------*/
PUBLIC void TerminarMontagem(char *Inicio)
{
  if (strcmp(Inicio, "")) 
  {
    fprintf(ArqAsm, ".global _start\n");
    fprintf(ArqAsm, "_start:\n");
    fprintf(ArqAsm, " %30s jmp   _%s\n", BRANCO, Inicio);
  }
	
  Remover(NGLOBAL);
}
/*-----------------------------------------*/
/* devolver o numero de elementos do vetor */
/*-----------------------------------------*/
PRIVATE long ObterNumeroDeElementos(int T)
{
  long NE;

  NE = Tabela[T]->Desl;
  while ((T+1 < TopoTab) &&
         (! strcmp(Tabela[T]->Nome, Tabela[T+1]->Nome)))
  {
    NE *= Tabela[T+1]->Desl;
    T++;
  }
  return NE;
}
/*--------------------------------*/
/* definir variavel global vetor */
/*--------------------------------*/
PRIVATE long DefinirVariavelGlobalVetor(int Escopo, char *Id, int Tipo)
{
  int  TB; /* tipo base */
  long NE; /* numero de elementos */
  long NB; /* numero de bytes */

  TB = Tabela[Tipo]->Tipo;
  NE = ObterNumeroDeElementos(Tipo);
  NB = NB_INTEIRO;
  Inserir(Id, CVET, NGLOBAL, NADA, Tipo);
  switch (TB) /* se tipo basico for ... */
  {
    case INTEIRO  : NB = NB_INTEIRO*NE;
                    if (Escopo == EXTERNO) ;
//                      fprintf(ArqAsm, "extrn  _%s:dword\n", Id);
                    else
                    if (Escopo == PRIVADO)
                    {
	              fprintf(ArqAsm, ".comm _%s, %ld, %d\n", Id, NB, ALIN_INT);
                    }
                    else
                    {
	              fprintf(ArqAsm, ".comm _%s, %ld, %d\n", Id, NB, ALIN_INT);
                    }
                    break;
    case CADEIA   : NB = (NB_CADEIA + 1)*NE;
                    if (Escopo == EXTERNO) ;
//                      fprintf(ArqAsm, "extrn  _%s:byte\n", Id);
                    else
                    if (Escopo == PRIVADO)
	              fprintf(ArqAsm, ".comm _%s, %ld, %d\n", Id, NB, ALIN_CAD);
                    else
                    {
	              fprintf(ArqAsm, ".comm _%s, %ld, %d\n", Id, NB, ALIN_CAD);
                    }
                    break;
    case PONTEIRO : NB = NB_PONTEIRO*NE;
                    if (Escopo == EXTERNO) ;
//                      fprintf(ArqAsm, "extrn  _%s:dword\n", Id);
                    else
                    if (Escopo == PRIVADO)
	              fprintf(ArqAsm, ".comm _%s, %ld, %d\n", Id, NB, ALIN_PON);
                    else
                    {
	              fprintf(ArqAsm, ".comm _%s, %ld, %d\n", Id, NB, ALIN_PON);
                    }
                    break;
  }
  return NB;
}
/*-------------------------------*/
/* definir variavel local vetor */
/*-------------------------------*/
PRIVATE long DefinirVariavelLocalVetor(int Tipo)
{
  int  TB; /* tipo base */
  long NE; /* numero de elementos */
  long NB; /* numero de bytes */

  TB = Tabela[Tipo]->Tipo;
  NE = ObterNumeroDeElementos(Tipo);
  NB = NB_INTEIRO;
  switch (TB) /* se tipo basico for ... */
  {
    case INTEIRO  : NB = NB_INTEIRO*NE;
                    break;
    case CADEIA   : NB = (NB_CADEIA + 1)*NE;
                    break;
    case PONTEIRO : NB = NB_PONTEIRO*NE;
                    break;
  }
  return NB;
}
/*---------------------------------------*/
/* definir e gerar codigo para variavel */
/*---------------------------------------*/
PUBLIC void DefinirVariavel(int Escopo, char *Id, int Tipo, long *N)
{
  if (Nivel == NGLOBAL)
  {
  /* variavel global */
    if (Tipo == INTEIRO)
    {
      Inserir(Id, CVAR, NGLOBAL, NADA, INTEIRO);
      if (Escopo == EXTERNO) ;
      else
      if (Escopo == PRIVADO)
      {
        fprintf(ArqAsm, "_%s:\n", Id);
        fprintf(ArqAsm, ".long 0\n");
      }
      else
      {
        fprintf(ArqAsm, ".global _%s\n", Id);
        fprintf(ArqAsm, "_%s:\n", Id);
        fprintf(ArqAsm, ".long 0\n");
      }
      EspacoGlobal += NB_INTEIRO;
    }
    else
    if (Tipo == CADEIA)
    {
      Inserir(Id, CVAR, NGLOBAL, NADA, CADEIA);
      if (Escopo == EXTERNO) ;
      else
      if (Escopo == PRIVADO)
        fprintf(ArqAsm, "_%s:\n", Id);
      else
      {
        fprintf(ArqAsm, ".comm _%s, %d, 1\n", Id, NB_CADEIA+1);
      }
      EspacoGlobal += (NB_CADEIA+1);
    }
    else
    if (Tipo == PONTEIRO)
    {
      Inserir(Id, CVAR, NGLOBAL, NADA, PONTEIRO);
      if (Escopo == EXTERNO) ;
      else
      if (Escopo == PRIVADO)
      {
        fprintf(ArqAsm, "_%s:\n", Id);
        fprintf(ArqAsm, ".long 0\n");
      }
      else
      {
        fprintf(ArqAsm, ".global _%s\n",  Id);
        fprintf(ArqAsm, "_%s:\n", Id);
        fprintf(ArqAsm, ".long 0\n");
      }
      EspacoGlobal += NB_PONTEIRO;
    }
    else /* variavel global estruturada */
    {
      EspacoGlobal += DefinirVariavelGlobalVetor(Escopo, Id, Tipo);
    }
    if (EspacoGlobal > TAM_DADOS) Abortar(7);
  }
  /* variavel local */
  else
  {
    if (Tipo == INTEIRO)
    {
      Inserir(Id, CVAR, NLOCAL, DeslocLocal+NB_INTEIRO, INTEIRO);
      EspacoLocal += NB_INTEIRO;
      DeslocLocal += NB_INTEIRO;
      *N          += NB_INTEIRO;
    }
    else
    if (Tipo == CADEIA)
    {
      Inserir(Id, CVAR, NLOCAL, DeslocLocal+NB_CADEIA+1, CADEIA);
      EspacoLocal += NB_CADEIA+1;
      DeslocLocal += NB_CADEIA+1;
      *N          += NB_CADEIA+1;
    }
    else
    if (Tipo == PONTEIRO)
    {
      Inserir(Id, CVAR, NLOCAL, DeslocLocal+NB_PONTEIRO, PONTEIRO);
      EspacoLocal += NB_PONTEIRO;
      DeslocLocal += NB_PONTEIRO;
      *N          += NB_PONTEIRO;
    }
    else /* variavel local estruturada */
    {
      *N = DefinirVariavelLocalVetor(Tipo);
      Inserir(Id, CVET, NLOCAL, DeslocLocal+(*N), Tipo);
      EspacoLocal += *N;
      DeslocLocal += *N;
    }
    if (EspacoLocal > TAM_PILHA)  Abortar(7);
  }
}
/*----------------------------------------*/
/* definir e gerar codigo para parametro */
/*----------------------------------------*/
PUBLIC void DefinirParametro(char *Id, int Tipo)
{
  if (Tipo == INTEIRO)
  {
    Inserir(Id, CPAR, NLOCAL, NADA, INTEIRO);
    EspacoLocal += NB_INTEIRO;
  }
  else
  if (Tipo == CADEIA)
  {
    Inserir(Id, CPAR, NLOCAL, NADA, CADEIA);
    EspacoLocal += NB_PONTEIRO; /* cadeia eh um ponteiro */
  }
  else
  if (Tipo == PONTEIRO)
  {
    Inserir(Id, CPAR, NLOCAL, NADA, PONTEIRO);
    EspacoLocal += NB_PONTEIRO;
  }
  else Abortar(13);
}
/*-----------------------------------------*/
/* ajustar os deslocamentos dos parametros */
/*-----------------------------------------*/
PUBLIC void AjustarParametros(int N)
{
  register int T;   /* primeiro parametro na tabela */
  register int Ind; /* indice da tabela */
  register int Nb;  /* numero de bytes */
  long D;  /* deslocamento inicial dos parametros */

  T  = TopoTab - 1;
  Nb = NB_INTEIRO;
  D  = DESL_INIC;
  for (Ind=1; Ind<=N; Ind++) /* N = numero de parametros */
  {
    Tabela[T]->Desl = D;
    switch(Tabela[T]->Tipo)
    {
      case INTEIRO  : Nb = NB_INTEIRO;  break;
      case CADEIA   : Nb = NB_PONTEIRO; break;
      case PONTEIRO : Nb = NB_PONTEIRO; break;
    }
    D += Nb;
    T--;
  }
  Tabela[T]->Tipo = N; /* num. de par. colocado no proc correspondente */
  DeslocLocal = 0;
}
/*---------------------------*/
/* gerar armazenamento local */
/*---------------------------*/
PUBLIC void DefinirVariavelLocal(long N)
{
  if (Nivel == NLOCAL) 
  {
    fprintf(ArqAsm, " %30s subl  $%ld, %%esp\n", BRANCO, N);
  }
}
/*----------------------------*/
/* iniciar programa principal */
/*----------------------------*/
PUBLIC void IniciarPrograma(char *Id)
{
  fprintf(ArqAsm, "\n.global _%s\n", Id);
  fprintf(ArqAsm, "_%s:\n",        Id);
}
/*-----------------------------*/
/* terminar programa principal */
/*-----------------------------*/
PUBLIC void TerminarPrograma(char *Id)
{
  fprintf(ArqAsm, " %30s movl  $0, %%ebx\n", BRANCO);
  fprintf(ArqAsm, " %30s movl  $1, %%eax\n", BRANCO);
  fprintf(ArqAsm, " %30s int   $0x80\n",     BRANCO);
}
/*----------------------*/
/* iniciar sub-rotina   */
/*----------------------*/
PUBLIC void IniciarSubRotina(int Escopo, char *Id)
{
  /* publico por default */
  if (Escopo != PRIVADO)
    fprintf(ArqAsm, "\n.global _%s\n",  Id);
  fprintf(ArqAsm, "_%s:\n", Id);
  fprintf(ArqAsm, " %-30s pushl %%ebp\n",        BRANCO);
  fprintf(ArqAsm, " %-30s movl  %%esp, %%ebp\n", BRANCO);
}
/*-----------------------*/
/* terminar sub-rotina   */
/*-----------------------*/
PUBLIC void TerminarSubRotina(char *Id)
{
  fprintf(ArqAsm, " %30s movl  %%ebp, %%esp\n", BRANCO);
  fprintf(ArqAsm, " %30s popl  %%ebp\n",        BRANCO);
  fprintf(ArqAsm, " %30s ret\n",                BRANCO);
}
/*---------------*/
/* iniciar bloco */
/*---------------*/
PUBLIC void IniciarBloco(char *Id, int TipoSub)
{
  /* NP = numero de parametros (na posicao de tipo)*/
  switch(TipoSub)
  {
    case PROC : Inserir(Id, CPROC, NGLOBAL, NADA, 0); break;
    case FUNC : Inserir(Id, CFUNC, NGLOBAL, NADA, 0); break;
  }
  DefinirNivel(NLOCAL);
  EspacoLocal = 0;
}
/*----------------*/
/* terminar bloco */
/*----------------*/
PUBLIC void TerminarBloco()
{
  Remover(NLOCAL);
  DefinirNivel(NGLOBAL);
}
/*-------------------------------------*/
/* ajustar o tipo de retorno da funcao */
/*-------------------------------------*/
PUBLIC void AjustarTipoDaFuncao(char *NomeFunc, int TipoRet)
{
  register int Indice;

  Indice = Procurar(NomeFunc);
  if (Indice != -1)
    Tabela[Indice]->Desl = TipoRet;
  else Abortar(3);
}
/*--------------------------*/
/* iniciar chamada a funcao */
/*--------------------------*/
PUBLIC void IniciarFuncao(int *NA, int *NB, int *TA)
{
  register int I;

  *NA = 0;       /* inicializa numero de argumentos */
  *NB = 0;       /* inicializa numero de bytes empilhado */
  *TA = TopoReg; /* salva topo auxiliar */
  for (I=0; I<TopoReg; I++)
    fprintf(ArqAsm, " %30s pushl %s\n", BRANCO, Reg32[I]);
  TopoReg = 0;
}
/*---------------------------*/
/* terminar chamada a funcao */
/*---------------------------*/
PUBLIC void TerminarFuncao(int *R, int *T, int NA, int NB, int TA, char *Id)
{
  register int NP;  /* numero de parametros */
  register int Ind; /* indice na tabela */
  register int I;

  if ((Ind = Procurar(Id)) == -1)
  {
    *T = INTEIRO; /* default caso funcao seja externa */
  }
  else
  {
    *T = (int)Tabela[Ind]->Desl; /* tipo de retorno (no deslocamento)*/
    NP = Tabela[Ind]->Tipo;      /* n. de parametros (em tipo)*/
    if (NA < NP) Abortar(9);
    if (NA > NP) Abortar(8);
  }
  fprintf(ArqAsm, " %30s call  _%s\n", BRANCO, Id);
  if (NB > 0)
    fprintf(ArqAsm, " %30s addl  $%d, %%esp\n", BRANCO, NB);
  TopoReg = TA;
  *R      = AlocarRegistrador();
  fprintf(ArqAsm, " %30s movl  %%eax, %s\n", BRANCO, Reg32[*R]);
  for (I=TA-1; I>=0; I--)
    fprintf(ArqAsm, " %30s popl  %s\n", BRANCO, Reg32[I]);
}
/*---------------------*/
/* iniciar fator vetor */
/*---------------------*/
PUBLIC void IniciarFatorVetor(int  *NI, int *TA, int *R, int *T, int *P,
                              char *Id)
{
  register int I;

  *P = Tabela[Procurar(Id)]->Tipo; /* encontra entrada do tipo do vetor */
  *T = Tabela[*P]->Tipo;           /* encontra tipo base do vetor */
  *R = AlocarRegistrador();
  *NI = 0;              /* inicializa numero de indices */
  *TA = TopoReg;        /* salva topo auxiliar */
  for (I=0; I<TopoReg; I++)
    fprintf(ArqAsm, " %30s pushl %s\n", BRANCO, Reg32[I]);
  TopoReg = 0;
  fprintf(ArqAsm, " %30s movl  $0, %%eax\n", BRANCO);
}
/*----------------------*/
/* terminar fator vetor */
/*----------------------*/
PUBLIC void TerminarFatorVetor(int NI, int TA, int R, char *Id)
{
  register int ND;  /* numero de dimensoes */
  register int I;
  register int Pos;
  register int Niv;
  register int Tip;
  long         Des;

  ND = ObterNumeroDeDimensoes(Id);
  if (NI > ND) Abortar(10);
  if (NI < ND) Abortar(11);
  Pos = Procurar(Id);
  Niv = Tabela[Pos]->Nivel;
  Des = Tabela[Pos]->Desl;
  Tip = Tabela[Pos]->Tipo;
  Tip = Tabela[Tip]->Tipo;
  TopoReg = TA;
  for (I=TA-1; I>=0; I--)
    fprintf(ArqAsm, " %30s popl  %s\n", BRANCO, Reg32[I]);
  if ((Tip == INTEIRO)||(Tip == PONTEIRO))
  {
    fprintf(ArqAsm, " %30s addl  %%eax, %%eax\n", BRANCO);
    fprintf(ArqAsm, " %30s addl  %%eax, %%eax\n", BRANCO);
    if (Niv == NGLOBAL)
      fprintf(ArqAsm, " %30s leal  _%s, %s\n", BRANCO, Id, Reg32[R]);
    else
      fprintf(ArqAsm, " %30s leal  -%ld(%%ebp), %s\n", BRANCO, Des, Reg32[R]);
    fprintf(ArqAsm, " %30s addl  %%eax, %s\n", BRANCO, Reg32[R]);
    if (R == 0) /* ebx ? */
    {
      fprintf(ArqAsm, " %30s movl  (%%ebx), %%ebx\n", BRANCO);
    }
    else
    {
      fprintf(ArqAsm, " %30s pushl %%ebx\n",       BRANCO);
      fprintf(ArqAsm, " %30s movl  %s, %%ebx\n",   BRANCO, Reg32[R]);
      fprintf(ArqAsm, " %30s movl  (%%ebx), %s\n", BRANCO, Reg32[R]);
      fprintf(ArqAsm, " %30s popl  %%ebx\n",       BRANCO);
    }
  }
  else
  if (Tip == CADEIA)
  {
    fprintf(ArqAsm, " %30s movl  $%d, %s\n", BRANCO, NB_CADEIA+1, Reg32[R]);
    fprintf(ArqAsm, " %30s mull  %s\n",      BRANCO, Reg32[R]);
    if (Niv == NGLOBAL)
      fprintf(ArqAsm, " %30s leal  _%s, %s\n", BRANCO, Id, Reg32[R]);
    else
      fprintf(ArqAsm, " %30s leal  -%ld(%%ebp), %s\n", BRANCO, Des, Reg32[R]);
    fprintf(ArqAsm, " %30s addl  %%eax, %s\n", BRANCO, Reg32[R]);
  }
}
/*-----------------------------*/
/* iniciar atribuicao de vetor */
/*-----------------------------*/
PUBLIC void IniciarAtribuicaoVetor(int *NI, int *T, int *P, char *Id)
{
  *P = Tabela[Procurar(Id)]->Tipo; /* encontra entrada do tipo do vetor */
  *T = Tabela[*P]->Tipo;           /* encontra tipo base do vetor */
  /* inicializa numero de indices */
  *NI = 0;
  fprintf(ArqAsm, " %30s movl  $0, %%eax\n", BRANCO);
}
/*------------------------------*/
/* terminar atribuicao de vetor */
/*------------------------------*/
PUBLIC void TerminarAtribuicaoVetor(int NI, char *Id)
{
  register int ND;  /* numero de dimensoes */
  register int Pos;
  register int Niv;
  register int Tip;
  long         Des;

  ND = ObterNumeroDeDimensoes(Id);
  if (NI > ND) Abortar(10);
  if (NI < ND) Abortar(11);
  Pos = Procurar(Id);
  Niv = Tabela[Pos]->Nivel;
  Des = Tabela[Pos]->Desl;
  Tip = Tabela[Pos]->Tipo;
  Tip = Tabela[Tip]->Tipo;
  if ((Tip == INTEIRO)||(Tip == PONTEIRO))
  {
    fprintf(ArqAsm, " %30s addl  %%eax, %%eax\n", BRANCO);
    fprintf(ArqAsm, " %30s addl  %%eax, %%eax\n", BRANCO);
    if (Niv == NGLOBAL)
      fprintf(ArqAsm, " %30s leal  _%s, %%ebx\n", BRANCO, Id);
    else
      fprintf(ArqAsm, " %30s leal  -%ld(%%ebp), %%ebx\n", BRANCO, Des);
    fprintf(ArqAsm, " %30s addl  %%ebx, %%eax\n", BRANCO);
  }
  else
  if (Tip == CADEIA)
  {
    fprintf(ArqAsm, " %30s movl  $%d, %%ebx\n", BRANCO, NB_CADEIA+1);
    fprintf(ArqAsm, " %30s mull  %%ebx\n",     BRANCO);
    if (Niv == NGLOBAL)
      fprintf(ArqAsm, " %30s leal  _%s, %%ebx\n", BRANCO, Id);
    else
      fprintf(ArqAsm, " %30s leal  -%ld(%%ebp), %%ebx\n", BRANCO, Des);
    fprintf(ArqAsm, " %30s addl  %%ebx, %%eax\n", BRANCO);
  }
  fprintf(ArqAsm, " %30s pushl %%eax\n", BRANCO);
}
/*---------------------------------*/
/* operar sobre dois registradores */
/*---------------------------------*/
PUBLIC void Operar(int Op, int R1, int R2, int T1, int T2)
{
  if ((T1 != INTEIRO) && (T1 != PONTEIRO)) Abortar(5);
  if ((T2 != INTEIRO) && (T2 != PONTEIRO)) Abortar(5);

  switch(Op)
  {
    case '+' : fprintf(ArqAsm, " %30s addl  %s, %s\n", BRANCO, Reg32[R2],
                                                               Reg32[R1]);
               break;
    case '-' : fprintf(ArqAsm, " %30s subl  %s, %s\n", BRANCO, Reg32[R2],
                                                               Reg32[R1]);
               break;
    case '*' : fprintf(ArqAsm, " %30s xchgl %%eax, %s\n", BRANCO, Reg32[R1]);
               fprintf(ArqAsm, " %30s imull %s\n",        BRANCO, Reg32[R2]);
               fprintf(ArqAsm, " %30s xchgl %%eax, %s\n", BRANCO, Reg32[R1]);
               break;
    case '/' : fprintf(ArqAsm, " %30s movl  $0, %%edx\n", BRANCO);
               fprintf(ArqAsm, " %30s xchgl %%eax, %s\n", BRANCO, Reg32[R1]);
               fprintf(ArqAsm, " %30s idivl %s\n",        BRANCO, Reg32[R2]);
               fprintf(ArqAsm, " %30s xchgl %%eax, %s\n", BRANCO, Reg32[R1]);
               break;
  }
  DesalocarRegistrador();
}
/*---------------------------------*/
/* operar sobre dois registradores */
/*---------------------------------*/
PUBLIC void OperarRelacao(char *Op, int R1, int R2, int T1, int T2)
{
  int Rot1;
  int Rot2;
  char Instr[4]; /* instrucao de salto condicional */

  if (T1 != T2)      Abortar(4);
  if (T1 != INTEIRO) Abortar(5);
  if (!strcmp(Op, "<"))  strcpy(Instr, "jl ");
  else
  if (!strcmp(Op, "<=")) strcpy(Instr, "jle");
  else
  if (!strcmp(Op, "<>")) strcpy(Instr, "jne");
  else
  if (!strcmp(Op, ">"))  strcpy(Instr, "jg ");
  else
  if (!strcmp(Op, ">=")) strcpy(Instr, "jge");
  else
  if (!strcmp(Op, "="))  strcpy(Instr, "je ");
  Rot1 = ObterRotulo();
  Rot2 = ObterRotulo();
  fprintf(ArqAsm, " %30s cmpl  %s, %s\n", BRANCO, Reg32[R2], Reg32[R1]);
  fprintf(ArqAsm, " %30s %s   R%d\n",    BRANCO, Instr,   Rot1);
  fprintf(ArqAsm, " %30s movl  $0, %s\n", BRANCO, Reg32[R1]);
  fprintf(ArqAsm, " %30s jmp   R%d\n",    BRANCO, Rot2);
  fprintf(ArqAsm, "R%d:\n",               Rot1);
  fprintf(ArqAsm, " %30s movl  $1, %s\n", BRANCO, Reg32[R1]);
  fprintf(ArqAsm, "R%d:\n", Rot2);
  DesalocarRegistrador();
}
/*--------------------*/
/* empilhar expressao */
/*--------------------*/
PUBLIC void EmpilharExpressao()
{
  fprintf(ArqAsm, " %30s pushl %%ebx\n", BRANCO);
  DesalocarRegistrador();
}
/*----------------------------------------*/
/* gerar codigo para acessar fator (vetor)*/
/*----------------------------------------*/
PUBLIC void GerarIndice1()
{
  fprintf(ArqAsm, " %30s addl  %%ebx, %%eax\n", BRANCO);
  DesalocarRegistrador();
}
/*---------------------------------------------------*/
/* gerar codigo para acessar fator (vetor) - parte 2 */
/*---------------------------------------------------*/
PUBLIC void GerarIndiceN(int D)
{
  /* D aponta para entrada que descreve dimensao a utilizar */

  AlocarRegistrador();
  fprintf(ArqAsm, " %30s movl  $%ld, %%ecx\n", BRANCO, Tabela[D]->Desl);
  fprintf(ArqAsm, " %30s mull  %%ecx\n",       BRANCO);
  DesalocarRegistrador();
  fprintf(ArqAsm, " %30s addl  %%ebx, %%eax\n", BRANCO);
  DesalocarRegistrador();
}
/*----------------------------------*/
/* gerar expressao do tipo endereco */
/*----------------------------------*/
PUBLIC void GerarEndereco(char *Id, int *T)
{
  register int Ind;  /* indice da tab. de simb. */
  register int Catg; /* categoria */
  register int Nivl; /* nivel */
  register int Tipo; /* tipo */
  long         Desl; /* deslocamento */

  if ((Ind = Procurar(Id)) == -1) Abortar(3);
  if ((Tabela[Ind]->Categ != CVAR) &&
      (Tabela[Ind]->Categ != CPAR) &&
      (Tabela[Ind]->Categ != CVET)) Abortar(6);
  Desl = Tabela[Ind]->Desl;
  Nivl = Tabela[Ind]->Nivel;
  Tipo = Tabela[Ind]->Tipo;
  Catg = Tabela[Ind]->Categ;
  AlocarRegistrador(); /* ebx */
  /* variavel global */
  if (Nivl == NGLOBAL)
    fprintf(ArqAsm, " %30s leal  _%s, %%ebx\n", BRANCO, Id);
  else
  {
  /* variavel local */
    if (Catg == CVET)
      fprintf(ArqAsm, " %30s leal  -%ld(%%ebp), %%ebx\n", BRANCO, Desl);
    else
    if (Catg == CVAR)
    {
      if (Tipo == INTEIRO)
        fprintf(ArqAsm, " %30s leal  -%ld(%%ebp), %%ebx\n", BRANCO, Desl);
      else
      if (Tipo == CADEIA)
        fprintf(ArqAsm, " %30s leal  -%ld(%%ebp), %%ebx\n", BRANCO, Desl);
      else
      if (Tipo == PONTEIRO)
        fprintf(ArqAsm, " %30s movl  -%ld(%%ebp), %%ebx\n", BRANCO, Desl);
    }
    else
    /* parametro */
    {
      if (Tipo == INTEIRO)
        fprintf(ArqAsm, " %30s leal  %ld(%%ebp), %%ebx\n", BRANCO, Desl);
      else
      if (Tipo == CADEIA)
        fprintf(ArqAsm, " %30s movl  %ld(%%ebp), %%ebx\n", BRANCO, Desl);
      else
      if (Tipo == PONTEIRO)
        fprintf(ArqAsm, " %30s movl  %ld(%%ebp), %%ebx\n", BRANCO, Desl);
    }
  }
  *T = PONTEIRO;
}
/*--------------------------------*/
/* gerar expressao do tipo cadeia */
/*--------------------------------*/
PUBLIC void GerarCadeia(char *Id, int *T)
{
  register char *PtrId;
  register int   Cont;
  register int   Rot1;
  register int   Rot2;

  PtrId = Id;
  Rot1 = ObterRotulo();
  Rot2 = ObterRotulo();
  *T = CADEIA;
  fprintf(ArqAsm, " %-30s jmp   R%d\n", BRANCO, Rot2);
  TerminarSegmentoDeCodigo();
  IniciarSegmentoDeDados();
  fprintf(ArqAsm, "R%d:\n", Rot1);
  Cont = 0;
  if (*PtrId) 
  {
    fprintf(ArqAsm, ".byte '%c'", *PtrId++);
    Cont++;
  }
  while (*PtrId)
  {
    if (Cont == NBYTE_LIN)
    {
      fprintf(ArqAsm, "\n.byte '%c'", *PtrId++);
      Cont++;
    }
    else
    {
      fprintf(ArqAsm, ",'%c'", *PtrId++);
      Cont++;
    }
  }
  if (Cont == 0) fprintf(ArqAsm, ".byte 0\n");
  else           fprintf(ArqAsm, "\n.byte 0\n");
  TerminarSegmentoDeDados();
  IniciarSegmentoDeCodigo();
  fprintf(ArqAsm, "R%d:\n", Rot2);
  fprintf(ArqAsm, " %30s leal  R%d, %s\n", BRANCO, Rot1, 
		                           Reg32[AlocarRegistrador()]);
}
/*------------------------------------*/
/* atribuir um valor a um registrador */
/*------------------------------------*/
PUBLIC void GerarFator(int *R, int  *T, int Tk, char *Id)
{
  register int Ind; /* indice da tab. de simb. */
  register int C;   /* categoria */
  register int N;   /* nivel */
  long         D;   /* deslocamento */

  C = CVAR;
  N = NGLOBAL;
  D = 0;
  *R = AlocarRegistrador();
  if (Tk == NUMERO)
  {
    *T = INTEIRO;
    fprintf(ArqAsm, " %30s movl  $%s, %s\n", BRANCO, Id, Reg32[*R]);
  }
  if (Tk == IDENTIF)
  {
    if ((Ind = Procurar(Id)) == -1) Abortar(3);
    else
    {
      *T = Tabela[Ind]->Tipo;
      D  = Tabela[Ind]->Desl;
      N  = Tabela[Ind]->Nivel;
      C  = Tabela[Ind]->Categ;
    }
    if ((C != CVAR) &&
        (C != CPAR)) Abortar(6);
    /* variavel global */
    if (N == NGLOBAL)
    {
      if (*T == INTEIRO)
        fprintf(ArqAsm, " %30s movl  _%s, %s\n", BRANCO, Id, Reg32[*R]);
      else
      if (*T == PONTEIRO)
        fprintf(ArqAsm, " %30s movl  _%s, %s\n", BRANCO, Id, Reg32[*R]);
      else
      if (*T == CADEIA)
        fprintf(ArqAsm, " %30s leal  _%s, %s\n",  BRANCO, Id, Reg32[*R]);
    }
    else
    {
    /* variavel local */
      if (C == CVAR)
      {
        if (*T == INTEIRO)
          fprintf(ArqAsm, " %30s movl  -%ld(%%ebp), %s\n", BRANCO, D, Reg32[*R]);
        else
        if (*T == PONTEIRO)
          fprintf(ArqAsm, " %30s movl  -%ld(%%ebp), %s\n", BRANCO, D, Reg32[*R]);
        else
        if (*T == CADEIA)
          fprintf(ArqAsm, " %30s leal  -%ld(%%ebp), %s\n", BRANCO, D, Reg32[*R]);
      }
      else
      {
      /* parametro */
        if (*T == INTEIRO)
          fprintf(ArqAsm, " %30s movl  %ld(%%ebp), %s\n", BRANCO, D, Reg32[*R]);
        else
        if (*T == PONTEIRO)
          fprintf(ArqAsm, " %30s movl  %ld(%%ebp), %s\n", BRANCO, D, Reg32[*R]);
        else
        if (*T == CADEIA)
          fprintf(ArqAsm, " %30s movl  %ld(%%ebp), %s\n", BRANCO, D, Reg32[*R]);
      }
    }
  }
}
/*--------------------------*/
/* gerar Fator -> valor(Id) */
/*--------------------------*/
PUBLIC void GerarFatorValor(int *R, int *T, char *Id)
{
  register int Ind; /* indice na tabela */
  register int N;   /* nivel */
  register int C;   /* categoria */
  long         D;   /* deslocamento */

  N = NGLOBAL;
  C = CVAR;
  D = 0;
  if ((Ind = Procurar(Id)) == -1) Abortar(3);
  else
  {
    *T = Tabela[Ind]->Tipo;
    N  = Tabela[Ind]->Nivel;
    C  = Tabela[Ind]->Categ;
    D  = Tabela[Ind]->Desl;
  }
  if (*T != PONTEIRO) Abortar(5);
  else *T = INTEIRO;
  *R = AlocarRegistrador();
  if (N == NLOCAL)
    if (C == CVAR) /* variavel local */
      fprintf(ArqAsm, " %30s movl  -%ld(%%ebp), %s\n", BRANCO, D, Reg32[*R]);
    else           /* parametro */
      fprintf(ArqAsm, " %30s movl  %ld(%%ebp), %s\n", BRANCO, D, Reg32[*R]);
  else             /* variavel global */
    fprintf(ArqAsm, " %30s movl  _%s, %s\n", BRANCO, Id, Reg32[*R]);
  if (*R) /* nao eh ebx */
  {
    fprintf(ArqAsm, " %30s pushl %%ebx\n",          BRANCO);
    fprintf(ArqAsm, " %30s movl  %s, %%ebx\n",      BRANCO, Reg32[*R]);
    fprintf(ArqAsm, " %30s movl  (%%ebx), %%ebx\n", BRANCO);
    fprintf(ArqAsm, " %30s movl  %%ebx, %s\n",      BRANCO, Reg32[*R]);
    fprintf(ArqAsm, " %30s popl  %%ebx\n",          BRANCO);
  }
  else    /* eh ebx */
  {
    fprintf(ArqAsm, " %30s movl  (%%ebx), %%ebx\n", BRANCO);
  }
}
/*----------------------*/
/* gerar sinal de menos */
/*----------------------*/
PUBLIC void GerarMenos(int M, int R)
{
  if (M) fprintf(ArqAsm, " %30s negl  %s\n", BRANCO, Reg32[R]);
}
/*------------------------------------------------------------*/
/* gerar codigo para valor(Id) do lado esquerdo da atribuicao */
/*------------------------------------------------------------*/
PUBLIC void GerarValor(int T, char *Id)
{
  register int Ind;  /* indice na tabela */
  register int Niv;  /* nivel */
  register int Cat;  /* categoria */
  register int TId;  /* tipo do Id */
  long         Desl; /* deslocamento */

  Niv  = NGLOBAL;
  Cat  = CVAR;
  TId  = INTEIRO;
  Desl = 0;
  if ((Ind = Procurar(Id)) == -1) Abortar(3);
  else
  {
    TId  = Tabela[Ind]->Tipo;
    Niv  = Tabela[Ind]->Nivel;
    Cat  = Tabela[Ind]->Categ;
    Desl = Tabela[Ind]->Desl;
  }

  if (T != INTEIRO) Abortar(4);    /* expressao deve ser do tipo inteiro */
  if (TId != PONTEIRO) Abortar(5); /* tipo do Id deve ser ponteiro */
  fprintf(ArqAsm, " %30s movl  %%ebx, %%eax\n", BRANCO);
  if (Niv == NLOCAL)
    if (Cat == CVAR) /* variavel local */
      fprintf(ArqAsm, " %30s movl  -%ld(%%ebp), %%ebx\n", BRANCO, Desl);
    else             /* parametro */
      fprintf(ArqAsm, " %30s movl  %ld(%%ebp), %%ebx\n", BRANCO, Desl);
  else               /* variavel global */
    fprintf(ArqAsm, " %30s movl  _%s, %%ebx\n", BRANCO, Id);
  fprintf(ArqAsm, " %30s movl  %%eax, (%%ebx)\n", BRANCO);
  DesalocarRegistrador();
}
/*------------------------------*/
/* gerar atribuicao mov id, reg */
/*------------------------------*/
PUBLIC void GerarAtribuicao(char *Id, int T)
{
  register int Ind;
  register int Tipo;
  register int Catg;
  register int Nivl;
  long         Desl;

  Tipo = INTEIRO;
  Catg = CVAR;
  Nivl = NGLOBAL;
  Desl = 0;
  if ((Ind = Procurar(Id)) == -1) Abortar(3);
  else
  {
    Tipo = Tabela[Ind]->Tipo;
    Catg = Tabela[Ind]->Categ;
    Desl = Tabela[Ind]->Desl;
    Nivl = Tabela[Ind]->Nivel;
  }
  if ((Catg != CVAR) &&
      (Catg != CPAR)) Abortar(6);
  if (Tipo != T)      Abortar(4);
  /* variavel global */
  if (Nivl == NGLOBAL)
  {
    if (Tipo == INTEIRO)
      fprintf(ArqAsm, " %30s movl  %%ebx, _%s\n", BRANCO, Id);
    else
    if (Tipo == PONTEIRO)
      fprintf(ArqAsm, " %30s movl  %%ebx, _%s\n", BRANCO, Id);
    else
    if (Tipo == CADEIA)
    {
      fprintf(ArqAsm, " %30s leal  _%s, %%ecx\n", BRANCO, Id);
      fprintf(ArqAsm, " %30s pushl %%ecx\n",      BRANCO);
      fprintf(ArqAsm, " %30s pushl %%ebx\n",      BRANCO);
      fprintf(ArqAsm, " %30s call  _copcad\n",    BRANCO);
      fprintf(ArqAsm, " %30s addl  $8, %%esp\n",  BRANCO);
    }
  }
  else
  {
    /* variavel local */
    if (Catg == CVAR)
    {
      if (Tipo == INTEIRO)
        fprintf(ArqAsm, " %30s movl  %%ebx, -%ld(%%ebp)\n", BRANCO, Desl);
      else
      if (Tipo == PONTEIRO)
        fprintf(ArqAsm, " %30s movl  %%ebx, -%ld(%%ebp)\n", BRANCO, Desl);
      else
      if (Tipo == CADEIA)
      {
        fprintf(ArqAsm, " %30s leal  -%ld(%%ebp), %%ecx\n", BRANCO, Desl);
        fprintf(ArqAsm, " %30s pushl %%ecx\n",              BRANCO);
        fprintf(ArqAsm, " %30s pushl %%ebx\n",              BRANCO);
        fprintf(ArqAsm, " %30s call  _copcad\n",            BRANCO);
        fprintf(ArqAsm, " %30s addl  $8, %%esp\n",          BRANCO);
      }
    }
    else
    {
    /* parametro */
      if (Tipo == INTEIRO)
        fprintf(ArqAsm, " %30s movl  %%ebx, %ld(%%ebp)\n", BRANCO, Desl);
      else
      if (Tipo == PONTEIRO)
        fprintf(ArqAsm, " %30s movl  %%ebx, %ld(%%ebp)\n", BRANCO, Desl);
      else
      if (Tipo == CADEIA)
      {
        fprintf(ArqAsm, " %30s movl  -%ld(%%ebp), %%ecx\n", BRANCO, Desl);
        fprintf(ArqAsm, " %30s pushl %%ecx\n",              BRANCO);
        fprintf(ArqAsm, " %30s pushl %%ebx\n",              BRANCO);
        fprintf(ArqAsm, " %30s call  _copcad\n",            BRANCO);
        fprintf(ArqAsm, " %30s add  $8, %%esp\n",           BRANCO);
      }
    }
  }
  DesalocarRegistrador();
}
/*------------------------------*/
/* gerar atribuicao para vetor  */
/*------------------------------*/
PUBLIC void GerarAtribuicaoVetor(char *Id, int T)
{
  register int Ind;
  register int Tipo;
  register int Catg;

  Tipo = INTEIRO;
  Catg = CVAR;	  
  if ((Ind = Procurar(Id)) == -1) Abortar(3);
  else
  {
    Tipo = Tabela[Ind]->Tipo;
    Tipo = Tabela[Tipo]->Tipo;
    Catg = Tabela[Ind]->Categ;
  }
  if (Catg != CVET) Abortar(6);
  if (Tipo != T)    Abortar(4);
  fprintf(ArqAsm, " %30s popl  %%eax\n", BRANCO);
  if ((Tipo == INTEIRO) || (Tipo == PONTEIRO))
  {
    fprintf(ArqAsm, " %30s xchgl %%ebx, %%eax\n",   BRANCO);
    fprintf(ArqAsm, " %30s movl  %%eax, (%%ebx)\n", BRANCO);
  }
  else
  if (Tipo == CADEIA)
  {
    fprintf(ArqAsm, " %30s pushl %%eax\n",     BRANCO);
    fprintf(ArqAsm, " %30s pushl %%ebx\n",     BRANCO);
//    fprintf(ArqAsm, "extrn _copcad:near\n");
    fprintf(ArqAsm, " %30s call  _copcad\n", BRANCO);
    fprintf(ArqAsm, " %30s addl  $8, %%esp\n",  BRANCO);
  }
  DesalocarRegistrador();
}
/*-------------------------------*/
/* gerar chamada de procedimento */
/*-------------------------------*/
PUBLIC void GerarChamada(char *Id, int NA, int NB)
{
  register int Ind; /* indice na tabela */
  register int Cat; /* categoria */
  register int NP;  /* numero de parametros */

  if ((Ind = Procurar(Id)) == -1)
  {
    Cat = CPROC;
  }
  else
  {
    Cat = Tabela[Ind]->Categ;     /* id deve ser proc interna */
    if (Cat != CPROC) Abortar(6);
    NP  = Tabela[Ind]->Tipo;      /* contem o numero de parametros */
    if (NA > NP) Abortar(8);
    if (NA < NP) Abortar(9);
  }
  fprintf(ArqAsm, " %30s call  _%s\n", BRANCO, Id);
  if (NB > 0)
    fprintf(ArqAsm, " %30s addl  $%d, %%esp\n", BRANCO, NB);
}
/*----------------------------------*/
/* gerar comando retorne expressao */
/*----------------------------------*/
PUBLIC void GerarRetorne()
{
  fprintf(ArqAsm, " %30s movl  %%ebx, %%eax\n", BRANCO);
  DesalocarRegistrador();
}
/*--------------------------*/
/* gerar comando se parte 1 */
/*--------------------------*/
PUBLIC void GerarSe1(int *Rot1, int *Rot2)
{
  *Rot1 = ObterRotulo();
  *Rot2 = ObterRotulo();
  fprintf(ArqAsm, " %30s cmpl  $0, %%ebx\n", BRANCO);
  fprintf(ArqAsm, " %30s jne   R%d\n",       BRANCO, *Rot1);
  fprintf(ArqAsm, " %30s jmp   R%d\n",       BRANCO, *Rot2);
  fprintf(ArqAsm, "R%d:\n",                  *Rot1);
  DesalocarRegistrador();
}
/*--------------------------*/
/* gerar comando se parte 2 */
/*--------------------------*/
PUBLIC void GerarSe2(int Rot2, int *Rot3)
{
  *Rot3 = ObterRotulo();
  fprintf(ArqAsm, " %30s jmp   R%d\n", BRANCO, *Rot3);
  fprintf(ArqAsm, "R%d:\n",           Rot2);
}
/*--------------------------*/
/* gerar comando se parte 3 */
/*--------------------------*/
PUBLIC void GerarSe3(int Rot3)
{
  fprintf(ArqAsm, "R%d:\n", Rot3);
}
/*--------------------------------*/
/* gerar comando enquanto parte 1 */
/*--------------------------------*/
PUBLIC void GerarEnquanto1(int *Rot1)
{
  *Rot1 = ObterRotulo();
  fprintf(ArqAsm, "R%d:\n", *Rot1);
}
/*--------------------------------*/
/* gerar comando enquanto parte 2 */
/*--------------------------------*/
PUBLIC void GerarEnquanto2(int *Rot3)
{
  int Rot2;

  Rot2  = ObterRotulo();
  *Rot3 = ObterRotulo();
  fprintf(ArqAsm, " %30s cmpl  $0, %%ebx\n", BRANCO);
  fprintf(ArqAsm, " %30s jne   R%d\n",       BRANCO, Rot2);
  fprintf(ArqAsm, " %30s jmp   R%d\n",       BRANCO, *Rot3);
  fprintf(ArqAsm, "R%d:\n",                  Rot2);
  DesalocarRegistrador();
}
/*--------------------------------*/
/* gerar comando enquanto parte 3 */
/*--------------------------------*/
PUBLIC void GerarEnquanto3(int Rot1, int Rot3)
{
  fprintf(ArqAsm, " %30s jmp   R%d\n", BRANCO, Rot1);
  fprintf(ArqAsm, "R%d:\n",           Rot3);
}
/*--------------------------------*/
/* gerar comando enquanto parte 1 */
/*--------------------------------*/
PUBLIC void GerarRepeticao1(int *Rot1)
{
  *Rot1 = ObterRotulo();
  fprintf(ArqAsm, "R%d:\n", *Rot1);
}
/*--------------------------------*/
/* gerar comando enquanto parte 2 */
/*--------------------------------*/
PUBLIC void GerarRepeticao2(int Rot1)
{
  int Rot2;

  Rot2 = ObterRotulo();
  fprintf(ArqAsm, " %30s cmpl  $0, %%ebx\n", BRANCO);
  fprintf(ArqAsm, " %30s jne   R%d\n",       BRANCO, Rot2);
  fprintf(ArqAsm, " %30s jmp   R%d\n",       BRANCO, Rot1);
  fprintf(ArqAsm, "R%d:\n", Rot2);
  DesalocarRegistrador();
}
/*--------------------------------------------------*/
/* desassociar o arquivo com o analisador semantico */
/*--------------------------------------------------*/
PUBLIC void TerminarAnalisadorSemantico()
{
  fclose(ArqAsm);
}
/*----------------------------*/
/* listar a tabela de simbolos */
/*----------------------------*/
PUBLIC void Listar()
{
  TSimbolo *P;
  int Ind;

  for (Ind=0; Ind<TopoTab; Ind++)
  {
    P = Tabela[Ind];
    printf("Nome=%30s, Categoria=%5d, Nivel=%1d, Desloc=%5ld, Tipo=%5d\n",
            P->Nome,   P->Categ,      P->Nivel,  P->Desl,     P->Tipo);
  }
}
