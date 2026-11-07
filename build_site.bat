@echo off
rem 生成 Hot 100 图解网站（Windows），需要 Python 3 和 Node.js 18+
cd /d "%~dp0"
python tools\gen_animations.py || goto :err
python tools\gen_site.py --json --data-only || goto :err
node www\build.mjs %* || goto :err
echo.
echo [完成] 已生成到 dist\。本地预览：python www\preview.py ，然后打开 http://localhost:8000/hot100-cpp-learning/
pause
exit /b 0
:err
echo [失败] 构建出错，请查看上面的报错。
pause
exit /b 1
