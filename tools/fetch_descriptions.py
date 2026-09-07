#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
fetch_descriptions.py — 从 leetcode.cn GraphQL 拉取 HOT 100 题目的中文描述，缓存到 data/descriptions.json。

用法：
    python tools/fetch_descriptions.py            # 只抓缺失的
    python tools/fetch_descriptions.py --force    # 全部重抓

只依赖 requests（pip install requests）。结果 json 结构：
    { "<题号>": {"slug": "...", "title": "中文题名", "zh": "<html>", "en": "<html>"} }
"""
import json, os, re, sys, time, io

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
DATA = os.path.join(ROOT, "data", "descriptions.json")
READ = os.path.join(ROOT, "README.md")

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

QUERY = """query questionData($titleSlug:String!){question(titleSlug:$titleSlug){translatedTitle titleSlug content translatedContent}}"""

def read_meta():
    """从 README 索引表读 {num: {'slug','title'}}。"""
    txt = open(READ, encoding='utf-8').read()
    m = {}
    for line in txt.split('\n'):
        r = re.match(r'^\|\s*\*\*Q(\d+)\*\*\s*\|\s*([^|]+)\|\s*(Easy|Medium|Hard)\s*\|\s*([^|]*)\|\s*\[([^\]]+)\]\(([^)]+)\)\s*\|$', line)
        if r:
            m[int(r.group(1))] = {'slug': r.group(5).strip(), 'title': r.group(2).strip()}
    return m

def main():
    import requests
    force = '--force' in sys.argv
    meta = read_meta()
    # load cache
    cached = {}
    if os.path.exists(DATA):
        cached = json.load(open(DATA, encoding='utf-8'))
    s = requests.Session()
    s.headers.update({
        'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36',
        'Referer': 'https://leetcode.cn/problems/',
        'content-type': 'application/json',
    })
    try:
        s.get('https://leetcode.cn/', timeout=20)  # 拿 cookie
    except Exception as e:
        print("warn: cookie 获取失败", e)
    todo = [n for n, info in meta.items() if force or str(n) not in cached]
    print(f"共 {len(meta)} 题，需抓取 {len(todo)} 题...")
    ok = fail = 0
    for i, num in enumerate(sorted(todo), 1):
        slug = meta[num]['slug']
        try:
            r = s.post('https://leetcode.cn/graphql/', json={"query": QUERY, "variables": {"titleSlug": slug}}, timeout=30)
            q = r.json().get('data', {}).get('question')
            if not q:
                # 可能被反爬，多试一次带 Referer 到具体题
                s.headers['Referer'] = 'https://leetcode.cn/problems/%s/' % slug
                r = s.post('https://leetcode.cn/graphql/', json={"query": QUERY, "variables": {"titleSlug": slug}}, timeout=30)
                q = r.json().get('data', {}).get('question')
            zh = q.get('translatedContent') or ''
            en = q.get('content') or ''
            if not zh and not en:
                raise ValueError('empty content')
            cached[str(num)] = {'slug': slug, 'title': q.get('translatedTitle') or meta[num]['title'],
                                'zh': zh, 'en': en}
            ok += 1
            print(f"  [{i}/{len(todo)}] Q{num} {slug} zh={len(zh)} en={len(en)}")
        except Exception as e:
            fail += 1
            print(f"  [FAIL] Q{num} {slug}: {e}")
        os.makedirs(os.path.dirname(DATA), exist_ok=True)
        json.dump(cached, open(DATA, 'w', encoding='utf-8'), ensure_ascii=False, indent=1)
        time.sleep(0.25)
    print(f"\n完成：成功 {ok}，失败 {fail}，总数 {len(cached)} -> {DATA}")

if __name__ == '__main__':
    main()
