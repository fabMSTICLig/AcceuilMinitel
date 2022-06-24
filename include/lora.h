/*
 *  author : rivera
 *  date : 03/2022
 *  file : lora.h
 */

#ifndef LORA_H_
#define LORA_H_

/* Variables */
#define PERIOD_S            (20U)

/* Lora variables to store frames */
#define MAX_TRY 5
#define PAYLOAD_MAX_LEN 50



/* Prototypes */
uint8_t init_lora(void);

uint8_t join_procedure (void);

int8_t send_message (char *message, int lmessage, char *user, int luser);
void frames (char *message, int lmessage, char *user, int luser);
int8_t send_lora (void);

void bipbipbip (uint16_t tempo_ms1, uint16_t tempo_ms2);


#endif