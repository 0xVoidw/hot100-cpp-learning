<script lang="ts">
  import { onDestroy, onMount } from 'svelte';
  import type { AnimationData, AnimationScript, Paint } from '../lib/types';

  export let script: AnimationScript;

  const palette = {
    base: '#eff2ff', stroke: '#6875e8', ink: '#18213b', muted: '#74809a',
    highlight: '#ffe5a3', done: '#c9f2d1', hot: '#ffd0c9', pointer: '#e45a68', pointer2: '#5969e9',
  };
  const speeds = [0.5, 0.75, 1, 1.5, 2];
  let index = 0;
  let speed = 1;
  let playing = false;
  let timer: ReturnType<typeof setInterval> | undefined;

  $: frame = script.frames[index] ?? { note: '', paint: [] };
  $: previousFrame = script.frames[Math.max(0, index - 1)] ?? frame;
  $: visibleData = frame.values ?? script.data;
  $: previousData = previousFrame.values ?? script.data;
  $: arrayData = asArray(visibleData);
  $: previousArray = asArray(previousData);
  $: gridData = asGrid(visibleData);
  $: previousGrid = asGrid(previousData);
  $: tree = script.kind === 'tree' ? layoutTree(arrayData) : emptyTree();
  $: story = splitNote(frame.note);
  $: pointers = frame.paint.filter((item) => item.f === 'pt' || item.f === 'npt');
  $: stateItems = frame.state?.length ? frame.state : deriveState(pointers, arrayData, gridData);
  // 显式引用依赖，保证每次换帧时阶段标签都重算。
  $: changedCount = script.kind === 'grid'
    ? gridData.reduce((total, row, r) => total + row.filter((_, c) => previousGrid[r]?.[c] !== gridData[r]?.[c]).length, 0)
    : arrayData.filter((value, i) => previousArray[i] !== value).length;
  $: pointerMoved = JSON.stringify(previousFrame.paint.filter((item) => item.f === 'pt' || item.f === 'npt')) !== JSON.stringify(pointers);
  $: phase = changedCount ? '更新状态' : pointerMoved ? '推进指针' : '建立不变量';

  type Value = string | number | null;
  type TreeNode = { id: number; value: Value; left?: TreeNode; right?: TreeNode; depth: number; x: number };
  type TreeLayout = { nodes: TreeNode[]; edges: Array<[TreeNode, TreeNode]>; width: number; height: number };

  function asArray(data: AnimationData): Value[] {
    return Array.isArray(data) && !Array.isArray(data[0]) ? data as Value[] : [];
  }
  function asGrid(data: AnimationData): Value[][] {
    return Array.isArray(data) && Array.isArray(data[0]) ? data as Value[][] : [];
  }
  function splitNote(note: string) {
    const pivot = note.search(/[：:]/);
    if (pivot < 0) return { title: '当前推演', detail: note };
    return { title: note.slice(0, pivot).trim() || '当前推演', detail: note.slice(pivot + 1).trim() };
  }
  function paint(kind: Paint['f']) { return frame.paint.filter((item) => item.f === kind); }
  function fillFor(predicate: (item: Paint) => boolean) {
    const item = frame.paint.find(predicate);
    return item ? (item.c ?? item.color ?? palette.highlight) : palette.base;
  }
  function arrayFill(i: number) { return fillFor((item) => item.f === 'box' && item.i === i); }
  function listFill(i: number) { return fillFor((item) => item.f === 'node' && item.i === i); }
  function gridFill(row: number, col: number) { return fillFor((item) => item.f === 'cell' && item.r === row && item.col === col); }
  function treeFill(id: number) { return fillFor((item) => item.f === 'tn' && item.id === id); }
  function changedArray(i: number) { return previousArray[i] !== arrayData[i]; }
  function changedGrid(row: number, col: number) { return previousGrid[row]?.[col] !== gridData[row]?.[col]; }
  function countChanges() {
    if (script.kind === 'grid') return gridData.reduce((total, row, r) => total + row.filter((_, c) => changedGrid(r, c)).length, 0);
    return arrayData.filter((_, i) => changedArray(i)).length;
  }
  function pointersChanged() {
    const old = previousFrame.paint.filter((item) => item.f === 'pt' || item.f === 'npt');
    return JSON.stringify(old) !== JSON.stringify(pointers);
  }
  function deriveState(items: Paint[], values: Value[], grid: Value[][]) {
    const result: Array<{ label: string; value: string }> = [];
    for (const item of items) {
      if (script.kind === 'grid') result.push({ label: item.label ?? item.k ?? '位置', value: `(${item.r ?? 0}, ${item.col ?? 0})` });
      else result.push({ label: item.label ?? item.k ?? '指针', value: String(item.i ?? 0) + (values[item.i ?? -1] != null ? ` · ${values[item.i ?? -1]}` : '') });
    }
    if (!result.length && grid.length) result.push({ label: '画布', value: `${grid.length} × ${Math.max(...grid.map((row) => row.length))}` });
    return result;
  }
  function pointerX(item: Paint) { return script.kind === 'list' ? 36 + (item.i ?? 0) * 88 + 28 : 34 + (item.i ?? 0) * 62 + 27; }
  function pointerY(item: Paint) { return script.kind === 'grid' ? 28 + (item.r ?? 0) * 50 - 2 : 48; }
  function pointerColor(item: Paint) { return item.color ?? (item.k === 'R' || item.k === 'fast' ? palette.pointer : palette.pointer2); }
  function setFrame(next: number) { index = Math.max(0, Math.min(script.frames.length - 1, next)); }
  function stop() { playing = false; if (timer) clearInterval(timer); timer = undefined; }
  function play() {
    if (playing) return stop();
    if (index >= script.frames.length - 1) setFrame(0);
    playing = true;
    timer = setInterval(() => index >= script.frames.length - 1 ? stop() : setFrame(index + 1), 1150 / speed);
  }
  function changeSpeed() { speed = speeds[(speeds.indexOf(speed) + 1) % speeds.length]; if (playing) { stop(); play(); } }
  function keydown(event: KeyboardEvent) {
    if ((event.target as HTMLElement)?.tagName === 'INPUT') return;
    if (event.key === 'ArrowLeft') { stop(); setFrame(index - 1); }
    if (event.key === 'ArrowRight') { stop(); setFrame(index + 1); }
    if (event.key === ' ') { event.preventDefault(); play(); }
  }
  onMount(() => { window.addEventListener('keydown', keydown); });
  onDestroy(() => { stop(); if (typeof window !== 'undefined') window.removeEventListener('keydown', keydown); });

  function emptyTree(): TreeLayout { return { nodes: [], edges: [], width: 320, height: 120 }; }
  function layoutTree(values: Value[]): TreeLayout {
    if (!values.length || values[0] == null) return emptyTree();
    const nodes: TreeNode[] = [];
    const make = (value: Value, depth: number) => { const node: TreeNode = { id: nodes.length, value, depth, x: 0 }; nodes.push(node); return node; };
    const root = make(values[0], 0); const queue = [root]; let cursor = 1;
    while (queue.length && cursor < values.length) {
      const parent = queue.shift()!;
      for (const side of ['left', 'right'] as const) {
        if (cursor >= values.length) break;
        const value = values[cursor++];
        if (value != null) { parent[side] = make(value, parent.depth + 1); queue.push(parent[side]!); }
      }
    }
    let order = 0;
    const walk = (node?: TreeNode) => { if (!node) return; walk(node.left); node.x = order++; walk(node.right); };
    walk(root);
    const edges: Array<[TreeNode, TreeNode]> = [];
    nodes.forEach((node) => { if (node.left) edges.push([node, node.left]); if (node.right) edges.push([node, node.right]); });
    return { nodes, edges, width: Math.max(280, 76 * Math.max(1, nodes.length - 1) + 56), height: 70 * (Math.max(...nodes.map((node) => node.depth)) + 1) + 56 };
  }
  function tx(node: TreeNode) { return 28 + node.x * 76; }
  function ty(node: TreeNode) { return 30 + node.depth * 70; }
