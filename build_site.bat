@echo off
rem 一键生成图解网站（Windows 双击即可）
cd /d "%~dp0"
echo 正在生成 Astro + Svelte + Tailwind 讲解网站...
python tools\gen_animations.py
if errorlevel 1 (
  echo.
  echo [失败] 动画数据生成出错。请检查上方报错。
  pause
  exit /b 1
)
pushd web
call npm run build
set BUILD_ERR=%errorlevel%
popd
if not "%BUILD_ERR%"=="0" (
  echo.
  echo [失败] 前端构建出错。请确认已安装 Node.js 和 npm，并检查上方报错。
  pause
  exit /b 1
)
echo.
echo [完成] 已生成到 site\ 文件夹。
echo 打开方式：双击 site\index.html（推荐在浏览器里打开）
pause
