#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

// プレイヤーの状態を管理するビットフラグ
#define STATUS_NORMAL 0x00  // 正常
#define STATUS_POISON 0x01  // 毒（攻撃力が下がる）
#define STATUS_BLIND  0x02  // 暗闇（敵の文字が見えにくくなる）

typedef struct {
    const char *name;
    int hp;
    int max_hp;
    unsigned char status;
} Player;

typedef struct {
    const char *name;
    int hp;
    int max_hp;
    int attack;
} Enemy;

void fill_words(char words[], int word_count);
void print_title(void);
void apply_opening_status(Player *player);
void print_battle_status(const Player *player, const Enemy *enemy);
char choose_target(const char words[], int word_count);
int read_input(char *input);
void player_turn(Player *player, Enemy *enemy, char target);
void enemy_turn(Player *player, const Enemy *enemy);

int main(void) {
    // 乱数の初期化
    srand((unsigned int)time(NULL));

    char words[26];
    int word_count = sizeof(words) / sizeof(words[0]);
    fill_words(words, word_count);

    // ゲームのステータス変数
    Player player = {"あなた", 10, 10, STATUS_NORMAL};
    Enemy enemy = {"クムドールの影", 3, 3, 2};

    print_title();
    apply_opening_status(&player);
    printf("\n戦闘開始！\n\n");

    // メインゲームループ
    while (player.hp > 0 && enemy.hp > 0) {
        char target = choose_target(words, word_count);

        print_battle_status(&player, &enemy);
        player_turn(&player, &enemy, target);

        printf("-----------------------------------------\n");
    }

    if (player.hp > 0) {
        printf("\n【勝利】クムドールの剣がまばゆく輝いた！\n");
        printf("あなたのタイピングスキルがレベルアップした！\n");
    } else {
        printf("\n【敗北】%sは膝をついた……。\n", player.name);
        printf("もう一度挑戦して、クムドールの試練を突破しよう！\n");
    }

    return 0;
}

void fill_words(char words[], int word_count) {
    // 'A' から順番に1文字ずつ進めて配列に詰め込む
    for (int i = 0; i < word_count; i++) {
        words[i] = 'A' + i;
    }
}

void print_title(void) {
    printf("=========================================\n");
    printf("     タイピングRPG ★ クムドールの試練     \n");
    printf("=========================================\n");
    printf("現れた文字を正確にタイプして、敵を倒せ！\n\n");
}

void apply_opening_status(Player *player) {
    // 敵の先制攻撃！ランダムで状態異常フラグを立てる
    if (rand() % 2 == 0) {
        player->status |= STATUS_POISON;
        printf("[警告] 敵の罠にかかり【毒】状態になった！（攻撃力半減）\n");
    } else {
        player->status |= STATUS_BLIND;
        printf("[警告] 敵の魔霧により【暗闇】状態になった！（文字が化ける）\n");
    }
}

void print_battle_status(const Player *player, const Enemy *enemy) {
    printf("%s HP: %d/%d | %s HP: %d/%d\n",
           player->name,
           player->hp,
           player->max_hp,
           enemy->name,
           enemy->hp,
           enemy->max_hp);
}

char choose_target(const char words[], int word_count) {
    // 今回タイピングする文字をランダムに決定
    return words[rand() % word_count];
}

int read_input(char *input) {
    printf("タイプキーを入力してEnter: ");
    return scanf(" %c", input) == 1; // 頭のスペースで改行を読み飛ばすC言語のテクニック
}

void player_turn(Player *player, Enemy *enemy, char target) {
    char input;

    // 暗闇フラグ（ビット演算）のチェック
    if (player->status & STATUS_BLIND) {
        // 暗闇状態なら、文字がヒント（小文字）になって難しくなる
        printf("敵の構え: [ %c ] (視界が悪い！大文字で打ち込め！)\n", target + 32);
    } else {
        printf("敵の構え: [ %c ]\n", target);
    }

    if (!read_input(&input)) {
        printf("➔ 入力が読み取れなかった！ ターンを失った！\n");
        return;
    }

    // 文字が一致しているか判定
    if (toupper((unsigned char)input) == target) {
        // 毒フラグ（ビット演算）のチェック
        if (player->status & STATUS_POISON) {
            printf("➔ 毒のせいで力が出ない！ 0.5のダメージ！\n");
            // 実数計算を避けるため、今回はHPを2回的中させて1減らす仕様を簡易再現（ここでは0ダメージ扱い）
            printf("（もう一度同じ文字を的中させろ！）\n");
            player->status &= ~STATUS_POISON; // 毒を解除
        } else {
            enemy->hp--;
            printf("➔ 見事なタイピング！ 剣が炸裂した！ (敵の残りHP: %d)\n", enemy->hp);
        }
    } else {
        printf("➔ ミス！ 手元が狂った！（反撃を受ける！）\n");
        enemy_turn(player, enemy);
    }
}

void enemy_turn(Player *player, const Enemy *enemy) {
    player->hp -= enemy->attack;

    if (player->hp < 0) {
        player->hp = 0;
    }

    printf("➔ %sの反撃！ %dダメージを受けた！ (あなたの残りHP: %d)\n",
           enemy->name,
           enemy->attack,
           player->hp);
}
