# kumdor_01.c 解説書

この文書は、`kumdor_01.c` の現在のコードを理解するための解説です。

このゲームは、ターミナル上で遊ぶタイピングRPGです。画面に表示されたアルファベットを入力し、正解すると敵にダメージを与えます。敵のHPを0にすれば勝利、プレイヤーのHPが0になると敗北です。

## 全体の構成

現在のコードは、大きく分けて次の要素で構成されています。

1. 標準ライブラリの読み込み
2. 状態異常を表すビットフラグ
3. プレイヤーと敵を表す `struct`
4. 関数プロトタイプ宣言
5. `main` 関数
6. 各処理を担当する関数

最初のバージョンでは、HPや状態異常を個別の変数で管理していました。現在は `Player` と `Enemy` という構造体にまとめています。これにより、今後レベル、経験値、攻撃力、防御力、所持アイテムなどを追加しやすくなっています。

## include の役割

```c
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
```

それぞれの役割は次の通りです。

| include | 使っている機能 |
| --- | --- |
| `stdio.h` | `printf`, `scanf` |
| `stdlib.h` | `rand`, `srand` |
| `ctype.h` | `toupper` |
| `time.h` | `time` |

`toupper` は、小文字入力を大文字として扱うために使っています。たとえば、正解が `A` のとき、プレイヤーが `a` と入力しても正解になります。

## 状態異常のビットフラグ

```c
#define STATUS_NORMAL 0x00
#define STATUS_POISON 0x01
#define STATUS_BLIND  0x02
```

状態異常はビットフラグで管理しています。

| 定数 | 値 | 意味 |
| --- | --- | --- |
| `STATUS_NORMAL` | `0x00` | 正常 |
| `STATUS_POISON` | `0x01` | 毒 |
| `STATUS_BLIND` | `0x02` | 暗闇 |

ビットフラグを使うと、複数の状態を1つの変数に同時に持たせることができます。

たとえば、毒と暗闇を同時に持たせる場合は次のようにできます。

```c
player.status = STATUS_POISON | STATUS_BLIND;
```

現在のゲームでは、開始時に毒または暗闇のどちらか一方がランダムで付与されます。

## Player 構造体

```c
typedef struct {
    const char *name;
    int hp;
    int max_hp;
    unsigned char status;
} Player;
```

`Player` はプレイヤーの状態をまとめる構造体です。

| メンバ | 意味 |
| --- | --- |
| `name` | プレイヤー名 |
| `hp` | 現在HP |
| `max_hp` | 最大HP |
| `status` | 状態異常 |

現在の初期値は `main` 関数内で次のように設定されています。

```c
Player player = {"あなた", 10, 10, STATUS_NORMAL};
```

つまり、プレイヤー名は「あなた」、現在HPは10、最大HPも10、状態異常なしで開始します。

## Enemy 構造体

```c
typedef struct {
    const char *name;
    int hp;
    int max_hp;
    int attack;
} Enemy;
```

`Enemy` は敵の状態をまとめる構造体です。

| メンバ | 意味 |
| --- | --- |
| `name` | 敵の名前 |
| `hp` | 現在HP |
| `max_hp` | 最大HP |
| `attack` | 攻撃力 |

現在の敵は `main` 関数内で次のように設定されています。

```c
Enemy enemy = {"クムドールの影", 3, 3, 2};
```

敵の名前は「クムドールの影」、HPは3、攻撃力は2です。

## 関数プロトタイプ宣言

```c
void fill_words(char words[], int word_count);
void print_title(void);
void apply_opening_status(Player *player);
void print_battle_status(const Player *player, const Enemy *enemy);
char choose_target(const char words[], int word_count);
int read_input(char *input);
void player_turn(Player *player, Enemy *enemy, char target);
void enemy_turn(Player *player, const Enemy *enemy);
```

これは「このあと、こういう関数が登場します」とコンパイラに知らせるための宣言です。

C言語では、関数を使う前にその関数の形を知っている必要があります。`main` 関数より下に関数本体を書く場合は、このようなプロトタイプ宣言が必要になります。

