/*
 *  author : rivera
 *  date : 03/2022
 *  file : lora.c
 */

/* Includes elementaires */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

/* Includes du projet */
#include "saisie.h"
#include "UI_minitel.h"
#include "lora.h"

/* Include Lora */
#include "net/loramac.h"
#include "semtech_loramac.h"
#include "sx127x.h"
#include "sx127x_netdev.h"
#include "sx127x_params.h"

/* Include lecture UART */
#include "msg.h"
#include "thread.h"
#include "fmt.h"
#include "periph/uart.h"
#include "stdio_uart.h"

/* Include timers */
#include "ztimer.h"


/* Prototypes */
static uint8_t xor(uint8_t * payload, uint16_t length, uint8_t first);


/* Variable globale */
uint8_t payload1[PAYLOAD_MAX_LEN+1];
uint8_t payload2[PAYLOAD_MAX_LEN+1];
uint8_t payload3[PAYLOAD_MAX_LEN+1];
uint8_t nb_payload;


/* Parametre du Module Lora */
static uint8_t deveui[LORAMAC_DEVEUI_LEN];
static uint8_t appeui[LORAMAC_APPEUI_LEN];
static uint8_t appkey[LORAMAC_APPKEY_LEN];

/* Module lora utilise */
static semtech_loramac_t loramac;
static sx127x_t sx127x;


/* Initialise le module lora sx127x, les parametres Lora et defini les parametres : devEUI, appEUI, appKey */
uint8_t init_lora(void)
{
    /* Initialize the loramac stack */
    sx127x_setup(&sx127x, &sx127x_params[0], 0);
    loramac.netdev = &sx127x.netdev;
    loramac.netdev->driver = &sx127x_driver;
    semtech_loramac_init(&loramac);

    /* Convert identifiers and application key */
    fmt_hex_bytes(deveui, CONFIG_LORAMAC_DEV_EUI_DEFAULT);
    fmt_hex_bytes(appeui, CONFIG_LORAMAC_APP_EUI_DEFAULT);
    fmt_hex_bytes(appkey, CONFIG_LORAMAC_APP_KEY_DEFAULT);
    
    semtech_loramac_set_deveui(&loramac, deveui);
    semtech_loramac_set_appeui(&loramac, appeui);
    semtech_loramac_set_appkey(&loramac, appkey);

    /* Use a fast datarate, e.g. BW125/SF7 in EU868 */
    /* Defini la modulation, facteur d'etalement et la bande passante 
     * LORAMAC_DR_5 : Modulation : ISM EU863-870, Facteur d'etalement : SF7, Bande passante : BW125
     */
    semtech_loramac_set_dr(&loramac, LORAMAC_DR_5);

    return 0;
}



/* Cree les trames en appelant la fonction frames()
 * puis envoi des trames avec send_lora(). Il y a MAX_TRY essai pour emettre, sinon abandon de l'envoi */
int8_t send_message (char *message, int lmessage, char *user, int luser)
{
    /* mise en forme des trames */
    frames (message, lmessage, user, luser); // separation du message en 1, 2 ou 3 trames
    printf("\r\n\n"); // affichage
    int i = 0;
    
    do
    {
        printf("Essai n%d de l'emission de la trame\r\n", i);
        ztimer_sleep(ZTIMER_SEC, 3);
        i++;
    }
    while (send_lora() != 0 && i < MAX_TRY);  /* emission des trames avec MAX_TRY essai */
    if (i >= MAX_TRY) return -1; /* L'envoi est un echec */
    
    /* L'envoi est un succes */
    return 0;
}


/* Cree les frames */
/* Subcommande : 1 (debut), 2 (milieu) ou 3 (fin)
 * Commande : 1 (livre d'or) ou 2 (horaire)
 * Position (existe que pour le payload2) : 1 à n
 */
