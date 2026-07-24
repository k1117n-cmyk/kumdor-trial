#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// プレイヤーの状態を管理するビットフラグ
#define STATUS_NORMAL 0x00  // 正常
#define STATUS_POISON 0x01  // 毒（攻撃力が下がる）
#define STATUS_BLIND  0x02  // 暗闇（敵の文字が見えにくくなる）

int main(void) {
    // 乱数の初期化
    srand((unsigned int)time(NULL));

    char words[26]; 
    // 'A' から順番に1文字ずつ進めて配列に詰め込む
    for (int i = 0; i < 26; i++) {
        words[i] = 'A' + i;
    }

    // ゲームのステータス変数
    int enemy_hp = 3;
    unsigned char player_status = STATUS_NORMAL;
    
    int word_count = sizeof(words) / sizeof(words[0]);

    printf("=========================================\n");
    printf("     タイピングRPG ★ クムドールの試練     \n");
    printf("=========================================\n");
    printf("現れた文字を正確にタイプして、敵を倒せ！\n\n");

    // 敵の先制攻撃！ランダムで状態異常フラグを立てる
    if (rand() % 2 == 0) {
        player_status |= STATUS_POISON;
        printf("[警告] 敵の罠にかかり【毒】状態になった！（攻撃力半減）\n");
    } else {
        player_status |= STATUS_BLIND;
        printf("[警告] 敵の魔霧により【暗闇】状態になった！（文字が化ける）\n");
    }
    printf("\n戦闘開始！\n\n");

    // メインゲームループ
    while (enemy_hp > 0) {
        // 今回タイピングする文字をランダムに決定
        char target = words[rand() % word_count];
        
        // 暗闇フラグ（ビット演算）のチェック
        if (player_status & STATUS_BLIND) {
            // 暗闇状態なら、文字がヒント（小文字）になって難しくなる
            printf("敵の構え: [ %c ] (視界が悪い！大文字で打ち込め！)\n", target + 32);
        } else {
            printf("敵の構え: [ %c ]\n", target);
        }

        printf("タイプキーを入力してEnter: ");
        char input;
        scanf(" %c", &input); // 頭のスペースで改行を読み飛ばすC言語のテクニック

        // 文字が一致しているか判定
        if (input == target) {
            // 毒フラグ（ビット演算）のチェック
            if (player_status & STATUS_POISON) {
                printf("➔ 毒のせいで力が出ない！ 0.5のダメージ！\n");
                // 実数計算を避けるため、今回はHPを2回的中させて1減らす仕様を簡易再現（ここでは0ダメージ扱い）
                printf("（もう一度同じ文字を的中させろ！）\n");
                player_status &= ~STATUS_POISON; // 毒を解除
            } else {
                enemy_hp--;
                printf("➔ 見事なタイピング！ 剣が炸裂した！ (敵の残りHP: %d)\n", enemy_hp);
            }
        } else {
            printf("➔ ミス！ 手元が狂った！（ダメージは受けないが仕切り直しだ）\n");
        }
        printf("-----------------------------------------\n");
    }

    printf("\n【勝利】クムドールの剣がまばゆく輝いた！\n");
    printf("あなたのタイピングスキルがレベルアップした！\n");

    return 0;
}

