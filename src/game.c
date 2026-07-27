#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void print_title(void);
static void print_prologue(void);
static void apply_opening_status(Player *player);
static void print_stage_transition(int next_stage_number, int stage_count);
static void print_stage_intro(int stage_number, int stage_count, const Stage *stage);
static void print_stage_clear(int stage_number, const Stage *stage, Player *player);
static void print_ending(const Player *player);
static void print_battle_start(const Enemy *enemy);
static void print_battle_status(const Player *player, const Enemy *enemy);
static void print_score(const Player *player, int stage_count);
static const char *status_name(unsigned char status);
static void gain_exp(Player *player, int exp);
static int load_game(Player *player, int *start_stage, int stage_count);
static int save_game(const Player *player, int next_stage, int stage_count);
static int prompt_next_stage(int next_stage_number, int stage_count);
static const char *choose_target(const char *const words[], int word_count);
static int read_input(char input[]);
static int is_correct_input(const char input[], const char target[]);
static int player_turn(Player *player, Enemy *enemy, const char target[], int current_stage, int stage_count);
static void enemy_turn(Player *player, const Enemy *enemy);

int run_game(void) {
    // 乱数の初期化
    srand((unsigned int)time(NULL));

    Player player = {"あなた", 10, 10, STATUS_NORMAL, 0, 0, 0, 0, 1, 0};
    int stage_count = 0;
    const Stage *stages = get_stages(&stage_count);
    int start_stage = 0;
    int quit_requested = 0;

    print_title();
    if (!load_game(&player, &start_stage, stage_count)) {
        print_prologue();
        apply_opening_status(&player);
    }

    for (int stage = start_stage; stage < stage_count && player.hp > 0 && !quit_requested; stage++) {
        Enemy enemy = stages[stage].enemy;

        player.reached_stage = stage + 1;
        if (stage > start_stage) {
            print_stage_transition(stage + 1, stage_count);
        }
        print_stage_intro(stage + 1, stage_count, &stages[stage]);
        print_battle_start(&enemy);

        // メインゲームループ
        while (player.hp > 0 && enemy.hp > 0 && !quit_requested) {
            const char *target = choose_target(stages[stage].words, stages[stage].word_count);

            print_battle_status(&player, &enemy);
            if (!player_turn(&player, &enemy, target, stage, stage_count)) {
                quit_requested = 1;
            }

            printf("-----------------------------------------\n");
        }

        if (player.hp > 0 && !quit_requested) {
            print_stage_clear(stage + 1, &stages[stage], &player);
            save_game(&player, stage + 1, stage_count);
            if (stage + 1 < stage_count && !prompt_next_stage(stage + 2, stage_count)) {
                quit_requested = 1;
            }
        }
    }

    if (quit_requested) {
        printf("\n【終了】クムドールの試練を中断しました。\n");
    } else if (player.hp > 0) {
        print_ending(&player);
    } else {
        printf("\n【敗北】%sは膝をついた……。\n", player.name);
        printf("もう一度挑戦して、クムドールの試練を突破しよう！\n");
    }

    print_score(&player, stage_count);

    return 0;
}

static void print_title(void) {
    printf("=========================================\n");
    printf("     タイピングRPG ★ クムドールの試練     \n");
    printf("                  %s                  \n", GAME_VERSION);
    printf("=========================================\n");
    printf("現れた課題をそのまま正確にタイプして、敵を倒せ！\n\n");
}

static void print_prologue(void) {
    printf("=========================================\n");
    printf("【プロローグ】\n\n");
    printf("ポーラ暦405年。銀河で名を知られたキーボード使いのあなたは、\n");
    printf("退屈を振り払うように、移民惑星ソルフェスへ向かった。\n");
    printf("だが到着した夜、クムドール王国からの使者が倒れ込み、こう告げる。\n");
    printf("「クムの森が石になり、王都との通信が途絶えました。どうか来てください」\n");
    printf("あなたは自動操縦船クム3号に乗り込む。目的地は小さな森林惑星、クムドール。\n");
    printf("しかし着陸直前、船の計器は意味のない文字列を吐き出し、空から墜ちた。\n");
    printf("失ったキー、散らばったスパイス、残ったライフはひとつ。\n");
    printf("それでも、指はまだホームポジションを覚えている。\n\n");
}

static void apply_opening_status(Player *player) {
    // 敵の先制攻撃！ランダムで状態異常フラグを立てる
    if (rand() % 2 == 0) {
        player->status |= STATUS_POISON;
        printf("[警告] 墜落跡に残った毒霧を吸い込み【毒】状態になった！（次の正解で毒を解除）\n");
    } else {
        player->status |= STATUS_BLIND;
        printf("[警告] 残骸から立ち上がった魔霧で【暗闇】状態になった！（視界が悪くなる）\n");
    }
}

static void print_stage_transition(int next_stage_number, int stage_count) {
    printf("\n-----------------------------------------\n");
    printf("次の試練へ進む: 第%d/%dステージ\n", next_stage_number, stage_count);
    printf("-----------------------------------------\n");
}

