#include "game.h"
#include "audio.h"
#include "battle.h"
#include "save.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[95m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_BOLD    "\033[1m"
static void print_title(void);
static void print_prologue(void);
static void print_stage_transition(int next_stage_number, int stage_count);
static void print_stage_intro(int stage_number, int stage_count, const Stage *stage);
static void print_stage_clear(int stage_number, const Stage *stage, Player *player);
static void apply_rest_event(const Stage *stage, Player *player, int stage_index);
static void print_rest_bonus_event(int stage_index);
static void print_rest_fortune(void);
static void print_rest_kum_moose(void);
static void print_rest_supply_cart(void);
static void print_stage_climax(const Stage *stage);
static void print_enemy_quote(const Stage *stage);
static void print_defeat_quote(const Stage *stage);
static void print_ending(const Player *player);
static void print_score(const Player *player, int stage_count, int quit_requested);
static void print_stage_scores(const Player *player, int stage_count);
static const char *stage_clear_rank(double accuracy, int miss, int input_error, int max_combo);
static const char *stage_clear_rank_color(const char *rank);
static void gain_exp(Player *player, int exp);
static int prompt_next_stage(int next_stage_number, int stage_count);
static const char *choose_target(const char *const words[], int word_count);
static const char *color(const char *code);
static void seed_random(void);

int run_game(void) {
    seed_random();
    atexit(cleanup_audio);

    Player player = create_player();
    int stage_count = 0;
    const Stage *stages = get_stages(&stage_count);
    int start_stage = 0;
    int quit_requested = 0;

    if (stage_count > MAX_STAGE_COUNT) {
        printf("ステージ数がスコア記録の上限を超えています。\n");
        return 1;
    }

    print_title();
    if (!load_game(&player, &start_stage, stage_count)) {
        print_prologue();
    }

    for (int stage = start_stage; stage < stage_count && player.hp > 0 && !quit_requested; stage++) {
        Enemy enemy = stages[stage].enemy;

        player.reached_stage = stage + 1;
        if (stage > start_stage) {
            print_stage_transition(stage + 1, stage_count);
        }
        start_stage_bgm(stage + 1);
        print_stage_intro(stage + 1, stage_count, &stages[stage]);
        print_battle_start(&enemy);
        print_enemy_quote(&stages[stage]);
        player.combo_count = 0;

        int climax_started = 0;

        // メインゲームループ
        while (player.hp > 0 && enemy.hp > 0 && !quit_requested) {
            const char *const *words = stages[stage].words;
            int word_count = stages[stage].word_count;

            if (!climax_started && enemy.hp * 2 <= enemy.max_hp) {
                climax_started = 1;
                print_stage_climax(&stages[stage]);
            }

            if (climax_started && stages[stage].climax_word_count > 0) {
                words = stages[stage].climax_words;
                word_count = stages[stage].climax_word_count;
            }

            const char *target = choose_target(words, word_count);

            print_battle_status(&player, &enemy);
            if (!player_turn(&player, &enemy, &stages[stage], target, stage, stage_count, climax_started)) {
                quit_requested = 1;
            }

            printf("-----------------------------------------\n");
        }

        if (player.hp > 0 && !quit_requested) {
            print_stage_clear(stage + 1, &stages[stage], &player);
            stop_bgm();
            apply_rest_event(&stages[stage], &player, stage);
            save_game(&player, stage + 1, stage_count);
            if (stage + 1 < stage_count && !prompt_next_stage(stage + 2, stage_count)) {
                quit_requested = 1;
            }
        }
    }

    stop_bgm();

    if (quit_requested) {
        printf("\n%s【終了】%sクムドールの試練を中断しました。\n",
               color(COLOR_BLUE),
               color(COLOR_RESET));
    } else if (player.hp > 0) {
        print_ending(&player);
    } else {
        printf("\n%s【敗北】%s%sは膝をついた……。\n",
               color(COLOR_RED),
               color(COLOR_RESET),
               player.name);
        printf("もう一度挑戦して、クムドールの試練を突破しよう！\n");
    }

    print_score(&player, stage_count, quit_requested);

    return 0;
}

static void print_title(void) {
    printf("%s%s=========================================\n", color(COLOR_BOLD), color(COLOR_CYAN));
    printf("     タイピングRPG ★ クムドールの試練     \n");
    printf("                  %s                  \n", GAME_VERSION);
    printf("=========================================%s\n", color(COLOR_RESET));
    printf("現れた課題をそのまま正確にタイプして、敵を倒せ！\n\n");
}

