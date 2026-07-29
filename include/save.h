#ifndef SAVE_H
#define SAVE_H

#include "game.h"

int load_game(Player *player, int *start_stage, int stage_count);
int save_game(const Player *player, int next_stage, int stage_count);

#endif
