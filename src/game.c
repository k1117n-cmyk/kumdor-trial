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
#define NEXT_STAGE_QUIT 0
#define NEXT_STAGE_CONTINUE 1
#define NEXT_STAGE_MENU 2

typedef struct {
    const char *name;
    const char *lesson;
    const char *keys;
    const char *fingers;
    const char *tip;
    const char *const *targets;
    int target_count;
} PreStage;

typedef struct {
    int stage_number;
    const char *label;
} MainEntryPoint;

static void print_title(void);
static void print_prologue(void);
static int run_start_menu(int *start_stage, int stage_count);
static void print_how_to_play(void);
static void run_prestage_menu(void);
static int run_main_entry_menu(int stage_count);
static void prepare_entry_player(Player *player, int start_stage);
static int prompt_start_choice(void);
static int prompt_prestage_choice(void);
static void run_all_prestages(void);
static int run_prestage(const PreStage *prestage, int prestage_number);
static void print_prestage_intro(const PreStage *prestage, int prestage_number);
static void print_prestage_help(void);
static void print_key_map_row(const char *label, const char *const items[], const char *const column_colors[]);
static int read_line(char input[]);
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

static const char *const home_left_practice[] = {
    "asdfg",
    "gfdsa",
    "a s d f g",
    "ff gg",
    "sad",
    "fad",
    "adds",
    "gas"
};

static const char *const home_right_practice[] = {
    "hjkl;",
    ";lkjh",
    "h j k l ;",
    "hh jj",
    "jkl;",
    "hjj",
    "kkll",
    "j;h"
};

static const char *const home_both_practice[] = {
    "asdf hjkl;",
    "fdsa ;lkjh",
    "a s d f j k l ;",
    "ff jj",
    "sad ask",
    "fall flask",
    "jkl; asdf",
    "fj fj"
};

static const char *const top_left_practice[] = {
    "qwert",
    "trewq",
    "q w e r t",
    "rr tt",
    "qwe",
    "were",
    "tree",
    "tweet"
};

static const char *const top_right_practice[] = {
    "yuiop",
    "poiuy",
    "y u i o p",
    "yy uu",
    "you",
    "pop",
    "poi",
    "yip",
    "upup"
};

static const char *const top_both_practice[] = {
    "qwert yuiop",
    "trewq poiuy",
    "q w e r t y u i o p",
    "rr uu",
    "qwe you",
    "type",
    "power quiet",
    "qwerty"
};

static const char *const bottom_left_practice[] = {
    "zxcvb",
    "bvcxz",
    "z x c v b",
    "vv bb",
    "zxc",
    "zcv",
    "bvc",
    "zz bb"
};

static const char *const bottom_right_practice[] = {
    "nm,./",
    "/.,mn",
    "n m , . /",
    "nn mm",
    "m,.",
    "nmn",
    "m,m",
    "n/m"
};

static const char *const bottom_both_practice[] = {
    "zxcvb nm,./",
    "bvcxz /.,mn",
    "z x c v b n m , . /",
    "vv nn",
    "zxc m,.",
    "vbn",
    "mix.z",
    "cave/m"
};

static const char *const number_left_practice[] = {
    "12345",
    "54321",
    "1 2 3 4 5",
    "44 55",
    "123",
    "2024",
    "405",
    "515"
};

static const char *const number_right_practice[] = {
    "67890",
    "09876",
    "6 7 8 9 0",
    "66 77",
    "789",
    "9090",
    "808",
    "670"
};

static const char *const number_both_practice[] = {
    "12345 67890",
    "54321 09876",
    "1 2 3 4 5 6 7 8 9 0",
    "44 77",
    "123 789",
    "2024 808",
    "405 670",
    "515 9090"
};