static void print_prologue(void) {
    printf("%s=========================================%s\n", color(COLOR_CYAN), color(COLOR_RESET));
    printf("%s【プロローグ】%s\n\n", color(COLOR_YELLOW), color(COLOR_RESET));
    printf("ポーラ暦405年。銀河で名を知られたキーボード使いのあなたは、\n");
    printf("退屈を振り払うように、移民惑星ソルフェスへ向かった。\n");
    printf("だが到着した夜、クムドール王国からの使者が倒れ込み、こう告げる。\n");
    printf("「クムの森が石になり、王都との通信が途絶えました。どうか来てください」\n");
    printf("あなたは自動操縦船クム3号に乗り込む。目的地は小さな森林惑星、クムドール。\n");
    printf("しかし着陸直前、船の計器は意味のない文字列を吐き出し、空から墜ちた。\n");
    printf("失ったキー、散らばったスパイス、残ったライフはひとつ。\n");
    printf("それでも、指はまだホームポジションを覚えている。\n\n");
}

Player create_player(void) {
    Player player = {0};

    player.name = "あなた";
    player.hp = 10;
    player.max_hp = 10;
    player.status = STATUS_NORMAL;
    player.level = 1;

    return player;
}

static void print_stage_transition(int next_stage_number, int stage_count) {
    printf("\n%s-----------------------------------------%s\n", color(COLOR_BLUE), color(COLOR_RESET));
    printf("%s次の試練へ進む: 第%d/%dステージ%s\n",
           color(COLOR_BLUE),
           next_stage_number,
           stage_count,
           color(COLOR_RESET));
    printf("%s-----------------------------------------%s\n", color(COLOR_BLUE), color(COLOR_RESET));
}

static void print_stage_intro(int stage_number, int stage_count, const Stage *stage) {
    printf("\n%s=========================================%s\n", color(COLOR_CYAN), color(COLOR_RESET));
    printf("%s【第%d/%dステージ】%s\n",
           color(COLOR_YELLOW),
           stage_number,
           stage_count,
           color(COLOR_RESET));
    printf("場所    : %s\n", stage->place);
    printf("%s\n", stage->story);
    printf("練習内容: %s\n", stage->lesson);
    printf("今回のキー: %s\n", stage->focus_keys);
    printf("指使い  : %s\n", stage->tip);
    printf("敵      : %s%s%s\n", color(COLOR_RED), stage->enemy.name, color(COLOR_RESET));
    printf("%s=========================================%s\n", color(COLOR_CYAN), color(COLOR_RESET));
}

static void print_stage_clear(int stage_number, const Stage *stage, Player *player) {
    printf("\n%s=========================================%s\n", color(COLOR_CYAN), color(COLOR_RESET));
    printf("%s【ステージ%d突破】%s\n", color(COLOR_GREEN), stage_number, color(COLOR_RESET));
    printf("%s%sを倒した！%s\n", color(COLOR_GREEN), stage->enemy.name, color(COLOR_RESET));
    print_defeat_quote(stage);
    gain_exp(player, stage->exp_reward);

    if (stage->reward_heal > 0 && player->hp < player->max_hp) {
        player->hp += stage->reward_heal;
        if (player->hp > player->max_hp) {
            player->hp = player->max_hp;
        }

        printf("%s%sでHPが%d回復した！%s (あなたのHP: %d/%d)\n",
               color(COLOR_GREEN),
               stage->reward_name,
               stage->reward_heal,
               color(COLOR_RESET),
               player->hp,
               player->max_hp);
    }

    int stage_index = stage_number - 1;
    int correct = player->stage_correct_counts[stage_index];
    int miss = player->stage_miss_counts[stage_index];
    int input_error = player->stage_input_error_counts[stage_index];
    int max_combo = player->stage_max_combo_counts[stage_index];
    int total = correct + miss + input_error;
    double accuracy = 0.0;
    const char *rank;

    if (total > 0) {
        accuracy = ((double)correct / (double)total) * 100.0;
    }

    rank = stage_clear_rank(accuracy, miss, input_error, max_combo);

    printf("ステージ成績: 正解 %d / ミス %d / 入力失敗 %d / 命中率 %.1f%% / 最大連続正解 %d\n",
           correct,
           miss,
           input_error,
           accuracy,
           max_combo);
    printf("突破評価    : %s%s%s\n",
           stage_clear_rank_color(rank),
           rank,
           color(COLOR_RESET));
    printf("%s\n", stage->clear_story);
    printf("%s=========================================%s\n", color(COLOR_CYAN), color(COLOR_RESET));
}

