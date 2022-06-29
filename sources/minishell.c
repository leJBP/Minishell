/**
 * @file minishell.c
 * @author Jean-Baptiste Prevost
 * @brief Minishell
 * @version 1.0
 * @date 2022-05-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <malloc.h>
#include <signal.h>
#include <fcntl.h> 

#include "readcmd.h"
#include "listeProcessus.h"

/* Déclaration des variables liées aux minishell */
const char *listCmdInterne = "exit cd lj sj bg fg susp"; //Liste des commandes interne
struct cmdline *commande; //Commande de l'utilisateur
int finShell = 0; //Determine si on ferme le shell ou non
int execFg; //Determine si il y a un processus en premier plan
int filsCMD; //pid du fils qui execute la commande
ListeFils *listeProc; //liste des processus en cours dans le minishell
sigset_t ens_shell; //Ensemble signaux a masquer    

/* Handler associé au signal SIGCHLD */
void handlerSigChld(int sigCHLD){
    int pid, wstatus;
    if (sigCHLD == SIGCHLD){
        do {
            pid = (int) waitpid(-1, &wstatus, WNOHANG | WUNTRACED | WCONTINUED);
            if (pid < 0){
                //printf("Fin de traitement signal SIGCHLD\n");
            }
            else if (pid > 0){
                
                if(WIFSTOPPED(wstatus)){
                changementEtat(listeProc, pid);
                } else if (WIFCONTINUED(wstatus)){
                changementEtat(listeProc, pid);
                } else if (WIFEXITED(wstatus)){
                supprimerProc(listeProc, pid);
                }
            }            
        } while(pid > 0);
    }      
}

/* Handler associé au signal SIGTSTP */
void handlerSigTstp(int sigTSTP){
    /* Mise en pause processus premier plan */
    if (execFg){
        printf("Il y a un processus en premier plan : %d\n", filsCMD);
        execFg = 0;
        //kill(filsCMD, SIGTSTP);
        ajoutProc(listeProc, filsCMD, commande->seq[0][0]);
    } else {
        printf("Il n'y a pas de processus en premier plan\n");
    }    
}

/* Handler associé au signal SIGINT */
void handlerSigInt(int sigINT){
    /* Inerruption processus premier plan */
    if (execFg){
        printf("Il y a un processus en premier plan : %d\n", filsCMD);
        //kill(filsCMD, SIGINT);
        supprimerProc(listeProc, filsCMD);
    } else {
        printf("Il n'y a pas de processus en premier plan\n");
    }
}

/* Procédure de redirection < */
void redirDrtGch(char* fichier){
    int descf1 = open(fichier, O_RDONLY); //Ouverture ou création d'un fichier
    /* Test retour d'appel de open */
    if (descf1 < 0){
        perror("Erreur execution fonction open in\n");
        exit(5);
    }
    int dup2descf1 = dup2(descf1, 0); //Redirection de la sortie standard
    /* Test de bonne redirection */
    if (dup2descf1 < 0){
        perror("Erreur de redirection entrée standard\n");
    }
}

/* Procédure de redirection > */
void redirGchDrt(char* fichier){
    int descf2 = open(fichier, O_WRONLY|O_CREAT|O_TRUNC, 0640); //Ouverture ou création d'un fichier
    /* Test retour d'appel de open */
    if (descf2 < 0){
        perror("Erreur execution fonction open out\n");
        exit(5);
    }
    int dup2descf2 = dup2(descf2, 1); //Redirection de la sortie standard
    /* Test de bonne redirection */
    if (dup2descf2 < 0){
        perror("Erreur de redirection sortie standard\n");
    }
}

