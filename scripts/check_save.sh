#!/bin/sh
set -eu

ROOT_DIR=$(cd "$(dirname "$0")/.." && pwd)
GAME="$ROOT_DIR/kumdor_01"
TMP_DIR=$(mktemp -d "${TMPDIR:-/tmp}/kumdor-save-test.XXXXXX")

cleanup() {
    rm -rf "$TMP_DIR"
}
trap cleanup EXIT

run_case() {
    name=$1
    input=$2
    expected=$3
    play_input=${4:-'y
:savequit'}

    case_dir="$TMP_DIR/$name"
    mkdir -p "$case_dir"

    if [ -n "$input" ]; then
        printf '%s\n' "$input" > "$case_dir/kumdor_save.txt"
    fi

    output=$(cd "$case_dir" && printf '%s\n' "$play_input" | KUMDOR_NO_BGM=1 "$GAME")

    printf '%s' "$output" | grep -q "$expected"
}

if [ ! -x "$GAME" ]; then
    echo "kumdor_01 が見つかりません。先に make を実行してください。" >&2
    exit 1
fi

v1_save='KUMDOR_SAVE_V1 0 10 10 0 2 1 0 1 1 0'
v2_save='KUMDOR_SAVE_V2 0 10 10 0 3 1 0 1 1 0
3 1 0
0 0 0
0 0 0
0 0 0
0 0 0
0 0 0
0 0 0
0 0 0
0 0 0
0 0 0'
v3_save='KUMDOR_SAVE_V3 0 10 10 0 4 1 0 1 1 0
4 1 0 4
0 0 0 0
0 0 0 0
0 0 0 0
0 0 0 0
0 0 0 0
0 0 0 0
0 0 0 0
0 0 0 0
0 0 0 0'
v4_save='KUMDOR_SAVE_V4 0 10 10 0 4 1 0 1 1 0
4 1 0 4
0 0 0 0
0 0 0 0
0 0 0 0
0 0 0 0
0 0 0 0
0 0 0 0
0 0 0 0
0 0 0 0
0 0 0 0
0 0 0 0
0 0 0 0
0 0 0 0
0 0 0 0'
v5_save=$( (
    printf 'KUMDOR_SAVE_V5 0 10 10 0 4 1 0 1 1 0\n'
    printf '4 1 0 4\n'
    i=1
    while [ "$i" -lt 20 ]; do
        printf '0 0 0 0\n'
        i=$((i + 1))
    done
) )
bad_header_save='KUMDOR_SAVE_X 0 10 10 0 0 0 0 1 1 0'
complete_save='KUMDOR_SAVE_V3 10 10 10 0 10 0 0 10 2 0
10 0 0 10
0 0 0 0
0 0 0 0
0 0 0 0
0 0 0 0
0 0 0 0
0 0 0 0
0 0 0 0
0 0 0 0
0 0 0 0'

run_case "v1" "$v1_save" "セーブゾーンの記録を読み込んだ"
run_case "v2" "$v2_save" "セーブゾーンの記録を読み込んだ"
run_case "v3" "$v3_save" "セーブゾーンの記録を読み込んだ"
run_case "v4" "$v4_save" "セーブゾーンの記録を読み込んだ"
run_case "v5" "$v5_save" "セーブゾーンの記録を読み込んだ"
run_case "bad-header" "$bad_header_save" "記録の刻印が読み取れません" 'y
2
:savequit'
run_case "complete" "$complete_save" "完全勝利の証が刻まれています" 'y
2
:savequit'

v6_case_dir="$TMP_DIR/save-v6"
mkdir -p "$v6_case_dir"
output=$(cd "$v6_case_dir" && printf '2\n:savequit\n' | KUMDOR_NO_BGM=1 "$GAME")
printf '%s' "$output" | grep -q "記録を刻んだ。ここで剣を収める。"
head -n 1 "$v6_case_dir/kumdor_save.txt" | grep -q '^KUMDOR_SAVE_V6 '
head -n 1 "$v6_case_dir/kumdor_save.txt" | grep -q ' 20$'

echo "save checks passed"
