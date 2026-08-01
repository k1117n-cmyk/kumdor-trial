#include "game.h"

static const char *const stage1_words[] = {
    "fj",
    "jf",
    "ff",
    "jj",
    "fff",
    "jjj",
    "fjjf",
    "jffj",
    "ffjj",
    "jjff",
    "f j",
    "j f",
    "fj fj",
    "jf jf",
    "ff jj",
    "fj jf",
    "jff fj"
};

static const char *const stage1_climax_words[] = {
    "jj ff",
    "ff jj",
    "f jjj",
    "fff j",
    "fj jff",
    "jff fj",
    "ff jjj",
    "jjj ff",
    "fj jf fj",
    "jf fj jf"
};

static const char *const stage2_words[] = {
    "sad",
    "fad",
    "asdf",
    "fdsa",
    "dad",
    "sass",
    "adds",
    "as df",
    "sad fad",
    "add sad",
    "sass dad",
    "asfd",
    "dfas",
    "fads",
    "sadaf"
};

static const char *const stage2_climax_words[] = {
    "asdf fdsa",
    "sad adds",
    "df asdf",
    "as df sad",
    "fads sad",
    "sass dad",
    "adds fada",
    "sad fad df",
    "asfd dfas",
    "dfas asfd"
};

static const char *const stage3_words[] = {
    "j;",
    "lj",
    "jkl;",
    ";lkj",
    "j k",
    "jj kk",
    "ll ;;",
    "jkl; j",
    "j;k;l",
    "j;kj",
    ";ljk",
    "kl;",
    "jlj",
    "kkl"
};

static const char *const stage3_climax_words[] = {
    "jkl; ;lkj",
    "j; k; l;",
    "jj kk ll",
    "j k l ;",
    ";lkj jkl;",
    "j;kj ;ljk",
    "kl; jlj",
    "jkl; j;"
};

static const char *const stage4_words[] = {
    "asdf jkl;",
    "fdsa ;lkj",
    "a s d f j k l ;",
    "ff jj",
    "sad ask",
    "fall flask",
    "jkl; asdf",
    "fj fj",
    "all ask",
    "sad fall",
    "flask",
    "as jkl;",
    "fall; ask"
};

static const char *const stage4_climax_words[] = {
    "asdf fdsa",
    "jkl; ;lkj",
    "sad ask fall",
    "ff jj fj",
    "fall flask",
    "all; ask",
    "jkl; asdf",
    "fj jf fj",
    "sad fall ask"
};

static const char *const stage5_words[] = {
    "qwe",
    "qwert",
    "trewq",
    "q w e r t",
    "rr tt",
    "qwe",
    "were",
    "tree",
    "tweet",
    "qwer",
    "tret",
    "qet",
    "rew"
};

static const char *const stage5_climax_words[] = {
    "qwert trewq",
    "q w e r t",
    "tree tweet",
    "were qwe",
    "rr tt qwe",
    "qwer tret",
    "rew qet"
};

static const char *const stage6_words[] = {
    "yuiop",
    "poiuy",
    "y u i o p",
    "yy uu",
    "you",
    "pop",
    "poi",
    "yip",
    "upup",
    "yui",
    "yoyo",
    "poop",
    "poppy"
};

static const char *const stage6_climax_words[] = {
    "yuiop poiuy",
    "y u i o p",
    "you pop",
    "yoyo poop",
    "poppy yip",
    "yy uu poi",
    "upup yui"
};

static const char *const stage7_words[] = {
    "qwert yuiop",
    "trewq poiuy",
    "q w e r t y u i o p",
    "rr uu",
    "qwe you",
    "type",
    "power quiet",
    "qwerty",
    "quiet",
    "power",
    "power",
    "equip",
    "we io",
    "quote",
    "upper",
    "equip power",
    "quiet poet",
    "pop quit"
};

static const char *const stage7_climax_words[] = {
    "equip power",
    "quiet poet",
    "pop quit",
    "qwerty pipe",
    "upper quote",
    "power equip",
    "we io qwe",
    "power quiet",
    "quote power",
    "pipe pop"
};

static const char *const stage8_words[] = {
    "z",
    "x",
    "c",
    "zxcvb",
    "bvcxz",
    "z x c v b",
    "vv bb",
    "zxc",
    "zcv",
    "bvc",
    "zz bb",
    "zvb",
    "cvc",
    "bzz"
};

static const char *const stage8_climax_words[] = {
    "zxcvb bvcxz",
    "z x c v b",
    "vv bb zxc",
    "zvb cvc",
    "zcv bvc",
    "zz bb bzz"
};

static const char *const stage9_words[] = {
    "nm,./",
    "/.,mn",
    "n m , . /",
    "nn mm",
    "m,.",
    "nmn",
    "m,m",
    "n/m",
    ",",
    ".",
    "/",
    ",.",
    "./",
    "mnm",
    "m/m"
};

static const char *const stage9_climax_words[] = {
    "nm,./ /.,mn",
    "n m , . /",
    "m,. n/m",
    "nn mm",
    "m,m mnm",
    "nmn m/m",
    ",. ./"
};

static const char *const stage10_words[] = {
    "zxcvb nm,./",
    "bvcxz /.,mn",
    "z x c v b n m , . /",
    "vv nn",
    "zxc m,.",
    "vbn",
    "z.x,c/",
    "x/c.z",
    "c,z.x",
    "z/c",
    "x.z",
    "c,/",
    "x, z."
};

static const char *const stage10_climax_words[] = {
    "zxcvb nm,./",
    "bvcxz /.,mn",
    "z.x,c/",
    "x/c.z",
    "c,z.x",
    "x, z.",
    "z/c x.z",
    "c,/ zxc"
};

