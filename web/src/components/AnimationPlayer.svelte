<script lang="ts">
  import { onDestroy } from 'svelte';
  import type { AnimationScript, Paint } from '../lib/types';

  export let script: AnimationScript;
  const colors = { base: '#eef2ff', stroke: '#6366f1', hl: '#ffe9a8', done: '#cdeccd', hot: '#ffd1cc', ptr: '#e2554d', ptr2: '#4f6ef7' };
  const speeds = [0.5, 0.75, 1, 1.5, 2];
  let index = 0;
  let speed = 1;
  let playing = false;
  let timer: ReturnType<typeof setInterval> | undefined;

  $: frame = script.frames[index] ?? { note: '', paint: [] };
  // values 存在时表示该帧之后的真实数据状态（交换、反转、BFS 等）。
  $: visibleData = frame.values ?? script.data;
  $: arrayData = Array.isArray(visibleData) && !Array.isArray(visibleData[0]) ? visibleData as Array<string | number | null> : [];
  $: gridData = Array.isArray(visibleData) && Array.isArray(visibleData[0]) ? visibleData as Array<Array<string | number | null>> : [];
  $: tree = script.kind === 'tree' ? layoutTree(arrayData) : { nodes: [], edges: [], width: 320, height: 100 };

  function paints(kind: Paint['f']) { return frame.paint.filter((p) => p.f === kind); }
  function fillFor(predicate: (p: Paint) => boolean) {
    const match = frame.paint.find(predicate);
    return match ? (match.c ?? match.color ?? colors.hl) : colors.base;
  }
  function arrayFill(i: number) { return fillFor((p) => p.f === 'box' && p.i === i); }
  function listFill(i: number) { return fillFor((p) => p.f === 'node' && p.i === i); }
  function gridFill(r: number, col: number) { return fillFor((p) => p.f === 'cell' && p.r === r && p.col === col); }
  function treeFill(id: number) { return fillFor((p) => p.f === 'tn' && p.id === id); }
  function pointerX(p: Paint) {
    if (script.kind === 'array') return 30 + (p.i ?? 0) * 56 + 24;
    if (script.kind === 'list') return 30 + (p.i ?? 0) * 86 + 26;
    if (script.kind === 'grid') return 30 + (p.col ?? 0) * 54 + 27;
    return 30;
  }
  function pointerY(p: Paint) {
    if (script.kind === 'grid') return 30 + (p.r ?? 0) * 44 - 8;
    return 44;
  }
  function setFrame(next: number) { index = Math.max(0, Math.min(script.frames.length - 1, next)); }
  function stop() { playing = false; if (timer) clearInterval(timer); timer = undefined; }
  function play() {
    if (playing) { stop(); return; }
    if (index >= script.frames.length - 1) setFrame(0);
    playing = true;
    timer = setInterval(() => { if (index >= script.frames.length - 1) stop(); else setFrame(index + 1); }, 900 / speed);
  }
  function changeSpeed() { speed = speeds[(speeds.indexOf(speed) + 1) % speeds.length]; if (playing) { stop(); play(); } }
  onDestroy(stop);

  type TreeNode = { id: number; value: string | number | null; left?: TreeNode; right?: TreeNode; depth: number; x: number };
  function layoutTree(values: Array<string | number | null>) {
    if (!values.length || values[0] == null) return { nodes: [] as TreeNode[], edges: [] as Array<[TreeNode, TreeNode]>, width: 320, height: 100 };
    const nodes: TreeNode[] = [];
    const make = (value: string | number | null, depth: number) => { const node: TreeNode = { id: nodes.length, value, depth, x: 0 }; nodes.push(node); return node; };
    const root = make(values[0], 0);
    const queue = [root]; let pos = 1;
    while (queue.length && pos < values.length) {
      const parent = queue.shift()!;
      if (pos < values.length) { const value = values[pos++]; if (value != null) { parent.left = make(value, parent.depth + 1); queue.push(parent.left); } }
      if (pos < values.length) { const value = values[pos++]; if (value != null) { parent.right = make(value, parent.depth + 1); queue.push(parent.right); } }
    }
    let order = 0;
    const walk = (node?: TreeNode) => { if (!node) return; walk(node.left); node.x = order++; walk(node.right); };
    walk(root);
    const edges: Array<[TreeNode, TreeNode]> = [];
    nodes.forEach((node) => { if (node.left) edges.push([node, node.left]); if (node.right) edges.push([node, node.right]); });
    return { nodes, edges, width: Math.max(260, 72 * Math.max(1, nodes.length - 1) + 52), height: 60 * (Math.max(...nodes.map((n) => n.depth)) + 1) + 52 };
  }
  function tx(node: TreeNode) { return 26 + node.x * 72; }
  function ty(node: TreeNode) { return 26 + node.depth * 60; }
</script>

