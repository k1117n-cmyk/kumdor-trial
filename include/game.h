#ifndef GAME_H
#define GAME_H

#define STATUS_NORMAL 0x00
#define STATUS_POISON 0x01
#define STATUS_BLIND  0x02

#define INPUT_BUFFER_SIZE 64
#define EXP_TO_LEVEL_UP 10
#define MAX_STAGE_COUNT 10
#define POISON_TURN_LIMIT 3
#define GAME_VERSION "v0.5.1"
#define SAVE_FILE "kumdor_save.txt"
#define SAVE_COMMAND ":save"
#define QUIT_COMMAND ":quit"
#define SAVE_QUIT_COMMAND ":savequit"
#define HELP_COMMAND ":help"
#define COMMANDS_COMMAND ":commands"
#define SHORT_HELP_COMMAND ":?"
#define BGM_COMMAND ":bgm"
#define MUTE_COMMAND ":mute"

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
    int combo_count;
    int poison_turns_remaining;
    int stage_correct_counts[MAX_STAGE_COUNT];
    int stage_miss_counts[MAX_STAGE_COUNT];
    int stage_input_error_counts[MAX_STAGE_COUNT];
    int stage_max_combo_counts[MAX_STAGE_COUNT];
} Player;

typedef enum {
    ENEMY_TRAIT_STANDARD,
    ENEMY_TRAIT_HEAVY_COUNTER,
    ENEMY_TRAIT_POISON_EDGE,
    ENEMY_TRAIT_BLIND_EDGE,
    ENEMY_TRAIT_REGEN_COUNTER
} EnemyTrait;

typedef struct {
    const char *name;
    int hp;
    int max_hp;
    int attack;
    EnemyTrait trait;
} Enemy;

typedef struct {
    Enemy enemy;
    const char *place;
    const char *story;
    const char *lesson;
    const char *focus_keys;
    const char *tip;
    const char *enemy_quote;
    const char *defeat_quote;
    const char *climax_quote;
    const char *const *miss_quotes;
    int miss_quote_count;
    const char *const *words;
    int word_count;
    const char *climax_message;
    const char *const *climax_words;
    int climax_word_count;
    const char *reward_name;
    int reward_heal;
    int exp_reward;
    const char *clear_story;
    const char *rest_story;
    const char *rest_hint;
    int rest_heal;
} Stage;

const Stage *get_stages(int *stage_count);
Player create_player(void);
int run_game(void);

#endif
