/**
 * @file listeProcessus.c
 * @author Jean-Baptiste Prevost
 * @brief Corp du module de gestion de la liste des processus du minishell
 * @version 1.0
 * @date 2022-05-24
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "listeProcessus.h"

/**
 * @brief Initialiser une liste des processus
 * @return ListeFils* 
 */
ListeFils *initListe()
{
    ListeFils *nouvelleListe = malloc(sizeof(ListeFils));
    nouvelleListe->debutListe = NULL;
    nouvelleListe->idNouveauFils = 0;
    return nouvelleListe;
}

/**
 * @brief Creer un nouveau processus pour la liste
 * @param id 
 * @param filsPid 
 * @param commande 
 * @return etatFils* 
 */
static etatFils *nouveauProc(int id, int filsPid, char *commande)
{
    etatFils *nouveauEtatFils = malloc(sizeof(etatFils));
    nouveauEtatFils->pid = filsPid;
    nouveauEtatFils->id = id;
    nouveauEtatFils->etat = "actif";
    nouveauEtatFils->cmd = commande;
    nouveauEtatFils->suivant = NULL;
    return nouveauEtatFils;
}

/**
 * @brief Permettre d'ajouter un processus a la liste 
 * @param liste 
 * @param pidFils 
 * @param commande 
 */
void ajoutProc(ListeFils *liste, int pidFils, char *commande)
{
    // printf("%d\n", liste->idNouveauFils);
    char *copieCmd = malloc(sizeof(char *));
    strcpy(copieCmd, commande); // Copie de la valeur de commande
    etatFils *nouveauEtatFils = nouveauProc(liste->idNouveauFils, pidFils, copieCmd);
    etatFils *copieEtat = liste->debutListe;
    if (copieEtat == NULL)
    {
        printf("ajout\n");
        liste->debutListe = nouveauEtatFils;
    }
    else
    {
        while (copieEtat->suivant != NULL)
        {
            copieEtat = copieEtat->suivant;
        }
        copieEtat->suivant = nouveauEtatFils;
    }
    liste->idNouveauFils++;
}

/**
 * @brief Convertir un id en pid 
 * @param liste 
 * @param idProc 
 * @return int 
 */
int id2pid(ListeFils *liste, int idProc)
{
    etatFils *copieEtat = liste->debutListe;
    int pidProc = -1;
    if (copieEtat == NULL)
    {
        printf("ID inexistant\n");
    }
    else
    {
        while (copieEtat->suivant != NULL && copieEtat->id != idProc)
        {
            copieEtat = copieEtat->suivant;
        }
        if (copieEtat->id != idProc)
        {
            printf("ID inexistant\n");
        }
        else
        {
            pidProc = copieEtat->pid;
        }
    }

    return pidProc;
}

/**
 * @brief Permettre de supprimer un processus de la liste
 * @param liste 
 * @param pidSupp 
 */
void supprimerProc(ListeFils *liste, int pidSupp)
{

    /* Verification si liste non nulle */
    if (liste->debutListe == NULL)
    {
       // printf("Il n'y a pas de processus en arrrière plan\n");
    }
    else if (liste->debutListe->suivant == NULL)
    {
        if (liste->debutListe->pid == pidSupp)
        {
            liste->debutListe = NULL;
            liste->idNouveauFils = 0;
        }
        else
        {
            printf("Le processus n'existe pas\n");
        }
    }
    else
    {
        etatFils *copieEtat = liste->debutListe;
        /* Recherche pid */
        while (copieEtat->suivant != NULL && copieEtat->pid != pidSupp)
        {
            copieEtat = copieEtat->suivant;
        }
        /* Décalage liste */
        while (copieEtat->suivant != NULL)
        {
            copieEtat = copieEtat->suivant;
        }
    }
}

/**
 * @brief Changer le nom de l'etat
 * @param etatCmd 
 */
static void chgtNom(etatFils *etatCmd)
{
    if (!strcmp(etatCmd->etat, "actif"))
    {
        etatCmd->etat = "suspendu";
    }
    else
    {
        etatCmd->etat = "actif";
    }
}

/**
 * @brief Changer l'etat d'un processus de la liste
 * @param liste 
 * @param pidChgt 
 */
void changementEtat(ListeFils *liste, int pidChgt)
{

    /* Verification si liste non nulle */
    if (liste->debutListe == NULL)
    {
        //printf("Il n'y a pas de processus en arrrière plan\n");
    }
    else if (liste->debutListe->suivant == NULL)
    {
        if (liste->debutListe->pid == pidChgt)
        {
            chgtNom(liste->debutListe);
        }
        else
        {
            printf("Le processus n'existe pas\n");
        }
    }
    else
    {
        etatFils *copieEtat = liste->debutListe;
        /* Recherche pid */
        while (copieEtat->suivant != NULL && copieEtat->pid != pidChgt)
        {
            copieEtat = copieEtat->suivant;
        }
        chgtNom(copieEtat);
    }
}

/**
 * @brief Afficher la liste des processus
 * @param liste 
 */
void afficherListe(ListeFils *liste)
{
    printf("id\t\tNom\t\tpid\t\tetat\n");
    etatFils *copieEtat = liste->debutListe;
    if (copieEtat == NULL)
    {
        printf("Il n'y a aucun proccessus en cours\n");
    }
    else if (copieEtat->suivant == NULL)
    {
        printf("%d\t\t%s\t\t%d\t\t%s\n", copieEtat->id, copieEtat->cmd, copieEtat->pid, copieEtat->etat);
    }
    else
    {
        while (copieEtat->suivant != NULL)
        {
            printf("%d\t\t%s\t\t%d\t\t%s\n", copieEtat->id, copieEtat->cmd, copieEtat->pid, copieEtat->etat);
            copieEtat = copieEtat->suivant;
        }
        printf("%d\t\t%s\t\t%d\t\t%s\n", copieEtat->id, copieEtat->cmd, copieEtat->pid, copieEtat->etat);
    }
}

/**
 * @brief Détruire la liste de processus 
 * @param liste 
 */
void detruireListe(ListeFils *liste)
{
    etatFils *copieEtat = liste->debutListe;
    if (copieEtat == NULL || copieEtat->suivant == NULL)
    {
        free(liste->debutListe);
        liste->debutListe = NULL;
        liste->idNouveauFils = 0;
    }
    else
    {
        etatFils *etatSup = copieEtat;
        while (copieEtat->suivant != NULL)
        {
            etatSup = copieEtat;
            copieEtat = copieEtat->suivant;
            free(etatSup);
        }
    }
}