/* Procédure de gestion des commandes demandant la création d'un fils */
void execCommande(struct cmdline *commande){

    int numCmd = 0; //Numéro de la commande a éxécuter
    int nbrCmd = 0; //Nombre de commande
    int retPipe = 1; //Retour ouverture de pipe
    int f2p[2]; //pipe
    pid_t pidFilsCMD;

    /* Nombre de commande */
    while (commande->seq[nbrCmd] != NULL) { 
        nbrCmd++; 
    }

    if (nbrCmd > 1){
            retPipe = pipe(f2p);
            if(retPipe < 0){
                printf("Erreur ouverture pipe\n");
            }
        }

    /* Execution des differentes commandes si il y a des pipes */
    while (numCmd < nbrCmd){        
        pidFilsCMD = fork(); //Création d'un fils
        filsCMD = pidFilsCMD;          
                
        if (pidFilsCMD == -1){            
            exit(1);
        }        
        if (pidFilsCMD == 0){ 
            /* Masquage signal interruption si commande arriere plan */
            if (commande->backgrounded != NULL){
                sigprocmask(SIG_BLOCK, &ens_shell, NULL);
            }

            /* Detection de redirection */
            if (commande->out != NULL){ //Redirection >
                redirGchDrt(commande->out);
            } 
            if (commande->in != NULL){ //Redirection <
                redirDrtGch(commande->in);
            }

            /* Création des tubes */
            if (retPipe != 1){ // Condition tube créé ?
                if(numCmd == 0){ /* Premier tube pour la premiere commande */
                    //printf("Tube debut + %d\n", numCmd);
                    int ret = close(f2p[0]); //Fermeture entrée standard fils premiere commande
                    if(ret < 0){
                        perror("Erreur fermeture entrée standard\n");
                    }
                    ret = dup2(f2p[1], 1); //Redirection sortie standard dans le pipe
                    if (ret < 0){
                        perror("Erreur de redirection sortie standard\n");
                    }
                    ret = close(f2p[1]); //Fermeture pipe sortie apres redirection
                    if(ret < 0){
                        perror("Erreur fermeture f2p[1] tube de départ\n");
                    }
                } else if(commande->seq[numCmd + 1] != NULL && commande->seq[numCmd] != NULL){ /* Tubes intermediaires */
                    //printf("Tube intermediaire + %d\n", numCmd);
                    int ret = dup2(f2p[0],0); //Redirection entrée standard commande intermediaire
                    if (ret < 0){
                        perror("Erreur redirection entrée commande intermediaire\n");
                    }
                    ret = close(f2p[0]); //Fermeture entrée tube apres redirection
                    if(ret < 0){
                        perror("Erreur fermeture f2p[0] tube intermediaire\n");
                    }
                    ret = close(f2p[1]); //Fermeture sortie tube 
                    if(ret < 0){
                        perror("Erreur fermeture f2p[1] tube intermediaire\n");
                    }
                } else if(commande->seq[numCmd + 1] == NULL && commande->seq[numCmd] != NULL){ /* Tube de fin */
                    //printf("Tube fin + %d\n", numCmd);                    
                    int ret = dup2(f2p[0], 0); //Redirection entrée standard sur entrée pipe
                    if (ret < 0){
                        perror("Erreur redirection entrée standard sur entrée pipe\n");
                    }
                    ret = close(f2p[0]); //Fermeture entrée pipe apres redirection
                    if(ret < 0){
                        perror("Erreur fermeture f2p[0] tube de fin\n");
                    }
                    ret = close(f2p[1]); //Fermeture sortie tube
                    if(ret < 0){
                        perror("Erreur fermeture sortie pipe commande fin\n");
                    }
                }
            }       
            /* Execution de la commande */
            execvp(commande->seq[numCmd][0], commande->seq[numCmd]);                
            exit(2);      
        }
        numCmd++;       
    }

    if (commande->backgrounded != NULL){
        ajoutProc(listeProc, pidFilsCMD, commande->seq[numCmd][0]);
    } else {
        execFg = 1;
        pause();
        //waitpid(pidFilsCMD, &etatFilsCMD, 0);
    }

    if (WIFEXITED(pidFilsCMD)){
        if (WEXITSTATUS(pidFilsCMD) == 0){
            printf("SUCCES\n");
        } else {
            printf("Commande inexistante\n");
        }
    } 

    /* Fermeture du pipe */
    if (retPipe == 0){
        if(close(f2p[0]) < 0){
            perror("Erreur fermeture f2p[0] pere\n");
        }
        if(close(f2p[1]) < 0){
            perror("Erreur fermeture f2p[1] pere\n");
        } 
    }
    
}

/* Gestion de la commande cd */
void cdInterne(char* chemin){
    int retChdir;
    if (chemin == NULL) {
            chdir(getenv("HOME"));
    } else {
        retChdir = chdir(chemin);
        if (retChdir != 0){
            perror("Impossible d'acceder a ce chemin\n");
        }
    }
}

/* Procédure d'affichage de la liste des jobs */
void ljInterne(){
    afficherListe(listeProc);
}

/* Procédure de stop jobs */
void sjInterne(char* idSj){
    int pidSj;

    if (idSj == NULL) {
        printf("Argument invalide : ID manquant\n");
    } else {
       pidSj = id2pid(listeProc, strtol(idSj, NULL, 10));
       if (pidSj >= 0){
            kill(pidSj, SIGSTOP);           
       }
    }
}