static void apply_rest_event(const Stage *stage, Player *player, int stage_index) {
    int healed = 0;
    int printed_rest_result = 0;

    if ((stage->rest_story == NULL || stage->rest_story[0] == '\0') &&
        (stage->rest_hint == NULL || stage->rest_hint[0] == '\0') &&
        stage->rest_heal <= 0) {
        return;
    }

    printf("\n%s【休息】%s\n", color(COLOR_CYAN), color(COLOR_RESET));

    if (stage->rest_story != NULL && stage->rest_story[0] != '\0') {
        printf("%s\n\n", stage->rest_story);
    }

    print_rest_bonus_event(stage_index);
    printf("\n");

    if (stage->rest_heal > 0 && player->hp < player->max_hp) {
        int before = player->hp;

        player->hp += stage->rest_heal;
        if (player->hp > player->max_hp) {
            player->hp = player->max_hp;
        }

        healed = player->hp - before;
    }

    if (player->status != STATUS_NORMAL) {
        player->status = STATUS_NORMAL;
        printf("%s状態異常が消えた。指先の感覚が戻ってくる。%s\n",
               color(COLOR_GREEN),
               color(COLOR_RESET));
        printed_rest_result = 1;
    }

    if (healed > 0) {
        printf("%sHPが%d回復した。%s (あなたのHP: %d/%d)\n",
               color(COLOR_GREEN),
               healed,
               color(COLOR_RESET),
               player->hp,
               player->max_hp);
        printed_rest_result = 1;
    }

    if (stage->rest_hint != NULL && stage->rest_hint[0] != '\0') {
        if (printed_rest_result) {
            printf("\n");
        }
        printf("%s次の助言: %s%s\n",
               color(COLOR_YELLOW),
               stage->rest_hint,
               color(COLOR_RESET));
    }
}

static void print_rest_bonus_event(int stage_index) {
    int rest_event = (stage_index / 3 + 1) % 3;

    if (rest_event == 1) {
        print_rest_kum_moose();
    } else if (rest_event == 2) {
        print_rest_supply_cart();
    } else {
        print_rest_fortune();
    }
}

static void print_rest_fortune(void) {
    static const char *const fortunes[] = {
        "焦る指は敵の影を追う。息を整えれば、次のキーが先に見える。",
        "打ち損じは敗北ではない。戻る場所を知るための足跡だ。",
        "長い課題ほど、最初の一打を静かに置け。剣はそこから伸びる。",
        "記号は罠ではない。形を見てから打てば、ただの道しるべになる。"
    };
    const char *message = fortunes[rand() % (int)(sizeof(fortunes) / sizeof(fortunes[0]))];

    printf("%s【旅の札】%s %s\n",
           color(COLOR_MAGENTA),
           color(COLOR_RESET),
           message);
}

static void print_rest_kum_moose(void) {
    static const char *const moose_messages[] = {
        "ホームポジションへ戻れ、モォォ……",
        "スペースも剣筋の一部、モォォ……",
        "Shiftは焦らず押せ、モォォ……"
    };
    static int last_message_index = -1;
    int message_count = (int)(sizeof(moose_messages) / sizeof(moose_messages[0]));
    int message_index = rand() % message_count;

    if (message_count > 1 && message_index == last_message_index) {
        message_index = (message_index + 1 + rand() % (message_count - 1)) % message_count;
    }

    last_message_index = message_index;

    printf("%s【クムムースの助言】%s\n",
           color(COLOR_MAGENTA),
           color(COLOR_RESET));
    printf("  < %s >\n", moose_messages[message_index]);
    printf("    o   \\_\\_    _/_/\n");
    printf("     o      \\__/\n");
    printf("            (oo)\\_______\n");
    printf("            (__)\\       )\\/\\\n");
    printf("                ||----w |\n");
    printf("                ||     ||\n");
}

static void print_rest_supply_cart(void) {
    static const char *const cargo_messages[] = {
        "温かいスープ",
        "替えの手袋",
        "磨かれたキーキャップ"
    };
    const char *cargo = cargo_messages[rand() % (int)(sizeof(cargo_messages) / sizeof(cargo_messages[0]))];

    printf("%s【補給車クム3号】%s  ==[ %s ]==>\n",
           color(COLOR_MAGENTA),
           color(COLOR_RESET),
           cargo);
    printf("小さな補給車が通り過ぎ、休息の場に少しだけ笑いが戻った。\n");
}

