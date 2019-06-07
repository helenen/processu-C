/************* PROCESSUS A SERVEUR ***************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/shm.h>

#define CLE 12

struct clientConsultation
{
    long mtype;
    pid_t pid;
    char nomSpectacle[60];
};

struct clientReservation
{
    long mtype;
    pid_t pid;
    char nomSpectacle[60];
    int nbPlace;
};

struct clientMsgRecu
{ //message de retour pour client Reservation
    long mtype;
    char msgRetour[60];
};

struct serveurConsultation
{
    long mtype;
    char nomSpectacle[60];
    int nbPlace;
};

typedef struct spectacle
{
    int numSpectacle;
    char *nomSpectacle;
    char dateSpectacle[10];
    int nbPlaceRestant;
} un_spectacle;

/*************************  CONSULTATION ******************************/

void consultation()
{

    /*   récupération  MSQ */
    int msqid;
    msqid = msgget((key_t)CLE, 0750);

    struct serveurConsultation serveur_Consultation;
    struct clientConsultation client_Consultation;
    struct spectacle *spec;

    while (1)
    {
        int i = 0;
        msgrcv(msqid, &client_Consultation, sizeof(struct clientConsultation) - sizeof(long), 24, 0); //le 24 c'est mtype

        int shmid;
        shmid = shmget((key_t)CLE, sizeof(struct spectacle), 0750);

        if ((spec = shmat(shmid, NULL, 0)) == (struct spectacle *)-1)
        {
            perror("shmat erreur ");
            exit(2);
        }

        printf("Je suis le serveur je viens de recevoir comme nom de spectacle : %s\n", client_Consultation.nomSpectacle); // le serveur reçoit le nom du Spectacle

        int j = 0;
        for (j = 0; j < 10; j++)
        {
            printf("%s", spec->nomSpectacle);
            spec += sizeof(struct spectacle);
        }

        strcpy(serveur_Consultation.nomSpectacle, client_Consultation.nomSpectacle);
        for (i = 0; i < 10; i++)
        {

            if (strcmp(client_Consultation.nomSpectacle, spec[i].nomSpectacle) == 0)
            { //si identique
                strcpy(serveur_Consultation.nomSpectacle, spec[i].nomSpectacle);
                serveur_Consultation.nbPlace = spec[i].nbPlaceRestant;

                printf("%s\n", "j'ai trouvé un spectacle identique et je vais envoyer à mon client \n");
                break;
            }

            else
            {

                printf("%s\n", "Ce spectacle n'existe pas");

                serveur_Consultation.nbPlace = -1;
            }
        }
        serveur_Consultation.mtype = client_Consultation.pid;
        printf("%s\n", serveur_Consultation.nomSpectacle);
        printf("%d\n", serveur_Consultation.nbPlace);
        msgsnd(msqid, &serveur_Consultation, sizeof(struct serveurConsultation) - sizeof(long), 0);
    }
}

/*************************  RESERVATION ******************************/

void reservation()
{

    /*   récupération  MSQ */
    int msqid;
    msqid = msgget((key_t)CLE, 0750);

    struct clientMsgRecu client_MsgRecu;
    struct clientReservation client_Reservation;
    struct spectacle *spec;

    while (1)
    {
        int i = 0;
        msgrcv(msqid, &client_Reservation, sizeof(struct clientReservation) - sizeof(long), 18, 0); //le 18 c'est mtype

        int shmid;
        shmid = shmget((key_t)CLE, sizeof(struct spectacle), 0750);

        if ((spec = shmat(shmid, NULL, 0)) == (struct spectacle *)-1)
        {
            perror("shmat erreur ");
            exit(2);
        }

        printf("Je suis le serveur je viens de recevoir comme nom de spectacle : %s\n", client_Reservation.nomSpectacle); // le serveur reçoit le nom du Spectacle

        for (i = 0; i < 10; i++)
        {

            if (strcmp(client_Reservation.nomSpectacle, spec[i].nomSpectacle) == 0)
            { //si identique
                if (client_Reservation.nbPlace <= spec[i].nbPlaceRestant)
                {
                    printf("Si je peux réserver\n");
                    strcpy(client_MsgRecu.msgRetour, "Vous pouvez réserver\n");
                    break;
                }
                else
                {
                    printf("Si je peux pas réserver\n");
                    strcpy(client_MsgRecu.msgRetour, "Vous ne pouvez pas réserver, nombre de place indisponible\n");
                    break;
                }
            }
            else
            {
                strcpy(client_MsgRecu.msgRetour, "Vous ne pouvez pas réserver, le nom du spectacle n'existe pas\n");
            }
        }
        client_MsgRecu.mtype = client_Reservation.pid;
        msgsnd(msqid, &client_MsgRecu, sizeof(struct clientMsgRecu) - sizeof(long), 0);
    }
}

int main(int argc, char **argv)
{

    int msqid;
    msqid = msgget((key_t)CLE, 0750 | IPC_CREAT | IPC_EXCL);

    //definir les structures
    struct clientConsultation client_Consultation;
    struct clientReservation client_Reservation;
    struct clientMsgRecu client_MsgRecu;
    struct serveurConsultation serveur_Consultation;
    un_spectacle tab[10] = {
        {1, "Spectacle1", "01/01/2010", 30},
        {2, "Spectacle2", "01/04/2010", 30},
        {3, "Spectacle3", "01/06/2010", 30},
        {4, "Spectacle4", "04/07/2010", 30},
        {5, "Spectacle5", "24/12/2010", 30},
        {6, "Spectacle6", "15/11/2010", 30}};

    int shmid;
    shmid = shmget((key_t)CLE, (10 * sizeof(struct spectacle **)), IPC_CREAT | 0750);

    //declarer les fork
    pid_t f;

    struct spectacle *spec;

    if ((spec = shmat(shmid, NULL, 0)) == (struct spectacle *)-1)
    {
        perror("shmat erreur vuycty");

        exit(2);
    }
    /* écriture dans le segment */
    //spec[0].numSpectacle= 3;
    //strcpy(spec[0].nomSpectacle, "Spe1");
    //strcpy(spec[0].dateSpectacle, "01/01/2010");
    //spec[0].nbPlaceRestant = 30;

    spec = &tab[0];
    /*
	for (int i = 0; i < 6; i++){
		printf("%s\n", spec.nomSpectacle);
		spec += sizeof(struct spectacle);
	}
	*/
    // il est ou? jeveux ecrire dedans

    f = fork();

    if (f < 0)
    {
        perror("erreur fork\n");
    }

    else if (f == 0)
    { //fils
        consultation();
    }

    else
    { //pere
        reservation();
    }

    //Destruction du segment
    shmctl(shmid, IPC_RMID, NULL);
    exit(0);
}
