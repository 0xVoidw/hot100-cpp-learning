#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
gen_site.py — 把 LeetCode Hot 100 学习库（src/*.cpp + README 索引）生成为一个
纯静态、可离线双击打开的站点。

用法：
    python tools/gen_site.py            # 生成 site/
    python tools/gen_site.py --json     # 导出构建数据（build/ 与 web/src/data/）
    python tools/gen_site.py --json --data-only  # 只导出数据，不生成旧版页面
"""
import re, os, sys, json, glob, html, collections
from math import inf

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
SRC = os.path.join(ROOT, "src")
OUT = os.path.join(ROOT, "site")
ASSETS = os.path.join(OUT, "assets")
PROBLEMS = os.path.join(OUT, "problems")

# ---------------------------------------------------------------------------
# 小工具
# ---------------------------------------------------------------------------
def brace_match(s, start, open_b='{', close_b='}'):
    """s[start] 必须是 open_b（普通状态），返回配对的 close_b 下标。找不到返回 -1。

    会跳过 string / char 字面量以及 // 与 /* */ 注释里的花括号，避免像
    `if (c == '{')` 这类字符字面量把括号计数打乱。
    """
    assert s[start] == open_b, f"expected '{open_b}' at {start}, got {s[start]!r}"
    depth = 0
    i = start
    n = len(s)
    while i < n:
        c = s[i]
        if c == open_b:
            depth += 1; i += 1; continue
        if c == close_b:
            depth -= 1
            if depth == 0:
                return i
            i += 1; continue
        if c == '"':
            i += 1
            while i < n and s[i] != '"':
                if s[i] == '\\':
                    i += 1
                i += 1
            i += 1; continue
        if c == "'":
            i += 1
            while i < n and s[i] != "'":
                if s[i] == '\\':
                    i += 1
                i += 1
            i += 1; continue
        if c == '/' and i + 1 < n and s[i + 1] == '/':
            j = s.find('\n', i)
            i = n if j < 0 else j
            continue
        if c == '/' and i + 1 < n and s[i + 1] == '*':
            j = s.find('*/', i + 2)
            i = n if j < 0 else j + 2
            continue
        i += 1
    return -1

def clean_inline(s):
    s = html.escape(s)
    # 轻量 Markdown：`code` 变 <code>
    s = re.sub(r'`([^`]+)`', r'<code>\1</code>', s)
    s = re.sub(r'\*\*([^*]+)\*\*', r'<strong>\1</strong>', s)
    return s

def split_csv_numbers(txt):
    txt = txt.strip()
    if not txt:
        return []
    out = []
    for t in txt.split(','):
        t = t.strip()
        if not t:
            continue
        try:
            out.append(int(t))
        except ValueError:
            pass
    return out

# ---------------------------------------------------------------------------
# 题目文档解析（src/*.cpp 中的 /* ... Q<num>. ... */ 块）
# ---------------------------------------------------------------------------
HDR_RE = re.compile(r'^\s*\*\s*Q(\d+)(\s*/\s*Q(\d+))?\.\s*(.+?)\s{2,}(Easy|Medium|Hard)(\s*/\s*(Easy|Medium|Hard))?\s*$', re.M)
URL_RE = re.compile(r'https://leetcode\.cn/problems/[^\s*/]+')

def parse_block_doc(block):
    """从一个 /* */ 块里抽 (nums, titles, diffs, url, body)。body 是去头尾后的讲解文本行。"""
    m = HDR_RE.search(block)
    if not m:
        return None
    nums = [m.group(1)]
    diffs = [m.group(5)]
    if m.group(3):
        nums.append(m.group(3))
    if m.group(7):
        diffs.append(m.group(7))
    title = m.group(4).strip()
    url_m = URL_RE.search(block)
    url = url_m.group(0) if url_m else ''
    lines = []
    for ln in block.split('\n'):
        t = ln.strip()
        t = re.sub(r'^\*\s?', '', t)
        t = t.strip()
        if not t:
            continue
        if t in ('/*', '*/', '*') or t.startswith('* '):
            continue
        if t.startswith('---') :
            continue
        if re.match(r'^Q\d+(\s*/\s*Q\d+)?\.\s', t):
            continue
        if t.startswith('https://'):
            continue
        lines.append(t)
    return {'nums': nums, 'title': title, 'diffs': diffs, 'url': url, 'body': lines}

NS_RE = re.compile(r'namespace\s+lc\d+\s*\{')
def grab_namespaces(text):
    """返回 text 里所有 namespace lcNNNN { ... } 的代码文本（有序）。"""
    out = []
    for m in NS_RE.finditer(text):
        bopen = text.find('{', m.start())
        if bopen < 0:
            continue
        bend = brace_match(text, bopen)
        if bend < 0:
            continue
        out.append(text[m.start():bend + 1])
    return out

TEST_RE = re.compile(r'TEST\s*\(\s*([a-zA-Z_]\w*)\s*,\s*([a-zA-Z_]\w*)\s*\)\s*\{')
def grab_tests(text):
    """返回 TEST 块列表 [(suite, name, body_text)]。"""
    res = []
    for m in TEST_RE.finditer(text):
        ob = m.end() - 1   # TEST_RE 以 '{' 结尾，m.end() 已越过开括号，故取它前一个字符
        if text[ob] != '{':
            continue
        cb = brace_match(text, ob)
        if cb < 0:
            continue
        res.append((m.group(1), m.group(2), text[m.end():cb]))
    return res

# ---------------------------------------------------------------------------
# 从 TEST 里提「可画图」的数据
# ---------------------------------------------------------------------------
def extract_diagram(testbodies):
    """对一份题目所有 TEST 文本，挑一张最合适的结构图数据。

    优先级：二叉树 > 链表(含环) > 矩阵/字符网格 > 一维数组 > 字符串。
    同一类内选最优（结点最多 / 格子最多 / 序列最长），避免被次要测试数据抢走。
    """
    joined = '\n'.join(testbodies)
    vecint = re.compile(r'vector<int>\s+(\w+)\s*(?:=\s*)?\{([^}]*)\}')

    # ---- 二叉树 ----
    trees = []
    for tm in re.finditer(r'buildTree\s*\(\s*\{([^}]*)\}', joined):
        tokens = [t.strip() for t in tm.group(1).split(',')]
        vals = []
        for t in tokens:
            if not t:
                continue
            if t.lower().startswith('nullopt') or t == 'null':
                vals.append(None)
            else:
                try:
                    vals.append(int(t))
                except ValueError:
                    vals.append(t)
        if any(v is not None for v in vals):
            trees.append(vals)
    if trees:
        vals = max(trees, key=lambda v: sum(1 for x in v if x is not None))
        return {'kind': 'tree', 'vals': vals}

    # ---- 链表（buildList({..}) / buildList(var) / var=buildList(..)；makeCycle 关联变量）----
    vecdecl = {}
    for m in vecint.finditer(joined):
        vecdecl[m.group(1)] = split_csv_numbers(m.group(2))
    cyc = {}
    for m in re.finditer(r'makeCycle\s*\(\s*(\w+)\s*,\s*(\d+)\s*\)', joined):
        cyc[m.group(1)] = int(m.group(2))
    candidates = []
    for lm in re.finditer(r'buildList\s*\(\s*\{\s*([^}]*)\s*\}\s*\)', joined):
        candidates.append((split_csv_numbers(lm.group(1)), None))
    for lm in re.finditer(r'(\w+)\s*=\s*buildList\s*\(\s*\{\s*([^}]*)\s*\}\s*\)', joined):
        candidates.append((split_csv_numbers(lm.group(2)), cyc.get(lm.group(1))))
    for lm in re.finditer(r'(\w+)\s*=\s*buildList\s*\(\s*([A-Za-z_]\w*)\s*\)', joined):
        if lm.group(2) in vecdecl:
            candidates.append((vecdecl[lm.group(2)], cyc.get(lm.group(1))))
    for lm in re.finditer(r'buildIntersect\s*\(\s*((?:[^{}]|\{[^{}]*})*)\s*\)', joined):
        for aa in re.finditer(r'\{([^{}]*)}', lm.group(1)):
            candidates.append((split_csv_numbers(aa.group(1)), None))
    for lm in re.finditer(r'buildRandList\s*\(\s*\{([^}]*)}', joined):
        candidates.append((split_csv_numbers(lm.group(1)), None))
    if candidates:
        withcycle = [c for c in candidates if c[1] is not None]
        best = max(withcycle, key=lambda c: len(c[0])) if withcycle else max(candidates, key=lambda c: len(c[0]))
        if best[0]:
            return {'kind': 'list', 'vals': best[0], 'cycle': best[1]}

    # ---- 矩阵 / 字符网格（vector<vector<int>> / vector<vector<char>> / vector<string>）----
    grids = []
    for mm in re.finditer(r'vector<vector<int>>\s+\w+\s*\{((?:[^{}]|\{[^{}]*\})*)', joined):
        rows = []
        for rm in re.finditer(r'\{([^{}]*)\}', mm.group(1)):
            rows.append(split_csv_numbers(rm.group(1)))
        if rows and any(rows):
            grids.append(rows)
    for mm in re.finditer(r'vector<vector<char>>\s+\w+\s*\{((?:[^{}]|\{[^{}]*\})*)', joined):
        rows = []
        for rm in re.finditer(r'\{([^{}]*)\}', mm.group(1)):
            cells = [c.strip().strip("'\"") for c in rm.group(1).split(',') if c.strip()]
            rows.append(cells)
        if rows:
            grids.append(rows)
    for sm in re.finditer(r'vector<string>\s+\w+\s*\{([^}]*)\}', joined):
        rows = [[c for c in re.findall(r'"([^"])"', s)] for s in re.findall(r'"[^"]*"', sm.group(1))]
        rows = [r for r in rows if r]
        if rows:
            grids.append(rows)
    if grids:
        rows = max(grids, key=lambda r: len(r) * max((len(x) for x in r), default=0))
        return {'kind': 'matrix', 'rows': rows}

    # ---- 一维数组 ----
    arrs = []
    for m in vecint.finditer(joined):
        v = split_csv_numbers(m.group(2))
        if len(v) >= 2:
            arrs.append(v)
    if arrs:
        return {'kind': 'array', 'vals': max(arrs, key=len)}

    # ---- 字符串（只认纯字母词，避免被测试里零散字面量干扰）----
    strs = re.findall(r'"([A-Za-z]{2,30})"', joined)
    if strs:
        return {'kind': 'string', 's': max(strs, key=len)}

    return None
# SVG 渲染
# ---------------------------------------------------------------------------
def esc(v):
    """SVG/text 节点里安全转义。"""
    return html.escape(str(v))


def render_tree_svg(vals):
    # 用 BFS 建树：level-order 数组 + None 空洞
    class N:
        __slots__ = ('v', 'left', 'right', 'depth', 'x')
        def __init__(self, v, depth):
            self.v = v; self.left = None; self.right = None
            self.depth = depth; self.x = 0
    if not vals or vals[0] is None:
        return ''
    root = N(vals[0], 0)
    q = [root]; i = 1
    while q and i < len(vals):
        cur = q.pop(0)
        if i < len(vals):
            if vals[i] is not None:
                cur.left = N(vals[i], cur.depth + 1); q.append(cur.left)
            i += 1
        if i < len(vals):
            if vals[i] is not None:
                cur.right = N(vals[i], cur.depth + 1); q.append(cur.right)
            i += 1
    # 中序遍历定 x
    nodes = []
    def inorder(n):
        if not n: return
        inorder(n.left); n.x = len(nodes); nodes.append(n); inorder(n.right)
    inorder(root)
    maxdepth = 0
    for n in nodes:
        maxdepth = max(maxdepth, n.depth)
    W = 80; H = 70; PAD = 30; R = 18
    width = W * max(1, len(nodes) - 1) + PAD * 2
    height = H * (maxdepth + 1) + PAD * 2
    parts = [f'<svg viewBox="0 0 {width} {height}" xmlns="http://www.w3.org/2000/svg" class="dia">']
    parts.append(f'<rect width="{width}" height="{height}" fill="#fbfbfd"/>')
    # 边
    def edge(a, b):
        x1 = PAD + a.x * W; y1 = PAD + a.depth * H + R
        x2 = PAD + b.x * W; y2 = PAD + b.depth * H - R
        parts.append(f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" stroke="#d0d5e0" stroke-width="2"/>')
    que = [root]
    while que:
        cur = que.pop(0)
        if cur.left: edge(cur, cur.left); que.append(cur.left)
        if cur.right: edge(cur, cur.right); que.append(cur.right)
    # 节点
    for n in nodes:
        cx = PAD + n.x * W; cy = PAD + n.depth * H
        parts.append(f'<circle cx="{cx}" cy="{cy}" r="{R}" fill="#4f6ef7" stroke="#3a55d6" stroke-width="2"/>')
        parts.append(f'<text x="{cx}" y="{cy+5}" text-anchor="middle" fill="#fff" font-size="15" font-family="Consolas,Menlo,monospace">{esc(n.v)}</text>')
    parts.append('</svg>')
    return '\n'.join(parts)

def render_list_svg(vals, cycle=None):
    if not vals:
        return ''
    SW = 54; GAP = 40; H = 60; PAD = 30
    n = len(vals)
    width = PAD * 2 + n * (SW + GAP) - GAP + 20
    height = H * 2 + PAD * 2
    arrowid = 'arr' + str(abs(hash(tuple(vals))) % 999999)
    parts = [f'<svg viewBox="0 0 {width} {height}" xmlns="http://www.w3.org/2000/svg" class="dia">',
             f'<defs><marker id="{arrowid}" markerWidth="8" markerHeight="8" refX="6" refY="3" orient="auto"><path d="M0,0 L6,3 L0,6 Z" fill="#4f6ef7"/></marker></defs>',
             f'<rect width="{width}" height="{height}" fill="#fbfbfd"/>']
    cx = []
    for i in range(n):
        x = PAD + i * (SW + GAP)
        cx.append(x + SW // 2)
        parts.append(f'<rect x="{x}" y="{PAD}" width="{SW}" height="{H}" rx="8" fill="#eef2ff" stroke="#4f6ef7" stroke-width="2"/>')
        parts.append(f'<text x="{x+SW//2}" y="{PAD+H//2+5}" text-anchor="middle" fill="#16204a" font-size="16" font-family="Consolas,Menlo,monospace">{esc(vals[i])}</text>')
        parts.append(f'<text x="{x+SW//2}" y="{PAD+H+16}" text-anchor="middle" fill="#8893b8" font-size="12">i{esc(i)}</text>')
    for i in range(n - 1):
        x1 = cx[i] + SW // 2; y = PAD + H // 2
        x2 = cx[i + 1] - SW // 2 - 2
        parts.append(f'<line x1="{x1}" y1="{y}" x2="{x2}" y2="{y}" stroke="#4f6ef7" stroke-width="2" marker-end="url(#{arrowid})"/>')
    if cycle is not None and 0 <= cycle < n:
        fromY = cx[-1] + SW // 2
        toX = cx[cycle]
        parts.append(f'<path d="M {fromY} {PAD + H} C {fromY} {PAD + H + 34} , {toX} {PAD + H + 34} , {toX} {PAD + H}" fill="none" stroke="#e2554d" stroke-width="2" marker-end="url(#{arrowid})"/>')
    parts.append('</svg>')
    return '\n'.join(parts)

def render_matrix_svg(rows):
    if not rows:
        return ''
    R = len(rows); C = max(len(r) for r in rows)
    # 给行、列坐标留出独立的边距。此前 j 标签被重复画在每一行底部，
    # i 标签又落在上一行单元格内，较大的矩阵会显得像“标号乱飞”。
    CS = 56; RS = 44
    LEFT = 36; TOP = 30; RIGHT = 16; BOTTOM = 16
    width = LEFT + C * CS + RIGHT; height = TOP + R * RS + BOTTOM
    parts = [f'<svg viewBox="0 0 {width} {height}" xmlns="http://www.w3.org/2000/svg" class="dia">']
    parts.append(f'<rect width="{width}" height="{height}" fill="#fbfbfd"/>')
    for j in range(C):
        x = LEFT + j * CS
        parts.append(f'<text x="{x+CS//2}" y="18" text-anchor="middle" fill="#8893b8" font-size="11">j{j}</text>')
    for i, row in enumerate(rows):
        y = TOP + i * RS
        parts.append(f'<text x="{LEFT-10}" y="{y+RS//2+4}" text-anchor="end" fill="#8893b8" font-size="11">i{esc(i)}</text>')
        for j in range(C):
            x = LEFT + j * CS
            val = row[j] if j < len(row) else ''
            zero = (val == 0)
            fill = '#ffe9e6' if zero else '#eef2ff'
            stroke = '#e2554d' if zero else '#4f6ef7'
            parts.append(f'<rect x="{x+2}" y="{y+2}" width="{CS-4}" height="{RS-4}" rx="6" fill="{fill}" stroke="{stroke}" stroke-width="1.6"/>')
            parts.append(f'<text x="{x+CS//2}" y="{y+RS//2+5}" text-anchor="middle" fill="#16204a" font-size="16" font-family="Consolas,Menlo,monospace">{esc(val)}</text>')
    parts.append('</svg>')
    return '\n'.join(parts)

def render_seq_svg(vals):
    """一维数组或字符串序列：方框 + 下标。"""
    if not vals:
        return ''
    SW = 46; GAP = 8; H = 50; PAD = 30
    n = len(vals)
    width = PAD * 2 + n * SW + (n - 1) * GAP
    height = PAD * 2 + H + 22
    parts = [f'<svg viewBox="0 0 {width} {height}" xmlns="http://www.w3.org/2000/svg" class="dia">']
    parts.append(f'<rect width="{width}" height="{height}" fill="#fbfbfd"/>')
    for i, v in enumerate(vals):
        x = PAD + i * (SW + GAP)
        parts.append(f'<rect x="{x}" y="{PAD}" width="{SW}" height="{H}" rx="7" fill="#eef2ff" stroke="#4f6ef7" stroke-width="1.8"/>')
        parts.append(f'<text x="{x+SW//2}" y="{PAD+H//2+6}" text-anchor="middle" fill="#16204a" font-size="16" font-family="Consolas,Menlo,monospace">{esc(v)}</text>')
        parts.append(f'<text x="{x+SW//2}" y="{PAD+H+16}" text-anchor="middle" fill="#8893b8" font-size="12">{esc(i)}</text>')
    parts.append('</svg>')
    return '\n'.join(parts)

# ---------------------------------------------------------------------------
# 手工「讲透」图解：DP 表 / 指针走位 / 数据结构，只为把「动起来」讲清楚。
# 这些是示意（SHOW = 该题的核心套路），数值是我 hand 写准的示例，未必与仓库测试一致。
# ---------------------------------------------------------------------------
def svg_seq(vals, marks=None):
    """一维序列：方框 + 下标；marks=[{i,label,color,dy,arrow}] 在上方标指针/窗口。"""
    if not vals:
        return ''
    SW, GAP, H, PAD = 50, 8, 52, 30
    n = len(vals)
    width = PAD * 2 + n * SW + (n - 1) * GAP
    height = PAD * 2 + H + 40
    p = [f'<svg viewBox="0 0 {width} {height}" xmlns="http://www.w3.org/2000/svg" class="dia">',
         f'<rect width="{width}" height="{height}" fill="#fbfbfd"/>']
    for i, v in enumerate(vals):
        x = PAD + i * (SW + GAP)
        p.append(f'<rect x="{x}" y="{PAD}" width="{SW}" height="{H}" rx="7" fill="#eef2ff" stroke="#4f6ef7" stroke-width="1.8"/>')
        p.append(f'<text x="{x+SW//2}" y="{PAD+H//2+6}" text-anchor="middle" fill="#16204a" font-size="16" font-family="Consolas,Menlo,monospace">{esc(v)}</text>')
        p.append(f'<text x="{x+SW//2}" y="{PAD+H+15}" text-anchor="middle" fill="#8893b8" font-size="12">{esc(i)}</text>')
    if marks:
        for mk in marks:
            idx = mk['i']; col = mk.get('color', '#e2554d'); dy = mk.get('dy', 0)
            cx = PAD + idx * (SW + GAP) + SW // 2
            ay = PAD - 6 - dy
            p.append(f'<line x1="{cx}" y1="{ay}" x2="{cx}" y2="{PAD}" stroke="{col}" stroke-width="2"/>')
            p.append(f'<polygon points="{cx-4},{PAD-8} {cx+4},{PAD-8} {cx},{PAD}" fill="{col}"/>')
            if mk.get('label'):
                p.append(f'<text x="{cx}" y="{ay-6}" text-anchor="middle" fill="{col}" font-size="13" font-weight="700">{mk["label"]}</text>')
            if mk.get('bracket'):
                p.append(f'<line x1="{PAD+idx*(SW+GAP)}" y1="{ay}" x2="{PAD+mk["bracket"]*(SW+GAP)+SW}" y2="{ay}" stroke="{col}" stroke-width="3"/>')
    p.append('</svg>')
    return '\n'.join(p)

def svg_grid(rows, hl=None, note=''):
    """二维表（DP/网格）。hl=(r,c) 高亮。"""
    if not rows:
        return ''
    R = len(rows); C = max(len(r) for r in rows)
    CS, RS, PAD = 58, 46, 24
    width = C * CS + PAD * 2; height = R * RS + PAD * 2 + (16 if note else 0)
    p = [f'<svg viewBox="0 0 {width} {height}" xmlns="http://www.w3.org/2000/svg" class="dia">',
         f'<rect width="{width}" height="{height}" fill="#fbfbfd"/>']
    for i, row in enumerate(rows):
        for j in range(C):
            val = row[j] if j < len(row) else ''
            x = PAD + j * CS; y = PAD + i * RS
            ishl = hl == (i, j)
            fill = '#fde047' if ishl else '#eef2ff'
            p.append(f'<rect x="{x+2}" y="{y+2}" width="{CS-4}" height="{RS-4}" rx="6" fill="{fill}" stroke="#4f6ef7" stroke-width="1.6"/>')
            p.append(f'<text x="{x+CS//2}" y="{y+RS//2+5}" text-anchor="middle" fill="#16204a" font-size="15" font-family="Consolas,Menlo,monospace">{esc(val)}</text>')
            if i == 0 or j == 0:
                p.append(f'<text x="{x+CS//2}" y="{y-2}" text-anchor="middle" fill="#c98a00" font-size="11">{j if i==0 else ""}</text>')
    if note:
        p.append(f'<text x="{PAD}" y="{height-4}" fill="#8893b8" font-size="12">{note}</text>')
    p.append('</svg>')
    return '\n'.join(p)

def hand_diagram(num):
    d = {}
    # Q1 两数之和：数组 + 哈希表已见过的值
    d[1] = (svg_seq([2, 7, 11, 15], [{'i': 0, 'label': 'l', 'color': '#4f6ef7'},
                                      {'i': 1, 'label': 'r', 'color': '#e2554d'}]) +
            '<div class="fig-cap">扫描到 <code>nums[1]=7</code>，查 <code>seen[9-7=2]</code> 命中 <code>下标0</code>，返回 <code>[0,1]</code>。先查后存，避免自配对。</div>')
    # Q70 爬楼梯
    d[70] = (svg_seq([1, 2, 3, 5, 8], [{'i': 0, 'label': 'n=1'}, {'i': 1, 'label': 'n=2'},
                                       {'i': 2, 'label': 'n=3'}, {'i': 3, 'label': 'n=4'},
                                       {'i': 4, 'label': 'n=5', 'color': '#e2554d'}]) +
             '<div class="fig-cap">到第 n 阶只能从 n-1 或 n-2 来：<code>dp[n]=dp[n-1]+dp[n-2]</code>。1,2,3,5,8 … 就是斐波那契。</div>')
    # Q62 不同路径
    d[62] = (svg_grid([[1,1,1,1,1,1,1],[1,2,3,4,5,6,7],[1,3,6,10,15,21,28]], hl=(2,6),
                      note='dp[i][j]=dp[i-1][j]+dp[i][j-1]，起点、顶行、左列都是 1；右下角 28 为答案。') +
             '<div class="fig-cap">只能向右/向下走。一个格子 = 上面 + 左边，填完即得路径总数。</div>')
    # Q118 杨辉三角
    d[118] = (svg_grid([[1],[1,1],[1,2,1],[1,3,3,1],[1,4,6,4,1]], note='每行首尾为 1，中间的 = 上一行相邻两个之和。') +
              '<div class="fig-cap">第 k 行第 j 个数 = C(k,j)。从上一行递推即可。</div>')
    # Q108 有序数组转 BST
    d[108] = (render_tree_svg([0, -10, 5, None, None, 3, 9]) +
              '<div class="fig-cap">有序数组 <code>[-10,-3,0,5,9]</code>：取中点为根(0)，左半[-10,-3]做左子树、右半[5,9]做右子树。因为有序，中点即「大小居中」。</div>')
    # Q155 最小栈
    d[155] = ('<div class="twocol">' + svg_seq([-2, 0, -3]) + svg_seq([-2, 0, -3]) + '</div>'
              '<div class="fig-cap">左边是正常栈，右边是「辅助最小栈」：每次压栈时，若新值 ≤ 当前最小值则同步压入。出栈时若弹出的值等于最小栈顶，则最小栈也弹。存过的「当时最小值」随时可查。</div>')
    # Q20 有效的括号
    d[20] = (svg_seq(list('()[]{}'), [{'i': 0, 'label': '('}, {'i': 1, 'label': ')', 'color': '#e2554d'},
                                       {'i': 2, 'label': '[', 'dy': 26}, {'i': 3, 'label': ']', 'color': '#e2554d', 'dy': 26},
                                       {'i': 4, 'label': '{', 'dy': 52}, {'i': 5, 'label': '}', 'color': '#e2554d', 'dy': 52}]) +
            '<div class="fig-cap">遍历字符：是左括号就入栈；是右括号就检查栈顶是否为对应左括号。栈是 LIFO，天然匹配「最近嵌套」的一对。</div>')
    # Q22 括号生成
    d[22] = (svg_seq(list('((()))'), [{'i': 0, 'label': 'a'}, {'i': 5, 'label': 'a\'', 'dy': 24}]) +
            '<div class="fig-cap">生成规则：能放左括号就放左；右括号只有当「已放右括号数 < 已放左括号数」时才放。最终长度 2n，且任意前缀 左≥右。</div>')
    # Q32 最长有效括号
    d[32] = (svg_seq(list(')()())'), [{'i': 1, 'label': 'L', 'color': '#4f6ef7'},
                                       {'i': 4, 'label': 'R', 'color': '#e2554d', 'bracket': 4}]) +
            '<div class="fig-cap">有效段 <code>()()</code> 长度 4（下标 1..4）。思路：存下标做栈，用「上一个不匹配位置」当左界，每段长度 = i - 栈顶。</div>')
    # Q51 N 皇后
    d[51] = (svg_grid([['.','Q','.','.'],['.','.','.','Q'],['Q','.','.','.'],['.','.','Q','.']], note='任意两个皇后不在同一行列对角线') +
            '<div class="fig-cap">逐行放皇后，col / 两条对角线判冲突。回溯：放不下就换一列；一种解是一棵搜索树的叶子。</div>')
    # Q146 LRU
    d[146] = ('<div class="fig-cap">容量 2，顺序操作 put(1,1) → put(2,2) → get(1) → put(3,3)。只有 get/put 命中时把该键移到「最近使用」端；put 新键且已满时踢掉「最久未用」端。</div>')
    # Q295 数据流中位数
    d[295] = ('<div class="fig-cap">两个堆：左=大顶堆存较小的一半，右=小顶堆存较大的一半，保持左.size ≤ 右.size 且相差 ≤ 1。所有数始终分成两段，中位数就是左堆顶 或 (左顶+右顶)/2。</div>')
    # Q42 接雨水（经典双指针）
    d[42] = (svg_seq([0, 1, 0, 2, 1, 0, 1, 3, 2, 1, 2, 1],
                     [{'i': 0, 'label': 'L', 'color': '#4f6ef7', 'dy': 0},
                      {'i': 11, 'label': 'R', 'color': '#e2554d', 'dy': 0}]) +
             '<div class="fig-cap">双指针：只移动「偏低」的一侧，维护左右最高柱。每根柱子能接的水 = min(左max,右max) - height[i]（≥0）。</div>')
    # Q3 无重复最长子串
    d[3] = (svg_seq(list('abcabcbb'), [{'i': 0, 'label': 'L', 'color': '#4f6ef7'},
                                       {'i': 2, 'label': 'R', 'color': '#e2554d', 'bracket': 2}]) +
            '<div class="fig-cap">滑动窗口 [L,R)：右指针扩；遇到重复就把 L 跳到重复字符后面。窗口内始终无重复，长度取最大。</div>')
    html = d.get(num, '')
    if not html:
        return ''
    return f'<div class="fig"><div class="fig-body">{html}</div></div>'
def diagram_html(d):
    if not d:
        return ''
    if d['kind'] == 'tree':
        svg = render_tree_svg(d['vals'])
        cap = '二叉树（来自本仓库测试数据，`nullopt` 表示空位）'
    elif d['kind'] == 'list':
        svg = render_list_svg(d['vals'], d.get('cycle'))
        cap = '链表结构（来自本仓库测试数据）' + ('；红箭头表示环' if d.get('cycle') is not None else '')
    elif d['kind'] == 'matrix':
        svg = render_matrix_svg(d['rows'])
        cap = '矩阵（来自本仓库测试数据，红色格子 = 0）'
    elif d['kind'] == 'array':
        svg = render_seq_svg(d['vals'])
        cap = '数组（来自本仓库测试数据，下标从 0 开始）'
    elif d['kind'] == 'string':
        svg = render_seq_svg(list(d['s']))
        cap = f'字符串 `<{d["s"]}>`（来自本仓库测试数据，下标从 0 开始）'
    else:
        return ''
    if not svg:
        return ''
    return f'<div class="fig"><div class="fig-body">{svg}</div><div class="fig-cap">{cap}</div></div>'

# ---------------------------------------------------------------------------
# 组装整个站
# ---------------------------------------------------------------------------
def read_file(path):
    return open(os.path.join(SRC, path), encoding='utf-8').read()

def main():
    os.makedirs(PROBLEMS, exist_ok=True)
    os.makedirs(ASSETS, exist_ok=True)
    use_json = '--json' in sys.argv
    data_only = '--data-only' in sys.argv

    # 1) 读 README 索引表 -> num -> (title,diff,套路,slug,url)
    readme = open(os.path.join(ROOT, 'README.md'), encoding='utf-8').read()
    table = {}
    for line in readme.split('\n'):
        m = re.match(r'^\|\s*\*\*Q(\d+)\*\*\s*\|\s*([^|]+)\|\s*(Easy|Medium|Hard)\s*\|\s*([^|]*)\|\s*\[([^\]]+)\]\(([^)]+)\)\s*\|$', line)
        if m:
            num = int(m.group(1))
            table[num] = {'title': m.group(2).strip(), 'diff': m.group(3),
                          'tag': m.group(4).strip(), 'slug': m.group(5).strip(), 'url': m.group(6).strip()}

    # 2) 解析每类 src 文件里的题目
    problems = []  # 每项: dict(num,title,diff,url,tag, category, body, code, tests, diagram, desc)
    descs = {}
    try:
        descs = json.load(open(os.path.join(ROOT, 'data', 'descriptions.json'), encoding='utf-8'))
    except FileNotFoundError:
        print('[warn] data/descriptions.json 不存在，将不显示题目描述。可运行 tools/fetch_descriptions.py 生成。')
    anims = {}
    try:
        anims = json.load(open(os.path.join(ROOT, 'data', 'animations.json'), encoding='utf-8'))
    except FileNotFoundError:
        pass
    file_order = sorted(glob.glob(os.path.join(SRC, '*.cpp')))
    header_map = {}   # 每文件第一个 /* */ 以外的中文/分类说明，这里略
    catname = lambda fname: os.path.basename(fname).split('_', 1)[1].replace('.cpp', '')

    for fpath in file_order:
        fname = os.path.basename(fpath)
        if fname == 'main.cpp':
            continue
        txt = read_file(fname)
        category = catname(fname)
        # 找出全部 doc 块及其在文本中的位置
        docs = []
        for m in re.finditer(r'/\*.*?\*/', txt, re.S):
            doc = parse_block_doc(m.group(0))
            if doc:
                docs.append((m.start(), m.end(), doc))
        # 相邻 doc 之间就是一个题目的区域：namespace + TEST
        for i, (dstart, dend, doc) in enumerate(docs):
            region = txt[dend: docs[i + 1][0] if i + 1 < len(docs) else len(txt)]
            nss = grab_namespaces(region)
            tests = grab_tests(region)
            testbodies = [b for (_, _, b) in tests]
            diag = extract_diagram(testbodies)
            bodies = doc['body']
            diffs = doc['diffs']
            for k, numstr in enumerate(doc['nums']):
                num = int(numstr)
                meta = table.get(num, {})
                code = nss[k] if k < len(nss) else (nss[0] if nss else '')
                d = diffs[k] if k < len(diffs) else meta.get('diff', '')
                problems.append({
                    'num': num,
                    'title': meta.get('title', doc['title']),
                    'diff': meta.get('diff', d),
                    'url': meta.get('url', doc['url']),
                    'slug': meta.get('slug', ''),
                    'tag': meta.get('tag', ''),
                    'category': category,
                    'body': bodies,
                    'code': code,
                    'tests': [(s, n, b) for (s, n, b) in tests],
                    'diagram': diag,
                    'desc': descs.get(str(num), {}),
                    'anim': anims.get(str(num)),
                })

    # 按题目号排序
    problems.sort(key=lambda p: p['num'])

    if use_json:
        # 前端复用解析后的源码/测试/描述/动画数据；图解预渲染为可信的本地 SVG/HTML。
        frontend_problems = []
        for problem in problems:
            item = dict(problem)
            # 力扣 HTML 偶尔在行尾带空格；导出前标准化，减少静态产物无意义 diff。
            desc = dict(problem['desc']) if isinstance(problem['desc'], dict) else {}
            for key, value in desc.items():
                if isinstance(value, str):
                    desc[key] = re.sub(r'(?m) +$', '', value)
            item['desc'] = desc
            item['diagramHtml'] = diagram_html(problem['diagram']) + hand_diagram(problem['num'])
            frontend_problems.append(item)
        targets = [
            os.path.join(ROOT, 'build', 'problems.json'),
            os.path.join(ROOT, 'web', 'src', 'data', 'problems.json'),
        ]
        for target in targets:
            os.makedirs(os.path.dirname(target), exist_ok=True)
            with open(target, 'w', encoding='utf-8', newline='\n') as f:
                json.dump(frontend_problems, f, ensure_ascii=False, indent=2)
        print(f"[json] 导出 {len(problems)} 题 -> build/problems.json, web/src/data/problems.json")

    if data_only:
        return

    # 3) 渲染页面
    write_assets()
    write_index(problems)
    write_cat_pages(problems)
    write_problem_pages(problems)
    print(f"[site] 生成 {len(problems)} 个题目页 -> {OUT}")

# ---- assets ----
def write_assets():
    css = r"""/* gen_site.css */
