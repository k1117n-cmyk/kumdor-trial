#ifndef GAME_H
#define GAME_H

#define STATUS_NORMAL 0x00
#define STATUS_POISON 0x01
#define STATUS_BLIND  0x02

#define INPUT_BUFFER_SIZE 64
#define EXP_TO_LEVEL_UP 10
#define GAME_VERSION "v0.2.0"
#define SAVE_FILE "kumdor_save.txt"
#define SAVE_COMMAND ":save"
#define QUIT_COMMAND ":quit"
#define SAVE_QUIT_COMMAND ":savequit"

typedef struct {
    const char *name;
    int hp;
    int max_hp;
    unsigned char status;
    int correct_count;
    int miss_count;
    int input_error_count;
    int reached_stage;
    int level;
    int exp;
} Player;

typedef struct {
    const char *name;
    int hp;
    int max_hp;
    int attack;
} Enemy;

typedef struct {
    Enemy enemy;
    const char *place;
    const char *story;
    const char *lesson;
    const char *tip;
    const char *const *words;
    int word_count;
    const char *reward_name;
    int reward_heal;
    int exp_reward;
    const char *clear_story;
} Stage;

const Stage *get_stages(int *stage_count);
int run_game(void);

#endif
