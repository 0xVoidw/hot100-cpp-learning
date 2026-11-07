#!/usr/bin/env node
// Hot 100 · C++ 学习站 —— 零依赖静态生成器
//
// 数据来源：
//   build/problems.json   由 tools/gen_site.py --json --data-only 从 C++ 注释导出
//   src/NN_*.cpp          文件头的「本类通用思路」
// 产物：dist/（直接发布到 GitHub Pages）
//
// 用法：node www/build.mjs [--base /hot100-cpp-learning/] [--out dist]

import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = path.dirname(fileURLToPath(import.meta.url));
const ROOT = path.dirname(HERE);
const args = process.argv.slice(2);
const arg = (name, fallback) => {
  const i = args.indexOf(name);
  return i >= 0 && args[i + 1] ? args[i + 1] : fallback;
};
let BASE = arg('--base', process.env.SITE_BASE || '/hot100-cpp-learning/');
if (!BASE.endsWith('/')) BASE += '/';
const OUT = path.resolve(ROOT, arg('--out', 'dist'));
const SITE_URL = 'https://0xvoidw.github.io';
const REPO_URL = 'https://github.com/0xVoidw/hot100-cpp-learning';

// ---------------------------------------------------------------- 数据
const dataFile = path.join(ROOT, 'build', 'problems.json');
if (!fs.existsSync(dataFile)) {
  console.error('缺少 build/problems.json，请先运行：python3 tools/gen_site.py --json --data-only');
  process.exit(1);
}
const problems = JSON.parse(fs.readFileSync(dataFile, 'utf8'));
const byNum = new Map(problems.map((p) => [p.num, p]));

const CATEGORY_NOTES = {
  hash: '把「查找另一个值」从重复扫描变成常数时间查询。',
  two_pointers: '利用有序性或两端信息，让两个指针只朝答案收缩。',
  sliding_window: '维护一个始终满足约束的连续区间，避免重复计算。',
  substring: '用前缀信息或窗口把连续区间问题拆成可复用的状态。',
  array: '围绕下标、原地状态和局部不变量组织一次或有限次扫描。',
  matrix: '把二维坐标关系转成有边界的行列遍历或原地标记。',
  linked_list: '指针不能丢：先保存 next，再改变连接关系。',
  binary_tree: '每个节点都问同一个递归问题，再把左右子树答案合并。',
  graph: '把「连通、依赖、扩散」转成 DFS、BFS 或拓扑序。',
  backtracking: '选择、递归、撤销选择，枚举搜索树并尽早剪枝。',
  binary_search: '答案具有单调性，就能每次排除一半候选。',
  stack: '栈保存「最近还没解决」的状态，适合嵌套和单调关系。',
  heap: '只维护当前最需要的少量候选，不必全排序。',
  greedy: '每一步做可证明的局部最优，把全局问题逐步缩小。',
  dp: '把大问题写成状态，让每个状态只算一次。',
  dp_multidim: '明确 dp[i][j] 的含义，再按依赖方向填表。',
  tricks: '借助位运算、数学规律或原地映射压缩空间。',
};

// 从 src/NN_xxx.cpp 文件头读出分类名、英文名、题目顺序和「通用思路」
const categories = [];
for (const file of fs.readdirSync(path.join(ROOT, 'src')).sort()) {
  const m = file.match(/^(\d\d)_(.+)\.cpp$/);
  if (!m) continue;
  const key = m[2];
  const text = fs.readFileSync(path.join(ROOT, 'src', file), 'utf8');
  const head = [];
  for (const line of text.split('\n').slice(1)) {
    if (/^\/\/\s*={5,}/.test(line)) break;
    if (!line.startsWith('//')) break;
    head.push(line.replace(/^\/\/ ?/, ''));
  }
  const titleLine = (head[0] || '').trim();
  const tm = titleLine.match(/^\d+\s*·\s*(.+?)\s*\(([^)]+)\)\s*(?:——\s*(.+))?$/);
  let i = 1;
  while (i < head.length && head[i].trim()) i++;
  const guide = head.slice(i).join('\n').replace(/^\n+|\s+$/g, '');
  const items = problems
    .filter((p) => p.category === key)
    .map((p) => ({ p, at: text.search(new RegExp(`\\*\\s*Q${p.num}\\.`)) }))
    .sort((a, b) => (a.at < 0 ? 1e9 : a.at) - (b.at < 0 ? 1e9 : b.at) || a.p.num - b.p.num)
    .map((x) => x.p);
  categories.push({
    key,
    no: m[1],
    name: tm ? tm[1] + (tm[3] ? `（${tm[3].trim()}）` : '') : key,
    en: tm ? tm[2] : key,
    note: CATEGORY_NOTES[key] || '',
    guide,
    problems: items,
  });
}
const catOf = new Map(categories.map((c) => [c.key, c]));
// 学习顺序 = 分类顺序 + 分类内顺序（用于上一题 / 下一题）
const ordered = categories.flatMap((c) => c.problems);
ordered.forEach((p, i) => { p.order = i; });

