/*
 * Copyright (C) 2022 FabMSTIC
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 *
 * @file
 * @brief       Example demonstrating the use of LoRaWAN with RIOT
 *
 */

/* Includes elementaires */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

/* Include lecture UART */
#include "msg.h"
#include "thread.h"
#include "fmt.h"
#include "periph/uart.h"
#include "stdio_uart.h"

/* Include timers */
#include "periph/rtc.h"
#include "timex.h"
#include "ztimer.h"

/* Includes du projet */
#include "saisie.h"
#include "UI_minitel.h"
#include "lora.h"



/* Define UART config */
#define UART  UART_DEV(0)
#define BAUDRATE (4800U)


/* Gestion des caracteres en entree */
static kernel_pid_t main_thread_pid;
static void rx_cb(void *uart, uint8_t c);


/* Prototypes */
void choix_menu_minitel(int *choix_menu);
uint8_t livre_d_or (char *prenom, char *etablissement, char *message);



int main(void)
{
    /* Init UART : 7 bits, parité pair, stop bit 1 */
    /* UART0 : PC terminal */
    uart_init(UART, BAUDRATE, rx_cb, (void *)UART);
    uart_mode (UART, UART_DATA_BITS_7, UART_PARITY_EVEN, UART_STOP_BITS_1);
    /* ================================================================ */

    /* thread pour la reception uart */
    main_thread_pid = thread_getpid();
    /* ================================================================ */
  
    /* Menu[{livre_dor ; horaire} , {validé ; non validé}] */
    int choix_menu[2] = {0, 0}; //tableau contenant l'état du menu item[0] choix et curseur et item[1] booléen validé ou non
    /* ================================================================ */

    /* Init Saisie */
    char prenom[MAX_USER]; // Variable to store the firstname
    char etablissement[MAX_ETABLISSEMENT]; // Variable to store the structure
    char message[MAX_MESSAGE]; // Variable to store the message
    /* ================================================================ */

    /* Effacement de la page sur le minitel */
    minitel_cursor_home();
    minitel_clear_page();

    /* Init Lora */
    init_lora(); // initialisation du module lora
    ztimer_sleep(ZTIMER_SEC, 1); // tempo 1s
    join_procedure(); // procedure de join
    /* ================================================================ */
    


    /* ======================== Boucle infinie ======================== */
    while(1)
    {
        /* affichage accueil */
        init_affichage();
        /* permet a l'utilisateur de choisir entre livre d'or et horaire */
        choix_menu_minitel(choix_menu);

        /* ========== Livre d'or ========== */
        if(choix_menu[0] == 0)
        {   
            livre_d_or (prenom, etablissement, message); // lancement de l'application livre d'or
        }
        /* ========== Set Horaire ========== */
        else if (choix_menu[0] == 1)
        {
            // Ajouter l'application une fois realisee
            printf("SET HORAIRE\r\n");
        }

        
        /* Reset du menu */
        choix_menu[0] = 0;
        choix_menu[1] = 0;
    }

    return 0;
}



/* thread pour la lecture d'un caractere sur la liaison uart */
static void rx_cb(void *uart, uint8_t c)
{
    /* A character was received on an UART interface and triggered
       this callback through an interruption, we forward it via a message
       to the main thread. */
    msg_t msg;
    msg.type = (int)uart;
    msg.content.value = (uint32_t)c;
    msg_send(&msg, main_thread_pid);
}




/* Permet de faire le choix entre le livre d'or et la partie horaire
 * Le curseur est deplace avec les touches * et # du clavier. Cette fonction
 * permet juste a l'utilisateur de faire son choix. Le lancement de l'application
 * n'est pas fait ici.
 */
void choix_menu_minitel(int *choix_menu)
{
    printf("\r\n");
    printf("Bonjour ! Je suis Michel le minitel, que voulez vous faire ?\r\n");
    printf("(choix avec les touches [*] et [#] puis valider avec ENTREE)\r\n");
    printf("_____________________________________________________________________________\r\n");
    printf("\r\n");
    //printf("     LIVRE D'OR     |     SET HORAIRE\r\n");
    printf("     LIVRE D'OR     \r\n");
    set_cursor_menu(0);

    minitel_bip();

    fflush(STDIO_UART_DEV); //on clear le buffer pour les detection touches menu
   

    /*boucle de menu*/
    while(1){
        
        /* Navigation avec les touches * et # */
        nav_menu(choix_menu);
        /* Affichage correspondant a la navigation utilisateur */
        //set_cursor_menu(choix_menu[0]);
        //uniquement livre d'or pour l'instant
        set_cursor_menu(0);
        printf("%c", 0x0D); // Curseur en debut de ligne
        fflush(STDIO_UART_DEV);

        if(choix_menu[1] == 1)
        {
            /*on clear la page et on sort de la boucle*/
            minitel_clear_page();
            minitel_cursor_home();
            break;
        }

    }
}



/* Application livre d'or
 *      - Fait la saisie par l'utilisateur du nom, etablissement, message avec
 *                  # input_user(), input_etablissement() et input()
 *      - Fait la separation des trames et l'emission lora avec 
 *                  # send_message() qui appelle trames() et send_lora()
 */
uint8_t livre_d_or (char *prenom, char *etablissement, char *message)
{
    int len_prenom_etablissment = 0;
    int len_message = 0;
    char prenom_etablissement[MAX_USER+MAX_ETABLISSEMENT+1]; // Variable to store the firstname

    /* Chaine a afficher lors de la saisie du message */
    char chaine_affichage[40];
    sprintf(chaine_affichage, "MESSAGE (%d caracteres max)", MAX_MESSAGE-1); // Le -1 vient du '\0'

    /* Affichage titre livre d'or*/
    print_header_livredor();
    printf("\r\n");

    ztimer_sleep(ZTIMER_SEC, 1); // tempo 1s
    
    int ret = 0;
    /* saisie du nom */
    ret = input_user(prenom);
    if(ret==-1) return -1;
    /* saisie de l'etablissement */
    ret = input_etablissement(etablissement);
    if(ret==-1) return -1;
    /* saisie du message */
    len_message = input(chaine_affichage, message, MAX_MESSAGE);
    if(len_message==-1) return -1;


    /* Creation de la chaine prenom + etablissement a partir de la saisie */
    strcpy(prenom_etablissement, prenom);
    strcat(prenom_etablissement, "(");
    strcat(prenom_etablissement, etablissement);
    strcat(prenom_etablissement, ")");
    /* calcul de la longueur de la chaine */
    len_prenom_etablissment = strlen(prenom_etablissement);


    /* Mise en forme des trames et emission lora */
    if (send_message (message, len_message, prenom_etablissement, len_prenom_etablissment) != 0) // separation du message en 1, 2 ou 3 trames
    {
        printf("Erreur lors de l'emission de la trame. Trop d'essais !\r\n");
        /* Attente entre deux utilisateur */
        ztimer_sleep(ZTIMER_SEC, 10); // tempo 10s
        return -2;
    }
    else
    {
        printf("\r\nMerci pour votre message ! A bientot !\r\n");    
        /* Attente entre deux utilisateur */
        ztimer_sleep(ZTIMER_SEC, 10); // tempo 10s
        return 0;
    }
}
