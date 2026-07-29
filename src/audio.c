#include "audio.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

extern int play_bgm_loop(const char *path, float volume);
#endif

#define DEFAULT_BGM_VOLUME 0.45f

#ifdef __APPLE__
static pid_t bgm_pid = -1;
static int current_stage_number = 0;
static int bgm_muted = 0;
static float get_bgm_volume(void);
#endif

void start_stage_bgm(int stage_number) {
#ifdef __APPLE__
    char bgm_path[64];
    float volume = get_bgm_volume();

    current_stage_number = stage_number;

    if (getenv("KUMDOR_NO_BGM") != NULL || bgm_muted) {
        return;
    }

    snprintf(bgm_path, sizeof(bgm_path), "BGM/kumdor_%02d.wav", stage_number);
    if (access(bgm_path, R_OK) != 0) {
        printf("[BGM] %s が見つからないため、このステージは無音で進みます。\n", bgm_path);
        return;
    }

    stop_bgm();

    bgm_pid = fork();
    if (bgm_pid < 0) {
        printf("[警告] BGM再生を開始できませんでした。このステージは無音で進みます。\n");
        return;
    }

    if (bgm_pid == 0) {
        if (setpgid(0, 0) != 0) {
            _exit(1);
        }

        _exit(play_bgm_loop(bgm_path, volume));
    }

    setpgid(bgm_pid, bgm_pid);
    printf("[BGM] %s を再生します。（音量: %.2f）\n", bgm_path, volume);
#else
    (void)stage_number;
#endif
}

void toggle_bgm(void) {
#ifdef __APPLE__
    if (getenv("KUMDOR_NO_BGM") != NULL) {
        printf("[BGM] KUMDOR_NO_BGM=1 のため、ゲーム中の切り替えは無効です。\n");
        return;
    }

    if (bgm_muted) {
        bgm_muted = 0;
        printf("[BGM] ONにしました。\n");
        if (current_stage_number > 0) {
            start_stage_bgm(current_stage_number);
        }
    } else {
        bgm_muted = 1;
        stop_bgm();
        printf("[BGM] OFFにしました。\n");
    }
#else
    printf("[BGM] この環境ではBGM再生に対応していません。\n");
#endif
}

void stop_bgm(void) {
#ifdef __APPLE__
    if (bgm_pid <= 0) {
        return;
    }

    kill(-bgm_pid, SIGTERM);
    waitpid(bgm_pid, NULL, 0);
    bgm_pid = -1;
#endif
}

void cleanup_audio(void) {
    stop_bgm();
}

#ifdef __APPLE__
static float get_bgm_volume(void) {
    const char *value = getenv("KUMDOR_BGM_VOLUME");
    char *end = NULL;
    float volume;

    if (value == NULL || value[0] == '\0') {
        return DEFAULT_BGM_VOLUME;
    }

    errno = 0;
    volume = strtof(value, &end);
    if (errno != 0 || end == value || *end != '\0' || volume < 0.0f || volume > 1.0f) {
        return DEFAULT_BGM_VOLUME;
    }

    return volume;
}
#endif
