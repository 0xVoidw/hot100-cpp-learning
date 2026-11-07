"""本地预览 dist/：把 /hot100-cpp-learning/ 映射到 dist 目录（与 GitHub Pages 路径一致）。

用法：python www/preview.py [端口，默认 8000]
"""
import http.server
import os
import sys

ROOT = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'dist')
PREFIX = '/hot100-cpp-learning'


class Handler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=ROOT, **kwargs)

    def translate_path(self, path):
        if path.startswith(PREFIX):
            path = path[len(PREFIX):] or '/'
        return super().translate_path(path)

    def send_error(self, code, message=None, explain=None):
        if code == 404 and os.path.exists(os.path.join(ROOT, '404.html')):
            self.send_response(404)
            self.send_header('Content-Type', 'text/html; charset=utf-8')
            self.end_headers()
            with open(os.path.join(ROOT, '404.html'), 'rb') as f:
                self.wfile.write(f.read())
            return
        super().send_error(code, message, explain)


if __name__ == '__main__':
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 8000
    if not os.path.isdir(ROOT):
        sys.exit('还没有 dist/，请先运行 bash build_site.sh 或 build_site.bat')
    print(f'预览：http://localhost:{port}{PREFIX}/  （Ctrl+C 结束）')
    http.server.ThreadingHTTPServer(('', port), Handler).serve_forever()