</script>

<section class="algo-player" aria-label="算法推演器">
  <header class="player-head">
    <div><span class="player-kicker">算法推演器 · {script.kind === 'array' ? '线性序列' : script.kind === 'grid' ? '二维状态' : script.kind === 'list' ? '指针连接' : '树形遍历'}</span><h3>{story.title}</h3></div>
    <div class="step-badge"><b>{String(index + 1).padStart(2, '0')}</b><span>/ {String(script.frames.length).padStart(2, '0')}</span></div>
  </header>

  <div class="player-layout">
    <div class="stage-column">
      <div class="scene-meta"><span class:has-change={changedCount > 0}>{phase}</span>{#if changedCount > 0}<small>{changedCount} 处数据发生变化</small>{:else}<small>观察当前不变量</small>{/if}</div>
      <div class="scene-scroll">
        {#if script.kind === 'array'}
          <svg class="scene" viewBox={`0 0 ${Math.max(300, 68 + arrayData.length * 62)} 162`} role="img" aria-label="数组状态">
            <defs><filter id="glow"><feGaussianBlur stdDeviation="2" result="blur"/><feMerge><feMergeNode in="blur"/><feMergeNode in="SourceGraphic"/></feMerge></filter></defs>
            <rect width="100%" height="100%" rx="16" fill="#fbfcff" />
            {#key index}{#each arrayData as value, i}
              <g class:changed={changedArray(i)} class="scene-item" transform={`translate(${34 + i * 62}, 62)`}>
                {#if changedArray(i)}<text x="27" y="-18" text-anchor="middle" class="old-value">{previousArray[i]} →</text>{/if}
                <rect width="54" height="54" rx="12" fill={arrayFill(i)} stroke={changedArray(i) ? '#e45a68' : palette.stroke} stroke-width={changedArray(i) ? 3 : 2} />
                <text x="27" y="33" text-anchor="middle" fill={palette.ink} font-size="17" font-family="monospace">{value}</text>
                <text x="27" y="74" text-anchor="middle" class="index-label">{i}</text>
              </g>
            {/each}{/key}
            {#each paint('pt') as item (item.k)}<g class="pointer" style={`transform:translate(${pointerX(item)}px,${pointerY(item)}px)`}><text x="0" y="-8" text-anchor="middle" fill={pointerColor(item)}>{item.label ?? item.k}</text><line x1="0" y1="0" x2="0" y2="25" stroke={pointerColor(item)} stroke-width="2.5"/><path d="M-5 23 L5 23 L0 30 Z" fill={pointerColor(item)} /></g>{/each}
          </svg>
        {:else if script.kind === 'grid'}
          <svg class="scene" viewBox={`0 0 ${Math.max(300, 64 + Math.max(...gridData.map((row) => row.length)) * 58)} ${72 + gridData.length * 50}`} role="img" aria-label="网格状态">
            <rect width="100%" height="100%" rx="16" fill="#fbfcff" />
            {#key index}{#each gridData as row, r}{#each row as value, col}
              <g class:changed={changedGrid(r, col)} class="scene-item" transform={`translate(${34 + col * 58},${38 + r * 50})`}>
                <rect width="54" height="46" rx="10" fill={gridFill(r, col)} stroke={changedGrid(r, col) ? '#e45a68' : palette.stroke} stroke-width={changedGrid(r, col) ? 2.8 : 1.7} />
                <text x="27" y="29" text-anchor="middle" fill={palette.ink} font-size="15" font-family="monospace">{value}</text>
                {#if changedGrid(r, col)}<circle cx="48" cy="7" r="4" fill="#e45a68" />{/if}
              </g>
            {/each}{/each}{/key}
          </svg>
        {:else if script.kind === 'list'}
          <svg class="scene" viewBox={`0 0 ${Math.max(310, 72 + arrayData.length * 88)} 170`} role="img" aria-label="链表状态">
            <defs><marker id="arrow" markerWidth="8" markerHeight="8" refX="6" refY="3" orient="auto"><path d="M0,0 L0,6 L7,3 z" fill="#6875e8" /></marker></defs>
            <rect width="100%" height="100%" rx="16" fill="#fbfcff" />
            {#key index}{#each arrayData as value, i}
              {#if i < arrayData.length - 1}<line x1={90 + i * 88} y1="89" x2={120 + i * 88} y2="89" stroke="#6875e8" stroke-width="2.4" marker-end="url(#arrow)" />{/if}
              <g class:changed={changedArray(i)} class="scene-item" transform={`translate(${34 + i * 88},62)`}>
                {#if changedArray(i)}<text x="28" y="-17" text-anchor="middle" class="old-value">{previousArray[i]} →</text>{/if}
                <rect width="56" height="56" rx="13" fill={listFill(i)} stroke={changedArray(i) ? '#e45a68' : palette.stroke} stroke-width={changedArray(i) ? 3 : 2} />
                <text x="28" y="35" text-anchor="middle" fill={palette.ink} font-size="17" font-family="monospace">{value}</text>
              </g>
            {/each}{/key}
            {#each pointers as item (item.k)}<g class="pointer" style={`transform:translate(${pointerX(item)}px,48px)`}><text x="0" y="-8" text-anchor="middle" fill={pointerColor(item)}>{item.label ?? item.k}</text><line x1="0" y1="0" x2="0" y2="25" stroke={pointerColor(item)} stroke-width="2.5"/><path d="M-5 23 L5 23 L0 30 Z" fill={pointerColor(item)} /></g>{/each}
          </svg>
        {:else}
          <svg class="scene" viewBox={`0 0 ${tree.width} ${tree.height}`} role="img" aria-label="二叉树状态">
            <rect width="100%" height="100%" rx="16" fill="#fbfcff" />
            {#each tree.edges as edge}<line x1={tx(edge[0])} y1={ty(edge[0]) + 19} x2={tx(edge[1])} y2={ty(edge[1]) - 19} stroke="#cbd5e1" stroke-width="2.3" />{/each}
            {#key index}{#each tree.nodes as node (node.id)}<g class="scene-item" transform={`translate(${tx(node)},${ty(node)})`}><circle r="19" fill={treeFill(node.id)} stroke={treeFill(node.id) === palette.base ? palette.stroke : '#e45a68'} stroke-width="2.3"/><text y="5" text-anchor="middle" fill={palette.ink} font-size="15" font-family="monospace">{node.value}</text></g>{/each}{/key}
          </svg>
        {/if}
      </div>
      <div class="timeline" aria-label="步骤时间线">{#each script.frames as _, step}<button class:current={step === index} class:complete={step < index} type="button" on:click={() => { stop(); setFrame(step); }} aria-label={`跳到第 ${step + 1} 步`}><i></i><span>{step + 1}</span></button>{/each}</div>
    </div>

    <aside class="inspector">
      <div class="inspector-block"><span class="label">这一帧在做什么</span><p>{story.detail}</p></div>
      <div class="inspector-block"><span class="label">变量检查器</span>{#if stateItems.length}<div class="state-grid">{#each stateItems as item}<div><small>{item.label}</small><b>{item.value}</b></div>{/each}</div>{:else}<p class="empty">本帧通过颜色强调当前结构关系。</p>{/if}</div>
      <div class="legend"><span><i class="focus"></i>正在处理</span><span><i class="done"></i>已确认</span><span><i class="changed"></i>本帧更新</span></div>
    </aside>
  </div>

  <footer class="player-controls">
    <div class="controls"><button type="button" on:click={() => { stop(); setFrame(0); }} title="第一帧">⏮</button><button type="button" on:click={() => { stop(); setFrame(index - 1); }} title="上一步">←</button><button class="play" type="button" on:click={play}>{playing ? '暂停' : '播放'} {playing ? 'Ⅱ' : '▶'}</button><button type="button" on:click={() => { stop(); setFrame(index + 1); }} title="下一步">→</button><button type="button" on:click={() => { stop(); setFrame(script.frames.length - 1); }} title="最后一帧">⏭</button></div>
    <div class="shortcuts"><button type="button" on:click={changeSpeed}>{speed}×</button><span>← → 单步 · 空格播放</span></div>
  </footer>
</section>

<style>
  .algo-player{overflow:hidden;border:1px solid var(--line);border-radius:1.25rem;background:var(--surface-solid);box-shadow:0 16px 36px rgba(43,51,93,.07)}.player-head{display:flex;align-items:center;justify-content:space-between;gap:1rem;padding:1rem 1.15rem;border-bottom:1px solid var(--line);background:linear-gradient(110deg,color-mix(in srgb,var(--brand) 8%,var(--surface-solid)),var(--surface-solid))}.player-kicker,.label{display:block;color:var(--brand);font-size:.68rem;font-weight:900;letter-spacing:.09em;text-transform:uppercase}.player-head h3{margin:.24rem 0 0;font-size:1.04rem;letter-spacing:-.02em}.step-badge{display:flex;align-items:baseline;gap:.2rem;border:1px solid color-mix(in srgb,var(--brand) 22%,var(--line));border-radius:.7rem;background:var(--surface-solid);padding:.42rem .56rem;font-family:var(--font-mono)}.step-badge b{color:var(--brand);font-size:1.05rem}.step-badge span{color:var(--muted);font-size:.72rem}.player-layout{display:grid;grid-template-columns:minmax(0,1.5fr) minmax(15rem,.72fr)}.stage-column{min-width:0;padding:1rem;border-right:1px solid var(--line)}.scene-meta{display:flex;align-items:center;gap:.55rem;margin-bottom:.7rem}.scene-meta span{border-radius:999px;background:color-mix(in srgb,var(--brand) 10%,var(--surface-solid));color:var(--brand);padding:.25rem .5rem;font-size:.7rem;font-weight:850}.scene-meta span.has-change{background:#fff0ee;color:#d34d5c}.scene-meta small{color:var(--muted);font-size:.74rem}.scene-scroll{overflow-x:auto;border:1px solid var(--line);border-radius:1rem;background:#fbfcff}.scene{display:block;min-width:100%;height:auto}.scene-item{animation:enter .38s cubic-bezier(.2,.9,.3,1)}.scene-item.changed{animation:changed .7s ease}.old-value{fill:#d34d5c;font-size:12px;font-family:var(--font-mono);font-weight:800}.index-label{fill:#8a94a8;font-size:12px}.pointer{transition:transform .48s cubic-bezier(.2,.9,.3,1)}.pointer text{font-size:13px;font-weight:850}.timeline{display:flex;align-items:center;gap:.1rem;overflow-x:auto;margin-top:.8rem;padding:.25rem}.timeline button{position:relative;display:grid;min-width:2rem;place-items:center;gap:.25rem;border:0;background:transparent;color:var(--muted);cursor:pointer}.timeline button::before{position:absolute;top:.42rem;left:-50%;z-index:0;width:100%;height:2px;background:var(--line);content:""}.timeline button:first-child::before{display:none}.timeline i{z-index:1;width:.7rem;height:.7rem;border:2px solid var(--line);border-radius:50%;background:var(--surface-solid)}.timeline span{font:700 .68rem var(--font-mono)}.timeline button.complete i{border-color:#73c98a;background:#73c98a}.timeline button.current{color:var(--brand)}.timeline button.current i{border-color:var(--brand);background:var(--brand);box-shadow:0 0 0 4px color-mix(in srgb,var(--brand) 14%,transparent)}.inspector{display:flex;flex-direction:column;gap:1rem;padding:1rem;background:color-mix(in srgb,var(--brand) 2%,var(--surface-solid))}.inspector-block{border-bottom:1px solid var(--line);padding-bottom:1rem}.inspector-block:last-of-type{border:0;padding:0}.inspector p{margin:.45rem 0 0;color:var(--muted);font-size:.86rem;line-height:1.7}.state-grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:.45rem;margin-top:.6rem}.state-grid div{min-width:0;border:1px solid var(--line);border-radius:.65rem;background:var(--surface-solid);padding:.42rem .5rem}.state-grid small{display:block;overflow:hidden;color:var(--muted);font-size:.65rem;text-overflow:ellipsis;white-space:nowrap}.state-grid b{display:block;overflow:hidden;margin-top:.16rem;color:var(--brand);font:.74rem var(--font-mono);text-overflow:ellipsis;white-space:nowrap}.empty{font-size:.8rem!important}.legend{display:flex;flex-wrap:wrap;gap:.6rem;color:var(--muted);font-size:.68rem}.legend span{display:flex;align-items:center;gap:.25rem}.legend i{width:.52rem;height:.52rem;border-radius:50%}.legend .focus{background:#ffe5a3}.legend .done{background:#c9f2d1}.legend .changed{background:#ffd0c9}.player-controls{display:flex;align-items:center;justify-content:space-between;gap:.7rem;border-top:1px solid var(--line);padding:.72rem 1rem;background:var(--surface-solid)}.controls,.shortcuts{display:flex;align-items:center;gap:.35rem}.controls button,.shortcuts button{border:1px solid var(--line);border-radius:.6rem;background:var(--surface);color:var(--ink);padding:.38rem .58rem;cursor:pointer;font-size:.78rem;font-weight:800}.controls button:hover,.shortcuts button:hover{border-color:var(--brand);color:var(--brand)}.controls .play{border-color:var(--brand);background:var(--brand);color:#fff;padding-inline:.8rem}.shortcuts span{color:var(--muted);font-size:.68rem}@keyframes enter{from{opacity:.25;transform:translateY(5px)}to{opacity:1;transform:translateY(0)}}@keyframes changed{0%{filter:drop-shadow(0 0 0 rgba(228,90,104,0));transform:scale(.88)}55%{filter:drop-shadow(0 0 7px rgba(228,90,104,.45));transform:scale(1.05)}100%{filter:drop-shadow(0 0 0 rgba(228,90,104,0));transform:scale(1)}}@media(max-width:850px){.player-layout{grid-template-columns:1fr}.stage-column{border-right:0;border-bottom:1px solid var(--line)}.inspector{display:grid;grid-template-columns:1fr 1fr}.legend{grid-column:1/-1}}@media(max-width:530px){.player-head,.player-controls{align-items:flex-start;flex-direction:column}.inspector{grid-template-columns:1fr}.timeline{margin-inline:-.5rem}.shortcuts span{display:none}}
</style>
