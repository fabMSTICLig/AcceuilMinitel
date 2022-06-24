/*
 *  author : rivera
 *  date : 03/2022
 *  file : saisie.h
 */

#ifndef SAISIE_H_
#define SAISIE_H_

/* User input max length */
#define MAX_USER 15
#define MAX_ETABLISSEMENT 8
#define MAX_MESSAGE 127


/* Prototypes */
int input_user (char *prenom);
int input_etablissement(char *chaine);
int input (const char *objet, char *chaine, uint8_t max_len);
int change_input (char *objet);

/* Function suppr */
int suppr (char *ch, uint8_t *len);
int suppr_saut_ligne (char *ch, uint8_t *len);
int suppr_classique (char *ch, uint8_t *len);


#endif
