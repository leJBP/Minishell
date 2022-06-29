# Minishell
A minishell in a shell

## Introduction
This program was a school project, I did a minishell with some intern command and not intern command. This project was made to work with UNIX primitive, like `fork(), kill(), ...`. Maybe there is some bug and memory leak remain.

## Compilation
To compile the program please use the Makefile in the sources folder by using the following command, the only requirement is to have a path to UNIX primitive : 

    make all

## Work

To execute the programm use the following command : 

    ./minishell
    
### Feature

To quit the programm there is two way :
- write `exit`
- `Ctrl + D`

Intern command : 
- change directory `cd path`, if you don't specify a path you we'll be bring to the root.
- list job `lj`, display the list of task which are in background.
- stop jobs `sj index`, pause the job with the specify job with the index.
- background job `bg index`, starts up again the specify job with the index
- foreground job `fg index`, job from background go to foreground
- suspension `susp`, pause the minishell
