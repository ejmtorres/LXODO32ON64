tudo:
	as --32 -o xes.o xes.s
	as --32 -o xcad.o xcad.s

finalizar :
	rm *.o
