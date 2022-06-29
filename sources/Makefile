# Définition des Variables (à compléter)
CC=gcc
CFLAGS=-Wall -pedantic 
LDFLAGS= 

# Règles (à compléter avec des variables automatiques)
all: minishell visu_readcmd clean

minishell: readcmd.o listeProcessus.o minishell.o
	$(CC) $(CFLAGS) $< listeProcessus.o $@.o -o $@

visu_readcmd: readcmd.o visu_readcmd.o
	$(CC) $(LDFLAGS) $< $@.o -o $@

makedepend:
	makedepend *.c -Y

clean:
	rm *.o

.PHONY: clean all makedepend

# DO NOT DELETE

listeProcessus.o: listeProcessus.h readcmd.h
minishell.o: readcmd.h
projet.o: readcmd.h listeProcessus.h
readcmd.o: readcmd.h
visu_readcmd.o: readcmd.h
