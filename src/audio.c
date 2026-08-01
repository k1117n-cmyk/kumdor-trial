#include "audio.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __APPLE__
#include <dirent.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

extern int play_bgm_loop(const char *path, float volume);
#endif

#define DEFAULT_BGM_VOLUME 0.45f
#define COLOR_RESET "\033[0m"
#define COLOR_BLUE  "\033[34m"

#define BGM_CONTEXT_NONE     0
#define BGM_CONTEXT_STAGE    1
#define BGM_CONTEXT_PRESTAGE 2

static const char *color(const char *code);

#ifdef __APPLE__
static pid_t bgm_pid = -1;
static int current_stage_number = 0;
static int current_bgm_context = BGM_CONTEXT_NONE;
static int bgm_muted = 0;
static float get_bgm_volume(void);
static void start_bgm_file(const char *bgm_path, const char *failure_message);
static int is_bgm_file_name(const char *name);
#endif

void start_stage_bgm(int stage_number) {
#ifdef __APPLE__
    char bgm_path[64];
    int fallback_stage_number = (stage_number - 1) % 10 + 1;

    current_stage_number = stage_number;
    current_bgm_context = BGM_CONTEXT_STAGE;

    if (getenv("KUMDOR_NO_BGM") != NULL || bgm_muted) {
        return;
    }

    snprintf(bgm_path, sizeof(bgm_path), "BGM/kumdor_%02d.wav", stage_number);
    if (access(bgm_path, R_OK) != 0) {
        snprintf(bgm_path, sizeof(bgm_path), "BGM/kumdor_%02d.wav", fallback_stage_number);
        if (access(bgm_path, R_OK) != 0) {
            return;
        }
    }

    start_bgm_file(bgm_path, "BGMを開始できませんでした。このステージは無音で進みます。");
#else
    (void)stage_number;
#endif
}

void start_random_prestage_bgm(void) {
#ifdef __APPLE__
    DIR *dir;
    struct dirent *entry;
    char bgm_paths[64][128];
    int bgm_count = 0;
    int selected_index;

    current_stage_number = 0;
    current_bgm_context = BGM_CONTEXT_PRESTAGE;

    if (getenv("KUMDOR_NO_BGM") != NULL || bgm_muted) {
        return;
    }

    dir = opendir("BGM");
    if (dir == NULL) {
        return;
    }

    while ((entry = readdir(dir)) != NULL && bgm_count < (int)(sizeof(bgm_paths) / sizeof(bgm_paths[0]))) {
        if (!is_bgm_file_name(entry->d_name)) {
            continue;
        }

        snprintf(bgm_paths[bgm_count], sizeof(bgm_paths[bgm_count]), "BGM/%s", entry->d_name);
        if (access(bgm_paths[bgm_count], R_OK) == 0) {
            bgm_count++;
        }
    }

    closedir(dir);

    if (bgm_count == 0) {
        return;
    }

    selected_index = rand() % bgm_count;
    start_bgm_file(bgm_paths[selected_index], "BGMを開始できませんでした。このプレステージは無音で進みます。");
#endif
}

void toggle_bgm(void) {
#ifdef __APPLE__
    if (getenv("KUMDOR_NO_BGM") != NULL) {
        printf("%s[音]%s KUMDOR_NO_BGM=1 のため、BGM切り替えは無効です。\n",
               color(COLOR_BLUE),
               color(COLOR_RESET));
        return;
    }

    if (bgm_muted) {
        bgm_muted = 0;
        printf("%s[音]%s BGMをONにしました。\n", color(COLOR_BLUE), color(COLOR_RESET));
        if (current_stage_number > 0) {
            start_stage_bgm(current_stage_number);
        } else if (current_bgm_context == BGM_CONTEXT_PRESTAGE) {
            start_random_prestage_bgm();
        }
    } else {
        bgm_muted = 1;
        stop_bgm();
        printf("%s[音]%s BGMをOFFにしました。\n", color(COLOR_BLUE), color(COLOR_RESET));
    }
#else
    printf("%s[音]%s この環境ではBGM再生に対応していません。\n",
           color(COLOR_BLUE),
           color(COLOR_RESET));
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

static const char *color(const char *code) {
    return getenv("NO_COLOR") == NULL ? code : "";
}

#ifdef __APPLE__
static void start_bgm_file(const char *bgm_path, const char *failure_message) {
    float volume = get_bgm_volume();

    stop_bgm();

    bgm_pid = fork();
    if (bgm_pid < 0) {
        printf("%s[音]%s %s\n",
               color(COLOR_BLUE),
               color(COLOR_RESET),
               failure_message);
        return;
    }

    if (bgm_pid == 0) {
        if (setpgid(0, 0) != 0) {
            _exit(1);
        }

        _exit(play_bgm_loop(bgm_path, volume));
    }

    setpgid(bgm_pid, bgm_pid);
    printf("%s[音]%s BGM: %s (音量 %.2f)\n",
           color(COLOR_BLUE),
           color(COLOR_RESET),
           bgm_path,
           volume);
}

static int is_bgm_file_name(const char *name) {
    const char *prefix = "kumdor_";
    const char *suffix = ".wav";
    size_t name_length = strlen(name);
    size_t prefix_length = strlen(prefix);
    size_t suffix_length = strlen(suffix);

    if (name_length <= prefix_length + suffix_length) {
        return 0;
    }

    if (strncmp(name, prefix, prefix_length) != 0) {
        return 0;
    }

    return strcmp(name + name_length - suffix_length, suffix) == 0;
}

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
