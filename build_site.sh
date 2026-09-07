#!/usr/bin/env bash
# 生成 LeetCode Hot 100 图解网站（macOS / Linux / git-bash）
set -e
cd "$(dirname "$0")"
if command -v python3 >/dev/null 2>&1; then PY=python3; else PY=python; fi
echo "正在生成图解网站..."
"$PY" tools/gen_animations.py
"$PY" tools/gen_site.py
echo
echo "[完成] 已生成到 site/，打开 site/index.html 即可浏览。"
