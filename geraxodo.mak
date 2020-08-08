COMP=gcc
OPC=-W -O2 -c

xodo: xodo.o anasem.o anasin.o analex.o
	${COMP} -oxodo xodo.o anasem.o anasin.o analex.o
	rm *.o
xodo.o: xodo.c
	${COMP} $(OPC) xodo.c
anasem.o: anasem.c anasem.h
	${COMP} $(OPC) anasem.c
anasin.o: anasin.c anasin.h
	${COMP} $(OPC) anasin.c
analex.o: analex.c analex.h
	${COMP} $(OPC) analex.c