/* Procédure remmise en execution en arrière plan */
void bgInterne(char* idBg){
    int pidBg;

    if (idBg == NULL) {
        printf("Argument invalide : ID manquant\n");
    } else {
       pidBg = id2pid(listeProc, strtol(idBg, NULL, 10));
       if (pidBg >= 0){
           kill(pidBg, SIGCONT);                     
       }
    }  
}
  
/* Procédure remise en execution en avant plan */
void fgInterne(char* idFg){
    int pidFg;

    if (idFg == NULL) {
        printf("Argument invalide : ID manquant\n");
    } else {
       pidFg = id2pid(listeProc, strtol(idFg, NULL, 10));
       if (pidFg >= 0){
           kill(pidFg, SIGCONT);  
           execFg = 1;
           filsCMD = pidFg;
           sleep(1);
           pause();                   
       }
    } 
}

/* Procédure suspension minishell */
void suspInterne(){
    printf("Shell en suspend, PID : %d \n", getpid());
    kill(getpid(), SIGSTOP);
}

/* Procédure de gestion des commandes interne */
void commandeInterne(struct cmdline *commande) {   
    
    if (!strcmp(commande->seq[0][0], "exit")){
        finShell = 1;
    } else if (!strcmp(commande->seq[0][0], "cd")){
        cdInterne(commande->seq[0][1]);
    } else if (!strcmp(commande->seq[0][0], "lj")){
        ljInterne();
    } else if (!strcmp(commande->seq[0][0], "sj")){
        sjInterne(commande->seq[0][1]);
    } else if (!strcmp(commande->seq[0][0], "bg")){
        bgInterne(commande->seq[0][1]);
    } else if (!strcmp(commande->seq[0][0], "fg")){
        fgInterne(commande->seq[0][1]);
    } else if (!strcmp(commande->seq[0][0], "susp")){
        suspInterne(commande->seq[0][1]);
    } else {
        printf("La commande n'existe pas\n");
    }
    
}

/* Boucle du shell */
int main() {
        
    sigemptyset(&ens_shell); //Initialisation ensemble des signaux à masquer

    /* Ajout signaux a l'ensemble à masquer pour les processus en arriere plan*/
    sigaddset(&ens_shell, SIGINT); //Ajout SIGINT à l'ensemble
    sigaddset(&ens_shell, SIGTSTP); //Ajout SIGTSTP à l'ensemble 

    /* Association du signal SIGCHLD a un nouveau traitant */
    struct sigaction handler;
    handler.sa_handler = handlerSigChld; //Association du nouveau traitant de SIGCHLD
    handler.sa_flags = SA_RESTART; //Permet de de continuer le programme a la fin du handlers
    sigemptyset(&handler.sa_mask);
    sigaction(SIGCHLD, &handler, NULL); 
    
    /* Association du signal SIGTSTP a un nouveau traitant */
    struct sigaction handlerSTOP;
    handlerSTOP.sa_handler = handlerSigTstp; //Association du nouveau traitant de SIGCHLD
    handlerSTOP.sa_flags = SA_RESTART; //Permet de de continuer le programme a la fin du handlers
    sigemptyset(&handlerSTOP.sa_mask);
    sigaction(SIGTSTP, &handlerSTOP, NULL); 

    /* Association du signal SIGINT a un nouveau traitant */
    struct sigaction handlerINT;
    handlerINT.sa_handler = handlerSigInt; //Association du nouveau traitant de SIGCHLD
    handlerINT.sa_flags = SA_RESTART; //Permet de de continuer le programme a la fin du handlers
    sigemptyset(&handlerINT.sa_mask);
    sigaction(SIGINT, &handlerINT, NULL); 

    /* Initialisation de la liste des processus propre au minishell */
    listeProc = initListe();

    /* Boucle du minishell */
    do  {    
        
        execFg = 0;
        printf(">>> ");
        commande = readcmd(); //Lecture entrée standard

        /* Verification entrée standard correcte */
        if (commande == NULL) {
            finShell = 1;
        } 
        else if (commande->seq[0] == NULL || commande->err != NULL){
            printf("\n");
        }          
        /* Execution interne ou externe */     
        else {            
            if (strstr(listCmdInterne, commande->seq[0][0])) {
                commandeInterne(commande);
            } else {    
                execCommande(commande);
            }   
        }    

    } while (!finShell);

    printf("Fin du minishell\n");
    /* Destruction de la liste des processus, libération de la mémoire */
    detruireListe(listeProc);
    
    return EXIT_SUCCESS;   
}
