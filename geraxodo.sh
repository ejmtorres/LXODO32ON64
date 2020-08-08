echo "################################"
echo "# Gerando o compilador xodo... #"
echo "################################"
cd fontes
make -f ../geraxodo.mak
mv xodo ..
echo "#####################################"
echo "# Gerando as bibliotecas do xodo... #"
echo "#####################################"
make -f ../geraxbib.mak
mv xes.o ..
mv xcad.o ..
cd ..