static void print_stage_climax(const Stage *stage) {
    if (stage->climax_message == NULL || stage->climax_message[0] == '\0') {
        return;
    }

    printf("\n%s【敵の殺気】%s%s\n", color(COLOR_YELLOW), color(COLOR_RESET), stage->climax_message);
    if (stage->climax_quote != NULL && stage->climax_quote[0] != '\0') {
        printf("%s%s%s\n", color(COLOR_MAGENTA), stage->climax_quote, color(COLOR_RESET));
    }
    printf("空気が張りつめる。敵は反撃の機会を狙い、構えも鋭く変わった。\n");
}

static void print_enemy_quote(const Stage *stage) {
    if (stage->enemy_quote == NULL || stage->enemy_quote[0] == '\0') {
        return;
    }

    printf("%s%s%s\n\n", color(COLOR_MAGENTA), stage->enemy_quote, color(COLOR_RESET));
}

static void print_defeat_quote(const Stage *stage) {
    if (stage->defeat_quote == NULL || stage->defeat_quote[0] == '\0') {
        return;
    }

    printf("%s%s%s\n", color(COLOR_MAGENTA), stage->defeat_quote, color(COLOR_RESET));
}

static void print_ending(const Player *player) {
    printf("\n%s【完全勝利】%s\n", color(COLOR_GREEN), color(COLOR_RESET));
    printf("%sが最後の課題を打ち抜いた瞬間、クムドールの剣がまばゆく輝いた！\n",
           player->name);
    printf("石化していたクムの森は緑を取り戻し、王都の通信塔にも光が戻る。\n");
    printf("女王マルクァ・ランドは、あなたを王国の救い手として迎えた。\n");
    printf("けれど本当の報酬は、視線を落とさず打ち切ったその両手に残っている。\n");
    printf("あなたのタイピングスキルがレベルアップした！\n");
}

static void print_score(const Player *player, int stage_count, int quit_requested) {
    int total_inputs = player->correct_count + player->miss_count + player->input_error_count;
    double accuracy = 0.0;

    if (total_inputs > 0) {
        accuracy = ((double)player->correct_count / (double)total_inputs) * 100.0;
    }

    printf("\n%s=========================================%s\n", color(COLOR_CYAN), color(COLOR_RESET));
    printf("%s【スコア】%s\n", color(COLOR_MAGENTA), color(COLOR_RESET));
    if (player->hp <= 0) {
        printf("結果        : 第%d/%dステージで敗北\n", player->reached_stage, stage_count);
    } else if (player->reached_stage >= stage_count && !quit_requested) {
        printf("結果        : 全%dステージ突破\n", stage_count);
    } else if (quit_requested) {
        printf("結果        : 第%d/%dステージで中断\n", player->reached_stage, stage_count);
    } else {
        printf("結果        : 第%d/%dステージまで到達\n", player->reached_stage, stage_count);
    }
    printf("レベル      : %d\n", player->level);
    printf("経験値      : %d/%d\n", player->exp, EXP_TO_LEVEL_UP);
    printf("正解数      : %d\n", player->correct_count);
    printf("ミス数      : %d\n", player->miss_count);
    printf("入力失敗    : %d\n", player->input_error_count);
    printf("命中率      : %.1f%%\n", accuracy);
    print_stage_scores(player, stage_count);

    if (accuracy >= 95.0 && player->reached_stage == stage_count && player->hp > 0) {
        printf("評価        : %s銀河級のキーボード使い%s\n", color(COLOR_GREEN), color(COLOR_RESET));
    } else if (accuracy >= 80.0) {
        printf("評価        : %s安定した剣さばき%s\n", color(COLOR_GREEN), color(COLOR_RESET));
    } else if (accuracy >= 60.0) {
        printf("評価        : %sまずは正確さを固めよう%s\n", color(COLOR_YELLOW), color(COLOR_RESET));
    } else {
        printf("評価        : %sホームポジションから再挑戦%s\n", color(COLOR_RED), color(COLOR_RESET));
    }

    printf("%s=========================================%s\n", color(COLOR_CYAN), color(COLOR_RESET));
}

static const char *stage_clear_rank(double accuracy, int miss, int input_error, int max_combo) {
    if (accuracy >= 100.0 && max_combo >= 8) {
        return "無傷の剣筋";
    }

    if (accuracy >= 90.0 && miss == 0 && input_error == 0) {
        return "集中の剣筋";
    }

    if (accuracy >= 80.0) {
        return "安定突破";
    }

    if (miss + input_error >= 4) {
        return "危険突破";
    }

    return "立て直しの一歩";
}

