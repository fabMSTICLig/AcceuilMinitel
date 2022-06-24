# Projet d'acceuil à base de minitel

# Projet Livre d'or

Projet initialement réalisé par BARBET Léo, LOMBARD Axel, RIVERA Antoine, dans le cadre d'un projet d'étude.

Ce projet permet aux utilisateurs du fabMSTIC de laisser un commentaire sur ce qu'ils sont venus faire au fablab. Le message est saisi depuis un minitel puis envoyer en LORA, il est ensuite récuperé sur le serveur web du fablab qui met à jour une page actualités.

## Spécification techniques

Le projet utilise le framework RIOT https://github.com/RIOT-OS/RIOT.


La carte reliée au minitel est la télécomande du kit Idosens https://github.com/CampusIoT/idosens


Attention il faut convertir le niveau de tension entre le minitel et la carte STM32.