static void print_stage_intro(int stage_number, int stage_count, const Stage *stage) {
    printf("\n=========================================\n");
    printf("【第%d/%dステージ】\n", stage_number, stage_count);
    printf("場所    : %s\n", stage->place);
    printf("%s\n", stage->story);
    printf("練習内容: %s\n", stage->lesson);
    printf("指使い  : %s\n", stage->tip);
    printf("敵      : %s\n", stage->enemy.name);
    printf("=========================================\n");
}

static void print_stage_clear(int stage_number, const Stage *stage, Player *player) {
    printf("\n=========================================\n");
    printf("【ステージ%d突破】\n", stage_number);
    printf("%sを倒した！\n", stage->enemy.name);
    gain_exp(player, stage->exp_reward);

    if (stage->reward_heal > 0 && player->hp < player->max_hp) {
        player->hp += stage->reward_heal;
        if (player->hp > player->max_hp) {
            player->hp = player->max_hp;
        }

        printf("%sでHPが%d回復した！ (あなたのHP: %d/%d)\n",
               stage->reward_name,
               stage->reward_heal,
               player->hp,
               player->max_hp);
    }

    printf("%s\n", stage->clear_story);
    printf("=========================================\n");
}

static void print_ending(const Player *player) {
    printf("\n【完全勝利】\n");
    printf("%sが最後の課題を打ち抜いた瞬間、クムドールの剣がまばゆく輝いた！\n",
           player->name);
    printf("石化していたクムの森は緑を取り戻し、王都の通信塔にも光が戻る。\n");
    printf("女王マルクァ・ランドは、あなたを王国の救い手として迎えた。\n");
    printf("けれど本当の報酬は、視線を落とさず打ち切ったその両手に残っている。\n");
    printf("あなたのタイピングスキルがレベルアップした！\n");
}

static void print_battle_start(const Enemy *enemy) {
    printf("\n*************** BATTLE START ***************\n");
    printf("%sが立ちはだかった！\n", enemy->name);
    printf("********************************************\n\n");
}

static void print_battle_status(const Player *player, const Enemy *enemy) {
    printf("%s Lv:%d EXP:%d/%d HP: %d/%d 状態:%s | %s HP: %d/%d\n",
           player->name,
           player->level,
           player->exp,
           EXP_TO_LEVEL_UP,
           player->hp,
           player->max_hp,
           status_name(player->status),
           enemy->name,
           enemy->hp,
           enemy->max_hp);
}

