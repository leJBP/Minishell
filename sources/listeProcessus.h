#ifndef LISTEPROCESSUS_H
#define LISTEPROCESSUS_H

struct etatFils{
    int pid;
    int id;
    char* etat;
    char* cmd;
    struct etatFils *suivant;
};
typedef struct etatFils etatFils;

struct ListeFils{
    etatFils *debutListe;
    int idNouveauFils;
};
typedef struct ListeFils ListeFils;

/* Initialiser la liste de processus */
ListeFils* initListe();

/* Permettre d'ajouter un processus a la liste des jobs */
void ajoutProc(ListeFils *liste, int pidFils, char* commande);

/* Permettre de convertir un id en pid */
int id2pid(ListeFils *liste, int idProc);

/* Retirer un processus de la liste qui est terminé */
void supprimerProc(ListeFils *liste, int pidSupp);

/* Permettre de modifier l'état d'un processus */
void changementEtat(ListeFils *liste, int pidChgt);

/* Afficher la liste des processus */
void afficherListe(ListeFils *liste);

/* Détruire la liste de processus */
void detruireListe(ListeFils *liste);

#endif