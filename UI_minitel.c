#include "periph/uart.h"
#include "stdio_uart.h"
#include "UI_minitel.h"




/* Affichage initial du Minitel */
void init_affichage(void)
{
    /*clear page + cursor home*/
    minitel_clear_page();
    minitel_cursor_home(); // positionne le curseur en haut a gauche 

    print_header_fabMSTIC();
}



void print_header_fabMSTIC(void){
    printf("  __________________________________________________\r\n");
    printf("\r\n");
    printf("|   %c%c%c        %c      %c   %c  %c%c%c%c%c %c%c%c%c%c %c%c%c%c%c %c%c%c%c%c  |\r\n", 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18);
    printf("|  %c           %c      %c%c %c%c  %c       %c     %c   %c      |\r\n", 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18);
    printf("|  %c%c%c   %c%c%c   %c%c%c%c   %c %c %c  %c%c%c%c%c   %c     %c   %c      |\r\n", 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18);
    printf("|  %c    %c  %c   %c  %c   %c   %c      %c   %c     %c   %c      |\r\n", 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18);
    printf("|  %c    %c%c%c%c%c  %c%c%c%c   %c   %c  %c%c%c%c%c   %c   %c%c%c%c%c %c%c%c%c%c  |\r\n", 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18);
    printf("  __________________________________________________\r\n");

}


void print_header_livredor(void){
 printf("  _    _                  _ _\r\n");     
 printf(" | |  (_)_ ___ _ ___   __| ( )___ _ _\r\n"); 
 printf(" | |__| \\ V / '_/ -_) / _` |// _ \\ '_|\r\n");
 printf(" |____|_|\\_/|_| \\___| \\__,_| \\___/_| \r\n");
 printf("________________________________________\r\n");
}


/*
* brief : fonction qui récupère les inputs clavier du minitel et qui met a jour le tableau d'état de menu
*Tab_menu[0] :  0 ou 1 en fonction de la touche (* et #)
*tab_menu[1] : 1 si touche entré (validation)
*/
void nav_menu(int *tab_menu){
    /* Local variable containing the message exchanged on UART interface between the STM32 and the Minitel */
    msg_t msg;
    /* Variable containing the input character */
    char buf = 0;

    msg_receive(&msg); // Waiting an incomming character
    buf = (char)msg.content.value;
    if(buf == 0x2A){ /*touche  * */
        //printf("gauche \r\n");
        tab_menu[0] = 0;
    }
    else if(buf == 0x23){ /*touche #*/
        //printf("droite \r\n");
        tab_menu[0] = 1;
    }
    else if(buf == 0x0D){
        tab_menu[1] = 1;
    }
    fflush(STDIO_UART_DEV);
}



/*
* brief : fonction qui place visuellement le curseur sur le choix du menu
* valeur de retour : 1 si index est négatif (validation)
                     0 si index est positif
*/
void set_cursor_menu(int index){
    if(index == 0){ 
        printf("%c", 0x0D);
        printf("     ~~~~~~~~~~                      ");
        fflush(STDIO_UART_DEV); 
    }
    else if(index == 1){
        printf("%c", 0x0D);
        printf("                          ~~~~~~~~~~~");
        fflush(STDIO_UART_DEV);
    }
}


/* verifie si l'utilisateur a valide un menu */
int valid_menu(void){
    /* Local variable containing the message exchanged on UART interface between the STM32 and the Minitel */
    msg_t msg;
    /* Variable containing the input character */
    char buf = 0;
    int r = 0;

    msg_receive(&msg); // Waiting an incomming character
    buf = (char)msg.content.value;

    if(buf == 0x0D){ /*touche entrée*/
        r = 1;
    }
    else{/*si rien*/
        r = 0;
    }
    return r;
}



/*
* brief : fonction qui retourne 1 si la touche fleche gauche est enfoncé
* retourne 2 si la touche fleche de droite est enfoncé
* retourne 0 sinon
*/
int test_fleche(void){
     /* Local variable containing the message exchanged on UART interface between the STM32 and the Minitel */
    msg_t msg1, msg2 ,msg3;
    /* Variable containing the input character */
    char buf1 = 0, buf2 = 0, buf3 = 0;
    int r = 0;

    fflush(STDIO_UART_DEV);

    msg_receive(&msg1); // Waiting an incomming character
    msg_receive(&msg2);
    msg_receive(&msg3);
    buf1 = (char)msg1.content.value;
    buf2 = (char)msg2.content.value;
    buf3 = (char)msg3.content.value;

    //printf("buf1 : %x | buf2 : %x |buf3 : %x )\r\n", buf1, buf2, buf3);

    if((buf1 == 0x44) && (buf2 == 0x1b) && (buf3 == 0x5b)){
        //printf("gauche \r\n");
        r = 1;
    }
    else if((buf1 == 0x43) && (buf2 == 0x1b) && (buf3 == 0x5b)){
        //printf("droite \r\n");
        r = 2;
    }
    
    return r;
}


/* Transmet sur l'UART len caractere du buffer */
ssize_t minitel_raw_uart_write(const void* buffer, size_t len)
{
    const uint8_t *buf = (const uint8_t *)buffer;
    /*const uint8_t cr = '\r';*/
    size_t rem = len;
    while (rem--) {
            /*if (*buf == '\n'){ uart_write(STDIO_UART_DEV, &cr, 1);}*/
            uart_write(STDIO_UART_DEV, buf, 1);
            buf++;
    }
    return len;
}



/* efface toute la page du minitel */
void minitel_clear_page(void){
	char del_page[4] = {0x1B, 0X5B, 0x32, 0x4A};
	minitel_raw_uart_write(del_page, 4);
}

/* positionne le curseur en haut a gauche en [0, 0] */
void minitel_cursor_home(void){
	char cursor_home[3] = {0x1B, 0x5B, 0x48};
	minitel_raw_uart_write(cursor_home, 3);
}

/* fait remonter le curseur d'une ligne */
void minitel_cursor_up(void){
    char cursor_up[3] = {0x1B, 0x5B, 0x41};
    minitel_raw_uart_write(cursor_up, 3);
}

/* fait aller à droite le curseur d'une case */
void minitel_cursor_right(void){
    char cursor_right[3] = {0x1B, 0x5B, 0x43};
    minitel_raw_uart_write(cursor_right, 3);
}

/* efface la ligne sur lequel est positionne le curseur */
void minitel_clear_line(void){
    char clear_line[3] = {0x1B, 0x5B, 0x4D};
    minitel_raw_uart_write(clear_line, 3);
}

/* effectue un bip du minitel */
void minitel_bip(void){
    char bip[3] = {0x07};
    minitel_raw_uart_write(bip, 1);
}