static const PreStage prestages[] = {
    {
        "真ん中の列 左手",
        "ホーム段の左手だけを覚える",
        "a / s / d / f / g",
        "a小指 / s薬指 / d中指 / f人差し指 / g人差し指",
        "fの突起を出発点にして、左手だけで打つ。",
        home_left_practice,
        (int)(sizeof(home_left_practice) / sizeof(home_left_practice[0]))
    },
    {
        "真ん中の列 右手",
        "ホーム段の右手だけを覚える",
        "h / j / k / l / ;",
        "h人差し指 / j人差し指 / k中指 / l薬指 / ;小指",
        "jの突起を出発点にして、右手だけで打つ。",
        home_right_practice,
        (int)(sizeof(home_right_practice) / sizeof(home_right_practice[0]))
    },
    {
        "真ん中の列 両手",
        "ホーム段を両手で覚える",
        "a / s / d / f / g / h / j / k / l / ;",
        "a小指 / s薬指 / d中指 / f,g人差し指 / h,j人差し指 / k中指 / l薬指 / ;小指",
        "fとjを出発点にして、打ったあと両手をホーム段へ戻す。",
        home_both_practice,
        (int)(sizeof(home_both_practice) / sizeof(home_both_practice[0]))
    },
    {
        "上の列 左手",
        "上段の左手だけを覚える",
        "q / w / e / r / t",
        "q小指 / w薬指 / e中指 / r人差し指 / t人差し指",
        "左手ホーム段から上へ伸ばし、打ったあと戻す。",
        top_left_practice,
        (int)(sizeof(top_left_practice) / sizeof(top_left_practice[0]))
    },
    {
        "上の列 右手",
        "上段の右手だけを覚える",
        "y / u / i / o / p",
        "y人差し指 / u人差し指 / i中指 / o薬指 / p小指",
        "右手ホーム段から上へ伸ばし、打ったあと戻す。",
        top_right_practice,
        (int)(sizeof(top_right_practice) / sizeof(top_right_practice[0]))
    },
    {
        "上の列 両手",
        "上段を両手で覚える",
        "q / w / e / r / t / y / u / i / o / p",
        "q小指 / w薬指 / e中指 / r,t人差し指 / y,u人差し指 / i中指 / o薬指 / p小指",
        "ホーム段から上へ伸ばし、打ったあとfとjへ戻す。",
        top_both_practice,
        (int)(sizeof(top_both_practice) / sizeof(top_both_practice[0]))
    },
    {
        "下の列 左手",
        "下段の左手だけを覚える",
        "z / x / c / v / b",
        "z小指 / x薬指 / c中指 / v人差し指 / b人差し指",
        "左手だけを下へ下げ、打ったあとホーム段へ戻す。",
        bottom_left_practice,
        (int)(sizeof(bottom_left_practice) / sizeof(bottom_left_practice[0]))
    },
    {
        "下の列 右手",
        "下段の右手だけを覚える",
        "n / m / , / . / /",
        "n人差し指 / m中指 / ,薬指 / .薬指 / /小指",
        "右手だけを下へ下げ、打ったあとホーム段へ戻す。",
        bottom_right_practice,
        (int)(sizeof(bottom_right_practice) / sizeof(bottom_right_practice[0]))
    },
    {
        "下の列 両手",
        "下段を両手で覚える",
        "z / x / c / v / b / n / m / , / . / /",
        "z小指 / x薬指 / c中指 / v,b人差し指 / n人差し指 / m中指 / ,薬指 / .薬指 / /小指",
        "両手を下へ下げ、打ったあとホーム段へ戻す。",
        bottom_both_practice,
        (int)(sizeof(bottom_both_practice) / sizeof(bottom_both_practice[0]))
    },
    {
        "数字 左手",
        "数字段の左手だけを覚える",
        "1 / 2 / 3 / 4 / 5",
        "1小指 / 2薬指 / 3中指 / 4人差し指 / 5人差し指",
        "数字を打ったら、左手をホーム段へ戻す。",
        number_left_practice,
        (int)(sizeof(number_left_practice) / sizeof(number_left_practice[0]))
    },
    {
        "数字 右手",
        "数字段の右手だけを覚える",
        "6 / 7 / 8 / 9 / 0",
        "6人差し指 / 7人差し指 / 8中指 / 9薬指 / 0小指",
        "数字を打ったら、右手をホーム段へ戻す。",
        number_right_practice,
        (int)(sizeof(number_right_practice) / sizeof(number_right_practice[0]))
    },
    {
        "数字 両手",
        "数字段を両手で覚える",
        "1 / 2 / 3 / 4 / 5 / 6 / 7 / 8 / 9 / 0",
        "1小指 / 2薬指 / 3中指 / 4,5人差し指 / 6,7人差し指 / 8中指 / 9薬指 / 0小指",
        "数字を打ったら、両手をホーム段へ戻す。",
        number_both_practice,
        (int)(sizeof(number_both_practice) / sizeof(number_both_practice[0]))
    }
};

