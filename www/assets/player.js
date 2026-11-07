/* 逐帧算法动画播放器：读取 <script type="application/json" data-anim>，用 SVG 绘制每一帧 */
(function () {
  'use strict';
  var NS = 'http://www.w3.org/2000/svg';
  var C = {
    base: '#f3efe4', stroke: '#8c8370', ink: '#1f1d18', muted: '#8a8372',
    focus: '#ffe9a8', changed: '#c2410c', edge: '#c9c1ad', p1: '#7c3aed', p2: '#c2410c',
  };
  var SPEEDS = [1, 1.5, 2, 0.5, 0.75];

  function el(name, attrs, text) {
    var e = document.createElementNS(NS, name);
    for (var k in attrs) if (attrs[k] != null) e.setAttribute(k, attrs[k]);
    if (text != null) e.textContent = text;
    return e;
  }
  function isGrid(d) { return Array.isArray(d) && Array.isArray(d[0]); }
  function show(v) { return v == null ? '∅' : String(v); }

  function layoutTree(values) {
    var nodes = [];
    if (!values.length || values[0] == null) return { nodes: nodes, w: 320, h: 120 };
    function make(v, depth) { var n = { id: nodes.length, v: v, depth: depth, x: 0 }; nodes.push(n); return n; }
    var root = make(values[0], 0), queue = [root], i = 1;
    while (queue.length && i < values.length) {
      var parent = queue.shift();
      ['left', 'right'].forEach(function (side) {
        if (i >= values.length) return;
        var v = values[i++];
        if (v != null) { parent[side] = make(v, parent.depth + 1); queue.push(parent[side]); }
      });
    }
    var order = 0;
    (function walk(n) { if (!n) return; walk(n.left); n.x = order++; walk(n.right); })(root);
    var maxDepth = Math.max.apply(null, nodes.map(function (n) { return n.depth; }));
    return { nodes: nodes, w: Math.max(320, 64 * (nodes.length - 1) + 80), h: 76 * maxDepth + 80 };
  }

  function Player(root) {
    this.root = root;
    this.s = JSON.parse(root.querySelector('[data-anim]').textContent);
    this.svg = root.querySelector('.stage-svg');
    this.note = root.querySelector('[data-note]');
    this.step = root.querySelector('[data-step]');
    this.vars = root.querySelector('[data-vars]');
    this.scrub = root.querySelector('[data-scrub]');
    this.playBtn = root.querySelector('[data-act="play"]');
    this.speedBtn = root.querySelector('[data-act="speed"]');
    this.i = 0; this.speed = 1; this.timer = null;
    this.ptrs = {};
    this.prepare();
    this.bind();
    this.draw();
  }

  Player.prototype.values = function (i) {
    var f = this.s.frames[i];
    return f && f.values ? f.values : this.s.data;
  };

  // 预先计算画布尺寸，播放过程中保持不变，避免画面跳动
  Player.prototype.prepare = function () {
    var s = this.s, self = this;
    var maxStack = 1, maxLen = 0, rows = 0, cols = 0;
    s.frames.forEach(function (f, i) {
      var v = self.values(i);
      if (isGrid(v)) { rows = Math.max(rows, v.length); v.forEach(function (r) { cols = Math.max(cols, r.length); }); }
      else maxLen = Math.max(maxLen, v.length);
      var count = {};
      (f.paint || []).forEach(function (p) {
        if (p.f === 'pt' || p.f === 'npt') { count[p.i] = (count[p.i] || 0) + 1; maxStack = Math.max(maxStack, count[p.i]); }
      });
    });
    this.top = 62;
    if (s.kind === 'array') { this.W = Math.max(320, 48 + maxLen * 62); this.H = this.top + 54 + 40; }
    else if (s.kind === 'list') { this.W = Math.max(320, 48 + maxLen * 88); this.H = this.top + 56 + (s.cycle != null ? 56 : 30); }
    else if (s.kind === 'grid') { this.W = Math.max(320, 48 + cols * 58); this.H = 40 + rows * 50; }
    else { this.tree = layoutTree(s.data); this.W = this.tree.w; this.H = this.tree.h; }
    this.svg.setAttribute('viewBox', '0 0 ' + this.W + ' ' + this.H);
    this.svg.style.maxWidth = Math.max(this.W, 480) + 'px';
    this.svg.style.minWidth = Math.min(this.W, 560) + 'px';
    this.svg.style.margin = '0 auto';
    this.layer = el('g'); this.ptrLayer = el('g');
    var defs = el('defs');
    var mk = el('marker', { id: 'ah-' + Math.random().toString(36).slice(2, 8), markerWidth: 8, markerHeight: 8, refX: 6, refY: 3, orient: 'auto' });
    mk.appendChild(el('path', { d: 'M0,0 L0,6 L7,3 z', fill: C.stroke }));
    defs.appendChild(mk);
    this.arrowId = mk.id;
    this.svg.appendChild(defs);
    this.svg.appendChild(this.layer);
    this.svg.appendChild(this.ptrLayer);
  };

  Player.prototype.fill = function (match) {
    var f = this.s.frames[this.i];
    var hit = (f.paint || []).filter(match).pop();
    return hit ? (hit.c || hit.color || C.focus) : C.base;
  };

  Player.prototype.draw = function () {
    var s = this.s, i = this.i, f = s.frames[i], self = this;
    var cur = this.values(i), prev = this.values(Math.max(0, i - 1));
    var L = this.layer;
    while (L.firstChild) L.removeChild(L.firstChild);
    var changed = function (a, b) { return i > 0 && JSON.stringify(a) !== JSON.stringify(b); };

    if (s.kind === 'array' || s.kind === 'list') {
      var step = s.kind === 'array' ? 62 : 88, size = s.kind === 'array' ? 54 : 56;
      cur.forEach(function (v, k) {
        var x = 24 + k * step, y = self.top;
        var g = el('g', { transform: 'translate(' + x + ',' + y + ')' });
        var inner = el('g', { 'class': changed(v, prev[k]) ? 'changed' : null });
        var fillC = self.fill(function (p) { return (p.f === 'box' || p.f === 'node') && p.i === k; });
        var ch = changed(v, prev[k]);
        inner.appendChild(el('rect', { 'class': 'cell', width: size, height: size, rx: s.kind === 'list' ? 28 : 11, fill: fillC, stroke: ch ? C.changed : C.stroke, 'stroke-width': ch ? 2.6 : 1.6 }));
        var label = show(v);
        inner.appendChild(el('text', { x: size / 2, y: size / 2 + 6, 'text-anchor': 'middle', fill: C.ink, 'font-size': label.length > 3 ? 13 : 17 }, label));
        g.appendChild(inner);
        if (s.kind === 'array') g.appendChild(el('text', { x: size / 2, y: size + 20, 'text-anchor': 'middle', fill: C.muted, 'font-size': 12 }, k));
        L.appendChild(g);
        if (s.kind === 'list' && k < cur.length - 1) {
          L.appendChild(el('line', { x1: x + size + 4, y1: y + size / 2, x2: x + step - 6, y2: y + size / 2, stroke: C.stroke, 'stroke-width': 2, 'marker-end': 'url(#' + self.arrowId + ')' }));
        }
      });
      if (s.kind === 'list' && s.cycle != null && cur.length) {
        var lx = 24 + (cur.length - 1) * 88 + 28, cx = 24 + s.cycle * 88 + 28, by = self.top + 56;
        L.appendChild(el('path', { d: 'M' + lx + ',' + (by + 2) + ' C' + lx + ',' + (by + 46) + ' ' + cx + ',' + (by + 46) + ' ' + cx + ',' + (by + 8), fill: 'none', stroke: C.changed, 'stroke-width': 2, 'stroke-dasharray': '5 4', 'marker-end': 'url(#' + self.arrowId + ')' }));
      }
    } else if (s.kind === 'grid') {
      cur.forEach(function (row, r) {
        row.forEach(function (v, c) {
          var g = el('g', { transform: 'translate(' + (24 + c * 58) + ',' + (20 + r * 50) + ')' });
          var ch = changed(v, prev[r] && prev[r][c]);
          var inner = el('g', { 'class': ch ? 'changed' : null });
          inner.appendChild(el('rect', { 'class': 'cell', width: 54, height: 46, rx: 9, fill: self.fill(function (p) { return p.f === 'cell' && p.r === r && p.col === c; }), stroke: ch ? C.changed : C.stroke, 'stroke-width': ch ? 2.4 : 1.4 }));
          var label = show(v);
          inner.appendChild(el('text', { x: 27, y: 29, 'text-anchor': 'middle', fill: C.ink, 'font-size': label.length > 3 ? 12 : 15 }, label));
          g.appendChild(inner);
          L.appendChild(g);
        });
      });
    } else {
      var t = this.tree;
      var X = function (n) { return 40 + n.x * 64; }, Y = function (n) { return 36 + n.depth * 76; };
      t.nodes.forEach(function (n) {
        ['left', 'right'].forEach(function (side) {
          if (n[side]) L.appendChild(el('line', { x1: X(n), y1: Y(n) + 20, x2: X(n[side]), y2: Y(n[side]) - 20, stroke: C.edge, 'stroke-width': 2 }));
        });
      });
      t.nodes.forEach(function (n) {
        var fc = self.fill(function (p) { return p.f === 'tn' && p.id === n.id; });
        var g = el('g', { transform: 'translate(' + X(n) + ',' + Y(n) + ')' });
        g.appendChild(el('circle', { 'class': 'cell', r: 20, fill: fc, stroke: fc === C.base ? C.stroke : C.changed, 'stroke-width': fc === C.base ? 1.6 : 2.2 }));
        g.appendChild(el('text', { y: 5, 'text-anchor': 'middle', fill: C.ink, 'font-size': 15 }, show(n.v)));
        L.appendChild(g);
      });
    }

    // 指针：同一 key 复用同一个元素，靠 CSS transition 平滑移动
    var seen = {}, stack = {}, total = {};
    (f.paint || []).forEach(function (p) { if (p.f === 'pt' || p.f === 'npt') total[p.i] = (total[p.i] || 0) + 1; });
    (f.paint || []).forEach(function (p) {
      if (p.f !== 'pt' && p.f !== 'npt') return;
      if (s.kind !== 'array' && s.kind !== 'list') return;
      var key = p.k || p.label || 'p';
      seen[key] = true;
      var stepX = s.kind === 'array' ? 62 : 88, size = s.kind === 'array' ? 54 : 56;
      var lvl = stack[p.i] = (stack[p.i] || 0) + 1;
      var spread = Math.min(16, (size - 10) / Math.max(1, total[p.i] - 1));
      var x = 24 + p.i * stepX + size / 2 + (lvl - 1 - (total[p.i] - 1) / 2) * spread, y = self.top - 34;
      var color = p.color || (/^(r|R|fast|right|hi)$/.test(key) ? C.p2 : C.p1);
      var g = self.ptrs[key];
      if (!g) {
        g = el('g', { 'class': 'ptr' });
        g.appendChild(el('text', { 'text-anchor': 'middle', y: -4, 'font-size': 12, 'font-weight': 700 }));
        g.appendChild(el('path', { d: 'M0,0 L0,0' }));
        self.ptrLayer.appendChild(g);
        self.ptrs[key] = g;
      }
      var len = self.top - y - 3;
      g.style.transform = 'translate(' + x + 'px,' + y + 'px)';
      g.style.opacity = 1;
      g.firstChild.textContent = p.label || p.k;
      g.firstChild.setAttribute('fill', color);
      g.lastChild.setAttribute('d', 'M0,2 L0,' + len + ' M-4,' + (len - 5) + ' L0,' + len + ' L4,' + (len - 5));
      g.lastChild.setAttribute('stroke', color);
      g.lastChild.setAttribute('stroke-width', 2);
      g.lastChild.setAttribute('fill', 'none');
    });
    Object.keys(this.ptrs).forEach(function (k) { if (!seen[k]) self.ptrs[k].style.opacity = 0; });

    // 旁注
    this.note.textContent = f.note || '';
    this.step.textContent = i + 1;
    this.scrub.value = i;
    var items = f.state && f.state.length ? f.state : (f.paint || []).filter(function (p) { return p.f === 'pt' || p.f === 'npt'; }).map(function (p) {
      var v = Array.isArray(cur) && !isGrid(cur) ? cur[p.i] : undefined;
      return { label: p.label || p.k, value: (s.kind === 'list' ? '节点 ' : '下标 ') + p.i + (v != null ? ' · ' + v : '') };
    });
    this.vars.innerHTML = '';
    items.forEach(function (it) {
      var d = document.createElement('div');
      var dt = document.createElement('dt'); dt.textContent = it.label;
      var dd = document.createElement('dd'); dd.textContent = it.value;
      d.appendChild(dt); d.appendChild(dd); self.vars.appendChild(d);
    });
  };

  Player.prototype.go = function (i) {
    var n = this.s.frames.length;
    this.i = Math.max(0, Math.min(n - 1, i));
    this.draw();
  };
  Player.prototype.stop = function () {
    clearInterval(this.timer); this.timer = null; this.playBtn.textContent = '播放';
  };
  Player.prototype.play = function () {
    var self = this;
    if (this.timer) return this.stop();
    if (this.i >= this.s.frames.length - 1) this.go(0);
    this.playBtn.textContent = '暂停';
    this.timer = setInterval(function () {
      if (self.i >= self.s.frames.length - 1) return self.stop();
      self.go(self.i + 1);
    }, 1300 / this.speed);
  };
  Player.prototype.bind = function () {
    var self = this;
    this.root.addEventListener('click', function (e) {
      var b = e.target.closest('[data-act]');
      if (!b) return;
      var a = b.dataset.act;
      if (a === 'play') return self.play();
      if (a === 'speed') {
        self.speed = SPEEDS[(SPEEDS.indexOf(self.speed) + 1) % SPEEDS.length];
        b.textContent = self.speed + '×';
        if (self.timer) { self.stop(); self.play(); }
        return;
      }
      self.stop();
      if (a === 'first') self.go(0);
      if (a === 'prev') self.go(self.i - 1);
      if (a === 'next') self.go(self.i + 1);
      if (a === 'last') self.go(self.s.frames.length - 1);
    });
    this.scrub.addEventListener('input', function () { self.stop(); self.go(+self.scrub.value); });
  };

  function init() {
    var players = Array.prototype.map.call(document.querySelectorAll('[data-player]'), function (r) { return new Player(r); });
    if (!players.length) return;
    var p = players[0];
    document.addEventListener('keydown', function (e) {
      var tag = (e.target.tagName || '').toLowerCase();
      if (tag === 'input' && e.target.type !== 'range' || tag === 'textarea' || e.metaKey || e.ctrlKey || e.altKey) return;
      if (document.querySelector('dialog[open]')) return;
      if (e.key === 'ArrowLeft') { e.preventDefault(); p.stop(); p.go(p.i - 1); }
      else if (e.key === 'ArrowRight') { e.preventDefault(); p.stop(); p.go(p.i + 1); }
      else if (e.key === ' ' && tag !== 'button') { e.preventDefault(); p.play(); }
    });
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', init); else init();
})();