## main 関数の流れ

`main` 関数はゲーム全体の進行役です。

```c
int main(void) {
    srand((unsigned int)time(NULL));
```

ここで乱数を初期化しています。`rand()` はそのままだと毎回同じ順番の乱数になるため、現在時刻を使って毎回違う結果になりやすくしています。

```c
char words[26];
int word_count = sizeof(words) / sizeof(words[0]);
fill_words(words, word_count);
```

`words` はタイピング対象になる文字の配列です。`A` から `Z` までの26文字が入ります。

`word_count` は配列の要素数です。

```c
Player player = {"あなた", 10, 10, STATUS_NORMAL};
Enemy enemy = {"クムドールの影", 3, 3, 2};
```

ここでプレイヤーと敵の初期状態を作っています。

```c
print_title();
apply_opening_status(&player);
printf("\n戦闘開始！\n\n");
```

タイトルを表示し、開始時の状態異常を付与して、戦闘開始メッセージを表示します。

## メインゲームループ

```c
while (player.hp > 0 && enemy.hp > 0) {
```

この `while` 文が戦闘の中心です。

条件は「プレイヤーのHPが0より大きい」かつ「敵のHPが0より大きい」です。つまり、どちらかが倒れるまでターンを繰り返します。

1ターンの流れは次の通りです。

1. タイピング対象の文字をランダムに選ぶ
2. 現在のHPを表示する
3. プレイヤーが入力して攻撃する
4. 敵が生きていれば反撃する
5. 区切り線を表示する

該当コードは次の部分です。

```c
char target = choose_target(words, word_count);

print_battle_status(&player, &enemy);
player_turn(&player, &enemy, target);

if (enemy.hp > 0) {
    enemy_turn(&player, &enemy);
}
```

`player_turn` のあとに `enemy.hp > 0` を確認しているため、敵を倒したターンでは敵の反撃は発生しません。

## 勝敗判定

ループを抜けたあと、プレイヤーのHPが残っているかで勝敗を判定しています。

```c
if (player.hp > 0) {
    printf("\n【勝利】クムドールの剣がまばゆく輝いた！\n");
    printf("あなたのタイピングスキルがレベルアップした！\n");
} else {
    printf("\n【敗北】%sは膝をついた……。\n", player.name);
    printf("もう一度挑戦して、クムドールの試練を突破しよう！\n");
}
```

敵のHPが0になれば勝利です。プレイヤーのHPが0になれば敗北です。

## fill_words 関数

```c
void fill_words(char words[], int word_count) {
    for (int i = 0; i < word_count; i++) {
        words[i] = 'A' + i;
    }
}
```

`words` 配列に `A` から `Z` までを入れる関数です。

`'A' + 0` は `A`、`'A' + 1` は `B`、`'A' + 2` は `C` になります。文字は内部的には数値として扱えるため、このような書き方ができます。

## print_title 関数

```c
void print_title(void) {
    printf("=========================================\n");
    printf("     タイピングRPG ★ クムドールの試練     \n");
    printf("=========================================\n");
    printf("現れた文字を正確にタイプして、敵を倒せ！\n\n");
}
```

ゲーム開始時のタイトル画面を表示する関数です。

今後、タイトル画面にメニューを追加する場合は、この関数を拡張する候補になります。

## apply_opening_status 関数

```c
void apply_opening_status(Player *player) {
    if (rand() % 2 == 0) {
        player->status |= STATUS_POISON;
        printf("[警告] 敵の罠にかかり【毒】状態になった！（攻撃力半減）\n");
    } else {
        player->status |= STATUS_BLIND;
        printf("[警告] 敵の魔霧により【暗闇】状態になった！（文字が化ける）\n");
    }
}
```

戦闘開始時に、プレイヤーへ状態異常を付与する関数です。

`rand() % 2` は `0` または `1` を返します。`0` の場合は毒、`1` の場合は暗闇になります。

```c
player->status |= STATUS_POISON;
```

これは、プレイヤーの状態に毒フラグを追加する処理です。

