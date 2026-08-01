#include "save.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#endif

#define SAVE_FORMAT_HEADER_V1 "KUMDOR_SAVE_V1"
#define SAVE_FORMAT_HEADER_V2 "KUMDOR_SAVE_V2"
#define SAVE_FORMAT_HEADER_V3 "KUMDOR_SAVE_V3"
#define SAVE_FORMAT_HEADER SAVE_FORMAT_HEADER_V3
#define VALID_STATUS_MASK (STATUS_POISON | STATUS_BLIND | STATUS_LOCKED)
#define COLOR_RESET "\033[0m"
#define COLOR_BLUE  "\033[34m"

static const char *color(const char *code);

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
        printf("試練の数が記録の石板に収まりません。新たに試練へ向かいます。\n");
        return 0;
    }

    printf("記録の石板が見つかりました。\n");
    printf("前回の続きから始めますか？ (y/n): ");
    if (fgets(answer, INPUT_BUFFER_SIZE, stdin) == NULL) {
        fclose(file);
        printf("記録を使わず、新たに試練へ向かいます。\n");
        return 0;
    }

#if defined(__unix__) || defined(__APPLE__)
    if (!isatty(STDIN_FILENO)) {
        printf("\n");
    }
#endif

    if (answer[0] != 'y' && answer[0] != 'Y') {
        fclose(file);
        printf("記録を使わず、新たに試練へ向かいます。\n");
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
        printf("記録の石板を読み取れませんでした。新たに試練へ向かいます。\n");
        return 0;
    }

    if (strcmp(header, SAVE_FORMAT_HEADER_V3) == 0) {
        for (int stage = 0; stage < MAX_STAGE_COUNT; stage++) {
            if (fscanf(file,
                       "%d %d %d %d",
                       &loaded_player.stage_correct_counts[stage],
                       &loaded_player.stage_miss_counts[stage],
                       &loaded_player.stage_input_error_counts[stage],
                       &loaded_player.stage_max_combo_counts[stage]) != 4) {
                fclose(file);
                printf("記録の石板を読み取れませんでした。新たに試練へ向かいます。\n");
                return 0;
            }
        }
    } else if (strcmp(header, SAVE_FORMAT_HEADER_V2) == 0) {
        for (int stage = 0; stage < MAX_STAGE_COUNT; stage++) {
            if (fscanf(file,
                       "%d %d %d",
                       &loaded_player.stage_correct_counts[stage],
                       &loaded_player.stage_miss_counts[stage],
                       &loaded_player.stage_input_error_counts[stage]) != 3) {
                fclose(file);
                printf("記録の石板を読み取れませんでした。新たに試練へ向かいます。\n");
                return 0;
            }
        }
    } else if (strcmp(header, SAVE_FORMAT_HEADER_V1) != 0) {
        fclose(file);
        printf("記録の刻印が読み取れません。新たに試練へ向かいます。\n");
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
        printf("記録の石板に乱れがあります。新たに試練へ向かいます。\n");
        return 0;
    }

    for (int stage = 0; stage < MAX_STAGE_COUNT; stage++) {
        if (loaded_player.stage_correct_counts[stage] < 0 ||
            loaded_player.stage_miss_counts[stage] < 0 ||
            loaded_player.stage_input_error_counts[stage] < 0 ||
            loaded_player.stage_max_combo_counts[stage] < 0 ||
            loaded_player.stage_max_combo_counts[stage] > loaded_player.stage_correct_counts[stage]) {
            printf("ステージ別の記録に乱れがあります。新たに試練へ向かいます。\n");
            return 0;
        }
    }

    if (next_stage >= stage_count) {
        printf("完全勝利の証が刻まれています。新たな試練として始めます。\n");
        return 0;
    }

    loaded_player.status = STATUS_NORMAL;
    loaded_player.poison_turns_remaining = 0;
    *player = loaded_player;
    *start_stage = next_stage;

    printf("記録の石板を読み込んだ。第%dステージの入口から再開する。\n", *start_stage + 1);
    return 1;
}

int save_game(const Player *player, int next_stage, int stage_count) {
    FILE *file = fopen(SAVE_FILE, "w");
    unsigned int saved_status = STATUS_NORMAL;

    if (file == NULL) {
        printf("%s[記録]%s 記録の石板に刻めませんでした。\n",
               color(COLOR_BLUE),
               color(COLOR_RESET));
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
                "%d %d %d %d\n",
                player->stage_correct_counts[stage],
                player->stage_miss_counts[stage],
                player->stage_input_error_counts[stage],
                player->stage_max_combo_counts[stage]);
    }

    fclose(file);

    if (next_stage < stage_count) {
        printf("%s[記録]%s 第%dステージの入口に記録を刻みました。\n",
               color(COLOR_BLUE),
               color(COLOR_RESET),
               next_stage + 1);
    } else {
        printf("%s[記録]%s 完全勝利の証を石板に刻みました。\n",
               color(COLOR_BLUE),
               color(COLOR_RESET));
    }

    return 1;
}

static const char *color(const char *code) {
    return getenv("NO_COLOR") == NULL ? code : "";
}
