#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/shm.h>

#define M 5
int N = 20;
int tube[2];

int MemP_id;

sem_t P1Producer;
sem_t P2Consumer;



void P1(){
    
    //Initialisation des sémaphores
    sem_t* P1Producer = sem_open("P1Producer",O_EXCL);
    sem_t* P2Consumer = sem_open("P2Consumer",O_EXCL);
    
    int *MemP  = (int *) shmat(MemP_id, NULL, 0);
    
    for (int i = 0; i < 20; i++){
        sem_wait(P2Consumer);
        int pos = i % 5;
        MemP[pos] = i;
        printf("\tLa valeur %d viens d'être mise en mémoire partagé a la position %d\n", i, pos);
        sem_post(P1Producer);

    }

    shmdt(MemP);

    exit(0);
}

void P2(){
    //Initialisation des sémaphores
    sem_t* P1Producer = sem_open("P1Producer",O_EXCL);
    sem_t* P2Consumer = sem_open("P2Consumer",O_EXCL);

    int *MemP  = (int *) shmat(MemP_id, NULL, 0);

    int valeur;

    for (int i = 0; i < 20; i++){
        sem_wait(P1Producer);
        int pos = i % 5;
        valeur = MemP[pos];
        printf("\t\tLa valeur %d viens d'être retiré de la mémoire partagé \n", valeur);
        write(tube[1], &valeur, sizeof(int));
        sem_post(P2Consumer);

    }

    shmdt(MemP);


    exit(0);
}

void P3(){
    int i = 0, valeur, taille;
    close(tube[1]);

    while(i < 20){
        taille = read(tube[0], &valeur, sizeof(int));
        if (taille > 0){
            printf("\t\t\tLa valeur extraite du Tube est %d \n", valeur);
		    i++;
        }

    }

    exit(0);
}


int main(int argc, char* argv[]){

    printf("Création de la mémoire partagé\n");
    MemP_id = shmget(IPC_PRIVATE, 5*sizeof(int), IPC_CREAT | 0777);
    
    printf("Création des sémaphores\n");
    sem_t* P2Consumer = sem_open("P2Consumer",O_CREAT,0777,5);
    sem_t* P1Producer = sem_open("P1Producer",O_CREAT,0777,0);

    int  id1 = fork();
            if(id1==0) {
                printf("\t\t--------\tCréation de P1\t--------\n");
                P1();
            }
    
            if (pipe(tube) < 0){
                exit(1);
            }else{
                printf("\t\t--------\tCréation du tube\t--------\n");
            }


            int  id2 = fork();
                    if(id2==0) 
                    {
                        printf("\t\t--------\tCréation de P2\t--------\n");
                        P2();
                    }
            int  id3 = fork();
                    if(id3==0) 
                    {
                        printf("\t\t--------\tCréation de P3\t--------\n");
                        P3();
                    }
        wait(0);
    	wait(0);
    	wait(0);



	sem_unlink("P2Consumer");
	sem_unlink("P1Producer");
    return 0;
}