static void print_score(const Player *player, int stage_count) {
    int total_inputs = player->correct_count + player->miss_count + player->input_error_count;
    double accuracy = 0.0;

    if (total_inputs > 0) {
        accuracy = ((double)player->correct_count / (double)total_inputs) * 100.0;
    }

    printf("\n=========================================\n");
    printf("【スコア】\n");
    printf("到達ステージ: %d/%d\n", player->reached_stage, stage_count);
    printf("レベル      : %d\n", player->level);
    printf("経験値      : %d/%d\n", player->exp, EXP_TO_LEVEL_UP);
    printf("正解数      : %d\n", player->correct_count);
    printf("ミス数      : %d\n", player->miss_count);
    printf("入力失敗    : %d\n", player->input_error_count);
    printf("命中率      : %.1f%%\n", accuracy);

    if (accuracy >= 95.0 && player->reached_stage == stage_count && player->hp > 0) {
        printf("評価        : 銀河級のキーボード使い\n");
    } else if (accuracy >= 80.0) {
        printf("評価        : 安定した剣さばき\n");
    } else if (accuracy >= 60.0) {
        printf("評価        : まずは正確さを固めよう\n");
    } else {
        printf("評価        : ホームポジションから再挑戦\n");
    }

    printf("=========================================\n");
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

static void gain_exp(Player *player, int exp) {
    player->exp += exp;
    printf("EXPを%d獲得した！\n", exp);

    while (player->exp >= EXP_TO_LEVEL_UP) {
        player->exp -= EXP_TO_LEVEL_UP;
        player->level++;
        player->max_hp += 2;
        player->hp = player->max_hp;

        printf("【レベルアップ】Lv:%d 最大HPが%dになった！ HP全回復！\n",
               player->level,
               player->max_hp);
    }

    printf("現在のEXP: %d/%d\n", player->exp, EXP_TO_LEVEL_UP);
}

static int load_game(Player *player, int *start_stage, int stage_count) {
    FILE *file = fopen(SAVE_FILE, "r");
    char answer[INPUT_BUFFER_SIZE];
    char header[INPUT_BUFFER_SIZE];
    int next_stage;
    int loaded_status;
    Player loaded_player = {"あなた", 10, 10, STATUS_NORMAL, 0, 0, 0, 0, 1, 0};

    if (file == NULL) {
        return 0;
    }

    printf("セーブデータが見つかりました。ロードしますか？ (y/n): ");
    if (fgets(answer, INPUT_BUFFER_SIZE, stdin) == NULL || (answer[0] != 'y' && answer[0] != 'Y')) {
        fclose(file);
        printf("新しく始めます。\n");
        return 0;
    }

    if (fscanf(file,
               "%63s %d %d %d %d %d %d %d %d %d %d",
               header,
               &next_stage,
               &loaded_player.hp,
               &loaded_player.max_hp,
               &loaded_status,
               &loaded_player.correct_count,
               &loaded_player.miss_count,
               &loaded_player.input_error_count,
               &loaded_player.reached_stage,
               &loaded_player.level,
               &loaded_player.exp) != 11) {
        fclose(file);
        printf("セーブデータを読み取れませんでした。新しく始めます。\n");
        return 0;
    }

    fclose(file);

    if (strcmp(header, "KUMDOR_SAVE_V1") != 0 ||
        next_stage < 0 ||
        next_stage > stage_count ||
        loaded_player.max_hp <= 0 ||
        loaded_player.hp < 0 ||
        loaded_player.hp > loaded_player.max_hp ||
        loaded_player.level <= 0 ||
        loaded_player.exp < 0 ||
        loaded_player.exp >= EXP_TO_LEVEL_UP) {
        printf("セーブデータの内容が不正です。新しく始めます。\n");
        return 0;
    }

    if (next_stage >= stage_count) {
        printf("セーブデータはクリア済みです。新しく始めます。\n");
        return 0;
    }

    loaded_player.status = (unsigned char)loaded_status;
    *player = loaded_player;
    *start_stage = next_stage;

    printf("セーブデータをロードしました。第%dステージから再開します。\n", *start_stage + 1);
    return 1;
}

static int save_game(const Player *player, int next_stage, int stage_count) {
    FILE *file = fopen(SAVE_FILE, "w");

    if (file == NULL) {
        printf("[警告] セーブデータを書き込めませんでした。\n");
        return 0;
    }

    fprintf(file,
            "KUMDOR_SAVE_V1 %d %d %d %u %d %d %d %d %d %d\n",
            next_stage,
            player->hp,
            player->max_hp,
            (unsigned int)player->status,
            player->correct_count,
            player->miss_count,
            player->input_error_count,
            player->reached_stage,
            player->level,
            player->exp);

    fclose(file);

    if (next_stage < stage_count) {
        printf("[セーブ] 第%dステージから再開できます。\n", next_stage + 1);
    } else {
        printf("[セーブ] 完全勝利の記録を保存しました。\n");
    }

    return 1;
}

static int prompt_next_stage(int next_stage_number, int stage_count) {
    char input[INPUT_BUFFER_SIZE];

    while (1) {
        printf("\n次の試練へ進みますか？ 第%d/%dステージ\n", next_stage_number, stage_count);
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

static int read_input(char input[]) {
    printf("課題を入力してEnter（:save/:quit/:savequit）: ");
    if (fgets(input, INPUT_BUFFER_SIZE, stdin) == NULL) {
        return 0;
    }

    input[strcspn(input, "\n")] = '\0';
    return 1;
}

static int is_correct_input(const char input[], const char target[]) {
    return strcmp(input, target) == 0;
}

static int player_turn(Player *player, Enemy *enemy, const char target[], int current_stage, int stage_count) {
    char input[INPUT_BUFFER_SIZE];

    // 暗闇フラグ（ビット演算）のチェック
    if (player->status & STATUS_BLIND) {
        printf("敵の構え: [ %s ] (視界が悪い！正確に打ち込め！)\n", target);
    } else {
        printf("敵の構え: [ %s ]\n", target);
    }

    if (!read_input(input)) {
        printf("➔ 入力が読み取れなかった！ 反撃を受ける！\n");
        player->input_error_count++;
        enemy_turn(player, enemy);
        return 1;
    }

    if (strcmp(input, SAVE_COMMAND) == 0) {
        save_game(player, current_stage, stage_count);
        printf("➔ セーブしました。現在のステージの先頭から再開できます。\n");
        return 1;
    }

    if (strcmp(input, QUIT_COMMAND) == 0) {
        printf("➔ 保存せずに終了します。\n");
        return 0;
    }

    if (strcmp(input, SAVE_QUIT_COMMAND) == 0) {
        save_game(player, current_stage, stage_count);
        printf("➔ セーブして終了します。\n");
        return 0;
    }

    // 課題が一致しているか判定
    if (is_correct_input(input, target)) {
        player->correct_count++;

        // 毒フラグ（ビット演算）のチェック
        if (player->status & STATUS_POISON) {
            printf("➔ 毒のせいで攻撃が届かない！\n");
            printf("（毒は消えた。もう一度課題を的中させろ！）\n");
            player->status &= ~STATUS_POISON; // 毒を解除
        } else {
            enemy->hp--;
            printf("➔ 見事なタイピング！ 剣が炸裂した！ (敵の残りHP: %d)\n", enemy->hp);
        }
    } else {
        player->miss_count++;
        printf("➔ ミス！ 手元が狂った！（反撃を受ける！）\n");
        enemy_turn(player, enemy);
    }

    return 1;
}

static void enemy_turn(Player *player, const Enemy *enemy) {
    player->hp -= enemy->attack;

    if (player->hp < 0) {
        player->hp = 0;
    }

    printf("➔ %sの反撃！ %dダメージを受けた！ (あなたの残りHP: %d)\n",
           enemy->name,
           enemy->attack,
           player->hp);
}
