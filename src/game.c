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
static void print_stage_climax(const Stage *stage);
static void print_enemy_quote(const Stage *stage);
static void print_ending(const Player *player);
static void print_score(const Player *player, int stage_count);
static void print_stage_scores(const Player *player, int stage_count);
static void gain_exp(Player *player, int exp);
static int prompt_next_stage(int next_stage_number, int stage_count);
static const char *choose_target(const char *const words[], int word_count);
static const char *color(const char *code);

int run_game(void) {
    // 乱数の初期化
    srand((unsigned int)time(NULL));
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
            if (!player_turn(&player, &enemy, target, stage, stage_count, climax_started)) {
                quit_requested = 1;
            }

            printf("-----------------------------------------\n");
        }

        if (player.hp > 0 && !quit_requested) {
            print_stage_clear(stage + 1, &stages[stage], &player);
            stop_bgm();
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

    print_score(&player, stage_count);

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
    printf("指使い  : %s\n", stage->tip);
    printf("敵      : %s%s%s\n", color(COLOR_RED), stage->enemy.name, color(COLOR_RESET));
    printf("%s=========================================%s\n", color(COLOR_CYAN), color(COLOR_RESET));
}

static void print_stage_clear(int stage_number, const Stage *stage, Player *player) {
    printf("\n%s=========================================%s\n", color(COLOR_CYAN), color(COLOR_RESET));
    printf("%s【ステージ%d突破】%s\n", color(COLOR_GREEN), stage_number, color(COLOR_RESET));
    printf("%s%sを倒した！%s\n", color(COLOR_GREEN), stage->enemy.name, color(COLOR_RESET));
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
    int total = correct + miss + input_error;
    double accuracy = 0.0;

    if (total > 0) {
        accuracy = ((double)correct / (double)total) * 100.0;
    }

    printf("ステージ成績: 正解 %d / ミス %d / 入力失敗 %d / 命中率 %.1f%%\n",
           correct,
           miss,
           input_error,
           accuracy);
    printf("%s\n", stage->clear_story);
    printf("%s=========================================%s\n", color(COLOR_CYAN), color(COLOR_RESET));
}

static void print_stage_climax(const Stage *stage) {
    if (stage->climax_message == NULL || stage->climax_message[0] == '\0') {
        return;
    }

    printf("\n%s【敵の殺気】%s%s\n", color(COLOR_YELLOW), color(COLOR_RESET), stage->climax_message);
    printf("空気が張りつめる。敵は反撃の機会を狙い、構えも鋭く変わった。\n");
}

static void print_enemy_quote(const Stage *stage) {
    if (stage->enemy_quote == NULL || stage->enemy_quote[0] == '\0') {
        return;
    }

    printf("%s%s%s\n\n", color(COLOR_MAGENTA), stage->enemy_quote, color(COLOR_RESET));
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

static void print_score(const Player *player, int stage_count) {
    int total_inputs = player->correct_count + player->miss_count + player->input_error_count;
    double accuracy = 0.0;

    if (total_inputs > 0) {
        accuracy = ((double)player->correct_count / (double)total_inputs) * 100.0;
    }

    printf("\n%s=========================================%s\n", color(COLOR_CYAN), color(COLOR_RESET));
    printf("%s【スコア】%s\n", color(COLOR_MAGENTA), color(COLOR_RESET));
    printf("到達ステージ: %d/%d\n", player->reached_stage, stage_count);
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
    printf("ST | 正解 | ミス | 失敗 | 命中率\n");
    printf("---+------+------+------+--------\n");

    for (int stage = 0; stage < stage_count; stage++) {
        int correct = player->stage_correct_counts[stage];
        int miss = player->stage_miss_counts[stage];
        int input_error = player->stage_input_error_counts[stage];
        int total = correct + miss + input_error;
        double accuracy;

        if (total == 0) {
            continue;
        }

        accuracy = ((double)correct / (double)total) * 100.0;
        printf("%2d | %4d | %4d | %4d | %6.1f%%\n",
               stage + 1,
               correct,
               miss,
               input_error,
               accuracy);

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
        printf("Enter: 進む / q: ここで終了（セーブ済み）: ");

        if (fgets(input, INPUT_BUFFER_SIZE, stdin) == NULL) {
            printf("\n入力が読み取れなかったため、ここで終了します。第%dステージから再開できます。\n",
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
            printf("第%dステージから再開できます。\n", next_stage_number);
            return 0;
        }

        printf("入力が不明です。Enterかqを入力してください。\n");
    }
}

static const char *choose_target(const char *const words[], int word_count) {
    // 今回タイピングする課題をランダムに決定
    return words[rand() % word_count];
}

static const char *color(const char *code) {
    return getenv("NO_COLOR") == NULL ? code : "";
}
