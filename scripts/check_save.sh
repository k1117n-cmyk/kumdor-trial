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

run_case "v1" "$v1_save" "記録の石板を読み込んだ"
run_case "v2" "$v2_save" "記録の石板を読み込んだ"
run_case "v3" "$v3_save" "記録の石板を読み込んだ"
run_case "bad-header" "$bad_header_save" "記録の刻印が読み取れません" 'y
1
:savequit'
run_case "complete" "$complete_save" "完全勝利の証が刻まれています" 'y
1
:savequit'

v3_case_dir="$TMP_DIR/save-v3"
mkdir -p "$v3_case_dir"
output=$(cd "$v3_case_dir" && printf '1\n:savequit\n' | KUMDOR_NO_BGM=1 "$GAME")
printf '%s' "$output" | grep -q "記録を刻んだ。ここで剣を収める。"
head -n 1 "$v3_case_dir/kumdor_save.txt" | grep -q '^KUMDOR_SAVE_V3 '

echo "save checks passed"
