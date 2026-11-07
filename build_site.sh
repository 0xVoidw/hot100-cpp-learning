#!/usr/bin/env bash
# 生成 Hot 100 图解网站（macOS / Linux / git-bash），只需要 Python 3 和 Node.js 18+，无需 npm install
set -e
cd "$(dirname "$0")"
if command -v python3 >/dev/null 2>&1; then PY=python3; else PY=python; fi
"$PY" tools/gen_animations.py
"$PY" tools/gen_site.py --json --data-only
node www/build.mjs "$@"
echo
echo "[完成] 已生成到 dist/。本地预览：$PY www/preview.py ，然后打开 http://localhost:8000/hot100-cpp-learning/"
