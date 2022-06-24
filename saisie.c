/*
 *  author : rivera
 *  date : 03/2022
 *  file : saisie.c
 */

/* Includes elementaires */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Includes du projet */
#include "saisie.h"
#include "UI_minitel.h"
#include "lora.h"

/* Includes lecture UART */
#include "msg.h"
#include "periph/uart.h"
#include "stdio_uart.h"

/* Include timers */
#include "ztimer.h"



/* Saisie du nom */
int input_user (char *chaine)
{
    int len = 0;
    /* Chaine a afficher lors de la saisie du nom */
    char chaine_affichage[40];
    sprintf(chaine_affichage, "prenom (%d caracteres max)", MAX_USER-1); // Le -1 vient du '\0'
    
    do
    {
       len = input(chaine_affichage, chaine, MAX_USER);
       if(len==-1) return -1;
       printf("\r\n");
       printf("Votre nom est : %s. ", chaine);
       fflush(STDIO_UART_DEV);
    }
    while (change_input("nom") != 0);

    return len;
}


/* Saisie de l'etablissement */
int input_etablissement(char *chaine)
{
    int len = 0;
    /* Chaine a afficher lors de la saisie du nom */
    char chaine_affichage[40];
    sprintf(chaine_affichage, "etablissement (%d characters max)", MAX_ETABLISSEMENT-1); // Le -1 vient du '\0'

    do
    {
        len = input(chaine_affichage, chaine, MAX_ETABLISSEMENT);
        if(len==-1) return -1;
        printf("\r\n");
        printf("Votre etablissement est : %s. ", chaine);
        fflush(STDIO_UART_DEV);
    }
    while (change_input("etablissement") != 0);

    return len;
}



int input (const char *objet, char *chaine, uint8_t max_len)
{
    /* Local variable containing the message exchanged on UART interface between the STM32 and the Minitel */
    msg_t msg;

    /* Variable containing the firstname length (MAX 10) */   
    uint8_t len = 0;
    /* Variable containing the firstname */
    char buffer[max_len];
    /* Variable containing the input character */
    char buf = 0;

    printf("Saisir votre %s puis saisir ENTRER (* pour suppr) : \r\n", objet);
    fflush(STDIO_UART_DEV);
    ztimer_sleep(ZTIMER_SEC, 1); // Wait 1s
    
    len = 0; // Suit la logique des tableaux (0 -> n)
    while (buf != '\r') // Stop if character received is '\r' (=ENTER) or '\n' or length >= 10
    {
        msg_receive(&msg); // Waiting an incomming character
        buf = (char)msg.content.value;
        if (buf == '*')  // Give the option to delete the previous character
        {
            suppr(buffer, &len);
        }
        else if (len < (max_len-1) && ((buf != '\r' && buf >= '\'' && buf <= 'z') || buf == ' ')) // '\r' == key ENTER == end of the firstname
        {
            printf("%c", buf); // echo on the Minitel
            fflush(STDIO_UART_DEV);
            buffer[len] = buf; // add the character in the firstname string
            len++;
            
            if ((len+1)%NB_COLONNES == 0) // Mise en Forme Affichage de la Saisie
            {
                printf("\r\n");
            }
        }else if (buf == 0x1b)
        {
         return -1;
        }
    }
    buffer[len] = '\0';

    strcpy(chaine, (const char*) buffer);
    return len;
}



int change_input (char *objet)
{
    /* Local variable containing the message exchanged on UART interface between the STM32 and the Minitel */
    msg_t msg;
    /* Variable containing the input character */
    char buf = 0;

    printf("Saisie correct ? %s O/N : ", objet);
    fflush(STDIO_UART_DEV);
    while (buf != '\r')
    {
        msg_receive(&msg);
        buf = (char)msg.content.value;
        
        if (buf == 'n' || buf == 'o' || buf == 'N' || buf == 'O')
        {
            printf("%c", buf);
            fflush(STDIO_UART_DEV);
            break;
        }
    }
    printf("\r\n");

    if (buf == 'o' || buf == 'O')
    {
        printf("_____________________________________________________________________________\r\n");
        return 0;
    }

    /* on efface les lignes précédente */
    int i;
    for (i = 0; i < 3; i++)
    {
        minitel_cursor_up();
        minitel_clear_line();
    }
    printf("%c", 0x0D);

    return 1;
}


/* Gere la fonction suppr en fonction du positionnement du curseur. Appelle suppr_classique() ou suppr_saut_ligne() */
int suppr (char *ch, uint8_t *len)
{
    if (ch == NULL && len == NULL) /* Suppr affichage uniquement, les caracteres ne sont pas stockes */
    {
        suppr_classique (ch, len);
    }
    else if ((*len+1)%NB_COLONNES == 0) /* Suppr special : caractere sur ligne superieur */
    {
        suppr_saut_ligne (ch, len);
    }
    else /* Suppr classique */
    {
        suppr_classique (ch, len);
    }
    return 1;
}



int suppr_saut_ligne (char *ch, uint8_t *len)
{
    /* Remonte le curseur d'une ligne */
    minitel_cursor_up();
    
    /* Suppr affichage */
    /* Place le curseur a l'avant dernier caractere de la ligne */
    int colonne;
    for (colonne = 0; colonne < NB_COLONNES-1; colonne++)
    {
        minitel_cursor_right();
    }
    /* Efface le caractere de l'affichage */
    printf(" "); // Un espace pour ecraser le caractere a suppr
    fflush(STDIO_UART_DEV); // Vide le buffer uart

    /* Suppr tableau */
    /* Efface caractere du tableau */
    ch[*len] = '\0';
    (*len)--;

    return 1;
}



int suppr_classique (char *ch, uint8_t *len)
{
    /* Suppr affichage */
    printf("%c %c", 0x08, 0x08); // Code ASCII curseur gauche = 0x08 puis un espace pour ecraser le precedent caractere
    fflush(STDIO_UART_DEV);
    

    /* Suppr tableau */
    /* Efface caractere du tableau */
    if ((*len) > 0)
    {
        ch[*len] = '\0';
        (*len)--;
    }
    return 1;
}