<div class="anim-shell">
  <div class="anim-note">{frame.note}</div>
  {#if frame.state?.length}
    <div class="anim-state">{#each frame.state as item}<span><small>{item.label}</small><b>{item.value}</b></span>{/each}</div>
  {/if}
  <div class="anim-scroll">
    {#if script.kind === 'array'}
      <svg class="anim-svg" viewBox={`0 0 ${Math.max(180, 60 + arrayData.length * 56)} 132`} aria-label="数组动画">
        <rect width="100%" height="100%" rx="12" fill="#fbfbfd" />
        {#each arrayData as value, i}
          <rect x={30 + i * 56} y="56" width="48" height="48" rx="8" fill={arrayFill(i)} stroke={colors.stroke} stroke-width="2" style="transition:fill .35s" />
          <text x={54 + i * 56} y="86" text-anchor="middle" fill="#16204a" font-size="16" font-family="monospace">{value}</text>
          <text x={54 + i * 56} y="120" text-anchor="middle" fill="#8893b8" font-size="12">{i}</text>
        {/each}
        {#each paints('pt') as p (p.k)}
          <g style={`transition:transform .45s cubic-bezier(.4,1,.4,1);transform:translate(${pointerX(p)}px,${pointerY(p)}px)`}>
            <text x="0" y="-7" text-anchor="middle" fill={p.color ?? (p.k === 'R' ? colors.ptr : colors.ptr2)} font-size="14" font-weight="700">{p.label ?? p.k}</text>
            <line x1="0" y1="0" x2="0" y2="21" stroke={p.color ?? colors.ptr} stroke-width="2.4" />
            <path d="M-4 19 L4 19 L0 25 Z" fill={p.color ?? colors.ptr} />
          </g>
        {/each}
      </svg>
    {:else if script.kind === 'grid'}
      <svg class="anim-svg" viewBox={`0 0 ${Math.max(180, 60 + Math.max(...gridData.map((r) => r.length)) * 54)} ${68 + gridData.length * 44}`} aria-label="网格动画">
        <rect width="100%" height="100%" rx="12" fill="#fbfbfd" />
        {#each gridData as row, r}
          {#each row as value, col}
            <rect x={32 + col * 54} y={32 + r * 44} width="50" height="40" rx="7" fill={gridFill(r, col)} stroke={colors.stroke} stroke-width="1.6" style="transition:fill .35s" />
            <text x={57 + col * 54} y={58 + r * 44} text-anchor="middle" fill="#16204a" font-size="14" font-family="monospace">{value}</text>
          {/each}
        {/each}
      </svg>
    {:else if script.kind === 'list'}
      <svg class="anim-svg" viewBox={`0 0 ${Math.max(220, 65 + arrayData.length * 86)} 146`} aria-label="链表动画">
        <rect width="100%" height="100%" rx="12" fill="#fbfbfd" />
        {#each arrayData as value, i}
          {#if i < arrayData.length - 1}<line x1={82 + i * 86} y1="82" x2={112 + i * 86} y2="82" stroke={colors.stroke} stroke-width="2" />{/if}
          <rect x={30 + i * 86} y="56" width="52" height="52" rx="8" fill={listFill(i)} stroke={colors.stroke} stroke-width="2" style="transition:fill .35s" />
          <text x={56 + i * 86} y="88" text-anchor="middle" fill="#16204a" font-size="16" font-family="monospace">{value}</text>
        {/each}
        {#each [...paints('npt'), ...paints('pt')] as p (p.k)}
          <g style={`transition:transform .45s cubic-bezier(.4,1,.4,1);transform:translate(${pointerX(p)}px,44px)`}>
            <text x="0" y="-7" text-anchor="middle" fill={p.color ?? colors.ptr} font-size="14" font-weight="700">{p.label ?? p.k}</text><line x1="0" y1="0" x2="0" y2="21" stroke={p.color ?? colors.ptr} stroke-width="2.4" /><path d="M-4 19 L4 19 L0 25 Z" fill={p.color ?? colors.ptr} />
          </g>
        {/each}
      </svg>
    {:else}
      <svg class="anim-svg" viewBox={`0 0 ${tree.width} ${tree.height}`} aria-label="二叉树动画">
        <rect width="100%" height="100%" rx="12" fill="#fbfbfd" />
        {#each tree.edges as edge}<line x1={tx(edge[0])} y1={ty(edge[0]) + 17} x2={tx(edge[1])} y2={ty(edge[1]) - 17} stroke="#cbd5e1" stroke-width="2" />{/each}
        {#each tree.nodes as node (node.id)}
          <circle cx={tx(node)} cy={ty(node)} r="17" fill={treeFill(node.id)} stroke={colors.stroke} stroke-width="2" style="transition:fill .35s" />
          <text x={tx(node)} y={ty(node) + 5} text-anchor="middle" fill="#16204a" font-size="14" font-family="monospace">{node.value}</text>
        {/each}
      </svg>
    {/if}
  </div>
  <div class="anim-control">
    <button type="button" on:click={() => { stop(); setFrame(0); }} title="第一帧">⏮</button>
    <button type="button" on:click={() => { stop(); setFrame(index - 1); }} title="上一步">◀</button>
    <button class="primary" type="button" on:click={play}>{playing ? '⏸ 暂停' : '▶ 播放'}</button>
    <button type="button" on:click={() => { stop(); setFrame(index + 1); }} title="下一步">▶</button>
    <button type="button" on:click={() => { stop(); setFrame(script.frames.length - 1); }} title="最后一帧">⏭</button>
    <span class="muted text-xs">速度</span><button type="button" on:click={changeSpeed}>{speed}×</button>
    <span class="frame">{index + 1} / {script.frames.length}</span>
  </div>
</div>