void frames (char *message, int lmessage, char *user, int luser)
{
    // Nombre de trames a envoyer
                            /* longueur utilisateur+etablissement + 2 caracteres fin de chaine 
                             * + longueur message + entete payload1 + entete payload2 
                             * + entete payload3 */
    nb_payload = (int)ceilf( (luser+2.0+lmessage+3.0+2.0+1.0) / 50.0 );

    // Octet0   Subcommande = bit0:3       Commande = bit4:7
    payload1[0]=1 | 1<<4;
    // Octet1   XOR (type Checksum) du message global
    payload1[1]=xor((uint8_t*)user,strlen(user),(lmessage+luser+2)); // payload1[1]=xor((uint8_t*)user+1,strlen(user)-1,user[0]);
    payload1[1]=xor((uint8_t*)message,strlen(message),payload1[1]);
    
    // Octet1   XOR (type Checksum) du message global
    payload1[2]=lmessage+luser+2; // Longueur du message + longeur nom utilisateur + 2 caracteres fin de chaine
    
    // Suite octets : Username et structure
    strncpy((char *)payload1+3, user, luser+1);

    // Calcul de l'espace restant dans le payload1
                                    /* taille payload1 - taille entete 
                                                            * - caractere fin ligne */
    int space = PAYLOAD_MAX_LEN - strlen((char *)payload1+3) - 3 - 1;
    // Completion du payload1 par "space" caracteres max du message
    strncpy((char *)payload1+3+luser+1, message, space);
    // Caractere fin de chaine
    payload1[50] = '\0';


    if (nb_payload >= 1)
    {
        int len_entete = 0; /* longueur entete payload2 qui est variable */
        
        if (nb_payload > 2)
        {
            // Nombre d'octets en entete
            len_entete = 2;
            // Octet0   Subcommande = bit0:3       Commande = bit4:7
            payload2[0]= 2 | 1<<4; //  2 : Milieu
            // Octet1 Position
            payload2[1]= 2;
        }
        else
        {
            // Nombre d'octets en entete
            len_entete = 1;
            // Octet0   Subcommande = bit0:3       Commande = bit4:7
            payload2[0]= 3 | 1<<4; // 3 : Fin
        }

        // Completion du payload par 50-len_entete-caractere_fin_chaine caracteres max du message
        strncpy((char *)payload2+len_entete, message+space, 50-len_entete-1);
        // Caractere fin de chaine
        payload2[50] = '\0';


        if (nb_payload >= 2)
        {  
            // Octet0   Subcommande = bit0:3       Commande = bit4:7
            payload3[0]= 3 | 1<<4;
            // Completion du payload 49 caracteres max du message
            strncpy((char *)payload3+1, message+space+50-len_entete-1, 49);
            // Caractere fin de chaine
            payload3[50] = '\0';
        }
        else
        {
            payload3[0] = '\0';
        }
    }
    else
    {
        payload2[0] = '\0';
        payload3[0] = '\0';
    }
}



/* Calcul le XOR du message */
static uint8_t xor(uint8_t * payload, uint16_t length, uint8_t first)
{
  uint8_t ret = first;
  for(uint16_t i=0;i<length;i++)
  {
    ret = ret^payload[i];
  }
  return ret;
}



/* Envoi les nb_payload trames Lora avec la fonction semtech_loramac_send(). 
 * Les trames sont contenues dans les payload1, payload2 et payload3 */
