#include "save.h"

#include <stdio.h>
#include <string.h>

#define SAVE_FORMAT_HEADER_V1 "KUMDOR_SAVE_V1"
#define SAVE_FORMAT_HEADER_V2 "KUMDOR_SAVE_V2"
#define SAVE_FORMAT_HEADER SAVE_FORMAT_HEADER_V2
#define VALID_STATUS_MASK (STATUS_POISON | STATUS_BLIND)

int load_game(Player *player, int *start_stage, int stage_count) {
    FILE *file = fopen(SAVE_FILE, "r");
    char answer[INPUT_BUFFER_SIZE];
    char header[INPUT_BUFFER_SIZE];
    int next_stage;
    int loaded_status;
    Player loaded_player = create_player();

    if (file == NULL) {
        return 0;
    }

    if (stage_count > MAX_STAGE_COUNT) {
        fclose(file);
        printf("ステージ数がスコア記録の上限を超えています。新しく始めます。\n");
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

    if (strcmp(header, SAVE_FORMAT_HEADER_V2) == 0) {
        for (int stage = 0; stage < MAX_STAGE_COUNT; stage++) {
            if (fscanf(file,
                       "%d %d %d",
                       &loaded_player.stage_correct_counts[stage],
                       &loaded_player.stage_miss_counts[stage],
                       &loaded_player.stage_input_error_counts[stage]) != 3) {
                fclose(file);
                printf("セーブデータを読み取れませんでした。新しく始めます。\n");
                return 0;
            }
        }
    } else if (strcmp(header, SAVE_FORMAT_HEADER_V1) != 0) {
        fclose(file);
        printf("セーブデータの形式が不明です。新しく始めます。\n");
        return 0;
    }

    fclose(file);

    if (next_stage < 0 ||
        next_stage > stage_count ||
        loaded_player.max_hp <= 0 ||
        loaded_player.hp < 0 ||
        loaded_player.hp > loaded_player.max_hp ||
        loaded_status < 0 ||
        (loaded_status & ~VALID_STATUS_MASK) != 0 ||
        loaded_player.correct_count < 0 ||
        loaded_player.miss_count < 0 ||
        loaded_player.input_error_count < 0 ||
        loaded_player.reached_stage < 0 ||
        loaded_player.reached_stage > stage_count ||
        loaded_player.level <= 0 ||
        loaded_player.exp < 0 ||
        loaded_player.exp >= EXP_TO_LEVEL_UP) {
        printf("セーブデータの内容が不正です。新しく始めます。\n");
        return 0;
    }

    for (int stage = 0; stage < MAX_STAGE_COUNT; stage++) {
        if (loaded_player.stage_correct_counts[stage] < 0 ||
            loaded_player.stage_miss_counts[stage] < 0 ||
            loaded_player.stage_input_error_counts[stage] < 0) {
            printf("セーブデータのステージ別成績が不正です。新しく始めます。\n");
            return 0;
        }
    }

    if (next_stage >= stage_count) {
        printf("セーブデータはクリア済みです。新しく始めます。\n");
        return 0;
    }

    loaded_player.status = STATUS_NORMAL;
    *player = loaded_player;
    *start_stage = next_stage;

    printf("セーブデータをロードしました。第%dステージから再開します。\n", *start_stage + 1);
    return 1;
}

int save_game(const Player *player, int next_stage, int stage_count) {
    FILE *file = fopen(SAVE_FILE, "w");
    unsigned int saved_status = STATUS_NORMAL;

    if (file == NULL) {
        printf("[警告] セーブデータを書き込めませんでした。\n");
        return 0;
    }

    fprintf(file,
            "%s %d %d %d %u %d %d %d %d %d %d\n",
            SAVE_FORMAT_HEADER,
            next_stage,
            player->hp,
            player->max_hp,
            saved_status,
            player->correct_count,
            player->miss_count,
            player->input_error_count,
            player->reached_stage,
            player->level,
            player->exp);

    for (int stage = 0; stage < MAX_STAGE_COUNT; stage++) {
        fprintf(file,
                "%d %d %d\n",
                player->stage_correct_counts[stage],
                player->stage_miss_counts[stage],
                player->stage_input_error_counts[stage]);
    }

    fclose(file);

    if (next_stage < stage_count) {
        printf("[セーブ] 第%dステージから再開できます。\n", next_stage + 1);
    } else {
        printf("[セーブ] 完全勝利の記録を保存しました。\n");
    }

    return 1;
}