static const MainEntryPoint main_entry_points[] = {
    {1, "最初から"},
    {2, "ホーム段 左手・右手・両手"},
    {5, "上段 左手・右手・両手"},
    {8, "下段 左手・右手・両手"},
    {11, "数字 左手・右手・両手"},
    {14, "総合練習の入口"},
    {15, "全キー練習 前半"},
    {18, "全キー練習 後半"}
};

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
show_start_menu:
        player = create_player();
        start_stage = 0;
        quit_requested = 0;
        if (!run_start_menu(&start_stage, stage_count)) {
            printf("\n%s【終了】%sクムドールの試練を始めずに終了しました。\n",
                   color(COLOR_BLUE),
                   color(COLOR_RESET));
            return 0;
        }
        if (start_stage == 0) {
            print_prologue();
        } else {
            prepare_entry_player(&player, start_stage);
            printf("\n%s【旅の分岐】%s第%dステージから練習を始めます。\n\n",
                   color(COLOR_CYAN),
                   color(COLOR_RESET),
                   start_stage + 1);
        }
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
            if (stage + 1 < stage_count) {
                int next_choice = prompt_next_stage(stage + 2, stage_count);
                if (next_choice == NEXT_STAGE_MENU) {
                    stop_bgm();
                    printf("\n%s【最初のメニューへ戻る】%s記録は第%dステージの入口に刻まれています。\n\n",
                           color(COLOR_BLUE),
                           color(COLOR_RESET),
                           stage + 2);
                    goto show_start_menu;
                }
                if (next_choice == NEXT_STAGE_QUIT) {
                    quit_requested = 1;
                }
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
    printf("A.P.405年。地球でキーボード道場を営むあなたは、\n");
    printf("変わらない稽古の日々を離れ、ズロワノフ経路を抜けてソルフェスへ向かった。\n");
    printf("ミソル空港のホテルに着いた夜、クムドール王国からの使者が倒れ込み、こう告げる。\n");
    printf("「クムの木が石になり、チャトフィッシュも湖から消えました。王都の通信も途絶えています」\n");
    printf("あなたは王室船クム3号に乗り込む。目的地は小さな森林惑星、クムドール。\n");
    printf("だが着陸直前、計器は意味のない文字列を吐き出し、船は空から墜ちた。\n");
    printf("失ったキー、散らばったスパイス、残ったライフはひとつ。\n");
    printf("それでも、ありふれたキーボードはまだ手元にある。\n");
    printf("人々が後に「クムドールの剣」と呼ぶものは、ここから目を覚ます。\n\n");
}

static int run_start_menu(int *start_stage, int stage_count) {
    while (1) {
        int choice = prompt_start_choice();

        if (choice == 0) {
            print_how_to_play();
            continue;
        }

        if (choice == 1) {
            *start_stage = 0;
            return 1;
        }

        if (choice == 2) {
            run_prestage_menu();
            continue;
        }

        if (choice == 3) {
            int selected_stage = run_main_entry_menu(stage_count);
            if (selected_stage >= 0) {
                *start_stage = selected_stage;
                return 1;
            }
            continue;
        }

        if (choice == -2) {
            return 0;
        }

        printf("選択できる番号を入力してください。\n");
    }
}

