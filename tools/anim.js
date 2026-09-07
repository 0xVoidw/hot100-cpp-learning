/* LCAnim — 离线算法动画引擎（无外部依赖）
 * 用法：LCAnim.run(containerEl, script)
 * script = { kind:'array'|'grid'|'list'|'tree', data, frames:[...], note0 }
 *   array: data=[v,...]            frames[i]= {note, paint:[...]}
 *   grid : data=[[v,...],...]
 *   list : data=[v,...], cycle?:idx
 *   tree : data=[v,...]  (层序，nullopt 用 null)
 * paint 元素：
 *   {f:'box', i}           数组正方形
 *   {f:'box', r,c}         grid 单元格
 *   {f:'pt', k, i, color, label}      数组指针（按 k 运动）
 *   {f:'pt', k, r, c, color, label}   grid 指针
 *   {f:'node', i}          链表结点
 *   {f:'npt', k, i, color, label}     链表指针
 *   {f:'tn', id, color}    树结点
 * 当 paint 里不出现某个元素时，该元素恢复默认色。指针若不在 paint，则隐藏。
 */
(function (global) {
  'use strict';
  var COL = { base: '#eef2ff', baseFill: '#eef2ff', baseStroke: '#4f6ef7',
              hl: '#ffe9a8', done: '#cdeccd', hot: '#ffd1cc', ptr: '#e2554d', ptr2: '#4f6ef7' };
  var PTS = { top: 26, svgPad: 30, box: 48, gap: 8, gridCS: 54, gridRS: 44, rowH: 62, colW: 56, nodeR: 17 };

  function el(tag, attrs) {
    var e = document.createElementNS('http://www.w3.org/2000/svg', tag);
    for (var k in attrs) e.setAttribute(k, attrs[k]);
    return e;
  }

  function run(host, script) {
    host.innerHTML = '';
    var wrap = document.createElement('div');
    wrap.className = 'anim';
    host.appendChild(wrap);

    var ctl = document.createElement('div');
    ctl.className = 'anim-ctl';
    ctl.innerHTML = '<button class="ab" data-a="first" title="回到第一帧">⏮</button>'
      + '<button class="ab" data-a="prev" title="上一步">◀</button>'
      + '<button class="ab ab-play" data-a="play" title="播放">▶ 播放</button>'
      + '<button class="ab" data-a="next" title="下一步">▶</button>'
      + '<button class="ab" data-a="last" title="最后一帧">⏭</button>'
      + '<span class="speed">速度</span><button class="ab" data-a="spd">1×</button>'
      + '<span class="frame">1 / ' + script.frames.length + '</span>';
    wrap.appendChild(ctl);

    var noteEl = document.createElement('div');
    noteEl.className = 'anim-note';
    wrap.appendChild(noteEl);

    var box = document.createElement('div');
    box.className = 'anim-box';
    wrap.appendChild(box);

    var svg = null, P = script.data, frames = script.frames;
    var idx = -1, playing = false, timer = null, spd = 1;
    // 解析指针位置缓存
    var ptrEls = {};   // k -> {el, x,y}  array/grid/list 统一用一个 <g> 定位

    function build() {
      if (script.kind === 'array') return buildArray();
      if (script.kind === 'grid') return buildGrid();
      if (script.kind === 'list') return buildList();
      if (script.kind === 'tree') return buildTree();
      return buildArray();
    }

    function buildArray() {
      var n = P.length, W = PTS.box + PTS.gap;
      var w = PTS.svgPad * 2 + n * W - PTS.gap, h = PTS.svgPad * 2 + PTS.box + PTS.top + 8;
      svg = el('svg', { viewBox: '0 0 ' + w + ' ' + h, width: '100%', 'class': 'dia' });
      svg.appendChild(el('rect', { width: w, height: h, fill: '#fbfbfd' }));
      for (var i = 0; i < n; i++) {
        var x = PTS.svgPad + i * W;
        var r = el('rect', { x: x, y: PTS.svgPad + PTS.top, width: PTS.box, height: PTS.box, rx: 8,
              fill: COL.baseFill, stroke: COL.baseStroke, 'stroke-width': 2, 'data-i': i });
        r.style.transition = 'fill .4s';
        svg.appendChild(r);
        var t = el('text', { x: x + PTS.box / 2, y: PTS.svgPad + PTS.top + PTS.box / 2 + 6,
              'text-anchor': 'middle', fill: '#16204a', 'font-size': 17, 'font-family': 'monospace' });
        t.textContent = P[i];
        t.setAttribute('data-v', '' + i);
        svg.appendChild(t);
        var li = el('text', { x: x + PTS.box / 2, y: PTS.svgPad + PTS.top + PTS.box + 18,
              'text-anchor': 'middle', fill: '#8893b8', 'font-size': 12 });
        li.textContent = i;
        svg.appendChild(li);
      }
      return svg;
    }

    function buildGrid() {
      var R = P.length, C = Math.max.apply(null, P.map(function (r) { return r.length; }));
      var w = PTS.svgPad * 2 + C * PTS.gridCS, h = PTS.svgPad * 2 + R * PTS.gridRS + 8;
      svg = el('svg', { viewBox: '0 0 ' + w + ' ' + h, width: '100%', 'class': 'dia' });
      svg.appendChild(el('rect', { width: w, height: h, fill: '#fbfbfd' }));
      for (var i = 0; i < R; i++) for (var j = 0; j < C; j++) {
        var x = PTS.svgPad + j * PTS.gridCS, y = PTS.svgPad + i * PTS.gridRS;
        var r = el('rect', { x: x + 2, y: y + 2, width: PTS.gridCS - 4, height: PTS.gridRS - 4, rx: 6,
              fill: COL.baseFill, stroke: COL.baseStroke, 'stroke-width': 1.6, 'data-rc': i + ',' + j });
        r.style.transition = 'fill .4s';
        svg.appendChild(r);
        var t = el('text', { x: x + PTS.gridCS / 2, y: y + PTS.gridRS / 2 + 5, 'text-anchor': 'middle',
              fill: '#16204a', 'font-size': 15, 'font-family': 'monospace', 'data-v': i + ',' + j });
        t.textContent = P[i][j];
        svg.appendChild(t);
      }
      return svg;
    }

    function buildList() {
      var n = P.length, SW = 52, GAP = 34, H = 52;
      var w = PTS.svgPad * 2 + n * (SW + GAP) - GAP + 20, h = PTS.svgPad * 2 + H + PTS.top + 26;
      svg = el('svg', { viewBox: '0 0 ' + w + ' ' + h, width: '100%', 'class': 'dia' });
      svg.appendChild(el('rect', { width: w, height: h, fill: '#fbfbfd' }));
      var cx = [];
      for (var i = 0; i < n; i++) {
        var x = PTS.svgPad + i * (SW + GAP); cx.push(x + SW / 2);
        var r = el('rect', { x: x, y: PTS.svgPad + PTS.top, width: SW, height: H, rx: 8, fill: COL.baseFill,
              stroke: COL.baseStroke, 'stroke-width': 2, 'data-i': i });
        r.style.transition = 'fill .4s'; svg.appendChild(r);
        var t = el('text', { x: x + SW / 2, y: PTS.svgPad + PTS.top + H / 2 + 5, 'text-anchor': 'middle',
              fill: '#16204a', 'font-size': 16, 'font-family': 'monospace' });
        t.textContent = P[i]; svg.appendChild(t);
      }
      for (var j = 0; j < n - 1; j++)
        svg.appendChild(el('line', { x1: cx[j] + SW / 2, y1: PTS.svgPad + PTS.top + H / 2,
              x2: cx[j + 1] - SW / 2 - 2, y2: PTS.svgPad + PTS.top + H / 2, stroke: COL.baseStroke, 'stroke-width': 2 }));
      return svg;
    }

    function buildTree() {
      // data 为层序数组（null 表示空位）。BFS 建树，中序定 x。
      var vals = P;
      var nodes = [];  // {v,depth,x,id}
      var idOfIdx = {}; var i;
      function mk(v, depth) { nodes.push({ v: v, depth: depth, x: 0, id: nodes.length }); return nodes[nodes.length - 1]; }
      var stack = [];
      if (!vals.length || vals[0] == null) { svg = el('svg', { viewBox: '0 0 200 60', 'class': 'dia' }); return svg; }
      var root = mk(vals[0], 0); stack.push(root); i = 1;
      var q = [root];
      while (q.length && i < vals.length) {
        var cur = q.shift();
        if (i < vals.length) { if (vals[i] != null) { cur.left = mk(vals[i], cur.depth + 1); q.push(cur.left); } i++; }
        if (i < vals.length) { if (vals[i] != null) { cur.right = mk(vals[i], cur.depth + 1); q.push(cur.right); } i++; }
      }
      // 中序定 x
      var counter = 0;
      function inorder(n) { if (!n) return; inorder(n.left); n.x = counter++; inorder(n.right); }
      inorder(root);
      var maxD = 0; nodes.forEach(function (n) { maxD = Math.max(maxD, n.depth); });
      var W = 72, H = 60, R = 17, PAD = 26;
      var w = W * Math.max(1, nodes.length - 1) + PAD * 2, h = H * (maxD + 1) + PAD * 2;
      svg = el('svg', { viewBox: '0 0 ' + w + ' ' + h, width: '100%', 'class': 'dia' });
      svg.appendChild(el('rect', { width: w, height: h, fill: '#fbfbfd' }));
      function px(n) { return PAD + n.x * W; }
      function py(n) { return PAD + n.depth * H; }
      function edge(a, b) {
        svg.appendChild(el('line', { x1: px(a), y1: py(a) + R, x2: px(b), y2: py(b) - R, stroke: '#d0d5e0', 'stroke-width': 2 }));
      }
      (function walk(n) { if (!n) return; if (n.left) edge(n, n.left); if (n.right) edge(n, n.right); walk(n.left); walk(n.right); })(root);
      nodes.forEach(function (n) {
        var c = el('circle', { cx: px(n), cy: py(n), r: R, fill: COL.baseFill, stroke: COL.baseStroke, 'stroke-width': 2, 'data-id': n.id });
        c.style.transition = 'fill .4s'; svg.appendChild(c);
        var t = el('text', { x: px(n), y: py(n) + 5, 'text-anchor': 'middle', fill: '#16204a', 'font-size': 15, 'font-family': 'monospace' });
        t.textContent = n.v; svg.appendChild(t);
      });
      // 暴露 position 查询
      script._treepos = function (id) { var n = nodes[id]; return { x: px(n), y: py(n), r: R }; };
      return svg;
    }

    box.appendChild(build());

    // ---- 指针元素 ----
    function ptrXY(kind, spec) {
      if (kind === 'array') {
        var x = PTS.svgPad + spec.i * (PTS.box + PTS.gap) + PTS.box / 2;
        return { x: x, y: PTS.svgPad + PTS.top - 12 };
      }
      if (kind === 'grid') {
        var x = PTS.svgPad + spec.c * PTS.gridCS + PTS.gridCS / 2;
        var y = PTS.svgPad + spec.r * PTS.gridRS + PTS.gridRS / 2;
        return { x: x, y: y - PTS.gridRS - 6 };
      }
      if (kind === 'list') {
        var x = PTS.svgPad + spec.i * (52 + 34) + 52 / 2;
        return { x: x, y: PTS.svgPad + PTS.top - 12 };
      }
      return { x: PTS.svgPad, y: PTS.svgPad };
    }
    function makePtr(spec) {
      var g = el('g', { 'data-ptr': spec.k });
      var col = spec.color || COL.ptr;
      var line = el('line', { x1: 0, y1: 0, x2: 0, y2: 22, stroke: col, 'stroke-width': 2.4 });
      var head = el('polygon', { points: '-4,20 4,20 0,26', fill: col });
      var lbl = el('text', { x: 0, y: -6, 'text-anchor': 'middle', fill: col, 'font-size': 14, 'font-weight': 700 });
      lbl.textContent = spec.label || spec.k;
      g.appendChild(line); g.appendChild(head); g.appendChild(lbl);
      g.style.transition = 'transform .5s cubic-bezier(.4,1,.4,1)';
      svg.appendChild(g);
      return g;
    }

    function clearFills() {
      svg.querySelectorAll('[data-i],[data-rc],[data-id]').forEach(function (r) {
        r.setAttribute('fill', COL.baseFill); r.setAttribute('stroke', COL.baseStroke);
      });
    }

    function applyFrame(i, instant) {
      var f = frames[i];
      // 说明文字
      noteEl.textContent = f.note || '';
      clearFills();
      // 处理 paint
      (f.paint || []).forEach(function (p) {
        if (p.f === 'box') {
          var r = svg.querySelector('[data-i="' + p.i + '"]');
          if (r) r.setAttribute('fill', p.c || COL.hl);
        } else if (p.f === 'cell') {
          var rc2 = svg.querySelector('[data-rc="' + p.r + ',' + p.col + '"]');
          if (rc2) rc2.setAttribute('fill', p.color || COL.hl);
        } else if (p.f === 'node') {
          var nr = svg.querySelector('[data-i="' + p.i + '"]');
          if (nr) nr.setAttribute('fill', p.c || COL.hl);
        } else if (p.f === 'tn') {
          var nc = svg.querySelector('[data-id="' + p.id + '"]');
          if (nc) nc.setAttribute('fill', p.c || COL.hl);
        }
      });
      // 指针：先隐藏所有，再挪到新位置
      svg.querySelectorAll('[data-ptr]').forEach(function (g) { g.style.opacity = '0'; });
      (f.paint || []).forEach(function (p) {
        if (p.f === 'pt' || p.f === 'npt') {
          var g = ptrEls[p.k];
          if (!g) { g = makePtr(p); ptrEls[p.k] = g; }
          g.style.opacity = '1';
          var xy = ptrXY(script.kind, p);
          if (instant) g.style.transition = 'none';
          g.style.transform = 'translate(' + xy.x + 'px,' + xy.y + 'px)';
          if (instant) g.style.transition = '';
          // 更新文字/颜色
          var col = p.color || (p.k === 'R' ? COL.ptr : COL.ptr2);
          var s = g.children; s[2].textContent = p.label || p.k;
          s[0].setAttribute('stroke', col); s[1].setAttribute('fill', col); s[2].setAttribute('fill', col);
        }
      });
    }

    function setIdx(i, instant) {
      if (i < 0) i = 0; if (i > frames.length - 1) i = frames.length - 1;
      idx = i; applyFrame(idx, instant);
      var fr = ctl.querySelector('.frame'); if (fr) fr.textContent = (idx + 1) + ' / ' + frames.length;
    }

    function play() {
      if (playing) { pause(); return; }
      playing = true; var pb = ctl.querySelector('.ab-play'); if (pb) pb.textContent = '⏸ 暂停';
      if (idx >= frames.length - 1) setIdx(0, true);
      timer = setInterval(function () {
        if (idx >= frames.length - 1) { pause(); return; }
        setIdx(idx + 1);
      }, 900 / spd);
    }
    function pause() {
      playing = false; clearInterval(timer);
      var pb = ctl.querySelector('.ab-play'); if (pb) pb.textContent = '▶ 播放';
    }

    ctl.addEventListener('click', function (ev) {
      var b = ev.target.closest && ev.target.closest('.ab'); if (!b) return;
      var a = b.getAttribute('data-a');
      if (a === 'play') play();
      else if (a === 'next') setIdx(Math.min(frames.length - 1, idx + 1));
      else if (a === 'prev') setIdx(Math.max(0, idx - 1));
      else if (a === 'first') setIdx(0, true);
      else if (a === 'last') setIdx(frames.length - 1, true);
      else if (a === 'spd') { var sp=[0.5,0.75,1,1.5,2]; var k=sp.indexOf(spd); spd=sp[(k+1)%sp.length]; b.textContent=spd+'×'; }
    });

    setIdx(0, true);
  }

  global.LCAnim = { run: run };
})(typeof window !== 'undefined' ? window : this);