static const char *const stage11_words[] = {
    "12345",
    "54321",
    "1 2 3 4 5",
    "44 55",
    "123",
    "515",
    "1145",
    "2424",
    "3454",
    "1515",
    "2345"
};

static const char *const stage11_climax_words[] = {
    "12345 54321",
    "1 2 3 4 5",
    "1515 2345",
    "44 55 515",
    "1145 2424",
    "3454 123"
};

static const char *const stage12_words[] = {
    "67890",
    "09876",
    "6 7 8 9 0",
    "66 77",
    "789",
    "9090",
    "808",
    "670",
    "7890",
    "6868",
    "9076"
};

static const char *const stage12_climax_words[] = {
    "67890 09876",
    "6 7 8 9 0",
    "789 9090",
    "808 670",
    "66 77 7890",
    "6868 9076"
};

static const char *const stage13_words[] = {
    "12345 67890",
    "54321 09876",
    "1 2 3 4 5 6 7 8 9 0",
    "44 77",
    "123 789",
    "2024 808",
    "405 670",
    "515 9090",
    "20260727",
    "102030",
    "9090 1010",
    "2026 0707",
    "405 1020",
    "7270 9090"
};

static const char *const stage13_climax_words[] = {
    "12345 67890",
    "54321 09876",
    "1 2 3 4 5 6 7 8 9 0",
    "2024 808",
    "405 670",
    "515 9090",
    "20260727",
    "102030",
    "9090 1010",
    "405 1020"
};

static const char *const stage14_words[] = {
    "FJ",
    "AsDf",
    "SWORD",
    "Crystal",
    "KeyMap",
    "ShiftKey",
    "Lv10",
    "HP-2",
    "Fire!",
    "Guard?",
    "Save:1",
    "Stage-8",
    "Type OK",
    "Run Fast",
    "Magic:ON",
    "No_Miss",
    "Kum-Dor",
    "A1b2C3",
    "Sword+1",
    "Gate-Open",
    "Code:405",
    "Map_Q3",
    "Run+Jump",
    "HP=Full",
    "Shift?OK",
    "Book#8",
    "Alert!",
    "Queen_1",
    "Type-Ready"
};

static const char *const stage14_climax_words[] = {
    "Magic:ON",
    "No_Miss",
    "Kum-Dor",
    "A1b2C3",
    "Sword+1",
    "Gate-Open",
    "Code:405",
    "Map_Q3",
    "Run+Jump",
    "HP=Full",
    "Shift?OK",
    "!?OK",
    "@@Gate",
    "#1Boss",
    "(OK?)",
    "RPG!",
    "Kumdor?",
    "Lv+1",
    "HP=10",
    "Final!",
    "Save?No",
    "Shift+/",
    "QWERTY!",
    "KUM-405",
    "Type:OK!",
    "Sword#10",
    "(Stage10)",
    "No_Miss!",
    "A1! b2?",
    "Kumdor_01",
    "HP=10/10",
    "Lv+1 OK",
    "Final-Boss!"
};

static const char *const stage15_words[] = {
    "!@#$%",
    "^&*()",
    "_+=",
    "\\|][",
    "}{{",
    "\"':;?",
    "!?OK",
    "@@Gate",
    "#1Boss",
    "(OK?)",
    "Shift+/",
    "HP=10",
    "Lv+1",
    "Save?No",
    "Type:OK!"
};

static const char *const stage15_climax_words[] = {
    "! @ # $ %",
    "^ & * ( )",
    "_ += \\ |",
    "][ }{",
    "\" ' : ; ?",
    "!?OK @@Gate",
    "#1Boss (OK?)",
    "Shift+/ Type:OK!"
};

static const char *const stage16_words[] = {
    "Q1!",
    "W2@",
    "E3#",
    "R4$",
    "T5%",
    "Y6^",
    "U7&",
    "I8*",
    "O9(",
    "P0)",
    "QWERTY!",
    "Type_20",
    "Map+Key",
    "Code=OK",
    "Run|Stop"
};

static const char *const stage16_climax_words[] = {
    "Q1! W2@",
    "E3# R4$",
    "T5% Y6^",
    "U7& I8*",
    "O9( P0)",
    "QWERTY! Type_20",
    "Map+Key Code=OK",
    "Run|Stop"
};

static const char *const stage17_words[] = {
    "asdf JKL;",
    "zxcvb NM,./",
    "Key[Map]",
    "Gate{Open}",
    "Path\\Root",
    "Left|Right",
    "Save:OK",
    "Quote\"OK",
    "It's OK",
    "mix? yes!",
    "HP[10]",
    "Lv{20}",
    "A_B+C",
    "J;K:L"
};

static const char *const stage17_climax_words[] = {
    "Key[Map] Gate{Open}",
    "Path\\Root Left|Right",
    "Save:OK Quote\"OK",
    "It's OK mix? yes!",
    "HP[10] Lv{20}",
    "A_B+C J;K:L"
};

static const char *const stage18_words[] = {
    "Kumdor_20",
    "BGM=10",
    "Stage[18]",
    "Combo+3",
    "No_Miss!",
    "HP:10/10",
    "Map{Q3}",
    "Gate|Open",
    "Shift+OK?",
    "Code#405",
    "A1! b2@",
    "C3# d4$",
    "E5% f6^",
    "G7& h8*",
    "I9( j0)"
};

static const char *const stage18_climax_words[] = {
    "Kumdor_20 BGM=10",
    "Stage[18] Combo+3",
    "No_Miss! HP:10/10",
    "Map{Q3} Gate|Open",
    "Shift+OK? Code#405",
    "A1! b2@ C3#"
};

static const char *const stage19_words[] = {
    "!@# $%^",
    "&*() _+=",
    "\\| ][ }{",
    "\"':; ?/",
    "Qaz! WsX@",
    "Edc# Rfv$",
    "Tgb% Yhn^",
    "Ujm& Ik,*",
    "Ol.( P;?/",
    "All_Keys+OK",
    "Read{Then}Type",
    "No[Look]?OK!"
};

