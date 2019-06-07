/************* PROCESSUS B CLIENT CONSULTATION ***************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <pthread.h>

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
{
    long mtype;
    char msgRetour[60];
};

struct serveurConsultation
{
    long mtype;
    char nomSpectacle[60];
    int nbPlace;
};

struct spectacle
{
    int numSpectacle;
    char nomSpectacle[60];
    char dateSpectacle[10];
    int nbPlaceRestant;
};

int main(int argc, char **argv)
{

    int msqid;

    // Définir les structures
    struct clientConsultation client_Consultation;
    struct clientReservation client_Reservation;
    struct clientMsgRecu client_MsgRecu;
    struct serveurConsultation serveur_Consultation;

    /*   allocation  MSQ */
    msqid = msgget((key_t)CLE, 0750);

    while (1)
    {

        char choix[60];
        char nomSpectacle[60];
        int place;

        printf("consultation ou reservation ? \n");
        scanf("%s", choix);

        if (strcmp(choix, "consultation") == 0)
        {

            client_Consultation.mtype = 24;
            client_Consultation.pid = getpid();
            printf("Saisir le nom du Spectacle ? ");
            scanf("%s", nomSpectacle);
            strcpy(client_Consultation.nomSpectacle, nomSpectacle);

            msgsnd(msqid, &client_Consultation, sizeof(struct clientConsultation) - sizeof(long), 0); // 0 option

            //réception
            msgrcv(msqid, &serveur_Consultation, sizeof(struct serveurConsultation) - sizeof(long), client_Consultation.pid, 0);

            printf("Je suis le client et je reçois comme nom du spectacle : %s\n", serveur_Consultation.nomSpectacle);
            printf("Je reçois %d comme nombres de place disponible\n", serveur_Consultation.nbPlace);
            //strcpy(client_Consultation.nomSpectacle, "NON");
        }

        if (strcmp(choix, "reservation") == 0)
        {

            printf("Saisir le nom du spectacle à réserver ? ");
            scanf("%s", nomSpectacle);

            printf("Quel est le nombre de place à réserver ? ");
            scanf("%d", &place);

            strcpy(client_Reservation.nomSpectacle, nomSpectacle);
            client_Reservation.mtype = 18;
            client_Reservation.pid = getpid();
            client_Reservation.nbPlace = place;

            msgsnd(msqid, &client_Reservation, sizeof(struct clientReservation) - sizeof(long), 0); // 0 option
            //printf("%s\n","Je suis le client et je veux savoir si pour spectacle il y a assez de place disponible pour réserver");
            printf("Je suis le client et j'envoie comme nom du spectacle : %s et comme place %d \n", client_Reservation.nomSpectacle, client_Reservation.nbPlace);

            printf("%s\n", client_Reservation.nomSpectacle);
            printf("%d\n", client_Reservation.nbPlace);
            msgrcv(msqid, &client_MsgRecu, sizeof(struct clientMsgRecu) - sizeof(long), client_Reservation.pid, 0);
            printf("Je suis le client et le serveur m'a répondu : %s\n", client_MsgRecu.msgRetour);
        }
    }

    //ajouter destruction avec choix avec quitter

    // Destruction du MSQ
    //msgctl(msqid, IPC_RMID, NULL);
    //exit(0);
}
