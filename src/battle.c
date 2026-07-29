#include "battle.h"
#include "audio.h"
#include "save.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__unix__) || defined(__APPLE__)
#include <termios.h>
#include <unistd.h>
#endif

#define COLOR_RESET  "\033[0m"
#define COLOR_RED    "\033[95m"
#define COLOR_GREEN  "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE   "\033[34m"

typedef enum {
    ENEMY_INTENT_NONE,
    ENEMY_INTENT_HEAVY,
    ENEMY_INTENT_POISON,
    ENEMY_INTENT_BLIND,
    ENEMY_INTENT_REGEN
} EnemyIntent;

static const char *status_name(unsigned char status);
static const char *enemy_trait_name(EnemyTrait trait);
static const char *enemy_trait_description(EnemyTrait trait);
static void print_help(void);
static int read_input(char input[], int hide_echo);
static int is_correct_input(const char input[], const char target[]);
static EnemyIntent choose_enemy_intent(const Enemy *enemy, int is_climax);
static void print_enemy_intent(EnemyIntent intent, const Enemy *enemy);
static void apply_miss_blind(Player *player, int current_stage);
static void maybe_apply_climax_poison(Player *player, const Enemy *enemy, int is_climax);
static void maybe_apply_climax_blind(Player *player, const Enemy *enemy, int is_climax);
static void enemy_turn(Player *player, Enemy *enemy, EnemyIntent intent);
static const char *color(const char *code);
static const char *hp_color(const Player *player);

void print_battle_start(const Enemy *enemy) {
    printf("\n%s*************** BATTLE START ***************%s\n",
           color(COLOR_RED),
           color(COLOR_RESET));
    printf("%s%sが立ちはだかった！%s\n",
           color(COLOR_RED),
           enemy->name,
           color(COLOR_RESET));
    printf("敵特性: %s - %s\n",
           enemy_trait_name(enemy->trait),
           enemy_trait_description(enemy->trait));
    printf("%s********************************************%s\n\n",
           color(COLOR_RED),
           color(COLOR_RESET));
}

void print_battle_status(const Player *player, const Enemy *enemy) {
    printf("%s Lv:%d EXP:%d/%d HP: %s%d/%d%s 状態:%s%s%s | %s%s%s HP: %d/%d\n",
           player->name,
           player->level,
           player->exp,
           EXP_TO_LEVEL_UP,
           hp_color(player),
           player->hp,
           player->max_hp,
           color(COLOR_RESET),
           player->status == STATUS_NORMAL ? color(COLOR_GREEN) : color(COLOR_YELLOW),
           status_name(player->status),
           color(COLOR_RESET),
           color(COLOR_RED),
           enemy->name,
           color(COLOR_RESET),
           enemy->hp,
           enemy->max_hp);
}