static const char *const stage19_climax_words[] = {
    "!@# $%^ &*()",
    "_+= \\| ][ }{",
    "\"':; ?/ OK",
    "Qaz! WsX@ Edc#",
    "Tgb% Yhn^ Ujm&",
    "All_Keys+OK"
};

static const char *const stage20_words[] = {
    "Kumdor_01!",
    "Final@Gate",
    "Sword#20",
    "HP=10/10",
    "Lv+1 OK?",
    "QWERTY{ASDF}",
    "ZXCV|NM,./",
    "Code:405",
    "Shift+_+=\\",
    "Key[Map]{OK}",
    "\"Final\"",
    "It's:OK?",
    "!@#$%^&*()",
    "All_Keys? Yes!",
    "Save?No_Run!"
};

static const char *const stage20_climax_words[] = {
    "Kumdor_01! Final@Gate",
    "Sword#20 HP=10/10",
    "Lv+1 OK? QWERTY{ASDF}",
    "ZXCV|NM,./ Code:405",
    "Shift+_+=\\ Key[Map]{OK}",
    "\"Final\" It's:OK?",
    "!@#$%^&*()",
    "All_Keys? Yes!"
};

static const char *const stage1_miss_quotes[] = {
    "「影が打鍵の乱れを拾ったぞ。落ち着いて構え直せ」",
    "「足元の影に合わせるな。課題の形を見ろ」"
};

static const char *const stage2_miss_quotes[] = {
    "「門柱が揺れた。次の一打は静かに置け」",
    "「左手の門は急ぐほど重くなる。列を見直せ」"
};

static const char *const stage3_miss_quotes[] = {
    "「鐘の音が乱れた。右手の列をもう一度見ろ」",
    "「響きが欠けたな。次は最後まで鳴らせ」"
};

static const char *const stage4_miss_quotes[] = {
    "「左右の音がずれた。両手をホーム段へ戻せ」",
    "「鐘の余韻に流されるな。左右を同じだけ見ろ」"
};

static const char *const stage5_miss_quotes[] = {
    "「水面が乱れた。左手の戻る場所を思い出せ」",
    "「流れに飲まれるな。伸ばす前に構えろ」"
};

static const char *const stage6_miss_quotes[] = {
    "「枝先の文字が逃げた。右手を上段へ静かに伸ばせ」",
    "「森の呪文がほどけた。上段を見てから打て」"
};

static const char *const stage7_miss_quotes[] = {
    "「熱が打鍵を乱したな。上段の左右を見直せ」",
    "「通路が震えた。左右の境を見失うな」"
};

static const char *const stage8_miss_quotes[] = {
    "「水晶が揺れた。左手下段の形を一つずつ見ろ」",
    "「下へ沈む指を急がせるな。打ったら戻せ」"
};

static const char *const stage9_miss_quotes[] = {
    "「記号の影が跳ねたな。右手下段を焦らず読め」",
    "「水晶の反射に惑うな。記号をひとつずつ見ろ」"
};

static const char *const stage10_miss_quotes[] = {
    "「洞窟の奥が揺れた。下段の左右を見失うな」",
    "「記号までまとめて追うな。形を分けて打て」"
};

static const char *const stage11_miss_quotes[] = {
    "「数字の階段が揺れた。左手の数字をまとまりで見直せ」",
    "「座標が乱れた。左手を数字段からホームへ戻せ」"
};

static const char *const stage12_miss_quotes[] = {
    "「右側の数字が跳ねた。右手だけで順番を取り戻せ」",
    "「塔の灯りがぶれた。数字を見てから打て」"
};

static const char *const stage13_miss_quotes[] = {
    "「数字列が崩れた。左右の数字を分けて見ろ」",
    "「座標が乱れた。まとまりで見直せ」"
};

static const char *const stage14_miss_quotes[] = {
    "「剣筋が乱れた。大小と数字を分けて見ろ」",
    "「総合の入口は逃げない。一息置いて打ち切れ」"
};

static const char *const stage15_miss_quotes[] = {
    "「記号の列が崩れた。Shiftを押す指を先に決めろ」",
    "「形だけを追うな。記号をひとつずつ読め」"
};

static const char *const stage16_miss_quotes[] = {
    "「数字と記号がずれた。上段から数字段へ静かに伸ばせ」",
    "「組み合わせに飲まれるな。まとまりで見直せ」"
};

static const char *const stage17_miss_quotes[] = {
    "「括弧の門が閉じた。開きと閉じを見分けろ」",
    "「引用符の影が跳ねた。課題全体を見てから打て」"
};

static const char *const stage18_miss_quotes[] = {
    "「通信文が乱れた。記号と英字を分けて読め」",
    "「長い課題ほど、最初の一打を静かに置け」"
};

static const char *const stage19_miss_quotes[] = {
    "「全キーの嵐に流されるな。左から順に区切れ」",
    "「記号の連なりは罠ではない。形を読めば道になる」"
};

static const char *const stage20_miss_quotes[] = {
    "「剣筋が乱れた。課題全体を見て戻れ」",
    "「最後の試練は逃げない。一息置いて打ち切れ」"
};