:root{
  --bg:#f7f8fc; --card:#ffffff; --ink:#18203a; --muted:#68718c;
  --line:#e5e8f2; --brand:#4f6ef7; --brand2:#3a55d6;
  --easy:#16a34a; --med:#d97706; --hard:#dc2626;
  --code-bg:#0f172a; --code-ink:#e2e8f0;
}
[data-theme=dark]{
  --bg:#0e1220; --card:#171c2e; --ink:#e7eaf6; --muted:#8b94b4;
  --line:#2a3250; --brand:#6c8bff; --brand2:#8fa7ff;
  --code-bg:#0a0e19; --code-ink:#d6ddf0;
}
*{box-sizing:border-box}
body{margin:0;font-family:-apple-system,"Segoe UI",Roboto,"PingFang SC","Microsoft YaHei",sans-serif;background:var(--bg);color:var(--ink);line-height:1.7}
a{color:var(--brand);text-decoration:none}
a:hover{text-decoration:underline}
.wrap{max-width:1080px;margin:0 auto;padding:0 20px}
.topbar{display:flex;align-items:center;gap:14px;padding:14px 0;border-bottom:1px solid var(--line);margin-bottom:6px}
.brand{font-weight:800;font-size:18px;color:var(--ink)}
.brand small{color:var(--muted);font-weight:500;font-size:12px}
.spacer{flex:1}
.iconbtn{background:var(--card);border:1px solid var(--line);border-radius:9px;padding:6px 11px;cursor:pointer;color:var(--ink);font-size:14px}
.hero{padding:22px 0 6px}
.hero h1{font-size:26px;margin:0 0 8px}
.hero p{margin:0;color:var(--muted);font-size:15px;max-width:70ch}
.controls{display:flex;flex-wrap:wrap;gap:10px;margin:18px 0 6px;align-items:center}
.search{flex:1 1 260px;background:var(--card);border:1px solid var(--line);border-radius:10px;padding:10px 14px;font-size:15px;color:var(--ink)}
.chip{border:1px solid var(--line);background:var(--card);border-radius:999px;padding:6px 13px;cursor:pointer;font-size:13px;color:var(--muted)}
.chip.on{background:var(--brand);border-color:var(--brand);color:#fff}
.progressbar{height:10px;background:var(--line);border-radius:999px;overflow:hidden;margin:8px 0}
.progressbar>div{height:100%;background:var(--brand);width:0;transition:width .3s}
.grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(230px,1fr));gap:14px;margin:14px 0 30px}
.card{background:var(--card);border:1px solid var(--line);border-radius:12px;padding:14px;display:flex;flex-direction:column;gap:8px;transition:box-shadow .15s}
.card:hover{box-shadow:0 6px 22px rgba(60,80,180,.10)}
.card .qt{font-weight:700;font-size:15px;color:var(--ink)}
.card .qt .no{color:var(--muted);font-weight:600;font-size:13px;margin-right:6px}
.card .tag{color:var(--brand);font-size:13px}
.badge{display:inline-block;padding:2px 9px;border-radius:999px;font-size:12px;font-weight:700}
.badge.Easy{background:#e7f7ec;color:var(--easy)}
.badge.Medium{background:#fdf0e3;color:var(--med)}
.badge.Hard{background:#fde8e8;color:var(--hard)}
[data-theme=dark] .badge.Easy{background:#12321f;color:#6ee7a0}
[data-theme=dark] .badge.Medium{background:#3a2a10;color:#f6c26b}
[data-theme=dark] .badge.Hard{background:#3a1616;color:#f39a9a}
.done{margin-left:auto}
.cat-h{font-size:20px;margin:26px 0 8px;color:var(--ink);border-left:4px solid var(--brand);padding-left:10px}
.center{text-align:center;color:var(--muted);padding:40px 0}
/* 题目页 */
.crumb{color:var(--muted);font-size:13px;padding:12px 0}
.crumb a{color:var(--muted)}
.dhead{display:flex;align-items:center;gap:12px;flex-wrap:wrap}
.dhead h1{font-size:24px;margin:0}
.meta{color:var(--muted);font-size:14px}
.sect{margin:26px 0}
.sect h2{font-size:19px;margin:0 0 10px;color:var(--ink)}
.card2{background:var(--card);border:1px solid var(--line);border-radius:12px;padding:16px}
.note{background:#f3f6ff;border:1px solid #dfe6ff;border-radius:10px;padding:12px 14px;font-size:14px;color:#27376e;margin:12px 0}
[data-theme=dark] .note{background:#1b2540;border-color:#2f3d63;color:#c9d4f5}
.bodytext p{margin:9px 0}
.bodytext .lab{font-weight:800;color:var(--brand)}
.desc p{margin:8px 0;font-size:15px}
.desc pre{background:var(--bg);border:1px solid var(--line);border-radius:9px;padding:12px 14px;overflow-x:auto;font-size:13px;color:var(--ink);font-family:Consolas,Menlo,monospace;line-height:1.6}
.desc code{background:var(--line);padding:1px 5px;border-radius:4px;font-family:Consolas,Menlo,monospace;font-size:.88em}
.desc strong{color:var(--ink)}
.desc strong.example{color:var(--ink)}
.fig{margin:14px 0}
.fig-body{background:var(--card);border:1px solid var(--line);border-radius:12px;padding:14px;overflow-x:auto}
.fig-body svg{max-width:100%;height:auto;display:block}
.fig-cap{color:var(--muted);font-size:13px;margin-top:8px}
.twocol{display:flex;gap:18px;flex-wrap:wrap}
.twocol>div{flex:1;min-width:200px}
pre.code{background:var(--code-bg);color:var(--code-ink);border-radius:12px;padding:16px;overflow-x:auto;font-family:Consolas,Menlo,monospace;font-size:13px;line-height:1.6;position:relative}
pre.code code{font-family:inherit}
.copy{position:absolute;top:10px;right:12px;background:rgba(255,255,255,.10);color:#fff;border:0;border-radius:7px;padding:4px 9px;font-size:12px;cursor:pointer}
.tok-c{color:#7c86a0;font-style:italic}.tok-k{color:#c084fc}.tok-s{color:#7ee0a3}.tok-n{color:#f59e0b}.tok-p{color:#8ab4ff}.tok-t{color:#7dd3fc}
.tests pre{background:var(--card);border:1px solid var(--line);border-radius:10px;padding:12px;overflow-x:auto;font-size:13px;color:var(--ink)}
.taglist{display:flex;flex-wrap:wrap;gap:8px}
.taglist a{background:var(--line);border-radius:999px;padding:4px 11px;font-size:13px;color:var(--ink)}
@media(max-width:600px){.hero h1{font-size:21px}.grid{grid-template-columns:1fr 1fr}}
/* 动画播放器 */
.anim{margin:6px 0}
.anim-ctl{display:flex;align-items:center;gap:6px;flex-wrap:wrap;padding:10px 12px;background:var(--card);border:1px solid var(--line);border-radius:12px 12px 0 0;border-bottom:0}
.anim-ctl .ab{background:var(--line);border:0;border-radius:8px;padding:6px 11px;cursor:pointer;font-size:14px;color:var(--ink)}
.anim-ctl .ab:hover{background:var(--brand);color:#fff}
.anim-ctl .ab-play{background:var(--brand);color:#fff;font-weight:700}
.anim-note{background:var(--card);border:1px solid var(--line);border-top:0;border-bottom:0;padding:8px 12px;color:var(--ink);font-size:14px;min-height:22px}
.anim-box{background:var(--card);border:1px solid var(--line);border-radius:0 0 12px 12px;padding:12px;overflow-x:auto}
.anim-box svg{max-width:100%;height:auto;display:block}
.anim .speed{color:var(--muted);font-size:13px;margin-left:8px}
.anim .frame{color:var(--muted);font-size:13px;margin-left:auto}
"""
    open(os.path.join(ASSETS, 'site.css'), 'w', encoding='utf-8').write(css)

    # 拷贝动画引擎 + 打包动画脚本库
    import shutil
    shutil.copyfile(os.path.join(ROOT, 'tools', 'anim.js'), os.path.join(ASSETS, 'anim.js'))
    anims = {}
    try:
        anims = json.load(open(os.path.join(ROOT, 'data', 'animations.json'), encoding='utf-8'))
    except FileNotFoundError:
        pass
    open(os.path.join(ASSETS, 'animations.js'), 'w', encoding='utf-8').write(
        'window.LC_ANIMS = ' + json.dumps(anims, ensure_ascii=False) + ';\n')

    js = r"""/* gen_site.js */
(function(){
  function ls(){try{return JSON.parse(localStorage.getItem('hot100_done')||'[]')}catch(e){return[]}}
  function set(a){localStorage.setItem('hot100_done',JSON.stringify(a))}
  function theme(){return localStorage.getItem('hot100_theme')||'light'}
  function applyTheme(){document.documentElement.setAttribute('data-theme',theme())}
  applyTheme();
  document.addEventListener('DOMContentLoaded',function(){
    var t=document.getElementById('themeBtn');
    if(t)t.addEventListener('click',function(){localStorage.setItem('hot100_theme',theme()==='dark'?'light':'dark');applyTheme();t.textContent=theme()==='dark'?'☀':'🌙';});
    // 进度
    var done=ls();
    function update(){var all=document.querySelectorAll('[data-num]');var d=0;all.forEach(function(e){if(done.indexOf(e.getAttribute('data-num'))>=0)d++});var pb=document.getElementById('progressbar');if(pb){var pct=all.length?Math.round(d/all.length*100):0;pb.style.width=pct+'%';var l=document.getElementById('progresslabel');if(l)l.textContent=pct+'% ('+d+'/'+all.length+')';}}
    document.querySelectorAll('[data-check]').forEach(function(cb){cb.addEventListener('change',function(){var n=cb.getAttribute('data-check');done=ls();if(cb.checked){if(done.indexOf(n)<0)done.push(n)}else{done=done.filter(function(x){return x!==n})}set(done);update();})});
    // 搜索/筛选
    var sq=document.getElementById('search');
    var ds=document.querySelectorAll('.chip[data-diff]');
    var cs=document.querySelectorAll('.chip[data-cat]');
    var diff='',cat='',txt='';
    function match(){document.querySelectorAll('.card').forEach(function(c){
      var okDiff=!diff||c.getAttribute('data-diff')===diff;
      var okCat=!cat||c.getAttribute('data-cat')===cat;
      var hay=(c.getAttribute('data-s')||'').toLowerCase();
      var okTxt=!txt||hay.indexOf(txt)>=0;
      c.style.display=(okDiff&&okCat&&okTxt)?'':'none';
    });}
    if(sq)sq.addEventListener('input',function(){txt=sq.value.trim().toLowerCase();match()});
    ds.forEach(function(b){b.addEventListener('click',function(){diff=diff===b.getAttribute('data-diff')?'':b.getAttribute('data-diff');ds.forEach(function(x){x.classList.remove('on')});if(diff)b.classList.add('on');match()})});
    cs.forEach(function(b){b.addEventListener('click',function(){cat=cat===b.getAttribute('data-cat')?'':b.getAttribute('data-cat');cs.forEach(function(x){x.classList.remove('on')});if(cat)b.classList.add('on');match()})});
    update();
  });
})();
"""
    open(os.path.join(ASSETS, 'site.js'), 'w', encoding='utf-8').write(js)

# ---- 高亮 ----
KWS = set("auto bool break case catch char class const continue default delete do double else enum explicit extern float for friend goto if inline int long namespace new operator private protected public return short signed sizeof static struct switch template this throw try typedef typename union unsigned using virtual void while".split())
KWS |= set("vector string unordered_map unordered_set map set deque stack queue priority_queue optional pair make_pair nullptr size_t".split())

def highlight(code):
    out = []; i = 0; n = len(code)
    tag = lambda k, t: f'<span class="tok-{k}">{html.escape(t)}</span>'
    while i < n:
        # 注释
        if code.startswith('//', i):
            j = code.find('\n', i); j = n if j < 0 else j
            out.append(tag('c', code[i:j])); i = j; continue
        if code.startswith('/*', i):
            j = code.find('*/', i + 2); j = n if j < 0 else j + 2
            out.append(tag('c', code[i:j])); i = j; continue
        # 字符串
        if code[i] in '"\'':
            end = i + 1
            while end < n and code[end] != code[i]:
                if code[end] == '\\': end += 1
                end += 1
            end = min(n, end + 1)
            out.append(tag('s', code[i:end])); i = end; continue
        # 数字
        if code[i].isdigit():
            j = i
            while j < n and (code[j].isalnum() or code[j] in '._'): j += 1
            out.append(tag('n', code[i:j])); i = j; continue
        # 预处理
        if code[i] == '#':
            j = code.find('\n', i); j = n if j < 0 else j
            out.append(tag('p', code[i:j])); i = j; continue
        # 标识符/关键字
        if code[i].isalpha() or code[i] == '_':
            j = i
            while j < n and (code[j].isalnum() or code[j] == '_'): j += 1
            w = code[i:j]
            if w in KWS: out.append(tag('k', w))
            else: out.append(html.escape(w))
            i = j; continue
        out.append(html.escape(code[i])); i += 1
    return ''.join(out)

# 难度->分类 顺序 / 难度分桶
DIFF_ORDER = ['Easy', 'Medium', 'Hard']

def category_from_cpp(name):
    # '01_hash' -> '哈希'
    return name  # 用英文名做 cat 即可，中文映射另列
CAT_LABEL = {
 'hash':'哈希', 'two_pointers':'双指针', 'sliding_window':'滑动窗口',
 'substring':'子串/前缀和', 'array':'数组', 'matrix':'矩阵',
 'linked_list':'链表', 'binary_tree':'二叉树', 'graph':'图',
 'backtracking':'回溯', 'binary_search':'二分查找', 'stack':'栈',
 'heap':'堆', 'greedy':'贪心', 'dp':'动态规划', 'dp_multidim':'二维 DP',
 'tricks':'技巧',
}

def body_html(body):
    out = []
    for line in body:
        m = re.match(r'^\s*([^：:]{1,14})[：:]\s?(.*)$', line)
        if m and len(m.group(1)) <= 14 and not m.group(2).startswith(' ') :
            lab = m.group(1); rest = m.group(2)
            out.append(f'<p><span class="lab">{html.escape(lab)}：</span>{clean_inline(rest)}</p>')
        else:
            out.append(f'<p>{clean_inline(line)}</p>')
    return '\n'.join(out)

def write_index(problems):
    cats = collections.OrderedDict()
    for p in problems:
        cats.setdefault(p['category'], []).append(p)
    body = []
    for cat, ps in cats.items():
        label = CAT_LABEL.get(cat, cat)
        body.append(f'<h2 class="cat-h" id="cat-{cat}">{html.escape(label)} <small style="color:var(--muted);font-size:13px">({len(ps)})</small></h2>')
        body.append('<div class="grid" data-catgroup="'+cat+'">')
        for p in ps:
            body.append(card_html(p))
        body.append('</div>')
    content = '\n'.join(body)
    page = index_template(content, problems)
    write_text_clean(os.path.join(OUT, 'index.html'), page)

def write_text_clean(path, text):
    """写入静态页时统一去除行尾空白，避免生成无意义的 Git diff。"""
    clean = '\n'.join(line.rstrip() for line in text.splitlines()) + '\n'
    with open(path, 'w', encoding='utf-8', newline='\n') as f:
        f.write(clean)

def card_html(p):
    diff = p['diff']
    done = ''
    s = (f"{p['num']} {p['title']} {diff} {p['tag']} {p['slug']} {p['category']}").lower()
    return (f'<a class="card" href="problems/q{p["num"]}.html" data-num="{p["num"]}" data-diff="{diff}" data-cat="{p["category"]}" data-s="{html.escape(s)}">'
            f'<div><span class="no">Q{p["num"]}</span><span class="qt" style="color:var(--ink)">{html.escape(p["title"])}</span></div>'
            f'<span class="badge {diff}">{diff}</span>'
            f'<div class="tag">{html.escape(p["tag"])}</div>'
            f'</a>')

def index_template(content, problems):
    total = len(problems)
    diffs = collections.Counter(p['diff'] for p in problems).most_common()
    diffBtns = ''.join(f'<button class="chip" data-diff="{d}">{d} ({n})</button>' for d, n in sorted(diffs, key=lambda x: DIFF_ORDER.index(x[0])))
    catBtns = ''.join(f'<button class="chip" data-cat="{c}">{CAT_LABEL.get(c,c)}</button>' for c in sorted(CAT_LABEL))
    return f'''<!doctype html>
<html lang="zh-CN" data-theme="light"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>LeetCode Hot 100 · C++ 学习站</title>
<link rel="stylesheet" href="assets/site.css">
<link rel="icon" href="data:image/svg+xml,<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100'><text y='0.9em' font-size='88'>📚</text></svg>"></head><body>
<div class="wrap">
<div class="topbar">
  <span class="brand">LeetCode Hot 100 · C++ <small>C++17 离线学习站</small></span>
  <span class="spacer"></span>
  <button class="iconbtn" id="themeBtn">🌙</button>
</div>
<div class="hero">
  <h1>LeetCode Hot 100 · 图解 + 思路 + 代码</h1>
  <p>共 {total} 题，一题一页。每页都有「核心套路 / 讲解 / 结构图解 / 带注释代码 / 测试用例」。
     适合新手：先按分组看，再按难度逐题点开。点题号前的小圆点可标记「已掌握」，进度自动保存。</p>
</div>
<div class="progressbar"><div id="progressbar" style="width:0"></div></div>
<div style="display:flex;justify-content:space-between;align-items:baseline"><span id="progresslabel" class="meta">0%</span><span class="meta">难度分布：{', '.join(f"{d} {n}" for d,n in sorted(diffs,key=lambda x:DIFF_ORDER.index(x[0])))}</span></div>
<div class="controls">
  <input id="search" class="search" placeholder="搜索：题号 / 题名 / 核心套路 / 分类…">
</div>
<div class="controls" style="margin-top:2px">
  <span class="meta">难度：</span>{diffBtns}
</div>
<div class="controls" style="margin-top:2px">
  <span class="meta">分类：</span>{catBtns}
</div>
{content}
<div class="center">— 共 {total} 题 —</div>
</div>
<script src="assets/site.js"></script></body></html>'''

def write_cat_pages(problems):
    # 不单独生成分类页；分类在 index 内分组。此函数预留。
    pass

def write_problem_pages(problems):
    for p in problems:
        num = p['num']
        # 同类题：同一分类里其它题
        sibs = [x for x in problems if x['category'] == p['category'] and x['num'] != num]
        sib_html = ''
        if sibs:
            items = ''.join(f'<a href="q{x["num"]}.html">Q{x["num"]} {html.escape(x["title"])}</a>' for x in sibs[:8])
            sib_html = f'<div class="sect"><h2>同类题（{html.escape(CAT_LABEL.get(p["category"], p["category"]))}）</h2><div class="taglist">{items}</div></div>'
        # 测试
        tests_html = ''
        if p['tests']:
            blk = []
            for (suite, name, body) in p['tests']:
                blk.append(f'<pre>{html.escape(body.strip())}</pre>')
            tests_html = f'<div class="sect"><h2>测试用例（本仓库，与代码一一对应）</h2>{"".join(blk)}</div>'
        page = problem_template(p, sib_html, tests_html)
        write_text_clean(os.path.join(PROBLEMS, f'q{num}.html'), page)

def clean_desc(h):
    """清洗力扣返回的题目描述 HTML：去掉装饰性空段、首尾空白。"""
    if not h:
        return ''
    h = re.sub(r'<p>&nbsp;</p>\s*', '', h)
    h = re.sub(r'\n\s*\n', '\n', h)
    h = h.strip()
    return h

def problem_template(p, sib_html, tests_html):
    body = body_html(p['body'])
    dia = diagram_html(p['diagram'])
    hand = hand_diagram(p['num'])
    figs = dia + hand
    code = highlight(p['code'])
    diff = p['diff']
    cat = CAT_LABEL.get(p['category'], p['category'])
    link = f'<a href="{html.escape(p["url"])}" target="_blank" rel="noopener">{html.escape(p["slug"] or "leetcode.cn")}</a>' if p['url'] else ''
    tagline = f'<div class="note">💡 <strong>核心套路：</strong>{html.escape(p["tag"])}</div>' if p['tag'] else ''
    desc_html = clean_desc(p['desc'].get('zh') or p['desc'].get('en') or '')
    desc_section = f'<div class="sect"><h2>题目描述</h2><div class="card2"><div class="desc">{desc_html}</div></div></div>' if desc_html else ''
    anim_section = ''
    anim_scripts = ''
    if p.get('anim'):
        num = p['num']
        anim_section = f'<div class="sect"><h2>动画演示</h2><div class="anim-host" id="anim-{num}"></div></div>'
        anim_scripts = ('<script src="../assets/anim.js"></script>\n' +
                        '<script src="../assets/animations.js"></script>\n' +
                        '<script>window.addEventListener(\'DOMContentLoaded\',function(){{var s=window.LC_ANIMS&&window.LC_ANIMS["' + str(num) + '"];var h=document.getElementById("anim-' + str(num) + '");if(s&&h&&window.LCAnim)LCAnim.run(h,s);}});</script>')
    return f'''<!doctype html>
<html lang="zh-CN" data-theme="light"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Q{p['num']} {html.escape(p['title'])} · LeetCode Hot 100</title>
<link rel="stylesheet" href="../assets/site.css">
<link rel="icon" href="data:image/svg+xml,<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100'><text y='0.9em' font-size='88'>📚</text></svg>"></head><body>
<div class="wrap">
<div class="topbar">
  <a class="brand" href="../index.html">← 返回</a>
  <span class="brand small">LeetCode Hot 100 · C++</span>
  <span class="spacer"></span>
  <button class="iconbtn" id="themeBtn">🌙</button>
</div>
<div class="crumb"><a href="../index.html">首页</a> / <a href="../index.html#cat-{p['category']}">{html.escape(cat)}</a> / Q{p['num']}</div>
<header class="dhead">
  <h1><span style="color:var(--muted);font-weight:600">Q{p['num']}</span> {html.escape(p['title'])}</h1>
  <span class="badge {diff}">{diff}</span>
  <label style="margin-left:auto;display:flex;align-items:center;gap:6px;color:var(--muted);font-size:14px;cursor:pointer">
    <input type="checkbox" data-check="{p['num']}"> 已掌握
  </label>
</header>
<div class="meta">分类 {html.escape(cat)} · 难度 {diff} · 原题 {link}</div>
{desc_section}
{tagline}
<div class="sect"><h2>讲解与思路</h2><div class="card2"><div class="bodytext">{body}</div></div></div>
{anim_section}
{"<div class='sect'><h2>结构图解</h2>"+figs+"</div>" if figs else ''}
<div class="sect"><h2>带注释代码</h2><pre class="code"><button class="copy" onclick="copyCode(this)">复制</button><code>{code}</code></pre></div>
{tests_html}
{sib_html}
<div class="center" style="padding:20px">— 完 —</div>
</div>
<script>function copyCode(b){{var c=b.parentNode.querySelector('code');var t=c.innerText;navigator.clipboard&&navigator.clipboard.writeText(t);b.textContent='已复制';setTimeout(function(){{b.textContent='复制'}},1200)}}</script>
<script src="../assets/site.js"></script>
{anim_scripts}</body></html>'''

if __name__ == '__main__':
    main()