int player_turn(Player *player, Enemy *enemy, const char target[], int current_stage, int stage_count, int is_climax) {
    char input[INPUT_BUFFER_SIZE];
    EnemyIntent intent = choose_enemy_intent(enemy, is_climax);

    maybe_apply_climax_poison(player, enemy, is_climax);
    maybe_apply_climax_blind(player, enemy, is_climax);
    print_enemy_intent(intent, enemy);

    if (player->status & STATUS_BLIND) {
        printf("敵の構え: [ %s%s%s ] %s(視界が悪い！正確に打ち込め！)%s\n",
               color(COLOR_YELLOW),
               target,
               color(COLOR_RESET),
               color(COLOR_RED),
               color(COLOR_RESET));
    } else {
        printf("敵の構え: [ %s%s%s ]\n", color(COLOR_YELLOW), target, color(COLOR_RESET));
    }

    if (!read_input(input, (player->status & STATUS_BLIND) != 0)) {
        printf("%s➔ 入力が読み取れなかった！ 反撃を受ける！%s\n",
               color(COLOR_RED),
               color(COLOR_RESET));
        player->input_error_count++;
        player->stage_input_error_counts[current_stage]++;
        player->combo_count = 0;
        enemy_turn(player, enemy, intent);
        return 1;
    }

    if (strcmp(input, HELP_COMMAND) == 0 ||
        strcmp(input, COMMANDS_COMMAND) == 0 ||
        strcmp(input, SHORT_HELP_COMMAND) == 0) {
        print_help();
        return 1;
    }

    if (strcmp(input, BGM_COMMAND) == 0 || strcmp(input, MUTE_COMMAND) == 0) {
        toggle_bgm();
        return 1;
    }

    if (strcmp(input, SAVE_COMMAND) == 0) {
        save_game(player, current_stage, stage_count);
        printf("%s➔ セーブしました。現在のステージの先頭から再開できます。%s\n",
               color(COLOR_BLUE),
               color(COLOR_RESET));
        return 1;
    }

    if (strcmp(input, QUIT_COMMAND) == 0) {
        printf("%s➔ 保存せずに終了します。%s\n", color(COLOR_BLUE), color(COLOR_RESET));
        return 0;
    }

    if (strcmp(input, SAVE_QUIT_COMMAND) == 0) {
        save_game(player, current_stage, stage_count);
        printf("%s➔ セーブして終了します。%s\n", color(COLOR_BLUE), color(COLOR_RESET));
        return 0;
    }

    if (is_correct_input(input, target)) {
        player->correct_count++;
        player->stage_correct_counts[current_stage]++;
        player->combo_count++;

        if (player->status & STATUS_POISON) {
            printf("%s➔ 毒のせいで攻撃が届かない！%s\n", color(COLOR_YELLOW), color(COLOR_RESET));
            printf("%s（毒は消えた。もう一度課題を的中させろ！）%s\n",
                   color(COLOR_GREEN),
                   color(COLOR_RESET));
            player->status &= ~STATUS_POISON;
        } else {
            enemy->hp--;
            printf("%s➔ 見事なタイピング！ 剣が炸裂した！%s (敵の残りHP: %d)\n",
                   color(COLOR_GREEN),
                   color(COLOR_RESET),
                   enemy->hp);

            if (player->combo_count > 0 && player->combo_count % 3 == 0 && enemy->hp > 0) {
                enemy->hp--;
                printf("%s（%d連続正解！ 剣筋が冴え、追加の一撃が入った！）%s (敵の残りHP: %d)\n",
                       color(COLOR_GREEN),
                       player->combo_count,
                       color(COLOR_RESET),
                       enemy->hp);
            }
        }

        if (player->status & STATUS_BLIND) {
            player->status &= ~STATUS_BLIND;
            printf("%s（暗闇が晴れた！）%s\n", color(COLOR_GREEN), color(COLOR_RESET));
        }
    } else {
        player->miss_count++;
        player->stage_miss_counts[current_stage]++;
        player->combo_count = 0;
        printf("%s➔ ミス！ 手元が狂った！（反撃を受ける！）%s\n",
               color(COLOR_RED),
               color(COLOR_RESET));
        enemy_turn(player, enemy, intent);
        apply_miss_blind(player, current_stage);
    }

    return 1;
}

static const char *status_name(unsigned char status) {
    if ((status & STATUS_POISON) && (status & STATUS_BLIND)) {
        return "毒+暗闇";
    }

    if (status & STATUS_POISON) {
        return "毒";
    }

    if (status & STATUS_BLIND) {
        return "暗闇";
    }

    return "通常";
}

static const char *enemy_trait_name(EnemyTrait trait) {
    switch (trait) {
        case ENEMY_TRAIT_HEAVY_COUNTER:
            return "強打";
        case ENEMY_TRAIT_POISON_EDGE:
            return "毒刃";
        case ENEMY_TRAIT_BLIND_EDGE:
            return "暗闇";
        case ENEMY_TRAIT_REGEN_COUNTER:
            return "再生";
        case ENEMY_TRAIT_STANDARD:
        default:
            return "標準";
    }
}

static const char *enemy_trait_description(EnemyTrait trait) {
    switch (trait) {
        case ENEMY_TRAIT_HEAVY_COUNTER:
            return "敵が本気を出すと反撃ダメージが1増える";
        case ENEMY_TRAIT_POISON_EDGE:
            return "反撃時と敵が本気を出した後に一時的な毒を付与する";
        case ENEMY_TRAIT_BLIND_EDGE:
            return "反撃時と敵が本気を出した後に一時的な暗闇を付与する";
        case ENEMY_TRAIT_REGEN_COUNTER:
            return "反撃時に自分のHPを1回復する";
        case ENEMY_TRAIT_STANDARD:
        default:
            return "特別な追加効果はない";
    }
}