static const char *stage_clear_rank_color(const char *rank) {
    if (strcmp(rank, "危険突破") == 0) {
        return color(COLOR_RED);
    }

    if (strcmp(rank, "立て直しの一歩") == 0) {
        return color(COLOR_YELLOW);
    }

    return color(COLOR_GREEN);
}

static void print_stage_scores(const Player *player, int stage_count) {
    int has_stage_score = 0;
    int weakest_stage = -1;
    double weakest_accuracy = 101.0;

    for (int stage = 0; stage < stage_count; stage++) {
        int total = player->stage_correct_counts[stage] +
                    player->stage_miss_counts[stage] +
                    player->stage_input_error_counts[stage];

        if (total == 0) {
            continue;
        }

        has_stage_score = 1;
        break;
    }

    if (!has_stage_score) {
        return;
    }

    printf("\nステージ別成績:\n");
    printf("ST | 正解 | ミス | 失敗 | 命中率 | 最大連続\n");
    printf("---+------+------+------+--------+---------\n");

    for (int stage = 0; stage < stage_count; stage++) {
        int correct = player->stage_correct_counts[stage];
        int miss = player->stage_miss_counts[stage];
        int input_error = player->stage_input_error_counts[stage];
        int max_combo = player->stage_max_combo_counts[stage];
        int total = correct + miss + input_error;
        double accuracy;

        if (total == 0) {
            continue;
        }

        accuracy = ((double)correct / (double)total) * 100.0;
        printf("%2d | %4d | %4d | %4d | %6.1f%% | %8d\n",
               stage + 1,
               correct,
               miss,
               input_error,
               accuracy,
               max_combo);

        if (accuracy < weakest_accuracy) {
            weakest_accuracy = accuracy;
            weakest_stage = stage + 1;
        }
    }

    if (weakest_stage >= 1 && weakest_accuracy < 100.0) {
        printf("最低命中率  : 第%dステージ（%.1f%%）\n", weakest_stage, weakest_accuracy);
    } else {
        printf("最低命中率  : なし\n");
    }
}

static void gain_exp(Player *player, int exp) {
    player->exp += exp;
    printf("%sEXPを%d獲得した！%s\n", color(COLOR_MAGENTA), exp, color(COLOR_RESET));

    while (player->exp >= EXP_TO_LEVEL_UP) {
        player->exp -= EXP_TO_LEVEL_UP;
        player->level++;
        player->max_hp += 2;
        player->hp = player->max_hp;

        printf("%s【レベルアップ】Lv:%d 最大HPが%dになった！ HP全回復！%s\n",
               color(COLOR_GREEN),
               player->level,
               player->max_hp,
               color(COLOR_RESET));
    }

    printf("現在のEXP: %d/%d\n", player->exp, EXP_TO_LEVEL_UP);
}

static int prompt_next_stage(int next_stage_number, int stage_count) {
    char input[INPUT_BUFFER_SIZE];

    while (1) {
        printf("\n%s次の試練へ進みますか？ 第%d/%dステージ%s\n",
               color(COLOR_YELLOW),
               next_stage_number,
               stage_count,
               color(COLOR_RESET));
        printf("Enter: 進む / q: ここで剣を収める（記録済み）: ");

        if (fgets(input, INPUT_BUFFER_SIZE, stdin) == NULL) {
            printf("\n入力が途切れた。次は第%dステージの入口に刻まれた記録から再開する。\n",
                   next_stage_number);
            return 0;
        }

        input[strcspn(input, "\n")] = '\0';

        if (input[0] == '\0' ||
            strcmp(input, "next") == 0 ||
            strcmp(input, "NEXT") == 0) {
            return 1;
        }

        if (strcmp(input, "q") == 0 ||
            strcmp(input, "Q") == 0 ||
            strcmp(input, SAVE_COMMAND) == 0 ||
            strcmp(input, QUIT_COMMAND) == 0 ||
            strcmp(input, SAVE_QUIT_COMMAND) == 0) {
            printf("次は第%dステージの入口に刻まれた記録から再開する。\n", next_stage_number);
            return 0;
        }

        printf("道が定まらない。進むならEnter、剣を収めるならqを入力してください。\n");
    }
}

static const char *choose_target(const char *const words[], int word_count) {
    // 今回タイピングする課題をランダムに決定
    return words[rand() % word_count];
}

static const char *color(const char *code) {
    return getenv("NO_COLOR") == NULL ? code : "";
}

static void seed_random(void) {
    unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)clock();

    srand(seed);
}
