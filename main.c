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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "msg.h"
#include "thread.h"
#include "fmt.h"

#if IS_USED(MODULE_PERIPH_RTC)
#include "periph/rtc.h"
#else
#include "timex.h"
#include "ztimer.h"
#endif
#include "xtimer.h"

#include "net/loramac.h"
#include "semtech_loramac.h"
#include "sx127x.h"
#include "sx127x_netdev.h"
#include "sx127x_params.h"

/* Messages are sent every 20s to respect the duty cycle on each channel */
#define PERIOD_S            (20U)

#define SENDER_PRIO         (THREAD_PRIORITY_MAIN - 1)
static kernel_pid_t sender_pid;
static char sender_stack[THREAD_STACKSIZE_MAIN / 2];

static semtech_loramac_t loramac;
static sx127x_t sx127x;

#if !IS_USED(MODULE_PERIPH_RTC)
static ztimer_t timer;
#endif
char *fullmessage = "Decoupe a la laser d'un boitier de carte electronique pour un projet de recherche autour des objects connectes";
static const char *message = "Decoupe a la laser d'un boitier";//18
static const char *message2 = " de carte electronique pour un projet de recherc";//18
static const char *message3 = "he autour des objects connectes";//32
static const char *user = "Germain L.(LIG)";//16
#define PAYLOAD_MAX_LEN 50
uint8_t payload[PAYLOAD_MAX_LEN];
uint8_t payload2[PAYLOAD_MAX_LEN];
uint8_t payload3[PAYLOAD_MAX_LEN];


static uint8_t deveui[LORAMAC_DEVEUI_LEN];
static uint8_t appeui[LORAMAC_APPEUI_LEN];
static uint8_t appkey[LORAMAC_APPKEY_LEN];

static uint8_t xor(uint8_t * payload, uint16_t length, uint8_t first)
{
  uint8_t ret = first;
  for(uint16_t i=0;i<length;i++)
  {
    ret = ret^payload[i];
  }
  return ret;
}

static void _alarm_cb(void *arg)
{
    (void) arg;
    msg_t msg;
    msg_send(&msg, sender_pid);
}

static void _prepare_next_alarm(void)
{
#if IS_USED(MODULE_PERIPH_RTC)
    struct tm time;
    rtc_get_time(&time);
    /* set initial alarm */
    time.tm_sec += PERIOD_S;
    mktime(&time);
    rtc_set_alarm(&time, _alarm_cb, NULL);
#else
    timer.callback = _alarm_cb;
    ztimer_set(ZTIMER_MSEC, &timer, PERIOD_S * MS_PER_SEC);
#endif
}

static void _send_message(void)
{
    printf("Sending: %s\n", message);
    loramac.cnf = LORAMAC_TX_UNCNF;
    /* Try to send the message */
        uint8_t ret = semtech_loramac_send(&loramac,
                                       (uint8_t *)payload, 50);
    if (ret != SEMTECH_LORAMAC_TX_DONE)  {
        printf("Cannot send message, ret code: %d\n",ret);
        return;
    }
    xtimer_sleep(7);
    /* Try to send the message */
    ret = semtech_loramac_send(&loramac,
                                       (uint8_t *)payload2, 50);
    if (ret != SEMTECH_LORAMAC_TX_DONE)  {
        printf("Cannot send message, ret code: %d\n",ret);
        return;
    }
    printf("Part 2 sent\n");
    xtimer_sleep(7);
    ret = semtech_loramac_send(&loramac,
                                       (uint8_t *)payload3, 33);
    if (ret != SEMTECH_LORAMAC_TX_DONE)  {
        printf("Cannot send message, ret code: %d\n",ret);
        return;
    }
    printf("Part 3 sent\n");


}

static void *sender(void *arg)
{
    (void)arg;

    msg_t msg;
    msg_t msg_queue[8];
    msg_init_queue(msg_queue, 8);

    while (1) {
        msg_receive(&msg);

        /* Trigger the message send */
        _send_message();

        /* Schedule the next wake-up alarm */
        _prepare_next_alarm();
    }

    /* this should never be reached */
    return NULL;
}

int main(void)
{
    puts("LoRaWAN Class A low-power application");
    puts("=====================================");

    xtimer_sleep(1);
    payload[0]=1 | 1<<4;
    payload[2]=127;
    strcpy((char *)payload+3,user);
    strcpy((char *)payload+3+strlen(user)+1,message);
    
    payload[1]=xor((uint8_t*)user+1,strlen(user)-1,user[0]);
    payload[1]=xor((uint8_t*)fullmessage,strlen(fullmessage),payload[1]);
    
    payload2[0]= 2 | 1<<4;
    payload2[1]=2;
    strcpy((char *)payload2+2,message2);
    
    payload3[0]= 3 | 1<<4;
    strcpy((char *)payload3+1,message3);
/*
    printf("Payload : ");
    for(uint8_t i=0; i< 50;i++)
    {
      printf("0x%x ", payload[i]);
    }
    printf("\r\n");

    printf("Payload2 : ");
    for(uint8_t i=0; i< 50;i++)
    {
      printf("0x%x ", payload2[i]);
    }
    printf("\r\n");
    printf("Payload3 : ");
    for(uint8_t i=0; i< 33;i++)
    {
      printf("0x%x ", payload3[i]);
    }
    printf("\r\n");
  */  
    /* Convert identifiers and application key */
    fmt_hex_bytes(deveui, CONFIG_LORAMAC_DEV_EUI_DEFAULT);
    fmt_hex_bytes(appeui, CONFIG_LORAMAC_APP_EUI_DEFAULT);
    fmt_hex_bytes(appkey, CONFIG_LORAMAC_APP_KEY_DEFAULT);

    /* Initialize the radio driver */

    sx127x_setup(&sx127x, &sx127x_params[0], 0);
    loramac.netdev = &sx127x.netdev;
    loramac.netdev->driver = &sx127x_driver;

    /* Initialize the loramac stack */
    semtech_loramac_init(&loramac);
    semtech_loramac_set_deveui(&loramac, deveui);
    semtech_loramac_set_appeui(&loramac, appeui);
    semtech_loramac_set_appkey(&loramac, appkey);

    /* Use a fast datarate, e.g. BW125/SF7 in EU868 */
    semtech_loramac_set_dr(&loramac, LORAMAC_DR_5);

    /* Start the Over-The-Air Activation (OTAA) procedure to retrieve the
     * generated device address and to get the network and application session
     * keys.
     */
    puts("Starting join procedure");
    if (semtech_loramac_join(&loramac, LORAMAC_JOIN_OTAA) != SEMTECH_LORAMAC_JOIN_SUCCEEDED) {
        puts("Join procedure failed");
        return 1;
    }
    puts("Join procedure succeeded");

    /* start the sender thread */
    sender_pid = thread_create(sender_stack, sizeof(sender_stack),
                               SENDER_PRIO, 0, sender, NULL, "sender");

    /* trigger the first send */
    msg_t msg;
    msg_send(&msg, sender_pid);
    return 0;
}