static void print_help(void) {
    printf("\n%s【ヘルプ】%s\n", color(COLOR_BLUE), color(COLOR_RESET));
    printf("表示された課題を完全一致で入力すると攻撃します。大文字・小文字、スペース、記号も区別します。\n");
    printf("毒状態では、正解しても攻撃できず毒の解除に使われます。\n");
    printf("暗闇状態では、入力中の文字が画面に表示されません。正解すると解除されます。\n");
    printf("タイプミスすると敵が反撃します。コマンド入力はターンを消費しません。\n");
    printf("敵には特性があります。戦闘開始時の説明を見て、ミス時の反撃に注意してください。\n");
    printf("コマンド:\n");
    printf("  %s       現在ステージの先頭から再開できる状態で保存\n", SAVE_COMMAND);
    printf("  %s       保存せずに終了\n", QUIT_COMMAND);
    printf("  %s   保存して終了\n", SAVE_QUIT_COMMAND);
    printf("  %s / %s BGMのON/OFFを切り替え\n", BGM_COMMAND, MUTE_COMMAND);
    printf("  %s       このヘルプを表示\n\n", HELP_COMMAND);
}

static int read_input(char input[], int hide_echo) {
#if defined(__unix__) || defined(__APPLE__)
    struct termios old_terminal;
    struct termios hidden_terminal;
    int echo_hidden = 0;
#endif

    if (hide_echo) {
        printf("課題を入力してEnter（暗闇: 入力は表示されません / :helpでヘルプ）: ");
    } else {
        printf("課題を入力してEnter（:helpでヘルプ）: ");
    }
    fflush(stdout);

#if defined(__unix__) || defined(__APPLE__)
    if (hide_echo && isatty(STDIN_FILENO) && tcgetattr(STDIN_FILENO, &old_terminal) == 0) {
        hidden_terminal = old_terminal;
        hidden_terminal.c_lflag &= (tcflag_t)~ECHO;
        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &hidden_terminal) == 0) {
            echo_hidden = 1;
        }
    }
#endif

    if (fgets(input, INPUT_BUFFER_SIZE, stdin) == NULL) {
#if defined(__unix__) || defined(__APPLE__)
        if (echo_hidden) {
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_terminal);
            printf("\n");
        }
#endif
        return 0;
    }

#if defined(__unix__) || defined(__APPLE__)
    if (echo_hidden) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_terminal);
        printf("\n");
    }
#endif

    input[strcspn(input, "\n")] = '\0';
    return 1;
}

static int is_correct_input(const char input[], const char target[]) {
    return strcmp(input, target) == 0;
}

static EnemyIntent choose_enemy_intent(const Enemy *enemy, int is_climax) {
    if (!is_climax || enemy->hp % 2 == 0) {
        return ENEMY_INTENT_NONE;
    }

    switch (enemy->trait) {
        case ENEMY_TRAIT_HEAVY_COUNTER:
            return ENEMY_INTENT_HEAVY;
        case ENEMY_TRAIT_POISON_EDGE:
            return ENEMY_INTENT_POISON;
        case ENEMY_TRAIT_BLIND_EDGE:
            return ENEMY_INTENT_BLIND;
        case ENEMY_TRAIT_REGEN_COUNTER:
            return ENEMY_INTENT_REGEN;
        case ENEMY_TRAIT_STANDARD:
        default:
            return ENEMY_INTENT_NONE;
    }
}

static void print_enemy_intent(EnemyIntent intent, const Enemy *enemy) {
    switch (intent) {
        case ENEMY_INTENT_HEAVY:
            printf("%s【予感】%s%sが大きく踏み込んだ。ミスすれば重い反撃が来る。\n",
                   color(COLOR_YELLOW),
                   color(COLOR_RESET),
                   enemy->name);
            break;
        case ENEMY_INTENT_POISON:
            printf("%s【予感】%s%sの刃先に毒の光がにじむ。ミスすれば毒を受ける。\n",
                   color(COLOR_YELLOW),
                   color(COLOR_RESET),
                   enemy->name);
            break;
        case ENEMY_INTENT_BLIND:
            printf("%s【予感】%s%sの影が視界の端で揺れる。ミスすれば暗闇が来る。\n",
                   color(COLOR_YELLOW),
                   color(COLOR_RESET),
                   enemy->name);
            break;
        case ENEMY_INTENT_REGEN:
            printf("%s【予感】%s%sが息を整えている。ミスすれば体勢を立て直される。\n",
                   color(COLOR_YELLOW),
                   color(COLOR_RESET),
                   enemy->name);
            break;
        case ENEMY_INTENT_NONE:
        default:
            break;
    }
}