// ---------------------------------------------------------------- 工具
const esc = (s) => String(s ?? '')
  .replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');
const url = (p = '') => BASE + p;
const pUrl = (num) => url(`problems/${num}/`);
const DIFF = { Easy: '简单', Medium: '中等', Hard: '困难' };
const diffKey = (d) => (d || '').toLowerCase();

function inline(text) {
  let s = esc(text);
  s = s.replace(/`([^`]+)`/g, '<code>$1</code>');
  s = s.replace(/\*\*([^*]+)\*\*/g, '<strong>$1</strong>');
  // 复杂度记号统一等宽
  s = s.replace(/(?<![\w>])(O\((?:[^()]|\([^()]*\))*\))/g, '<code class="big-o">$1</code>');
  return s;
}

// 把 C++ 注释里被硬折行的讲解重新拼成段落，并按「标签：」归类
const LABEL_RE = /^([^：:，。,；;「」]{1,18}?)[：:]\s*(.*)$/;
const ENUM_RE = /^([①②③④⑤⑥⑦⑧⑨]|\d+[)）.、]|[-·•])\s*/;
function parseBody(lines) {
  // 每段：{ label, blocks: [{ li: bool, text }] }
  const paras = [];
  let lastEnds = true;
  for (const raw of lines) {
    const line = raw.trim();
    if (!line || line === '/') continue;
    const lm = line.match(LABEL_RE);
    const looksLabel = lm && !/^[a-z_]\w*$/i.test(lm[1]) && !/[=<>[\]]/.test(lm[1]);
    const cur = paras[paras.length - 1];
    if (looksLabel) {
      paras.push({ label: lm[1].trim(), blocks: lm[2] ? [{ li: false, text: lm[2] }] : [] });
    } else if (!cur) {
      paras.push({ label: '', blocks: [{ li: false, text: line }] });
    } else if (ENUM_RE.test(line)) {
      cur.blocks.push({ li: true, text: line });
    } else {
      const lb = cur.blocks[cur.blocks.length - 1];
      if (!lb || lastEnds || lb.li) cur.blocks.push({ li: false, text: line });
      else lb.text = joinText(lb.text, line);
    }
    lastEnds = /[。；;！!？?]$/.test(line) || ENUM_RE.test(line);
  }
  return paras;
}
function renderBlocks(blocks) {
  let out = '', inList = false;
  for (const b of blocks) {
    if (b.li && !inList) { out += '<ul>'; inList = true; }
    if (!b.li && inList) { out += '</ul>'; inList = false; }
    out += b.li ? `<li>${inline(b.text)}</li>` : `<p>${inline(b.text)}</p>`;
  }
  return inList ? out + '</ul>' : out;
}
// 图解：按 viewBox 宽度给出自然尺寸，避免小图被放大；修正说明文字里的 `<...>` 被当成标签
function fixDiagram(html) {
  return html
    .replace(/<svg viewBox="0 0 (\d+(?:\.\d+)?) (\d+(?:\.\d+)?)"/g, (m, w) => `${m} style="width:${w}px;min-width:min(${w}px,520px)"`)
    .replace(/(<div class="fig-cap">)([\s\S]*?)(<\/div>)/g, (m, a, body, c) => a + body.replace(/`([^`]*)`/g, (mm, t) => `<code>${esc(t.replace(/^<(.*)>$/, '$1'))}</code>`) + c);
}
function joinText(a, b) {
  if (!a) return b;
  return /[A-Za-z0-9)\]}]$/.test(a) && /^[A-Za-z0-9(\[{]/.test(b) ? `${a} ${b}` : a + b;
}
function kindOf(label) {
  if (/^复杂度/.test(label)) return 'cx';
  if (/易错|注意|别用|反例|坑|缺一不可|细节/.test(label)) return 'warn';
  if (/对比|另解|备用|变体|延伸|推广|解法|数学解|家族|优化/.test(label)) return 'alt';
  if (/^思路/.test(label) || !label) return 'main';
  return 'note';
}

// ------------------------------------------------------- C++ 语法高亮
const KW = new Set(('alignas alignof auto bool break case catch char class const constexpr continue decltype default delete do double else enum explicit extern false float for friend goto if inline int long mutable namespace new noexcept nullptr operator private protected public return short signed sizeof static static_cast struct switch template this throw true try typedef typename union unsigned using virtual void volatile while size_t int64_t uint32_t uint64_t long').split(' '));
const TYPES = new Set(('vector string unordered_map unordered_set map set multiset pair deque queue stack priority_queue array bitset ListNode TreeNode Node Solution Trie LRUCache MinStack MedianFinder function greater less tuple optional').split(' '));
function highlight(code) {
  const re = /(\/\/[^\n]*|\/\*[\s\S]*?\*\/)|("(?:\\.|[^"\\])*"|'(?:\\.|[^'\\])*')|(^[ \t]*#[^\n]*)|(\b\d[\d.']*[uUlLfF]*\b)|([A-Za-z_]\w*)(\s*\()?|([\s\S])/gm;
  let out = '';
  let m;
  while ((m = re.exec(code))) {
    if (m[1]) out += `<span class="t-c">${esc(m[1])}</span>`;
    else if (m[2]) out += `<span class="t-s">${esc(m[2])}</span>`;
    else if (m[3]) out += `<span class="t-p">${esc(m[3])}</span>`;
    else if (m[4]) out += `<span class="t-n">${esc(m[4])}</span>`;
    else if (m[5]) {
      const w = m[5];
      const cls = KW.has(w) ? 't-k' : TYPES.has(w) ? 't-t' : m[6] ? 't-f' : /^CHECK/.test(w) ? 't-f' : '';
      out += cls ? `<span class="${cls}">${esc(w)}</span>` : esc(w);
      if (m[6]) out += esc(m[6]);
    } else out += esc(m[7]);
  }
  return out;
}
function codeBlock(code, label, extraClass = '') {
  const clean = code.replace(/^\n+|\s+$/g, '');
  const lines = clean.split('\n').length;
  return `<figure class="code ${extraClass}">
  <figcaption><span>${esc(label)}</span><span class="code-meta">${lines} 行</span><button type="button" class="copy" data-copy>复制</button></figcaption>
  <pre><code>${highlight(clean)}</code></pre>
</figure>`;
}
function dedent(s) {
  const lines = s.replace(/^\n+|\s+$/g, '').split('\n');
  const pad = Math.min(...lines.filter((l) => l.trim()).map((l) => l.match(/^ */)[0].length));
  return lines.map((l) => l.slice(pad)).join('\n');
}

// ---------------------------------------------------------------- 外壳
const ICON = {
  sun: '<svg viewBox="0 0 24 24" aria-hidden="true"><circle cx="12" cy="12" r="4"/><path d="M12 2v2M12 20v2M4.9 4.9l1.4 1.4M17.7 17.7l1.4 1.4M2 12h2M20 12h2M4.9 19.1l1.4-1.4M17.7 6.3l1.4-1.4"/></svg>',
  moon: '<svg viewBox="0 0 24 24" aria-hidden="true"><path d="M20 14.5A8 8 0 0 1 9.5 4a8 8 0 1 0 10.5 10.5z"/></svg>',
  search: '<svg viewBox="0 0 24 24" aria-hidden="true"><circle cx="11" cy="11" r="7"/><path d="m20 20-3.5-3.5"/></svg>',
  play: '<svg viewBox="0 0 24 24" aria-hidden="true"><path d="M8 5v14l11-7z"/></svg>',
  ext: '<svg viewBox="0 0 24 24" aria-hidden="true"><path d="M14 4h6v6M20 4l-9 9M18 14v5a1 1 0 0 1-1 1H5a1 1 0 0 1-1-1V7a1 1 0 0 1 1-1h5"/></svg>',
  gh: '<svg viewBox="0 0 24 24" aria-hidden="true"><path d="M12 2a10 10 0 0 0-3.2 19.5c.5.1.7-.2.7-.5v-1.7c-2.8.6-3.4-1.3-3.4-1.3-.4-1.2-1.1-1.5-1.1-1.5-.9-.6.1-.6.1-.6 1 .1 1.5 1 1.5 1 .9 1.5 2.4 1.1 3 .8.1-.6.3-1.1.6-1.3-2.2-.3-4.6-1.1-4.6-5 0-1.1.4-2 1-2.7-.1-.3-.4-1.3.1-2.7 0 0 .8-.3 2.8 1a9.6 9.6 0 0 1 5 0c1.9-1.3 2.7-1 2.7-1 .6 1.4.2 2.4.1 2.7.6.7 1 1.6 1 2.7 0 3.9-2.3 4.7-4.6 5 .4.3.7.9.7 1.9v2.8c0 .3.2.6.7.5A10 10 0 0 0 12 2z"/></svg>',
};

function layout({ title, description, body, page, extraHead = '' }) {
  const full = title ? `${title} · Hot 100 C++ 图解` : 'Hot 100 C++ 图解 · 力扣热题 100 的 C++17 学习站';
  return `<!doctype html>
<html lang="zh-CN">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>${esc(full)}</title>
<meta name="description" content="${esc(description)}">
<meta name="theme-color" content="#f6f3ec" media="(prefers-color-scheme: light)">
<meta name="theme-color" content="#16150f" media="(prefers-color-scheme: dark)">
<link rel="icon" href="${url('assets/icon.svg')}" type="image/svg+xml">
<link rel="stylesheet" href="${url('assets/style.css')}">
<script>try{var t=localStorage.getItem('h100:theme');if(t)document.documentElement.dataset.theme=t}catch(e){}</script>
${extraHead}
</head>
<body data-page="${page}" data-base="${BASE}">
<a class="skip" href="#main">跳到正文</a>
<header class="topbar">
  <div class="topbar-in">
    <a class="brand" href="${url()}"><span class="brand-mark">H</span><span class="brand-text">Hot 100<em>C++ 图解</em></span></a>
    <nav class="topnav" aria-label="主导航">
      <a href="${url()}"${page === 'home' ? ' aria-current="page"' : ''}>题库</a>
      <a href="${url('patterns/')}"${page === 'patterns' ? ' aria-current="page"' : ''}>套路</a>
      <a href="${REPO_URL}" class="hide-sm">源码</a>
    </nav>
    <button type="button" class="search-trigger" data-open-search aria-label="搜索题目">${ICON.search}<span>搜索题目</span><kbd>/</kbd></button>
    <button type="button" class="icon-btn" data-theme-toggle aria-label="切换深浅色">${ICON.moon}${ICON.sun}</button>
  </div>
</header>
<main id="main">
${body}
</main>
<footer class="foot">
  <div class="foot-in">
    <span>力扣热题 100 · C++17 题解与图解</span>
    <span>题目描述版权归 <a href="https://leetcode.cn/">力扣</a> 所有 · <a href="${REPO_URL}">${ICON.gh} 0xVoidw/hot100-cpp-learning</a></span>
  </div>
</footer>
<dialog class="palette" id="palette" aria-label="搜索题目">
  <div class="palette-box">
    <label class="palette-input">${ICON.search}<input type="search" placeholder="题号、题名或套路关键词…" autocomplete="off" spellcheck="false"><kbd>Esc</kbd></label>
    <ul class="palette-list" role="listbox"></ul>
    <p class="palette-hint"><span>↑↓ 选择</span><span>↵ 打开</span><span>/ 随时唤起</span></p>
  </div>
</dialog>
<script src="${url('assets/index.js')}"></script>
<script src="${url('assets/app.js')}" defer></script>
</body>
</html>
`;
}

// ---------------------------------------------------------------- 首页
function chip(label, attrs = '') { return `<button type="button" class="chip" ${attrs}>${label}</button>`; }

function homePage() {
  const animCount = problems.filter((p) => p.anim).length;
  const testCount = problems.reduce((n, p) => n + (p.tests?.length || 0), 0);
  const diffCount = { Easy: 0, Medium: 0, Hard: 0 };
  problems.forEach((p) => { diffCount[p.diff] = (diffCount[p.diff] || 0) + 1; });

  const chapters = categories.map((c) => `
  <section class="chapter" id="c-${c.key}" data-cat="${c.key}">
    <header class="chapter-head">
      <span class="chapter-no">${c.no}</span>
      <div class="chapter-title">
        <h2><a href="${url('patterns/')}#${c.key}">${esc(c.name)}</a></h2>
        <p>${esc(c.note)}</p>
      </div>
      <div class="chapter-meter" data-meter="${c.key}" title="已掌握 / 本类题数"><b>0</b>/${c.problems.length}<i><s style="width:0"></s></i></div>
    </header>
    <ol class="rows">
${c.problems.map((p) => `      <li class="row" data-num="${p.num}" data-diff="${diffKey(p.diff)}" data-anim="${p.anim ? 1 : 0}" data-text="${esc(`${p.num} ${p.title} ${p.tag} ${c.name}`.toLowerCase())}">
        <a href="${pUrl(p.num)}">
          <span class="row-num">${p.num}</span>
          <span class="row-main"><span class="row-title">${esc(p.title)}</span><span class="row-tag">${esc(p.tag)}</span></span>
          <span class="row-side">${p.anim ? `<span class="has-anim" title="有算法动画">${ICON.play}</span>` : ''}<span class="diff diff-${diffKey(p.diff)}">${DIFF[p.diff] || p.diff}</span><span class="status-dot" aria-hidden="true"></span></span>
        </a>
      </li>`).join('\n')}
    </ol>
  </section>`).join('\n');

  const body = `
<section class="hero">
  <div class="hero-in">
    <div class="hero-copy">
      <p class="kicker">LeetCode 热题 100 · C++17 · 中文讲解</p>
      <h1>把一百道题，<br>读成<em>十七个套路</em>。</h1>
      <p class="lede">每题配原题描述、分步思路、复杂度与易错点、可运行的 C++17 题解和单元测试；${animCount} 道题能一帧一帧看算法怎么走。</p>
      <dl class="stats">
        <div><dt>题目</dt><dd>${problems.length}</dd></div>
        <div><dt>分类</dt><dd>${categories.length}</dd></div>
        <div><dt>算法动画</dt><dd>${animCount}</dd></div>
        <div><dt>单元测试</dt><dd>${testCount}</dd></div>
      </dl>
    </div>
    <aside class="progress-card" aria-label="我的进度">
      <div class="ring" data-ring>
        <svg viewBox="0 0 120 120" aria-hidden="true"><circle cx="60" cy="60" r="52" class="ring-bg"/><circle cx="60" cy="60" r="52" class="ring-learn" pathLength="100"/><circle cx="60" cy="60" r="52" class="ring-done" pathLength="100"/></svg>
        <div class="ring-text"><b data-count="done">0</b><span>/ ${problems.length} 已掌握</span></div>
      </div>
      <ul class="progress-legend">
        <li><i class="lg-done"></i>已掌握 <b data-count="done">0</b></li>
        <li><i class="lg-learn"></i>学习中 <b data-count="learn">0</b></li>
        <li><i class="lg-todo"></i>未开始 <b data-count="todo">${problems.length}</b></li>
      </ul>
      <div class="progress-actions">
        <a class="btn btn-primary" data-continue href="${pUrl(ordered[0].num)}">从第一题开始</a>
        <button type="button" class="btn" data-random>随机一道没掌握的</button>
      </div>
      <p class="progress-note">进度只保存在这台设备的浏览器里。</p>
    </aside>
  </div>
</section>

<div class="toolbar" role="search">
  <div class="toolbar-in">
    <label class="filter-search">${ICON.search}<input id="q" type="search" placeholder="筛选：题号 / 题名 / 套路" autocomplete="off"></label>
    <div class="chips" data-group="diff" role="group" aria-label="难度">
      ${chip('全部', 'data-v="" aria-pressed="true"')}${chip(`简单 <small>${diffCount.Easy}</small>`, 'data-v="easy" aria-pressed="false"')}${chip(`中等 <small>${diffCount.Medium}</small>`, 'data-v="medium" aria-pressed="false"')}${chip(`困难 <small>${diffCount.Hard}</small>`, 'data-v="hard" aria-pressed="false"')}
    </div>
    <div class="chips" data-group="status" role="group" aria-label="进度">
      ${chip('全部', 'data-v="" aria-pressed="true"')}${chip('未开始', 'data-v="0" aria-pressed="false"')}${chip('学习中', 'data-v="1" aria-pressed="false"')}${chip('已掌握', 'data-v="2" aria-pressed="false"')}
    </div>
    <button type="button" class="chip chip-toggle" data-anim-only aria-pressed="false">${ICON.play} 只看有动画</button>
  </div>
</div>

<div class="catalog">
  <nav class="toc" aria-label="分类目录">
    <p class="toc-title">分类</p>
    <ol>
${categories.map((c) => `      <li><a href="#c-${c.key}"><span>${c.no}</span>${esc(c.name)}<small>${c.problems.length}</small></a></li>`).join('\n')}
    </ol>
  </nav>
  <div class="chapters">
${chapters}
    <p class="empty-result" hidden>没有符合条件的题目，换个筛选试试。</p>
  </div>
</div>`;
  return layout({ title: '', page: 'home', description: '力扣热题 100 全部题目的 C++17 题解：原题描述、分步思路、复杂度、易错点、单元测试和逐帧算法动画。', body });
}

// ---------------------------------------------------------------- 题目页
function problemPage(p) {
  const c = catOf.get(p.category);
  const prev = ordered[p.order - 1];
  const next = ordered[p.order + 1];
  const paras = parseBody(p.body || []);
  const cx = paras.filter((x) => kindOf(x.label) === 'cx');
  const rest = paras.filter((x) => kindOf(x.label) !== 'cx');
  const cxText = cx.map((x) => x.blocks.map((b) => b.text).join('')).join('；');
  const grab = (word) => {
    const at = cxText.indexOf(word);
    if (at < 0) return null;
    let depth = 0, out = '';
    for (const ch of cxText.slice(at + word.length)) {
      if ('(（'.includes(ch)) depth++;
      if (')）'.includes(ch)) depth--;
      if (depth <= 0 && '，,；;'.includes(ch)) break;
      out += ch;
    }
    return [null, out.trim()];
  };
  const tm = grab('时间');
  const sm = grab('空间');
  const LABELS = { warn: '易错', alt: '拓展', note: '要点', main: '思路' };

  const explain = rest.map((x) => {
    const k = kindOf(x.label);
    return `<div class="para para-${k}">
  <p class="para-label">${esc(x.label || LABELS[k])}</p>
  ${renderBlocks(x.blocks)}
</div>`;
  }).join('\n');

  const sections = [
    ['desc', '题目'],
    ['idea', '思路'],
    ...(p.diagramHtml ? [['figure', '图解']] : []),
    ...(p.anim ? [['anim', '动画']] : []),
    ['code', '代码'],
    ...(p.tests?.length ? [['tests', '测试']] : []),
  ];

  const siblings = c.problems.map((q) => `<li><a href="${pUrl(q.num)}" data-num="${q.num}"${q.num === p.num ? ' aria-current="page"' : ''}><span class="status-dot" aria-hidden="true"></span><span class="sib-num">${q.num}</span><span class="sib-title">${esc(q.title)}</span></a></li>`).join('');

  const body = `
<div class="problem-layout">
  <aside class="side" aria-label="${esc(c.name)} 分类题目">
    <div class="side-in">
      <a class="side-cat" href="${url()}#c-${c.key}"><span>${c.no}</span>${esc(c.name)}</a>
      <p class="side-note">${esc(c.note)}</p>
      <ol class="siblings">${siblings}</ol>
      <a class="side-more" href="${url('patterns/')}#${c.key}">本类通用套路 →</a>
    </div>
  </aside>

  <article class="problem" data-num="${p.num}">
    <nav class="crumbs" aria-label="位置"><a href="${url()}">题库</a><span>/</span><a href="${url()}#c-${c.key}">${c.no} ${esc(c.name)}</a></nav>
    <header class="problem-head">
      <h1><span class="h-num">${p.num}.</span> ${esc(p.title)}</h1>
      <div class="meta">
        <span class="diff diff-${diffKey(p.diff)}">${DIFF[p.diff] || p.diff}</span>
        <span class="meta-item">${esc(c.name)}</span>
        ${p.anim ? `<a class="meta-item meta-anim" href="#anim">${ICON.play}${p.anim.frames.length} 帧动画</a>` : ''}
        <a class="meta-item meta-link" href="${esc(p.url)}" target="_blank" rel="noopener">在力扣上做${ICON.ext}</a>
      </div>
      ${p.tag ? `<p class="takeaway"><span>一句话套路</span>${inline(p.tag)}</p>` : ''}
      <div class="status-switch" role="radiogroup" aria-label="学习进度">
        <button type="button" role="radio" data-status="0" aria-checked="true">未开始</button>
        <button type="button" role="radio" data-status="1" aria-checked="false">学习中</button>
        <button type="button" role="radio" data-status="2" aria-checked="false">已掌握</button>
      </div>
    </header>

    <nav class="sect-nav" aria-label="本页章节">
      ${sections.map(([id, label]) => `<a href="#${id}">${label}</a>`).join('')}
    </nav>

    <section id="desc" class="sect">
      <h2>题目描述</h2>
      <div class="lc-desc">${p.desc?.zh || '<p>暂无描述，请到力扣查看原题。</p>'}</div>
    </section>

    <section id="idea" class="sect">
      <h2>思路</h2>
      ${tm || sm ? `<div class="cx">${tm ? `<div><span>时间</span>${inline(tm[1].trim())}</div>` : ''}${sm ? `<div><span>空间</span>${inline(sm[1].trim())}</div>` : ''}</div>` : (cxText ? `<p class="cx-line">复杂度：${inline(cxText)}</p>` : '')}
      <div class="paras">${explain}</div>
    </section>

    ${p.diagramHtml ? `<section id="figure" class="sect">
      <h2>图解</h2>
      <div class="figs">${fixDiagram(p.diagramHtml)}</div>
    </section>` : ''}

    ${p.anim ? `<section id="anim" class="sect">
      <h2>逐帧动画</h2>
      <div class="player" data-player>
        <div class="player-stage"><svg class="stage-svg" role="img" aria-label="算法状态"></svg></div>
        <div class="player-side">
          <p class="player-step"><b data-step>1</b> / ${p.anim.frames.length}</p>
          <p class="player-note" data-note aria-live="polite"></p>
          <dl class="player-vars" data-vars></dl>
        </div>
        <div class="player-bar">
          <button type="button" data-act="first" aria-label="第一帧">⏮</button>
          <button type="button" data-act="prev" aria-label="上一帧">←</button>
          <button type="button" data-act="play" class="play-btn">播放</button>
          <button type="button" data-act="next" aria-label="下一帧">→</button>
          <button type="button" data-act="last" aria-label="最后一帧">⏭</button>
          <input type="range" min="0" max="${p.anim.frames.length - 1}" value="0" data-scrub aria-label="跳到某一帧">
          <button type="button" data-act="speed" class="speed">1×</button>
        </div>
        <script type="application/json" data-anim>${JSON.stringify(p.anim).replace(/</g, '\\u003c')}</script>
      </div>
      <p class="hint">快捷键：← → 单步，空格播放 / 暂停。</p>
    </section>` : ''}

    <section id="code" class="sect">
      <h2>C++17 题解</h2>
      ${codeBlock(p.code, `src/${c.no}_${c.key}.cpp`)}
    </section>

    ${p.tests?.length ? `<section id="tests" class="sect">
      <h2>单元测试</h2>
      <p class="hint">以下用例都在本仓库里实际编译运行过，图解里的数据也取自这里。</p>
      ${p.tests.map(([, name, b]) => codeBlock(dedent(b), `TEST ${name}`, 'code-test')).join('\n')}
    </section>` : ''}

    <nav class="pager" aria-label="上一题和下一题">
      ${prev ? `<a class="pager-prev" href="${pUrl(prev.num)}" data-key-prev><small>← 上一题</small><span>${prev.num}. ${esc(prev.title)}</span></a>` : '<span></span>'}
      ${next ? `<a class="pager-next" href="${pUrl(next.num)}" data-key-next><small>下一题 →</small><span>${next.num}. ${esc(next.title)}</span></a>` : '<span></span>'}
    </nav>
  </article>
</div>`;
  const plain = (p.desc?.zh || '').replace(/<[^>]+>/g, '').replace(/&nbsp;/g, ' ').replace(/\s+/g, ' ').trim();
  return layout({
    title: `${p.num}. ${p.title}`,
    page: 'problem',
    description: `${p.num}. ${p.title}（${DIFF[p.diff]}）C++ 题解：${p.tag}。${plain.slice(0, 80)}`,
    body,
    extraHead: p.anim ? `<script src="${url('assets/player.js')}" defer></script>` : '',
  });
}

// ---------------------------------------------------------------- 套路页
function patternsPage() {
  const body = `
<section class="page-head">
  <p class="kicker">套路速查</p>
  <h1>十七类题，各自的那几行心法</h1>
  <p class="lede">摘自每个分类源文件开头的总结。做完一类题再回来读一遍，比单题刷十遍更管用。</p>
  <nav class="pattern-jump" aria-label="分类">${categories.map((c) => `<a href="#${c.key}">${c.no} ${esc(c.name)}</a>`).join('')}</nav>
</section>
<div class="patterns">
${categories.map((c) => `  <section class="pattern" id="${c.key}">
    <header><span class="chapter-no">${c.no}</span><div><h2>${esc(c.name)}</h2><p class="en">${esc(c.en)}</p></div></header>
    <p class="pattern-note">${esc(c.note)}</p>
    ${c.guide ? `<div class="guide">${inline(c.guide)}</div>` : ''}
    <ul class="pattern-list">
${c.problems.map((p) => `      <li><a href="${pUrl(p.num)}"><span class="row-num">${p.num}</span><span class="pl-title">${esc(p.title)}</span><span class="pl-tag">${esc(p.tag)}</span><span class="diff diff-${diffKey(p.diff)}">${DIFF[p.diff]}</span></a></li>`).join('\n')}
    </ul>
  </section>`).join('\n')}
</div>`;
  return layout({ title: '套路速查', page: 'patterns', description: '力扣热题 100 的 17 类题目通用套路与每题一句话要点。', body });
}

function notFoundPage() {
  return layout({
    title: '页面不存在',
    page: '404',
    description: '页面不存在',
    body: `<section class="page-head nf"><p class="kicker">404</p><h1>这一页走丢了</h1><p class="lede">可以按 <kbd>/</kbd> 搜索题目，或者<a href="${url()}">回到题库</a>。</p></section>`,
  });
}

// ---------------------------------------------------------------- 输出
function write(rel, content) {
  const file = path.join(OUT, rel);
  fs.mkdirSync(path.dirname(file), { recursive: true });
  fs.writeFileSync(file, content);
}

fs.rmSync(OUT, { recursive: true, force: true });
for (const f of fs.readdirSync(path.join(HERE, 'assets'))) {
  write(`assets/${f}`, fs.readFileSync(path.join(HERE, 'assets', f)));
}
const index = ordered.map((p) => ({
  n: p.num, t: p.title, d: diffKey(p.diff), g: p.tag, c: catOf.get(p.category).name, a: p.anim ? 1 : 0,
}));
write('assets/index.js', `window.H100=${JSON.stringify({ base: BASE, problems: index })};\n`);
write('index.html', homePage());
write('patterns/index.html', patternsPage());
write('404.html', notFoundPage());
for (const p of problems) write(`problems/${p.num}/index.html`, problemPage(p));
write('.nojekyll', '');
write('sitemap.xml', `<?xml version="1.0" encoding="UTF-8"?>
<urlset xmlns="http://www.sitemaps.org/schemas/sitemap/0.9">
${['', 'patterns/', ...ordered.map((p) => `problems/${p.num}/`)].map((u) => `  <url><loc>${SITE_URL}${BASE}${u}</loc></url>`).join('\n')}
</urlset>
`);
console.log(`[site] ${problems.length} 题 · ${categories.length} 类 -> ${path.relative(ROOT, OUT) || OUT}（base ${BASE}）`);
