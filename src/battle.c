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
#define COLOR_MAGENTA "\033[35m"
#define MASKED_TARGET_BUFFER_SIZE INPUT_BUFFER_SIZE

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
static void make_blind_target(const char target[], char masked_target[], size_t masked_target_size);
static void print_miss_hint(const char input[], const char target[]);
static const char *choose_message(const char *const messages[], int message_count);
static void print_stage_miss_quote(const Stage *stage);
static EnemyIntent choose_enemy_intent(const Enemy *enemy, int is_climax);
static void print_enemy_intent(EnemyIntent intent, const Enemy *enemy);
static void apply_miss_blind(Player *player, int current_stage);
static void apply_poison(Player *player, const char message[]);
static void tick_poison(Player *player);
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
    printf("%s[状態]%s %s Lv:%d EXP:%d/%d HP:%s%d/%d%s 状態:%s%s%s | %s%s%s HP:%d/%d\n",
           color(COLOR_BLUE),
           color(COLOR_RESET),
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

int player_turn(Player *player, Enemy *enemy, const Stage *stage, const char target[], int current_stage, int stage_count, int is_climax) {
    char input[INPUT_BUFFER_SIZE];
    EnemyIntent intent = choose_enemy_intent(enemy, is_climax);

    maybe_apply_climax_poison(player, enemy, is_climax);
    maybe_apply_climax_blind(player, enemy, is_climax);
    print_enemy_intent(intent, enemy);

    if (player->status & STATUS_BLIND) {
        char masked_target[MASKED_TARGET_BUFFER_SIZE];

        make_blind_target(target, masked_target, sizeof(masked_target));
        printf("%s[課題]%s %s%s%s %s(暗闇: 一部が見えず、入力も表示されない)%s\n",
               color(COLOR_YELLOW),
               color(COLOR_RESET),
               color(COLOR_YELLOW),
               masked_target,
               color(COLOR_RESET),
               color(COLOR_RED),
               color(COLOR_RESET));
    } else {
        printf("%s[課題]%s %s%s%s\n",
               color(COLOR_YELLOW),
               color(COLOR_RESET),
               color(COLOR_YELLOW),
               target,
               color(COLOR_RESET));
    }

    if (!read_input(input, (player->status & STATUS_BLIND) != 0)) {
        int was_poisoned = (player->status & STATUS_POISON) != 0;

        printf("%s➔ 入力が読み取れなかった！ 反撃を受ける！%s\n",
               color(COLOR_RED),
               color(COLOR_RESET));
        player->input_error_count++;
        player->stage_input_error_counts[current_stage]++;
        player->combo_count = 0;
        enemy_turn(player, enemy, intent);
        if (was_poisoned) {
            tick_poison(player);
        }
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
        printf("%s➔ 記録を刻んだ。次はこの試練の入口から始まる。%s\n",
               color(COLOR_BLUE),
               color(COLOR_RESET));
        return 1;
    }

    if (strcmp(input, QUIT_COMMAND) == 0) {
        printf("%s➔ 記録を刻まず、ここで剣を収める。%s\n", color(COLOR_BLUE), color(COLOR_RESET));
        return 0;
    }

    if (strcmp(input, SAVE_QUIT_COMMAND) == 0) {
        save_game(player, current_stage, stage_count);
        printf("%s➔ 記録を刻んだ。ここで剣を収める。%s\n", color(COLOR_BLUE), color(COLOR_RESET));
        return 0;
    }

    if (is_correct_input(input, target)) {
        player->correct_count++;
        player->stage_correct_counts[current_stage]++;
        player->combo_count++;
        if (player->combo_count > player->stage_max_combo_counts[current_stage]) {
            player->stage_max_combo_counts[current_stage] = player->combo_count;
        }

        if (player->status & STATUS_POISON) {
            printf("%s➔ 毒のせいで攻撃が届かない！%s\n", color(COLOR_YELLOW), color(COLOR_RESET));
            printf("%s（毒は消えた。もう一度課題を的中させろ！）%s\n",
                   color(COLOR_GREEN),
                   color(COLOR_RESET));
            player->status &= ~STATUS_POISON;
            player->poison_turns_remaining = 0;
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
        int was_poisoned = (player->status & STATUS_POISON) != 0;

        player->miss_count++;
        player->stage_miss_counts[current_stage]++;
        player->combo_count = 0;
        printf("%s➔ ミス！ 手元が狂った！（反撃を受ける！）%s\n",
               color(COLOR_RED),
               color(COLOR_RESET));
        print_stage_miss_quote(stage);
        print_miss_hint(input, target);
        enemy_turn(player, enemy, intent);
        if (was_poisoned) {
            tick_poison(player);
        }
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
            return "敵が本気を出した後、予告された重撃の反撃ダメージが1増える";
        case ENEMY_TRAIT_POISON_EDGE:
            return "反撃時と敵が本気を出した後に毒を付与し、失敗が続くと追加ダメージを与える";
        case ENEMY_TRAIT_BLIND_EDGE:
            return "反撃時と敵が本気を出した後に暗闇を付与し、課題の一部を見えにくくする";
        case ENEMY_TRAIT_REGEN_COUNTER:
            return "本気状態中の反撃時に自分のHPを1回復する";
        case ENEMY_TRAIT_STANDARD:
        default:
            return "特別な追加効果はない";
    }
}

static void print_help(void) {
    printf("\n%s【ヘルプ】%s\n", color(COLOR_BLUE), color(COLOR_RESET));
    printf("[課題] に表示された文字列を完全一致で入力すると攻撃します。大文字・小文字、スペース、記号も区別します。\n");
    printf("[状態] にはHP、状態異常、敵HPが表示されます。\n");
    printf("毒状態では、正解しても攻撃できず毒の解除に使われます。毒中に失敗が続くと追加ダメージを受けます。\n");
    printf("暗闇状態では、課題の一部と入力中の文字が画面に表示されません。正解すると解除されます。\n");
    printf("タイプミスすると敵が反撃し、入力のずれに応じた短いヒントが出ます。\n");
    printf("3連続正解するたびに追加の一撃が入ります。\n");
    printf("コマンド入力はターンを消費しません。\n");
    printf("コマンド:\n");
    printf("  %-10s 現在ステージの入口に記録を刻む\n", SAVE_COMMAND);
    printf("  %-10s 記録せずに終了\n", QUIT_COMMAND);
    printf("  %-10s 記録を刻んで終了\n", SAVE_QUIT_COMMAND);
    printf("  %-10s このヘルプを表示\n", HELP_COMMAND);
    printf("  %-10s BGMのON/OFFを切り替え（KUMDOR_NO_BGM=1では無効）\n", BGM_COMMAND);
    printf("  %-10s BGMのON/OFFを切り替え（KUMDOR_NO_BGM=1では無効）\n\n", MUTE_COMMAND);
}

static int read_input(char input[], int hide_echo) {
#if defined(__unix__) || defined(__APPLE__)
    struct termios old_terminal;
    struct termios hidden_terminal;
    int echo_hidden = 0;
#endif

    if (hide_echo) {
        printf("課題を入力してEnter（暗闇: 課題の一部と入力は表示されません / :helpでヘルプ）: ");
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

static void make_blind_target(const char target[], char masked_target[], size_t masked_target_size) {
    size_t length = strlen(target);
    int visible_count = 0;

    if (masked_target_size == 0) {
        return;
    }
    masked_target[0] = '\0';

    for (size_t i = 0; i + 1 < masked_target_size && target[i] != '\0'; i++) {
        if (target[i] == ' ') {
            masked_target[i] = ' ';
        } else if (rand() % 2 == 0) {
            masked_target[i] = '?';
        } else {
            masked_target[i] = target[i];
            visible_count++;
        }
        masked_target[i + 1] = '\0';
    }

    if (visible_count == 0 && length > 0) {
        for (size_t i = 0; masked_target[i] != '\0'; i++) {
            if (target[i] != ' ') {
                masked_target[i] = target[i];
                break;
            }
        }
    }
}

static void print_miss_hint(const char input[], const char target[]) {
    static const char *const short_input_messages[] = {
        "剣筋が届いていない。最後の一文字まで振り抜け！",
        "あと一歩で届く。指先の灯を最後まで消すな！",
        "詠唱が途中で切れた。残りの文字までつなげ！"
    };
    static const char *const long_input_messages[] = {
        "踏み込みすぎた。余分な一打が敵に読まれている！",
        "勢いが余った。不要な文字が剣先を乱している！",
        "打鍵がはみ出した。余計な足音を立てるな！"
    };
    static const char *const first_char_messages[] = {
        "初太刀がそれた。最初のキーから構え直せ！",
        "出だしで足場を踏み外した。最初の一打を見極めろ！",
        "最初の符が違う。門を開く一文字目を狙え！"
    };
    static const char *const middle_char_messages[] = {
        "%zu文字目で刃がぶれた。そこまでは道筋が見えている！",
        "%zu文字目で術式が乱れた。流れを切らずに立て直せ！",
        "%zu文字目に敵の影が差した。そこを越えれば届く！"
    };
    size_t input_length = strlen(input);
    size_t target_length = strlen(target);
    size_t compare_length = input_length < target_length ? input_length : target_length;

    if (input_length < target_length) {
        printf("%s（%s）%s\n",
               color(COLOR_YELLOW),
               choose_message(short_input_messages, (int)(sizeof(short_input_messages) / sizeof(short_input_messages[0]))),
               color(COLOR_RESET));
        return;
    }

    if (input_length > target_length) {
        printf("%s（%s）%s\n",
               color(COLOR_YELLOW),
               choose_message(long_input_messages, (int)(sizeof(long_input_messages) / sizeof(long_input_messages[0]))),
               color(COLOR_RESET));
        return;
    }

    for (size_t i = 0; i < compare_length; i++) {
        if (input[i] == target[i]) {
            continue;
        }

        if (i == 0) {
            printf("%s（%s）%s\n",
                   color(COLOR_YELLOW),
                   choose_message(first_char_messages, (int)(sizeof(first_char_messages) / sizeof(first_char_messages[0]))),
                   color(COLOR_RESET));
        } else {
            printf("%s（", color(COLOR_YELLOW));
            printf(choose_message(middle_char_messages, (int)(sizeof(middle_char_messages) / sizeof(middle_char_messages[0]))),
                   i + 1);
            printf("）%s\n",
                   color(COLOR_RESET));
        }
        return;
    }
}

static const char *choose_message(const char *const messages[], int message_count) {
    return messages[rand() % message_count];
}

static void print_stage_miss_quote(const Stage *stage) {
    static const Stage *last_stage = NULL;
    static int last_quote_index = -1;
    int quote_index;

    if (stage->miss_quotes == NULL || stage->miss_quote_count <= 0) {
        return;
    }

    quote_index = rand() % stage->miss_quote_count;
    if (stage->miss_quote_count > 1 &&
        last_stage == stage &&
        quote_index == last_quote_index) {
        quote_index = (quote_index + 1 + rand() % (stage->miss_quote_count - 1)) % stage->miss_quote_count;
    }

    last_stage = stage;
    last_quote_index = quote_index;
    printf("%s%s%s\n", color(COLOR_MAGENTA), stage->miss_quotes[quote_index], color(COLOR_RESET));
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

    apply_poison(player, "敵の刃から毒霧が広がった！");
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

    if (intent == ENEMY_INTENT_HEAVY) {
        printf("%s（敵の反撃が怒りを帯びて重くなっている！）%s\n",
               color(COLOR_YELLOW),
               color(COLOR_RESET));
    }

    if ((enemy->trait == ENEMY_TRAIT_POISON_EDGE || intent == ENEMY_INTENT_POISON) &&
        !(player->status & STATUS_POISON)) {
        apply_poison(player, "毒刃を受けた。");
    }

    if ((enemy->trait == ENEMY_TRAIT_BLIND_EDGE || intent == ENEMY_INTENT_BLIND) &&
        !(player->status & STATUS_BLIND)) {
        player->status |= STATUS_BLIND;
        printf("%s（視界が暗くなった。課題を落ち着いて確認しろ！）%s\n",
               color(COLOR_YELLOW),
               color(COLOR_RESET));
    }

    if ((intent == ENEMY_INTENT_REGEN ||
         (enemy->trait == ENEMY_TRAIT_REGEN_COUNTER && enemy->hp * 2 <= enemy->max_hp)) &&
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

static void apply_poison(Player *player, const char message[]) {
    player->status |= STATUS_POISON;
    player->poison_turns_remaining = POISON_TURN_LIMIT;
    printf("%s（%s%d回以内に正解しないと毒が深く回る！）%s\n",
           color(COLOR_YELLOW),
           message,
           player->poison_turns_remaining,
           color(COLOR_RESET));
}

static void tick_poison(Player *player) {
    if (player->hp <= 0 ||
        !(player->status & STATUS_POISON) ||
        player->poison_turns_remaining <= 0) {
        return;
    }

    player->poison_turns_remaining--;

    if (player->poison_turns_remaining > 0) {
        printf("%s（毒が体に回る。あと%d回以内に正解して解除しろ！）%s\n",
               color(COLOR_YELLOW),
               player->poison_turns_remaining,
               color(COLOR_RESET));
        return;
    }

    player->hp--;
    if (player->hp < 0) {
        player->hp = 0;
    }
    player->poison_turns_remaining = POISON_TURN_LIMIT;
    printf("%s（毒が深く回り、1ダメージを受けた！ 次の猶予: %d回）%s (あなたの残りHP: %d)\n",
           color(COLOR_RED),
           player->poison_turns_remaining,
           color(COLOR_RESET),
           player->hp);
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