static int run_main_entry_menu(int stage_count) {
    char input[INPUT_BUFFER_SIZE];
    int entry_count = (int)(sizeof(main_entry_points) / sizeof(main_entry_points[0]));
    char *endptr;
    long choice;

    while (1) {
        printf("\n%s【旅の分岐】%s\n", color(COLOR_CYAN), color(COLOR_RESET));
        printf("旅を再開したい区切りから本編を始められます。\n");
        for (int i = 0; i < entry_count; i++) {
            if (main_entry_points[i].stage_number <= stage_count) {
                printf("%d: 第%dステージから - %s\n",
                       i + 1,
                       main_entry_points[i].stage_number,
                       main_entry_points[i].label);
            }
        }
        printf("b: 最初のメニューへ戻る\n");
        printf("選択: ");

        if (!read_line(input)) {
            printf("\n入力が途切れたため、最初のメニューへ戻ります。\n");
            return -1;
        }

        if (strcmp(input, "b") == 0 ||
            strcmp(input, "B") == 0 ||
            strcmp(input, "back") == 0 ||
            strcmp(input, "BACK") == 0) {
            return -1;
        }

        choice = strtol(input, &endptr, 10);
        if (input[0] != '\0' && *endptr == '\0' && choice >= 1 && choice <= entry_count) {
            int selected_stage_number = main_entry_points[choice - 1].stage_number;
            if (selected_stage_number <= stage_count) {
                return selected_stage_number - 1;
            }
        }

        printf("選択できる番号を入力してください。\n");
    }
}

static void prepare_entry_player(Player *player, int start_stage) {
    int bonus_levels = start_stage / 3;

    player->level += bonus_levels;
    player->max_hp += bonus_levels * 2;
    player->hp = player->max_hp;
    player->reached_stage = start_stage + 1;
}

static void run_prestage_menu(void) {
    int choice;
    int prestage_count = (int)(sizeof(prestages) / sizeof(prestages[0]));

    start_random_prestage_bgm();

    printf("%s【ビッグハンド訓練場】%s\n", color(COLOR_CYAN), color(COLOR_RESET));
    printf("本編の前に、巨大な手の訓練装置でキーと指の対応を練習できます。ミスしてもHPは減りません。\n");
    printf("Enterで全部のレッスン、数字で個別レッスン、bで最初のメニューへ戻ります。\n");
    printf("メニュー中も :help / :bgm / :mute を使えます。\n");
    print_finger_key_map();

    while (1) {
        choice = prompt_prestage_choice();

        if (choice == 0) {
            printf("最初のメニューへ戻ります。\n\n");
            stop_bgm();
            return;
        }

        if (choice == -1) {
            run_all_prestages();
            stop_bgm();
            return;
        }

        if (choice == -2) {
            print_prestage_help();
            continue;
        }

        if (choice == -3) {
            toggle_bgm();
            continue;
        }

        if (choice >= 1 && choice <= prestage_count) {
            if (run_prestage(&prestages[choice - 1], choice)) {
                printf("\nビッグハンドのレッスンを終えました。最初のメニューへ戻ります。\n\n");
            } else {
                printf("\nビッグハンドのレッスンを中断しました。最初のメニューへ戻ります。\n\n");
            }
            stop_bgm();
            return;
        }

        printf("選択できる番号を入力してください。\n");
    }
}

static void print_how_to_play(void) {
    char input[INPUT_BUFFER_SIZE];

    printf("\n%s=========================================%s\n", color(COLOR_CYAN), color(COLOR_RESET));
    printf("%s【ゲームの遊び方】%s\n",
           color(COLOR_YELLOW),
           color(COLOR_RESET));
    printf("表示された課題を、そのまま正確に入力してEnterを押します。\n");
    printf("正解すると敵にダメージを与え、ミスすると敵の反撃を受けます。\n");
    printf("大文字、小文字、スペース、記号はすべて区別します。\n");
    printf("3回連続で正解すると追加の一撃が入ります。\n");
    printf("敵HPを0にするとステージ突破、あなたのHPが0になると敗北です。\n");
    printf("戦闘中は %s / %s / %s で中断やヘルプ表示ができます。\n",
           SAVE_COMMAND,
           QUIT_COMMAND,
           HELP_COMMAND);
    printf("指の位置に不安がある場合は、先にビッグハンド訓練場で練習できます。\n");
    printf("%s=========================================%s\n", color(COLOR_CYAN), color(COLOR_RESET));
    printf("Enterで最初のメニューへ戻る: ");
    read_line(input);
    printf("\n");
}