int8_t send_lora (void)
{
    /* Trois bip du minitel au debut de la transmission */
    bipbipbip (200, 400);

    printf("=========================================================\r\n");
    printf("Debut de la procedure d'envoi\r\n");
    printf("=========================================================\r\n");
    
    loramac.cnf = LORAMAC_TX_UNCNF;
    
    /* On essaye d'envoyer la premiere trame */
    if (nb_payload >= 1)
    {
        // longueur premier payload
        int len_payload1 = 0;
        len_payload1 += strlen((char *)payload1);
        len_payload1 += strlen((char *)(payload1 + len_payload1 + 1)) + 1; /* +1 pour le \0 qui separe le nom+etablissement du message */

        /* Envoi de la premiere trame */
        uint8_t ret = semtech_loramac_send(&loramac, (uint8_t *)payload1, len_payload1);
        if (ret != SEMTECH_LORAMAC_TX_DONE)
        {
            /* Gestion des erreurs */
            if (ret == SEMTECH_LORAMAC_DUTYCYCLE_RESTRICTED)
            {
                printf("CODE ERREUR : %d, Non respect du temps d'emission\r\n", ret);
            }
            else
            {
                printf("CODE ERREUR : %d, Echec d'envoi du message\r\n", ret);    
            }
            return -1;
        }
        printf("Partie 1 envoyee !\r\n");
        printf("=========================================================\r\n");

        ztimer_sleep(ZTIMER_SEC, 4); // Tempo

        if (nb_payload >= 2)
        {
            /* On essaye d'envoyer la seconde trame */
            ret = semtech_loramac_send(&loramac, (uint8_t *)payload2, strlen((char *)payload2)+1);
            if (ret != SEMTECH_LORAMAC_TX_DONE)
            {
                /* Gestion des erreurs */
                if (ret == SEMTECH_LORAMAC_DUTYCYCLE_RESTRICTED)
                {
                    printf("CODE ERREUR : %d, Non respect du temps d'emission\r\n", ret);
                }
                else
                {
                    printf("CODE ERREUR : %d, Echec d'envoi du message\r\n", ret);    
                }
                return -1;
            }
            printf("Partie 2 envoyee !\r\n");
            printf("=========================================================\r\n");

            ztimer_sleep(ZTIMER_SEC, 4); // Tempo     
        

            /* On essaye d'envoyer la troisieme trame */
            if (nb_payload >= 3)
            {
                ret = semtech_loramac_send(&loramac, (uint8_t *)payload3, strlen((char *)payload3)+1);
                if (ret != SEMTECH_LORAMAC_TX_DONE)
                {
                    /* Gestion des erreurs */
                    if (ret == SEMTECH_LORAMAC_DUTYCYCLE_RESTRICTED)
                    {
                        printf("CODE ERREUR : %d, Non respect du temps d'emission\r\n", ret);
                    }
                    else
                    {
                        printf("CODE ERREUR : %d, Echec d'envoi du message\r\n", ret);    
                    }
                    return -1;
                }
                printf("Partie 3 envoyee !\r\n");
                printf("=========================================================\r\n");
            }
        }
    }

    printf("La procedure d'envoi est un succes\r\n");
    printf("=========================================================\r\n");
    return 0;
}


/* Son sur le minitel - 3 bips */
void bipbipbip (uint16_t tempo_ms1, uint16_t tempo_ms2)
{
    const uint8_t bip = 0x7;
    uart_write(STDIO_UART_DEV, &bip, 1); // bip
    ztimer_sleep(ZTIMER_MSEC, tempo_ms1); // delai 1
    uart_write(STDIO_UART_DEV, &bip, 1);
    ztimer_sleep(ZTIMER_MSEC, tempo_ms2); // delai 2
    uart_write(STDIO_UART_DEV, &bip, 1);
}


/* Realisation de la procedure de join 1 fois, au lancement du minitel. 
 * Le join etablie la connexion entre la passerelle et carte idosens pour un devEUI.
 */
uint8_t join_procedure (void)
{
    /* Start the Over-The-Air Activation (OTAA) procedure to retrieve the
     * generated device address and to get the network and application session
     * keys.
     */
    int i = 0;
    uint8_t ret;
    do
    {
        i++;   
        ret = semtech_loramac_join(&loramac, LORAMAC_JOIN_OTAA);     
    }
    while (ret != SEMTECH_LORAMAC_JOIN_SUCCEEDED && i < MAX_TRY);
    if (i >= MAX_TRY) /* La procdure de join est un echec */
    {
        /* Gestion des erreurs */
        if (ret == SEMTECH_LORAMAC_JOIN_FAILED)
        {
            printf("\r\nCODE ERROR %d : Echec dans la procedure !\r\n", ret);
        }
        else if (ret == SEMTECH_LORAMAC_ALREADY_JOINED)
        {
            printf("\r\nCODE ERROR %d : Procedure join deja effectuee !\r\n", ret);
        }
        else if (ret == SEMTECH_LORAMAC_BUSY)
        {
            printf("\r\nCODE ERROR %d : Module Lora en cours d'utilisation !\r\n", ret);
        }
        else
        {
            printf("\r\nCODE ERROR %d !\r\n", ret);
        }
        printf("\r\nLa procedure join a echoue. Arret du programme !\r\n");
        exit(1); // Arret du programme
    }
    return 0;
}