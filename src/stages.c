#include "game.h"

static const char *const stage1_words[] = {
    "f",
    "j",
    "fj",
    "jf",
    "ff",
    "jj",
    "fjjf",
    "jffj",
    "f j",
    "j f",
    "fj fj",
    "jf jf"
};

static const char *const stage2_words[] = {
    "a",
    "s",
    "d",
    "f",
    "as",
    "df",
    "sad",
    "fad",
    "asdf",
    "fdsa",
    "dad",
    "sass",
    "adds",
    "as df"
};

static const char *const stage3_words[] = {
    "j",
    "k",
    "l",
    ";",
    "jk",
    "kl",
    "j;",
    "lj",
    "jkl;",
    ";lkj",
    "all",
    "fall",
    "flak",
    "ask",
    "j k"
};

static const char *const stage4_words[] = {
    "r",
    "t",
    "y",
    "u",
    "rt",
    "yu",
    "fr",
    "ju",
    "rty",
    "rtyu",
    "try",
    "turf",
    "trust",
    "fury"
};

static const char *const stage5_words[] = {
    "q",
    "w",
    "e",
    "i",
    "o",
    "p",
    "we",
    "io",
    "qwe",
    "iop",
    "quiet",
    "power",
    "paper",
    "equip",
    "we io"
};

static const char *const stage6_words[] = {
    "g",
    "h",
    "v",
    "b",
    "n",
    "m",
    "gh",
    "vb",
    "nm",
    "bgn",
    "human",
    "moving",
    "gaming",
    "humble",
    "gh nm"
};

static const char *const stage7_words[] = {
    "z",
    "x",
    "c",
    ",",
    ".",
    "/",
    "zxc",
    "cxz",
    ",.",
    "./",
    "z/c",
    "m,./",
    "?",
    "x.z",
    "c,/"
};

static const char *const stage8_words[] = {
    "asdf",
    "jkl;",
    "FJ",
    "AsDf",
    "SWORD",
    "Crystal",
    "Lv10",
    "HP-2",
    "Fire!",
    "Guard?",
    "Save:1",
    "Stage-8",
    "Type OK"
};

static const char *const stage9_words[] = {
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "0",
    "123",
    "890",
    "135",
    "246",
    "9090",
    "10 10",
    "2026"
};

static const char *const stage10_words[] = {
    "!",
    "@",
    "#",
    "$",
    "%",
    "^",
    "&",
    "*",
    "(",
    ")",
    "!?",
    "@@",
    "#1",
    "(ok)",
    "RPG!",
    "Kumdor?",
    "Lv+1",
    "HP=10"
};

static const Stage stages[] = {
    {
        {"クムドールの影", 3, 3, 1},
        "ホームポジション: f / j / スペース",
        "左右の人差し指をfとjに置き、右親指でスペースを押す。",
        stage1_words,
        (int)(sizeof(stage1_words) / sizeof(stage1_words[0])),
        "おにぎり",
        2,
        2
    },
    {
        {"左手の番人", 4, 4, 2},
        "左手ホーム段: a / s / d / f",
        "左手の指をa s d fに置き、打ったあと位置を崩さない。",
        stage2_words,
        (int)(sizeof(stage2_words) / sizeof(stage2_words[0])),
        "砲丸豆",
        2,
        3
    },
    {
        {"右手の番人", 4, 4, 2},
        "右手ホーム段: j / k / l / ;",
        "右手の指をj k l ;に置き、セミコロンまで小指で覚える。",
        stage3_words,
        (int)(sizeof(stage3_words) / sizeof(stage3_words[0])),
        "水中パン",
        2,
        3
    },
    {
        {"湖底の影", 5, 5, 2},
        "人差し指の上段: r / t / y / u",
        "fとjから人差し指を伸ばして打ち、すぐホームに戻す。",
        stage4_words,
        (int)(sizeof(stage4_words) / sizeof(stage4_words[0])),
        "クムの実",
        3,
        4
    },
    {
        {"上段の魔術師", 5, 5, 2},
        "上段の広がり: q / w / e / i / o / p",
        "小指・薬指・中指を上段へ伸ばし、戻る位置を意識する。",
        stage5_words,
        (int)(sizeof(stage5_words) / sizeof(stage5_words[0])),
        "空気アメ",
        3,
        4
    },
    {
        {"溶岩の番人", 6, 6, 3},
        "中央から下段: g / h / v / b / n / m",
        "人差し指の担当範囲を広げ、左右の境目を覚える。",
        stage6_words,
        (int)(sizeof(stage6_words) / sizeof(stage6_words[0])),
        "星チーズ",
        3,
        5
    },
    {
        {"水晶洞の幻", 6, 6, 3},
        "下段と記号: z / x / c / , / . / slash / ?",
        "下段は手が沈みやすいので、打ったあとホームへ戻す。",
        stage7_words,
        (int)(sizeof(stage7_words) / sizeof(stage7_words[0])),
        "着色タラコ",
        4,
        5
    },
    {
        {"古文書の魔術師", 7, 7, 3},
        "総合練習: 大文字・小文字・記号",
        "速さより正確さを優先し、Shiftや記号も落ち着いて打つ。",
        stage8_words,
        (int)(sizeof(stage8_words) / sizeof(stage8_words[0])),
        "3色ダンゴ",
        4,
        6
    },
    {
        {"数字の壁", 7, 7, 3},
        "数字キー: 1から0",
        "ホームポジションを見失わない範囲で、指を数字段へ伸ばす。",
        stage9_words,
        (int)(sizeof(stage9_words) / sizeof(stage9_words[0])),
        "健康ドリンク",
        4,
        6
    },
    {
        {"クムドールの試練", 8, 8, 3},
        "最終練習: Shift記号と混合課題",
        "小指でShiftを押しながら、反対側の指で記号を打つ。",
        stage10_words,
        (int)(sizeof(stage10_words) / sizeof(stage10_words[0])),
        "立喰いソバ",
        10,
        8
    }
};

const Stage *get_stages(int *stage_count) {
    *stage_count = (int)(sizeof(stages) / sizeof(stages[0]));
    return stages;
}
