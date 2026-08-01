#ifndef BATTLE_H
#define BATTLE_H

#include "game.h"

void print_battle_start(const Enemy *enemy);
void print_battle_status(const Player *player, const Enemy *enemy);
int player_turn(Player *player, Enemy *enemy, const Stage *stage, const char target[], int current_stage, int stage_count, int is_climax);

#endif