static int prompt_start_choice(void) {
    char input[INPUT_BUFFER_SIZE];

    printf("%s【最初のメニュー】%s\n", color(COLOR_CYAN), color(COLOR_RESET));
    printf("0: ゲームの遊び方\n");
    printf("1: ビッグハンド訓練場で練習する\n");
    printf("2: クムドールへ向かう\n");
    printf("3: 旅の途中から始める\n");
    printf("q: 終了\n");
    printf("選択: ");

    if (!read_line(input)) {
        printf("\n入力が途切れたため、終了します。\n");
        return -2;
    }

    if (input[0] == '\0' || strcmp(input, "2") == 0) {
        return 1;
    }

    if (strcmp(input, "0") == 0) {
        return 0;
    }

    if (strcmp(input, "1") == 0) {
        return 2;
    }

    if (strcmp(input, "3") == 0) {
        return 3;
    }

    if (strcmp(input, "q") == 0 ||
        strcmp(input, "Q") == 0 ||
        strcmp(input, QUIT_COMMAND) == 0) {
        return -2;
    }

    return -1;
}

static int prompt_prestage_choice(void) {
    char input[INPUT_BUFFER_SIZE];
    int prestage_count = (int)(sizeof(prestages) / sizeof(prestages[0]));
    char *endptr;
    long choice;

    printf("\n");
    printf("Enter: 全部レッスン / b: 戻る / :help: ヘルプ / :bgm: BGM切替\n");
    for (int i = 0; i < prestage_count; i++) {
        printf("%d: %s\n", i + 1, prestages[i].name);
    }
    printf("選択: ");

    if (!read_line(input)) {
        printf("\n入力が途切れたため、最初のメニューへ戻ります。\n");
        return 0;
    }

    if (input[0] == '\0') {
        return -1;
    }

    if (strcmp(input, HELP_COMMAND) == 0 ||
        strcmp(input, COMMANDS_COMMAND) == 0 ||
        strcmp(input, SHORT_HELP_COMMAND) == 0) {
        return -2;
    }

    if (strcmp(input, BGM_COMMAND) == 0 || strcmp(input, MUTE_COMMAND) == 0) {
        return -3;
    }

    if (strcmp(input, "b") == 0 ||
        strcmp(input, "B") == 0 ||
        strcmp(input, "back") == 0 ||
        strcmp(input, "BACK") == 0) {
        return 0;
    }

    choice = strtol(input, &endptr, 10);
    if (input[0] != '\0' && *endptr == '\0') {
        if (choice >= 1 && choice <= prestage_count) {
            return (int)choice;
        }
        return prestage_count + 1;
    }

    return prestage_count + 1;
}

static void run_all_prestages(void) {
    int prestage_count = (int)(sizeof(prestages) / sizeof(prestages[0]));

    for (int i = 0; i < prestage_count; i++) {
        if (!run_prestage(&prestages[i], i + 1)) {
            printf("\nビッグハンドのレッスンを中断しました。最初のメニューへ戻ります。\n\n");
            return;
        }
    }

    printf("\nすべてのビッグハンドレッスンを終えました。最初のメニューへ戻ります。\n\n");
}

static int run_prestage(const PreStage *prestage, int prestage_number) {
    char input[INPUT_BUFFER_SIZE];
    int correct_count = 0;
    int miss_count = 0;

    print_prestage_intro(prestage, prestage_number);

    for (int i = 0; i < prestage->target_count; i++) {
        const char *target = prestage->targets[i];
        int remaining_count = prestage->target_count - i;

        while (1) {
            printf("%s[ビッグハンド %d/%d]%s %s%s%s\n",
                   color(COLOR_YELLOW),
                   remaining_count,
                   prestage->target_count,
                   color(COLOR_RESET),
                   color(COLOR_YELLOW),
                   target,
                   color(COLOR_RESET));
            printf("同じ文字を入力してEnter（:q終了 / :helpでヘルプ）: ");

            if (!read_line(input)) {
                printf("\n入力が途切れたため、この練習を終了します。\n");
                return 0;
            }

            if (strcmp(input, ":q") == 0 ||
                strcmp(input, ":Q") == 0 ||
                strcmp(input, "q") == 0 ||
                strcmp(input, "Q") == 0) {
                printf("このビッグハンドレッスンを終了します。\n");
                return 0;
            }

            if (strcmp(input, HELP_COMMAND) == 0 ||
                strcmp(input, COMMANDS_COMMAND) == 0 ||
                strcmp(input, SHORT_HELP_COMMAND) == 0) {
                print_prestage_help();
                continue;
            }

            if (strcmp(input, BGM_COMMAND) == 0 || strcmp(input, MUTE_COMMAND) == 0) {
                toggle_bgm();
                continue;
            }

            if (strcmp(input, target) == 0) {
                correct_count++;
                printf("%sOK%s ビッグハンドがうなずいた。指の位置を保ったまま次へ進みます。\n",
                       color(COLOR_GREEN),
                       color(COLOR_RESET));
                break;
            }

            miss_count++;
            printf("%sもう一度。%s ビッグハンドが同じ課題を差し出した。ゆっくり同じ順番で打ちます。\n",
                   color(COLOR_RED),
                   color(COLOR_RESET));
        }
    }

    printf("%s【%s 完了】%s 正解 %d / やり直し %d\n",
           color(COLOR_GREEN),
           prestage->name,
           color(COLOR_RESET),
           correct_count,
           miss_count);
    return 1;
}