static const Stage stages[] = {
    {
        {"クムドールの影", 10, 10, 1, ENEMY_TRAIT_STANDARD},
        "アズドフ村の墜落跡",
        "船の残骸から這い出すと、村人たちは遠巻きにこちらを見ていた。抜け落ちたキーを探す前に、黒い影が足元から立ち上がる。",
        "ホームポジション: f / j / スペース",
        "f / j / Space",
        "左右の人差し指をfとjに置き、右親指でスペースを押す。",
        "「足元を見るな。指先だけで、わたしを裂いてみろ」",
        "「足元ではなく、前を見ていたか。ならば道は開く」",
        "「影はまだ半分残る。足元より、課題を見ろ」",
        stage1_miss_quotes,
        (int)(sizeof(stage1_miss_quotes) / sizeof(stage1_miss_quotes[0])),
        stage1_words,
        (int)(sizeof(stage1_words) / sizeof(stage1_words[0])),
        "クムドールの影が足元で二つに割れ、左右の指を同時に惑わせてくる。",
        stage1_climax_words,
        (int)(sizeof(stage1_climax_words) / sizeof(stage1_climax_words[0])),
        "おにぎり",
        2,
        2,
        "影がほどけ、村長は北へ続く道を指した。墜落で散らばったスパイスの一部も戻り、村人たちの疑いは少しだけ薄れる。まずは左手のキーを取り戻さなければならない。",
        "",
        "",
        0
    },
    {
        {"左手の番人", 10, 10, 2, ENEMY_TRAIT_HEAVY_COUNTER},
        "アズドフ東の石畳",
        "石化したクムの木が道をふさぎ、古い門柱だけがかろうじて残っている。左手の番人が、通行料の代わりに正確な打鍵を求めた。",
        "左手ホーム段: a / s / d / f",
        "a / s / d / f",
        "左手の指をa s d fに置き、打ったあと位置を崩さない。",
        "「左手の迷いは、門を閉ざす音になる」",
        "「左手の列は覚えられた。次は右手の番だ」",
        "「門柱は重くなる。左手を逃がすな」",
        stage2_miss_quotes,
        (int)(sizeof(stage2_miss_quotes) / sizeof(stage2_miss_quotes[0])),
        stage2_words,
        (int)(sizeof(stage2_words) / sizeof(stage2_words[0])),
        "左手の番人が門柱を叩き、a s d fの並びを速く刻み始めた。",
        stage2_climax_words,
        (int)(sizeof(stage2_climax_words) / sizeof(stage2_climax_words[0])),
        "砲丸豆",
        2,
        3,
        "門柱のくぼみからa、s、dの感覚が戻った。石畳に刻まれていた古い案内文字が浮かび上がり、ジュクルン村への近道を示す。東へ向かう足取りが少し軽くなる。",
        "",
        "",
        0
    },
    {
        {"右手の番人", 10, 10, 2, ENEMY_TRAIT_STANDARD},
        "ジュクルン村の鐘楼",
        "村の鐘は鳴らない。通信塔から落ちた金属片が鐘楼に刺さり、右手の番人がその下で目を光らせている。",
        "右手ホーム段: j / k / l / ;",
        "j / k / l / ;",
        "右手の指をj k l ;に置き、セミコロンまで小指で覚える。",
        "「鐘は鳴らぬ。鳴らすのは、おまえの右手だ」",
        "「鐘は鳴った。右手の小指まで届いた証だ」",
        "「鐘楼の影を濃くする。右手だけで見抜け」",
        stage3_miss_quotes,
        (int)(sizeof(stage3_miss_quotes) / sizeof(stage3_miss_quotes[0])),
        stage3_words,
        (int)(sizeof(stage3_words) / sizeof(stage3_words[0])),
        "右手の番人が鐘楼の影を揺らし、セミコロンを混ぜた構えに変えた。",
        stage3_climax_words,
        (int)(sizeof(stage3_climax_words) / sizeof(stage3_climax_words[0])),
        "水中パン",
        2,
        3,
        "鐘楼が低く鳴り、j、k、l、;の列が指先に戻った。止まっていた村の掲示板にも通信塔の断片的な警告が流れ始める。村人は湖底へ抜ける古い道を教えてくれた。",
        "鐘楼の下で村人が温かい布を差し出した。短い休息のあいだ、左右のホーム段の感覚が指に沈み直していく。",
        "次は左右のホーム段を両手で使う。打ったあと、必ずfとjへ戻ることを意識しろ。",
        3
    },
    {
        {"ホーム段の双影", 10, 10, 2, ENEMY_TRAIT_REGEN_COUNTER},
        "ミルファ湖の沈んだ桟橋",
        "湖面は凍っていないのに、底だけが青白く固まっている。沈んだ桟橋の影から、左右のホーム段を同時に試すものが現れた。",
        "ホーム段 両手: a / s / d / f / j / k / l / ;",
        "a / s / d / f / j / k / l / ;",
        "左右の指をホーム段に置き、打ったあと必ずfとjへ戻す。",
        "「左右の音をそろえられるか。湖底は両手の乱れを映す」",
        "「左右のホーム段はつながった。次は上段へ進め」",
        "「水底の流れを速める。両手の戻る場所を失うな」",
        stage4_miss_quotes,
        (int)(sizeof(stage4_miss_quotes) / sizeof(stage4_miss_quotes[0])),
        stage4_words,
        (int)(sizeof(stage4_words) / sizeof(stage4_words[0])),
        "ホーム段の双影が水面を波立たせ、左右のホーム段を続けて打たせる流れを作った。",
        stage4_climax_words,
        (int)(sizeof(stage4_climax_words) / sizeof(stage4_climax_words[0])),
        "クムの実",
        3,
        4,
        "湖底に細い水路が開き、王都の方角へ流れが戻った。左右のホーム段をつないだ感覚が、次の上段へ伸びる道を示している。",
        "",
        "",
        0
    },
    {
        {"左上段の魔術師", 10, 10, 3, ENEMY_TRAIT_POISON_EDGE},
        "クムの森の入口",
        "森の木々は灰色に固まり、葉の代わりに文字の欠片を落としている。左上段の魔術師は左手だけで届く枝先から課題をばらまいた。",
        "上段 左手: q / w / e / r / t",
        "q / w / e / r / t",
        "左手を上段へ伸ばし、打ったあとa s d fへ戻す。",
        "「左の枝先は逃げる。追うなら正確に追え」",
        "「左手は枝先まで届いた。次は右側の梢だ」",
        "「枝先の呪文を重ねる。左手上段まで届かせろ」",
        stage5_miss_quotes,
        (int)(sizeof(stage5_miss_quotes) / sizeof(stage5_miss_quotes[0])),
        stage5_words,
        (int)(sizeof(stage5_words) / sizeof(stage5_words[0])),
        "左上段の魔術師が枝先の文字を束ね、q w e r tを続けて刻む呪文に変えた。",
        stage5_climax_words,
        (int)(sizeof(stage5_climax_words) / sizeof(stage5_climax_words[0])),
        "空気アメ",
        3,
        4,
        "石の枝に小さな緑が戻った。左手の上段に触れた文字の欠片が、右側の森へ続く道を示している。",
        "",
        "",
        0
    },
    {
        {"右上段の魔術師", 10, 10, 3, ENEMY_TRAIT_HEAVY_COUNTER},
        "ズロワノフ通路跡",
        "森を抜けると、地面の裂け目に赤い光が走っていた。右上段の魔術師が、通路の向こうから右手の上段だけを照らしている。",
        "上段 右手: y / u / i / o / p",
        "y / u / i / o / p",
        "右手を上段へ伸ばし、打ったあとj k l ;へ戻す。",
        "「右の枝先まで届くか。熱に急かされるな」",
        "「右手も上段へ届いた。両手の上段へ進め」",
        "「通路の熱を上げる。右手上段の形を見せろ」",
        stage6_miss_quotes,
        (int)(sizeof(stage6_miss_quotes) / sizeof(stage6_miss_quotes[0])),
        stage6_words,
        (int)(sizeof(stage6_words) / sizeof(stage6_words[0])),
        "右上段の魔術師が通路を赤く染め、y u i o pを続けて打たせる呪文を押し出した。",
        stage6_climax_words,
        (int)(sizeof(stage6_climax_words) / sizeof(stage6_climax_words[0])),
        "星チーズ",
        3,
        5,
        "裂け目の熱が静まり、右手上段の感覚が指先へ戻った。クム3号は着陸直前に外部から入力を上書きされている。墜落は事故ではなかった。",
        "ズロワノフ通路跡の熱が引き、石壁の陰に小さな補給箱が見つかった。焦げた手袋を外すと、指先に落ち着きが戻ってくる。",
        "次は上段を両手で使う。左右に広がっても、打ったあとホーム段へ戻れ。",
        4
    },
    {
        {"上段の双術師", 10, 10, 3, ENEMY_TRAIT_BLIND_EDGE},
        "ケムリ苔の水晶洞",
        "熱病を鎮めるケムリ苔は、水晶の中に閉じ込められていた。洞窟の幻は上段全体の揺さぶりで集中を奪いにくる。",
        "上段 両手: q / w / e / r / t / y / u / i / o / p",
        "q / w / e / r / t / y / u / i / o / p",
        "上段を両手で打ち分け、打ったあとホームへ戻す。",
        "「水晶は手元を映さない。上段の影だけを読め」",
        "「上段の影を読んだか。水晶はもう目を惑わせない」",
        "「水晶を曇らせる。上段の左右を読め」",
        stage7_miss_quotes,
        (int)(sizeof(stage7_miss_quotes) / sizeof(stage7_miss_quotes[0])),
        stage7_words,
        (int)(sizeof(stage7_words) / sizeof(stage7_words[0])),
        "上段の双術師が水晶に文字を反射させ、上段全体を使う構えになった。",
        stage7_climax_words,
        (int)(sizeof(stage7_climax_words) / sizeof(stage7_climax_words[0])),
        "着色タラコ",
        4,
        5,
        "水晶が砕け、ケムリ苔が息を吹き返した。上段の文字は整い、洞窟のさらに深い下段へ進む道が開く。",
        "",
        "",
        0
    },
    {
        {"左下段の幻", 10, 10, 3, ENEMY_TRAIT_POISON_EDGE},
        "王立図書庫の地下",
        "王都クミエルの地下には、古いキーボード術の記録が眠っていた。左下段の幻は、左手を下へ沈める課題で封印を守っている。",
        "下段 左手: z / x / c / v / b",
        "z / x / c / v / b",
        "左手を下段へ下げ、打ったあとホーム段へ戻す。",
        "「下へ沈む左手を見せろ。封印は戻る指を見ている」",
        "「左下段はほどけた。次は右下段の記号だ」",
        "「封印を重ねる。左下段の形を刻め」",
        stage8_miss_quotes,
        (int)(sizeof(stage8_miss_quotes) / sizeof(stage8_miss_quotes[0])),
        stage8_words,
        (int)(sizeof(stage8_words) / sizeof(stage8_words[0])),
        "左下段の幻がページをめくり、z x c v bを続けて刻む封印を開いた。",
        stage8_climax_words,
        (int)(sizeof(stage8_climax_words) / sizeof(stage8_climax_words[0])),
        "3色ダンゴ",
        4,
        6,
        "封印の一枚が剥がれ、左下段の感覚が戻った。古文書の奥には、右下段の記号を示す古い図が残されている。",
        "",
        "",
        0
    },
    {
        {"右下段の幻", 10, 10, 4, ENEMY_TRAIT_REGEN_COUNTER},
        "クミエル通信塔",
        "塔の壁面には、墜落直前に見た記号列が焼き付いていた。右下段の幻は、右手の下段と記号で階段を隠している。",
        "下段 右手: n / m / , / . / slash",
        "n / m / , / . / Slash",
        "右手を下段へ下げ、記号を見たら一呼吸置いて打つ。",
        "「右下段は形が似る。だが順番を誤れば道は消える」",
        "「右下段は正された。下段の両手へ進め」",
        "「記号の階段を組み替える。順番を崩すな」",
        stage9_miss_quotes,
        (int)(sizeof(stage9_miss_quotes) / sizeof(stage9_miss_quotes[0])),
        stage9_words,
        (int)(sizeof(stage9_words) / sizeof(stage9_words[0])),
        "右下段の幻が座標を組み替え、n m , . / の長い列で階段を隠した。",
        stage9_climax_words,
        (int)(sizeof(stage9_climax_words) / sizeof(stage9_climax_words[0])),
        "健康ドリンク",
        4,
        6,
        "記号列が整列し、塔の上層へ続く扉が開いた。墜落時に乱れた計器の表示も、少しずつ正しい形へ戻っていく。",
        "通信塔の踊り場で、古い端末がまだ淡く光っていた。指を休めるあいだ、塔の風が毒気と焦りを洗い流していく。",
        "次は下段を両手で使う。記号が混じっても、左右の担当を見失うな。",
        5
    },
    {
        {"下段の双幻", 10, 10, 4, ENEMY_TRAIT_HEAVY_COUNTER},
        "通信塔最上階",
        "塔の最上階へ続く床に、下段の文字と記号が剣の形に並ぶ。下段の双幻は、左右の下段を同時に求めてきた。",
        "下段 両手: z / x / c / v / b / n / m / , / . / slash",
        "z / x / c / v / b / n / m / , / . / Slash",
        "下段と記号を両手で打ち分け、打ったあとホームへ戻す。",
        "「下段すべてを取り戻すなら、記号まで目を逸らすな」",
        "「下段はつながった。次は数字段へ上がれ」",
        "「下段を剣に重ねる。最後まで打ち切れ」",
        stage10_miss_quotes,
        (int)(sizeof(stage10_miss_quotes) / sizeof(stage10_miss_quotes[0])),
        stage10_words,
        (int)(sizeof(stage10_words) / sizeof(stage10_words[0])),
        "下段の双幻が剣の形に光り、下段と記号の打ち分けを一息で求めてきた。",
        stage10_climax_words,
        (int)(sizeof(stage10_climax_words) / sizeof(stage10_climax_words[0])),
        "立喰いソバ",
        4,
        6,
        "下段の文字が剣へ戻り、残る欠けは数字段だけになった。塔の外で通信灯が点滅し、数字の階段が現れる。",
        "",
        "",
        0
    },
    {
        {"左数字の壁", 10, 10, 4, ENEMY_TRAIT_POISON_EDGE},
        "クミエル通信塔 左翼",
        "塔の左翼には、墜落直前に見た左側の数字列が焼き付いていた。左数字の壁は、左手の数字だけを正確に刻む者を通す。",
        "数字 左手: 1 / 2 / 3 / 4 / 5",
        "1 / 2 / 3 / 4 / 5",
        "左手を数字段へ伸ばし、打ったあとホーム段へ戻す。",
        "「左の数字は階段の始まりだ。順番を崩すな」",
        "「左の数字は整った。次は右の数字を戻せ」",
        "「左数字の階段を組み替える。まとまりを見失うな」",
        stage11_miss_quotes,
        (int)(sizeof(stage11_miss_quotes) / sizeof(stage11_miss_quotes[0])),
        stage11_words,
        (int)(sizeof(stage11_words) / sizeof(stage11_words[0])),
        "左数字の壁が座標を組み替え、1 2 3 4 5を続けて打たせる階段を作った。",
        stage11_climax_words,
        (int)(sizeof(stage11_climax_words) / sizeof(stage11_climax_words[0])),
        "通信ゼリー",
        4,
        6,
        "左側の数字列が整列し、塔の計器が半分だけ息を吹き返した。右側の数字がまだ乱れ、頂上への扉を閉ざしている。",
        "",
        "",
        0
    },
    {
        {"右数字の壁", 10, 10, 4, ENEMY_TRAIT_REGEN_COUNTER},
        "クミエル通信塔 右翼",
        "塔の右翼では、右側の数字列が淡く点滅していた。右数字の壁は、右手だけで数字段を渡る力を試してくる。",
        "数字 右手: 6 / 7 / 8 / 9 / 0",
        "6 / 7 / 8 / 9 / 0",
        "右手を数字段へ伸ばし、打ったあとホーム段へ戻す。",
        "「右の数字は頂上へ続く。急げば足場は消える」",
        "「右の数字も整った。両手で数字段をつなげ」",
        "「右数字の階段を組み替える。順番を崩すな」",
        stage12_miss_quotes,
        (int)(sizeof(stage12_miss_quotes) / sizeof(stage12_miss_quotes[0])),
        stage12_words,
        (int)(sizeof(stage12_words) / sizeof(stage12_words[0])),
        "右数字の壁が座標を組み替え、6 7 8 9 0を続けて打たせる階段を作った。",
        stage12_climax_words,
        (int)(sizeof(stage12_climax_words) / sizeof(stage12_climax_words[0])),
        "健康ドリンク",
        4,
        6,
        "右側の数字列が整列し、塔の計器が正しい座標を示した。左右の数字をつなげれば、最上階の封印に届く。",
        "通信塔の踊り場で、古い端末がまだ淡く光っていた。指を休めるあいだ、塔の風が毒気と焦りを洗い流していく。",
        "次は数字段を両手で使う。数字と記号が混じっても、左右の担当を見失うな。",
        5
    },
    {
        {"数字の双壁", 10, 10, 4, ENEMY_TRAIT_BLIND_EDGE},
        "クミエル通信塔 上層",
        "塔の上層では、左右の数字列がひとつの座標へ結ばれていた。数字の双壁は、両手の数字段と記号を混ぜて最後の扉を守る。",
        "数字 両手: 1 / 2 / 3 / 4 / 5 / 6 / 7 / 8 / 9 / 0",
        "1 / 2 / 3 / 4 / 5 / 6 / 7 / 8 / 9 / 0",
        "数字段を両手で打ち分け、打ったあとホーム段へ戻す。",
        "「数字は嘘をつかない。だが左右を誤れば扉は閉じる」",
        "「数字段はつながった。最後の試練へ進め」",
        "「数字の階段を組み替える。まとまりで見直せ」",
        stage13_miss_quotes,
        (int)(sizeof(stage13_miss_quotes) / sizeof(stage13_miss_quotes[0])),
        stage13_words,
        (int)(sizeof(stage13_words) / sizeof(stage13_words[0])),
        "数字の双壁が座標を組み替え、数字と記号の長い列で階段を隠した。",
        stage13_climax_words,
        (int)(sizeof(stage13_climax_words) / sizeof(stage13_climax_words[0])),
        "星チーズ",
        4,
        7,
        "数字列が整列し、塔の上層へ続く最後の扉が開いた。墜落時に乱れた計器の数字も、正しい座標へ戻っていく。",
        "",
        "",
        0
    },
    {
        {"総合の門番", 10, 10, 4, ENEMY_TRAIT_HEAVY_COUNTER},
        "通信塔最上階",
        "塔の最上階へ続く門で、英字、数字、記号が剣の形に並ぶ。総合の門番は、これまで覚えた打鍵を混ぜて求めてきた。",
        "総合練習: Shift記号と混合課題",
        "Shift / 大文字 / 数字 / 記号 / Space",
        "Shift、大文字、数字、記号、スペースを正確に切り替える。",
        "「総合の門を抜けるなら、大小と記号から目を逸らすな」",
        "「混ざった課題を見切ったな。全キーの試練へ進め」",
        "「門の文字を重ねる。最後まで打ち切れ」",
        stage14_miss_quotes,
        (int)(sizeof(stage14_miss_quotes) / sizeof(stage14_miss_quotes[0])),
        stage14_words,
        (int)(sizeof(stage14_words) / sizeof(stage14_words[0])),
        "総合の門番が剣の形に光り、英字、数字、記号を混ぜた課題を一息で求めてきた。",
        stage14_climax_words,
        (int)(sizeof(stage14_climax_words) / sizeof(stage14_climax_words[0])),
        "立喰いソバ",
        4,
        7,
        "総合の門が開き、すべてのキーを使う追加の試練が姿を見せた。ここから先は、記号も括弧も引用符もまとめて読み切らなければならない。",
        "",
        "",
        0
    },
    {
        {"記号列の番人", 10, 10, 4, ENEMY_TRAIT_POISON_EDGE},
        "通信塔 記号回廊",
        "壁一面にShift記号が刻まれた回廊で、記号列の番人が待っていた。数字段の上に隠れた記号を、順番どおりに呼び戻す試練だ。",
        "全キー練習: Shift記号の基礎",
        "! / @ / # / $ / % / ^ / & / * / ( / ) / _ / + / = / \\ / | / ] / [ / } / { / \" / ' / : / ; / ? / /",
        "記号を見たら、必要なShiftと担当指を先に決めてから打つ。",
        "「記号の姿を読め。形を見れば指は迷わない」",
        "「記号列は整った。英字と数字を混ぜた道へ進め」",
        "「記号を束ねる。ひとつでも形を落とすな」",
        stage15_miss_quotes,
        (int)(sizeof(stage15_miss_quotes) / sizeof(stage15_miss_quotes[0])),
        stage15_words,
        (int)(sizeof(stage15_words) / sizeof(stage15_words[0])),
        "記号列の番人が壁の記号を一斉に光らせ、Shiftを押す指まで試してきた。",
        stage15_climax_words,
        (int)(sizeof(stage15_climax_words) / sizeof(stage15_climax_words[0])),
        "記号せんべい",
        4,
        7,
        "記号回廊の文字が整い、通信塔の奥へ続く鍵穴が開いた。次は英字と数字、記号を同時に読む必要がある。",
        "記号回廊の床が少し温まり、指先の緊張がほどける。",
        "次は英字と数字、Shift記号がまとまって出る。左から区切って読め。",
        4
    },
    {
        {"数字記号の調律師", 10, 10, 4, ENEMY_TRAIT_REGEN_COUNTER},
        "通信塔 調律室",
        "調律室では、数字とShift記号が同じ弦の上で震えている。数字記号の調律師は、上段と数字段の切り替えを試してきた。",
        "全キー練習: 英字、数字、Shift記号",
        "英字 / 数字 / ! @ # $ % ^ & * ( ) / _ + = / |",
        "英字から数字段へ伸ばしても、打ったあとホームへ戻す。",
        "「数字と記号を同じ弦で鳴らせ。急げば音は割れる」",
        "「調律は済んだ。括弧と引用符の間を抜けろ」",
        "「弦を張り替える。数字も記号も落とすな」",
        stage16_miss_quotes,
        (int)(sizeof(stage16_miss_quotes) / sizeof(stage16_miss_quotes[0])),
        stage16_words,
        (int)(sizeof(stage16_words) / sizeof(stage16_words[0])),
        "数字記号の調律師が弦を鳴らし、数字とShift記号の組み合わせを重ねてきた。",
        stage16_climax_words,
        (int)(sizeof(stage16_climax_words) / sizeof(stage16_climax_words[0])),
        "調律キャンディ",
        4,
        7,
        "調律室の音がそろい、塔の壁に括弧と引用符の道が浮かび上がった。",
        "",
        "",
        0
    },
    {
        {"括弧の門番", 10, 10, 4, ENEMY_TRAIT_BLIND_EDGE},
        "通信塔 括弧門",
        "括弧門には、角括弧、波括弧、引用符が並んでいた。括弧の門番は、似た形の記号を見分ける力を求めてくる。",
        "全キー練習: 括弧、引用符、区切り記号",
        "[ / ] / { / } / \" / ' / : / ; / ? / / / \\ / |",
        "開く記号と閉じる記号を見分け、引用符も課題どおりに打つ。",
        "「門は左右で形が違う。開くか閉じるか、見てから打て」",
        "「括弧門は開いた。長い通信文へ進め」",
        "「門を閉じる。引用符まで読み切れ」",
        stage17_miss_quotes,
        (int)(sizeof(stage17_miss_quotes) / sizeof(stage17_miss_quotes[0])),
        stage17_words,
        (int)(sizeof(stage17_words) / sizeof(stage17_words[0])),
        "括弧の門番が左右の門を入れ替え、括弧と引用符を混ぜてきた。",
        stage17_climax_words,
        (int)(sizeof(stage17_climax_words) / sizeof(stage17_climax_words[0])),
        "括弧まんじゅう",
        4,
        8,
        "括弧門が開き、通信塔の中枢から長い入力文が流れ込み始めた。",
        "",
        "",
        0
    },
    {
        {"通信文の編纂者", 10, 10, 5, ENEMY_TRAIT_POISON_EDGE},
        "通信塔 編纂室",
        "編纂室では、英字、数字、記号が短い通信文に組み直されていた。通信文の編纂者は、まとまりごとの読解を試す。",
        "全キー練習: 短い通信文",
        "英字 / 数字 / 記号 / Space / Shift",
        "長い課題は、記号やスペースで区切ってから打つ。",
        "「文は一文字ずつでは遅い。まとまりを読め」",
        "「通信文は整った。全キーの嵐へ進め」",
        "「文を組み替える。区切りを見失うな」",
        stage18_miss_quotes,
        (int)(sizeof(stage18_miss_quotes) / sizeof(stage18_miss_quotes[0])),
        stage18_words,
        (int)(sizeof(stage18_words) / sizeof(stage18_words[0])),
        "通信文の編纂者が文を組み替え、英字、数字、記号を続けて読ませてきた。",
        stage18_climax_words,
        (int)(sizeof(stage18_climax_words) / sizeof(stage18_climax_words[0])),
        "編纂スープ",
        5,
        8,
        "編纂室の文が整い、クムドールの剣を封じる最後の防壁が見えた。",
        "編纂室の端末が静かに光り、休む指先へ温かな風を送った。",
        "次は全キーの混合課題だ。最初に課題全体を見て、区切りごとに打て。",
        5
    },
    {
        {"全キーの嵐", 10, 10, 5, ENEMY_TRAIT_REGEN_COUNTER},
        "通信塔 暴風層",
        "暴風層では、すべてのキーが風のように流れていた。全キーの嵐は、左右、段、Shiftの切り替えをまとめて浴びせてくる。",
        "全キー練習: 記号連続と左右混合",
        "全英字 / 数字 / Shift記号 / 括弧 / 引用符 / Space",
        "全体を一度見て、左から短いまとまりに分けて打つ。",
        "「全キーの風を読め。追うな、区切れ」",
        "「嵐を抜けたな。最後の試練へ進め」",
        "「風を荒らす。すべてのキーを見失うな」",
        stage19_miss_quotes,
        (int)(sizeof(stage19_miss_quotes) / sizeof(stage19_miss_quotes[0])),
        stage19_words,
        (int)(sizeof(stage19_words) / sizeof(stage19_words[0])),
        "全キーの嵐が塔を揺らし、記号と英字を一息で求めてきた。",
        stage19_climax_words,
        (int)(sizeof(stage19_climax_words) / sizeof(stage19_climax_words[0])),
        "暴風ドリンク",
        5,
        8,
        "暴風層が静まり、クムドールの剣を封じる最後の扉が開いた。",
        "",
        "",
        0
    },
    {
        {"クムドールの試練", 10, 10, 5, ENEMY_TRAIT_HEAVY_COUNTER},
        "通信塔最上階",
        "塔の最上階で、抜け落ちたすべてのキーが剣の形に並ぶ。クムドールの試練は、英字、数字、記号、スペースをすべて重ねて求めてきた。",
        "最終練習: 全キー混合課題",
        "全英字 / 数字 / ! @ # $ % ^ & * ( ) _ + = \\ | ] [ } { \" ' : ; ? / / Space",
        "課題全体を見て、まとまりごとに正確に打ち切る。",
        "「すべてのキーを取り戻すなら、最後まで目を逸らすな」",
        "「最後まで目を逸らさなかったな。クムドールの剣は応えた」",
        "「すべてのキーを剣に重ねる。最後まで打ち切れ」",
        stage20_miss_quotes,
        (int)(sizeof(stage20_miss_quotes) / sizeof(stage20_miss_quotes[0])),
        stage20_words,
        (int)(sizeof(stage20_words) / sizeof(stage20_words[0])),
        "クムドールの試練が剣の形に光り、すべてのキーを一息で求めてきた。",
        stage20_climax_words,
        (int)(sizeof(stage20_climax_words) / sizeof(stage20_climax_words[0])),
        "立喰いソバ",
        10,
        10,
        "剣はあなたの入力に応え、石化の呪文を一文字ずつ上書きしていく。抜け落ちていたキーはすべて元の場所へ戻り、塔の外で森がざわめき始めた。",
        "",
        "",
        0
    },
};

const Stage *get_stages(int *stage_count) {
    *stage_count = (int)(sizeof(stages) / sizeof(stages[0]));
    return stages;
}