static void apply_miss_blind(Player *player, int current_stage) {
    if (current_stage < 2 || (player->status & STATUS_BLIND)) {
        return;
    }

    player->status |= STATUS_BLIND;
    printf("%s（焦りで視界が乱れた。次の入力は手元を頼れない！）%s\n",
           color(COLOR_YELLOW),
           color(COLOR_RESET));
}

static void maybe_apply_climax_poison(Player *player, const Enemy *enemy, int is_climax) {
    if (!is_climax ||
        enemy->trait != ENEMY_TRAIT_POISON_EDGE ||
        (player->status & STATUS_POISON) ||
        rand() % 3 != 0) {
        return;
    }

    player->status |= STATUS_POISON;
    printf("%s（敵の刃から毒霧が広がった！ 次の正解は毒の解除に使われる！）%s\n",
           color(COLOR_YELLOW),
           color(COLOR_RESET));
}

static void maybe_apply_climax_blind(Player *player, const Enemy *enemy, int is_climax) {
    if (!is_climax ||
        enemy->trait != ENEMY_TRAIT_BLIND_EDGE ||
        (player->status & STATUS_BLIND) ||
        rand() % 3 != 0) {
        return;
    }

    player->status |= STATUS_BLIND;
    printf("%s（敵の気配が視界を奪った！ 次の入力は手元を頼れない！）%s\n",
           color(COLOR_YELLOW),
           color(COLOR_RESET));
}

static void enemy_turn(Player *player, Enemy *enemy, EnemyIntent intent) {
    int damage = enemy->attack;

    if (enemy->trait == ENEMY_TRAIT_HEAVY_COUNTER && enemy->hp * 2 <= enemy->max_hp) {
        damage++;
    }

    if (intent == ENEMY_INTENT_HEAVY) {
        damage++;
    }

    player->hp -= damage;

    if (player->hp < 0) {
        player->hp = 0;
    }

    printf("%s➔ %sの反撃！ %dダメージを受けた！%s (あなたの残りHP: %d)\n",
           color(COLOR_RED),
           enemy->name,
           damage,
           color(COLOR_RESET),
           player->hp);

    if (enemy->trait == ENEMY_TRAIT_HEAVY_COUNTER && damage > enemy->attack) {
        printf("%s（敵の反撃が怒りを帯びて重くなっている！）%s\n",
               color(COLOR_YELLOW),
               color(COLOR_RESET));
    }

    if ((enemy->trait == ENEMY_TRAIT_POISON_EDGE || intent == ENEMY_INTENT_POISON) &&
        !(player->status & STATUS_POISON)) {
        player->status |= STATUS_POISON;
        printf("%s（毒刃を受けた。次の正解は毒の解除に使われる！）%s\n",
               color(COLOR_YELLOW),
               color(COLOR_RESET));
    }

    if ((enemy->trait == ENEMY_TRAIT_BLIND_EDGE || intent == ENEMY_INTENT_BLIND) &&
        !(player->status & STATUS_BLIND)) {
        player->status |= STATUS_BLIND;
        printf("%s（視界が暗くなった。課題を落ち着いて確認しろ！）%s\n",
               color(COLOR_YELLOW),
               color(COLOR_RESET));
    }

    if ((enemy->trait == ENEMY_TRAIT_REGEN_COUNTER || intent == ENEMY_INTENT_REGEN) &&
        enemy->hp < enemy->max_hp) {
        enemy->hp++;
        printf("%s（%sは体勢を立て直し、HPを1回復した！ 敵HP: %d/%d）%s\n",
               color(COLOR_YELLOW),
               enemy->name,
               enemy->hp,
               enemy->max_hp,
               color(COLOR_RESET));
    }
}

static const char *color(const char *code) {
    return getenv("NO_COLOR") == NULL ? code : "";
}

static const char *hp_color(const Player *player) {
    if (player->hp * 3 <= player->max_hp) {
        return color(COLOR_RED);
    }

    if (player->hp * 2 <= player->max_hp) {
        return color(COLOR_YELLOW);
    }

    return color(COLOR_GREEN);
}
