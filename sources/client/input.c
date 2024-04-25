#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "requests.h"

uint8_t input_game_type() {
    uint8_t gameType;
    printf("Quel est le type de la partie que vous voulez rejoindre (1 ou 2): \n");
    printf("\t 1. Mode 4 adversaires.\n");
    printf("\t 2. Mode équipes.\n");
    scanf("%hhd", &gameType);
    if(gameType == 1) return CREQ_MODE4;
    return CREQ_TEAMS;
}