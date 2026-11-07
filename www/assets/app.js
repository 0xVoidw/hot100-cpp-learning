/* Hot 100 · C++ 图解 —— 全站交互（无依赖） */
(function () {
  'use strict';
  var H = window.H100 || { base: '/', problems: [] };
  var page = document.body.dataset.page;
  var $ = function (s, r) { return (r || document).querySelector(s); };
  var $$ = function (s, r) { return Array.prototype.slice.call((r || document).querySelectorAll(s)); };

  // ---------- 本地存储（隐私模式等环境下可能不可用）
  function load(key, fallback) {
    try { var v = localStorage.getItem(key); return v == null ? fallback : JSON.parse(v); } catch (e) { return fallback; }
  }
  function save(key, value) { try { localStorage.setItem(key, JSON.stringify(value)); } catch (e) { /* 忽略 */ } }
  var status = load('h100:status', {}) || {};
  function getStatus(n) { return status[n] || 0; }
  function setStatus(n, s) {
    if (s) status[n] = s; else delete status[n];
    save('h100:status', status);
    paintStatus();
  }

  // ---------- 主题
  var themeBtn = $('[data-theme-toggle]');
  if (themeBtn) themeBtn.addEventListener('click', function () {
    var root = document.documentElement;
    var dark = root.dataset.theme ? root.dataset.theme === 'dark' : matchMedia('(prefers-color-scheme: dark)').matches;
    root.dataset.theme = dark ? 'light' : 'dark';
    try { localStorage.setItem('h100:theme', root.dataset.theme); } catch (e) { /* 忽略 */ }
  });

  // ---------- 进度上色（首页行、侧栏、题目页开关）
  function paintStatus() {
    $$('[data-num]').forEach(function (el) {
      if (el.classList.contains('problem')) return;
      var s = getStatus(el.dataset.num);
      if (s) el.dataset.status = s; else delete el.dataset.status;
    });
    var prob = $('.problem');
    if (prob) {
      var cur = getStatus(prob.dataset.num);
      $$('.status-switch button').forEach(function (b) { b.setAttribute('aria-checked', String(+b.dataset.status === cur)); });
    }
    if (page === 'home') paintHome();
  }

  // ---------- 首页
  function paintHome() {
    var total = H.problems.length;
    var done = 0, learn = 0;
    H.problems.forEach(function (p) { var s = getStatus(p.n); if (s === 2) done++; else if (s === 1) learn++; });
    $$('[data-count="done"]').forEach(function (e) { e.textContent = done; });
    $$('[data-count="learn"]').forEach(function (e) { e.textContent = learn; });
    $$('[data-count="todo"]').forEach(function (e) { e.textContent = total - done - learn; });
    var d = $('.ring-done'), l = $('.ring-learn');
    if (d) { d.style.strokeDasharray = (done / total * 100) + ' 100'; d.style.opacity = done ? 1 : 0; }
    if (l) { l.style.strokeDasharray = ((done + learn) / total * 100) + ' 100'; l.style.opacity = (done + learn) ? .55 : 0; }
    $$('[data-meter]').forEach(function (m) {
      var rows = $$('#c-' + m.dataset.meter + ' .row');
      var k = rows.filter(function (r) { return getStatus(r.dataset.num) === 2; }).length;
      $('b', m).textContent = k;
      $('s', m).style.width = (rows.length ? k / rows.length * 100 : 0) + '%';
    });
    var cont = $('[data-continue]');
    var last = load('h100:last', null);
    var next = H.problems.filter(function (p) { return getStatus(p.n) !== 2; })[0];
    if (cont && last && H.problems.some(function (p) { return p.n === last; })) {
      var lp = H.problems.filter(function (p) { return p.n === last; })[0];
      cont.href = H.base + 'problems/' + last + '/';
      cont.textContent = '继续：' + lp.n + '. ' + lp.t;
    } else if (cont && next && (done || learn)) {
      cont.href = H.base + 'problems/' + next.n + '/';
      cont.textContent = '下一道：' + next.n + '. ' + next.t;
    }
  }

  if (page === 'home') {
    var filters = { q: '', diff: '', status: '', anim: false };
    var q = $('#q');
    function apply() {
      var any = false;
      $$('.chapter').forEach(function (ch) {
        var shown = 0;
        $$('.row', ch).forEach(function (r) {
          var ok = (!filters.q || r.dataset.text.indexOf(filters.q) >= 0) &&
            (!filters.diff || r.dataset.diff === filters.diff) &&
            (filters.status === '' || String(getStatus(r.dataset.num)) === filters.status) &&
            (!filters.anim || r.dataset.anim === '1');
          r.hidden = !ok;
          if (ok) shown++;
        });
        ch.hidden = !shown;
        if (shown) any = true;
      });
      var empty = $('.empty-result');
      if (empty) empty.hidden = any;
    }
    if (q) q.addEventListener('input', function () { filters.q = q.value.trim().toLowerCase(); apply(); });
    $$('.chips[data-group]').forEach(function (group) {
      group.addEventListener('click', function (e) {
        var b = e.target.closest('.chip');
        if (!b) return;
        $$('.chip', group).forEach(function (c) { c.setAttribute('aria-pressed', String(c === b)); });
        filters[group.dataset.group] = b.dataset.v;
        apply();
      });
    });
    var animBtn = $('[data-anim-only]');
    if (animBtn) animBtn.addEventListener('click', function () {
      filters.anim = !filters.anim;
      animBtn.setAttribute('aria-pressed', String(filters.anim));
      apply();
    });
    var rnd = $('[data-random]');
    if (rnd) rnd.addEventListener('click', function () {
      var pool = H.problems.filter(function (p) { return getStatus(p.n) !== 2; });
      if (!pool.length) pool = H.problems;
      var p = pool[Math.floor(Math.random() * pool.length)];
      location.href = H.base + 'problems/' + p.n + '/';
    });
    // 目录高亮当前分类
    var tocLinks = $$('.toc a');
    if ('IntersectionObserver' in window && tocLinks.length) {
      var io = new IntersectionObserver(function (entries) {
        entries.forEach(function (en) {
          if (!en.isIntersecting) return;
          tocLinks.forEach(function (a) { a.classList.toggle('is-active', a.getAttribute('href') === '#' + en.target.id); });
        });
      }, { rootMargin: '-140px 0px -65% 0px' });
      $$('.chapter').forEach(function (c) { io.observe(c); });
    }
  }

  // ---------- 题目页
  if (page === 'problem') {
    var prob = $('.problem');
    var num = +prob.dataset.num;
    save('h100:last', num);
    $$('.status-switch button').forEach(function (b) {
      b.addEventListener('click', function () { setStatus(num, +b.dataset.status); });
    });
    // 章节导航高亮
    var navLinks = $$('.sect-nav a');
    if ('IntersectionObserver' in window && navLinks.length) {
      var so = new IntersectionObserver(function (entries) {
        entries.forEach(function (en) {
          if (!en.isIntersecting) return;
          navLinks.forEach(function (a) { a.classList.toggle('is-active', a.getAttribute('href') === '#' + en.target.id); });
        });
      }, { rootMargin: '-120px 0px -60% 0px' });
      $$('.sect').forEach(function (s) { so.observe(s); });
    }
    // 当前题在侧栏可见
    var curSib = $('.siblings a[aria-current="page"]');
    if (curSib && curSib.scrollIntoView) curSib.scrollIntoView({ block: 'nearest' });
  }

  // ---------- 复制代码
  document.addEventListener('click', function (e) {
    var b = e.target.closest('[data-copy]');
    if (!b) return;
    var code = b.closest('.code').querySelector('pre').innerText;
    var done = function () {
      b.textContent = '已复制'; b.classList.add('done');
      setTimeout(function () { b.textContent = '复制'; b.classList.remove('done'); }, 1400);
    };
    if (navigator.clipboard && navigator.clipboard.writeText) navigator.clipboard.writeText(code).then(done, function () {});
    else {
      var ta = document.createElement('textarea'); ta.value = code; document.body.appendChild(ta); ta.select();
      try { document.execCommand('copy'); done(); } catch (err) { /* 忽略 */ }
      ta.remove();
    }
  });

  // ---------- 搜索面板
  var dlg = $('#palette');
  var input = dlg && $('input', dlg);
  var list = dlg && $('.palette-list', dlg);
  var results = [], sel = 0;
  var DIFF = { easy: '简单', medium: '中等', hard: '困难' };
  function esc(s) { return String(s).replace(/[&<>"]/g, function (c) { return { '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;' }[c]; }); }
  function search(text) {
    var t = text.trim().toLowerCase();
    if (!t) return H.problems.slice(0, 30);
    var scored = [];
    H.problems.forEach(function (p) {
      var s = 0;
      if (String(p.n) === t) s = 100;
      else if (String(p.n).indexOf(t) === 0) s = 60;
      if (p.t.toLowerCase().indexOf(t) >= 0) s = Math.max(s, 50);
      if (p.c.toLowerCase().indexOf(t) >= 0) s = Math.max(s, 30);
      if (p.g.toLowerCase().indexOf(t) >= 0) s = Math.max(s, 20);
      if (s) scored.push([s, p]);
    });
    scored.sort(function (a, b) { return b[0] - a[0] || a[1].n - b[1].n; });
    return scored.slice(0, 40).map(function (x) { return x[1]; });
  }
  function render() {
    results = search(input.value);
    sel = Math.min(sel, Math.max(0, results.length - 1));
    list.innerHTML = results.length ? results.map(function (p, i) {
      return '<li><a role="option" href="' + H.base + 'problems/' + p.n + '/" aria-selected="' + (i === sel) + '" data-num="' + p.n + '">' +
        '<span class="row-num">' + p.n + '</span><span><b>' + esc(p.t) + '</b><span class="pl-sub">' + esc(p.c) + ' · ' + esc(p.g) + '</span></span>' +
        '<span class="diff diff-' + p.d + '">' + DIFF[p.d] + '</span></a></li>';
    }).join('') : '<li class="empty">没有找到相关题目</li>';
    paintStatus();
  }
  function move(d) {
    if (!results.length) return;
    sel = (sel + d + results.length) % results.length;
    $$('a', list).forEach(function (a, i) { a.setAttribute('aria-selected', String(i === sel)); if (i === sel) a.scrollIntoView({ block: 'nearest' }); });
  }
  function openSearch() {
    if (!dlg || dlg.open) return;
    input.value = ''; sel = 0; render();
    if (dlg.showModal) dlg.showModal(); else dlg.setAttribute('open', '');
    input.focus();
  }
  if (dlg) {
    $$('[data-open-search]').forEach(function (b) { b.addEventListener('click', openSearch); });
    input.addEventListener('input', function () { sel = 0; render(); });
    input.addEventListener('keydown', function (e) {
      if (e.key === 'ArrowDown') { e.preventDefault(); move(1); }
      else if (e.key === 'ArrowUp') { e.preventDefault(); move(-1); }
      else if (e.key === 'Enter' && results[sel]) { e.preventDefault(); location.href = H.base + 'problems/' + results[sel].n + '/'; }
    });
    dlg.addEventListener('click', function (e) { if (e.target === dlg) dlg.close(); });
  }

  // ---------- 全局快捷键
  document.addEventListener('keydown', function (e) {
    var tag = (e.target.tagName || '').toLowerCase();
    var typing = tag === 'input' || tag === 'textarea' || e.target.isContentEditable;
    if ((e.key === '/' && !typing) || ((e.metaKey || e.ctrlKey) && e.key.toLowerCase() === 'k')) {
      e.preventDefault(); openSearch(); return;
    }
    if (typing || e.metaKey || e.ctrlKey || e.altKey) return;
    if (page === 'problem' && !document.querySelector('[data-player]')) {
      if (e.key === 'ArrowLeft') { var p = $('[data-key-prev]'); if (p) location.href = p.href; }
      if (e.key === 'ArrowRight') { var n = $('[data-key-next]'); if (n) location.href = n.href; }
    }
    if (page === 'problem' && (e.key === '[' || e.key === ']')) {
      var link = $(e.key === '[' ? '[data-key-prev]' : '[data-key-next]');
      if (link) location.href = link.href;
    }
  });

  paintStatus();
})();