static void print_prestage_intro(const PreStage *prestage, int prestage_number) {
    printf("\n%s=========================================%s\n", color(COLOR_CYAN), color(COLOR_RESET));
    printf("%s【ビッグハンド レッスン%d: %s】%s\n",
           color(COLOR_YELLOW),
           prestage_number,
           prestage->name,
           color(COLOR_RESET));
    printf("練習内容: %s\n", prestage->lesson);
    printf("訓練装置: ビッグハンド\n");
    printf("今回のキー: %s\n", prestage->keys);
    printf("担当指  : %s\n", prestage->fingers);
    printf("指使い  : %s\n", prestage->tip);
    print_finger_key_map();
    printf("%s=========================================%s\n", color(COLOR_CYAN), color(COLOR_RESET));
}

static void print_prestage_help(void) {
    printf("\n%s【ビッグハンド ヘルプ】%s\n", color(COLOR_CYAN), color(COLOR_RESET));
    printf("ビッグハンドが差し出す練習課題を、そのまま正確に入力してEnterを押します。\n");
    printf("ミスしてもHPは減らず、ビッグハンドが同じ課題をもう一度出します。\n");
    printf("練習カウントは残り数です。正解すると 8/8 から 7/8 のように減っていきます。\n");
    printf("メニューではEnterで全部レッスン、数字で個別レッスン、bで最初のメニューへ戻ります。\n");
    printf("コマンド:\n");
    printf("  %-10s このビッグハンドレッスンを終了して最初のメニューへ戻る\n", ":q");
    printf("  %-10s このヘルプを表示\n", HELP_COMMAND);
    printf("  %-10s BGMのON/OFFを切り替え（KUMDOR_NO_BGM=1では無効）\n", BGM_COMMAND);
    printf("  %-10s BGMのON/OFFを切り替え（KUMDOR_NO_BGM=1では無効）\n\n", MUTE_COMMAND);
}

void print_finger_key_map(void) {
    const char *const column_colors[] = {
        COLOR_RED,
        COLOR_YELLOW,
        COLOR_GREEN,
        COLOR_CYAN,
        COLOR_CYAN,
        COLOR_CYAN,
        COLOR_CYAN,
        COLOR_GREEN,
        COLOR_YELLOW,
        COLOR_RED
    };
    const char *const fingers[] = {"L5", "L4", "L3", "L2", "L2", "R2", "R2", "R3", "R4", "R5"};
    const char *const numbers[] = {"[1]", "[2]", "[3]", "[4]", "[5]", "[6]", "[7]", "[8]", "[9]", "[0]"};
    const char *const top[] = {"[q]", "[w]", "[e]", "[r]", "[t]", "[y]", "[u]", "[i]", "[o]", "[p]"};
    const char *const middle[] = {"[a]", "[s]", "[d]", "[f]", "[g]", "[h]", "[j]", "[k]", "[l]", "[;]"};
    const char *const bottom[] = {"[z]", "[x]", "[c]", "[v]", "[b]", "[n]", "[m]", "[,]", "[.]", "[/]"};

    printf("\n%s【指とキーの対応】%s\n", color(COLOR_CYAN), color(COLOR_RESET));
    print_key_map_row("指", fingers, column_colors);
    print_key_map_row("数", numbers, column_colors);
    print_key_map_row("上", top, column_colors);
    print_key_map_row("中", middle, column_colors);
    print_key_map_row("下", bottom, column_colors);
    printf("親指:                 %s[space]%s\n", color(COLOR_MAGENTA), color(COLOR_RESET));
    printf("目印: f=左人差し指の出発点 / j=右人差し指の出発点\n");
    printf("凡例: %sL5/R5小指%s %sL4/R4薬指%s %sL3/R3中指%s %sL2/R2人差し指%s / %s親指%s\n",
           color(COLOR_RED),
           color(COLOR_RESET),
           color(COLOR_YELLOW),
           color(COLOR_RESET),
           color(COLOR_GREEN),
           color(COLOR_RESET),
           color(COLOR_CYAN),
           color(COLOR_RESET),
           color(COLOR_MAGENTA),
           color(COLOR_RESET));
}