`|=` はビットOR代入です。既存の状態を残したまま、新しい状態を追加できます。

## print_battle_status 関数

```c
void print_battle_status(const Player *player, const Enemy *enemy) {
    printf("%s HP: %d/%d | %s HP: %d/%d\n",
           player->name,
           player->hp,
           player->max_hp,
           enemy->name,
           enemy->hp,
           enemy->max_hp);
}
```

プレイヤーと敵のHPを表示します。

引数に `const Player *` と `const Enemy *` を使っています。これは、この関数の中ではプレイヤーや敵の中身を変更しないという意味です。

## choose_target 関数

```c
char choose_target(const char words[], int word_count) {
    return words[rand() % word_count];
}
```

タイピング対象の文字をランダムに1つ選びます。

`rand() % word_count` によって、`0` から `word_count - 1` までの番号を作り、その番号に対応する文字を返しています。

## read_input 関数

```c
int read_input(char *input) {
    printf("タイプキーを入力してEnter: ");
    return scanf(" %c", input) == 1;
}
```

プレイヤーの入力を1文字読み取る関数です。

`scanf(" %c", input)` の `" %c"` には、先頭に空白があります。この空白には、前回入力された改行などの空白文字を読み飛ばす効果があります。

戻り値は、入力に成功したかどうかです。

| 戻り値 | 意味 |
| --- | --- |
| `1` | 入力成功 |
| `0` | 入力失敗 |

実際には `scanf(...) == 1` の結果を返しているため、成功なら真、失敗なら偽として扱えます。

## player_turn 関数

```c
void player_turn(Player *player, Enemy *enemy, char target)
```

プレイヤーの1ターンを処理する関数です。

主な処理は次の通りです。

1. 状態異常に応じて敵の構えを表示する
2. 入力を受け取る
3. 入力が正解か判定する
4. 正解なら敵にダメージを与える
5. 毒状態ならダメージを与えず、毒を解除する
6. 不正解ならミスメッセージを表示する

### 暗闇状態の表示

```c
if (player->status & STATUS_BLIND) {
    printf("敵の構え: [ %c ] (視界が悪い！大文字で打ち込め！)\n", target + 32);
} else {
    printf("敵の構え: [ %c ]\n", target);
}
```

`player->status & STATUS_BLIND` は、暗闇状態かどうかを調べています。

暗闇状態の場合、正解文字を小文字として表示しています。たとえば、正解が `A` なら画面には `a` と表示されます。

現在は `target + 32` で小文字化しています。`A` から `Z` の範囲では動作しますが、今後は `tolower()` を使うとより安全です。

### 入力失敗時

```c
if (!read_input(&input)) {
    printf("➔ 入力が読み取れなかった！ ターンを失った！\n");
    return;
}
```

入力が読み取れなかった場合は、そのターンを終了します。

ここで `return` しているため、それ以降の正解判定や攻撃処理は行われません。

### 小文字入力への対応

```c
if (toupper((unsigned char)input) == target) {
```

入力文字を `toupper` で大文字に変換してから、正解文字と比較しています。

そのため、正解が `A` の場合、`A` でも `a` でも正解になります。

`(unsigned char)input` としているのは、`toupper` に安全な値を渡すためです。C言語では、`char` が負の値を持つ環境があり、そのまま渡すと未定義動作になる可能性があります。

### 毒状態の処理

```c
if (player->status & STATUS_POISON) {
    printf("➔ 毒のせいで力が出ない！ 0.5のダメージ！\n");
    printf("（もう一度同じ文字を的中させろ！）\n");
    player->status &= ~STATUS_POISON;
} else {
    enemy->hp--;
    printf("➔ 見事なタイピング！ 剣が炸裂した！ (敵の残りHP: %d)\n", enemy->hp);
}
```

毒状態のときに正解しても、現在の実装では敵HPを減らしません。その代わり、毒状態を解除します。

```c
player->status &= ~STATUS_POISON;
```

これは毒フラグを外す処理です。

