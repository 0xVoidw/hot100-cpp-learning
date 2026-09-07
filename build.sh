#!/usr/bin/env bash
# =============================================================================
#  一键编译 + 跑测试（不需要 CMake，只要有 g++ 或 clang++）
#      bash build.sh            # 编译并跑全部用例
#      bash build.sh linked_list  # 只跑某组 / 某一题（子串过滤）
#      bash build.sh -c         # 先清掉 build/ 再编译
# =============================================================================
set -euo pipefail
cd "$(dirname "$0")"

CLEAN=0
FILTER=""
for arg in "$@"; do
  if [[ "$arg" == "-c" || "$arg" == "--clean" ]]; then CLEAN=1; else FILTER="$arg"; fi
done

CXX="${CXX:-}"
if [[ -z "$CXX" ]]; then
  for c in g++ clang++ c++; do
    if command -v "$c" >/dev/null 2>&1; then CXX="$c"; break; fi
  done
fi
if [[ -z "$CXX" ]]; then
  echo "找不到编译器，请先安装 g++ / clang++，或设置环境变量 CXX" >&2
  exit 1
fi

mkdir -p build
BIN=build/hot100
[[ "$(uname -s 2>/dev/null || true)" == MINGW* || "$(uname -s 2>/dev/null || true)" == MSYS* ]] && BIN=build/hot100.exe
[[ $CLEAN == 1 ]] && rm -f "$BIN"

echo "==> $CXX 编译中（-std=c++17 -Wall -Wextra -O1）"
# shellcheck disable=SC2086
"$CXX" -std=c++17 -Wall -Wextra -Wno-unused-parameter -O1 -Iinclude src/*.cpp -o "$BIN"

echo "==> 运行测试"
if [[ -n "$FILTER" ]]; then
  exec "./$BIN" "$FILTER"
else
  exec "./$BIN"
fi