static void print_key_map_row(const char *label, const char *const items[], const char *const column_colors[]) {
    printf("%s:  ", label);
    for (int i = 0; i < 10; i++) {
        if (i == 9) {
            printf("%s%s%s", color(column_colors[i]), items[i], color(COLOR_RESET));
        } else {
            printf("%s%-4s%s", color(column_colors[i]), items[i], color(COLOR_RESET));
        }
        if (i == 4) {
            printf("   ");
        }
    }
    printf("\n");
}

static int read_line(char input[]) {
    if (fgets(input, INPUT_BUFFER_SIZE, stdin) == NULL) {
        return 0;
    }

    input[strcspn(input, "\n")] = '\0';
    return 1;
}

Player create_player(void) {
    Player player = {0};

    player.name = "あなた";
    player.hp = 10;
    player.max_hp = 10;
    player.status = STATUS_NORMAL;
    player.poison_turns_remaining = 0;
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
        player->poison_turns_remaining = 0;
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
    printf("       ______________________\n");
    printf("      /  KUMDOR TRAVEL PASS  \\\n");
    printf("     /________________________\\\n");
    printf("     |  []  []  []  []  []   |\n");
    printf("     |    next key ahead     |\n");
    printf("     |_______________________|\n");
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
    printf("      _____________\n");
    printf("  ___/ KUM-3  ____ \\___\n");
    printf(" / _  _  _  _/   \\_  _ \\\n");
    printf("|_[ ]_[ ]_[ ]_ %s _[ ]_|\n", cargo);
    printf("   O           O\n");
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
    printf("石化していたクムの木は緑を取り戻し、湖面にはチャトフィッシュの声が戻る。\n");
    printf("王都の通信塔にも光が戻り、巨大な手の影は大地から消えていった。\n");
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
        printf("Enter: 進む / b: 最初のメニューへ戻る / q: ここで剣を収める（記録済み）: ");

        if (fgets(input, INPUT_BUFFER_SIZE, stdin) == NULL) {
            printf("\n入力が途切れた。次は第%dステージの入口に刻まれた記録から再開する。\n",
                   next_stage_number);
            return NEXT_STAGE_QUIT;
        }

        input[strcspn(input, "\n")] = '\0';

        if (input[0] == '\0' ||
            strcmp(input, "next") == 0 ||
            strcmp(input, "NEXT") == 0) {
            return NEXT_STAGE_CONTINUE;
        }

        if (strcmp(input, "b") == 0 ||
            strcmp(input, "B") == 0 ||
            strcmp(input, "back") == 0 ||
            strcmp(input, "BACK") == 0) {
            return NEXT_STAGE_MENU;
        }

        if (strcmp(input, "q") == 0 ||
            strcmp(input, "Q") == 0 ||
            strcmp(input, SAVE_COMMAND) == 0 ||
            strcmp(input, QUIT_COMMAND) == 0 ||
            strcmp(input, SAVE_QUIT_COMMAND) == 0) {
            printf("次は第%dステージの入口に刻まれた記録から再開する。\n", next_stage_number);
            return NEXT_STAGE_QUIT;
        }

        printf("道が定まらない。進むならEnter、戻るならb、剣を収めるならqを入力してください。\n");
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