`~STATUS_POISON` は、毒のビットだけが0で、それ以外が1の値を作ります。そこに `&=` を使うことで、毒だけを消して他の状態は残せます。

## enemy_turn 関数

```c
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
```

敵の反撃を処理する関数です。

敵の `attack` の値だけ、プレイヤーのHPを減らします。

HPがマイナスになると表示が不自然になるため、0未満になった場合は0に補正しています。

## ポインタを使っている理由

`player_turn` や `enemy_turn` では、引数に `Player *player` や `Enemy *enemy` が使われています。

これは、関数の中で元のデータを書き換えたいからです。

たとえば、次の処理では敵のHPを減らしています。

```c
enemy->hp--;
```

もし構造体をポインタではなく通常の値として渡すと、関数内でコピーが作られます。そのコピーを変更しても、`main` 関数側の敵HPは変わりません。

そのため、HPや状態異常のようにゲーム進行で変わる値は、ポインタで渡して直接更新しています。

## const ポインタを使っている理由

次の関数では `const` が付いています。

```c
void print_battle_status(const Player *player, const Enemy *enemy);
void enemy_turn(Player *player, const Enemy *enemy);
```

`const` は「この関数の中では中身を変更しません」という意味です。

たとえば `print_battle_status` は表示するだけなので、プレイヤーや敵のHPを書き換える必要がありません。そのため `const` を付けています。

`enemy_turn` ではプレイヤーHPは減らしますが、敵の情報は読むだけです。そのため、`Player *player` は変更可能、`const Enemy *enemy` は読み取り専用になっています。

## 現在のゲームバランス

現在の初期値は次の通りです。

| 項目 | 値 |
| --- | --- |
| プレイヤーHP | 10 |
| 敵HP | 3 |
| 敵攻撃力 | 2 |
| 正解時の通常ダメージ | 1 |
| 毒状態での正解 | 0ダメージ、毒解除 |

すべてミスした場合、敵の攻撃を5回受けるとプレイヤーHPが0になります。

敵HPは3なので、毒がない状態なら3回正解すると勝利します。毒状態で始まった場合は、最初の正解で毒解除、そこから3回正解すると勝利です。

## 今後拡張しやすいポイント

現在の構造にしたことで、次の拡張がしやすくなっています。

| 拡張案 | 追加しやすい場所 |
| --- | --- |
| プレイヤー攻撃力 | `Player` に `attack` を追加 |
| レベル | `Player` に `level` を追加 |
| 経験値 | `Player` に `exp` を追加 |
| 敵の種類 | `Enemy` の配列を作る |
| 複数ステージ | `main` の戦闘開始部分をループ化 |
| 単語入力 | `char target` を文字列に変更 |
| 状態異常のターン数 | `Player` に状態ごとの残りターンを追加 |
| アイテム | `Player` に所持数やインベントリを追加 |

## 注意点

現在のコードには、今後改善したほうがよい点もあります。

1. 毒の説明と実装が少し違う

   表示では「攻撃力半減」「0.5ダメージ」と言っていますが、実装上は0ダメージです。将来的にはHPを2倍の単位で管理するか、攻撃力を導入すると自然になります。

2. 暗闇時の小文字化が `target + 32`

   今は `A` から `Z` だけなので動作しますが、Cとしては `tolower()` を使うほうが安全です。

3. 敵の反撃が毎ターン発生する

   プレイヤーが正解しても敵を倒しきれなければ反撃されます。これはターン制RPGとして自然ですが、ゲームバランスは今後調整が必要です。

4. 入力はまだ1文字だけ

   タイピングRPGとして育てるなら、次の段階で単語入力にするとゲーム性が大きく上がります。

## コンパイル方法

ターミナルで次のコマンドを実行するとコンパイルできます。

```sh
cc -Wall -Wextra -pedantic kumdor_01.c -o kumdor_01
```

実行するには次のコマンドを使います。

```sh
./kumdor_01
```

`-Wall -Wextra -pedantic` は、Cコンパイラに警告を多めに出してもらうためのオプションです。小さなミスを早めに見つけやすくなります。
