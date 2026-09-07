<script lang="ts">
  import { onMount } from 'svelte';
  import type { Difficulty, ProblemSummary } from '../lib/types';
  import { categoryMeta } from '../lib/teaching';

  export let problems: ProblemSummary[] = [];
  export let hrefPrefix = 'problems/';
  const difficulties: Difficulty[] = ['Easy', 'Medium', 'Hard'];
  let query = '';
  let selectedDiff = '全部';
  let selectedCategory = '全部';
  let animatedOnly = false;
  let done = new Set<number>();

  onMount(() => {
    try { done = new Set(JSON.parse(localStorage.getItem('hot100-done') ?? '[]')); } catch { done = new Set(); }
  });

  $: categories = [...new Set(problems.map((p) => p.category))];
  $: filtered = problems.filter((p) => {
    const haystack = `${p.num} ${p.title} ${p.tag} ${p.category}`.toLowerCase();
    return (!query || haystack.includes(query.toLowerCase()))
      && (selectedDiff === '全部' || p.diff === selectedDiff)
      && (selectedCategory === '全部' || p.category === selectedCategory)
      && (!animatedOnly || Boolean(p.anim));
  });
  $: progress = problems.length ? Math.round((done.size / problems.length) * 100) : 0;

  function toggleDone(num: number, event: MouseEvent) {
    event.preventDefault();
    event.stopPropagation();
    const next = new Set(done);
    next.has(num) ? next.delete(num) : next.add(num);
    done = next;
    localStorage.setItem('hot100-done', JSON.stringify([...done]));
  }

  function categoryLabel(category: string) { return categoryMeta[category]?.label ?? category; }
</script>

<section id="problems" class="pb-14">
  <div class="mb-5 flex flex-wrap items-end justify-between gap-4">
    <div>
      <p class="eyebrow">你的刷题面板</p>
      <h2 class="mt-1 text-2xl font-black tracking-tight">从一题开始，建立自己的算法地图</h2>
    </div>
    <div class="progress-card glass">
      <div class="flex items-center justify-between gap-5 text-xs text-[var(--muted)]"><span>掌握进度</span><strong class="text-[var(--ink)]">{done.size} / {problems.length}</strong></div>
      <div class="mt-2 h-2 overflow-hidden rounded-full bg-[var(--line)]"><div class="h-full rounded-full bg-gradient-to-r from-violet-500 to-fuchsia-500 transition-all" style={`width:${progress}%`}></div></div>
    </div>
  </div>

  <div class="glass rounded-2xl p-4 sm:p-5">
    <label class="search-box">
      <span>⌕</span>
      <input bind:value={query} placeholder="搜索题号、题名、核心套路…" aria-label="搜索题目" />
      {#if query}<button type="button" on:click={() => query = ''}>清除</button>{/if}
    </label>
    <div class="mt-4 flex flex-wrap gap-2">
      <span class="filter-title">难度</span>
      {#each ['全部', ...difficulties] as item}
        <button class:active={selectedDiff === item} class={`chip ${item.toLowerCase()}`} type="button" on:click={() => selectedDiff = item}>{item}</button>
      {/each}
      <span class="filter-title ml-1">模式</span>
      <button class:active={animatedOnly} class="chip" type="button" on:click={() => animatedOnly = !animatedOnly}>▶ 仅看动图</button>
    </div>
    <div class="mt-2 flex flex-wrap gap-2">
      <span class="filter-title">分类</span>
      <button class:active={selectedCategory === '全部'} class="chip" type="button" on:click={() => selectedCategory = '全部'}>全部</button>
      {#each categories as category}
        <button class:active={selectedCategory === category} class="chip" type="button" on:click={() => selectedCategory = category}>{categoryLabel(category)}</button>
      {/each}
    </div>
  </div>

  <div class="mt-5 flex items-center justify-between text-sm text-[var(--muted)]"><span>显示 <strong class="text-[var(--ink)]">{filtered.length}</strong> 道题</span><span>{animatedOnly ? '动图讲解模式' : '全部内容模式'}</span></div>
  <div class="mt-4 grid gap-3 sm:grid-cols-2 lg:grid-cols-3">
    {#each filtered as problem (problem.num)}
      <a class="problem-card glass" class:done={done.has(problem.num)} href={`${hrefPrefix}${problem.num}/`}>
        <button class="done-toggle" type="button" aria-label={`标记 Q${problem.num} 已掌握`} on:click={(event) => toggleDone(problem.num, event)}>{done.has(problem.num) ? '✓' : ''}</button>
        <div class="flex items-center justify-between gap-3">
          <span class={`difficulty ${problem.diff.toLowerCase()}`}>{problem.diff}</span>
          {#if problem.anim}<span class="anim-badge">▶ 动图</span>{/if}
        </div>
        <div class="mt-4 flex gap-2"><span class="q-number">Q{problem.num}</span><h3>{problem.title}</h3></div>
        <p>{problem.tag}</p>
        <div class="mt-4 flex items-center justify-between border-t border-[var(--line)] pt-3 text-xs text-[var(--muted)]"><span>{categoryLabel(problem.category)}</span><span class="go">学习 →</span></div>
      </a>
    {/each}
  </div>
</section>

<style>
  .progress-card { width: min(17rem, 100%); border-radius: 1rem; padding: .8rem 1rem; }
  .search-box { display:flex; align-items:center; gap:.65rem; border:1px solid var(--line); border-radius:.85rem; background:var(--surface-solid); padding:.15rem .65rem; color:var(--muted); }
  .search-box input { min-width:0; flex:1; border:0; outline:0; background:transparent; color:var(--ink); padding:.65rem 0; }
  .search-box button { border:0; border-radius:.45rem; background:var(--line); color:var(--muted); padding:.3rem .5rem; cursor:pointer; font-size:.75rem; }
  .filter-title { display:inline-flex; align-items:center; color:var(--muted); font-size:.78rem; font-weight:700; margin-right:.15rem; }
  .chip { border:1px solid var(--line); border-radius:999px; background:var(--surface-solid); color:var(--muted); padding:.38rem .68rem; cursor:pointer; font-size:.78rem; font-weight:700; }
  .chip:hover,.chip.active { border-color:var(--brand); background:color-mix(in srgb,var(--brand) 12%,var(--surface-solid)); color:var(--brand); }
  .problem-card { position:relative; display:block; border-radius:1.05rem; padding:1rem; transition:transform .18s ease,border-color .18s ease,box-shadow .18s ease; }
  .problem-card:hover { transform:translateY(-3px); border-color:color-mix(in srgb,var(--brand) 40%,var(--line)); box-shadow:0 18px 35px rgba(58,53,130,.13); }
  .problem-card.done { background:color-mix(in srgb,#34d399 7%,var(--surface)); }
  .problem-card h3 { margin:0; font-size:1.05rem; line-height:1.35; font-weight:850; }
  .problem-card p { min-height:2.8em; margin:.7rem 0 0; color:var(--muted); font-size:.85rem; line-height:1.6; }
  .q-number { color:var(--brand); font:800 .75rem var(--font-mono); padding-top:.2rem; }
  .difficulty,.anim-badge { border-radius:999px; padding:.22rem .5rem; font-size:.68rem; font-weight:800; }
  .difficulty.easy { color:#15803d;background:#dcfce7; }.difficulty.medium { color:#b45309;background:#fef3c7; }.difficulty.hard { color:#be123c;background:#ffe4e6; }
  :global(.dark) .difficulty.easy{background:#143323;color:#86efac}:global(.dark) .difficulty.medium{background:#3a2b12;color:#fcd34d}:global(.dark) .difficulty.hard{background:#3c1823;color:#fda4af}
  .anim-badge { color:#6d28d9;background:#ede9fe; }:global(.dark) .anim-badge{background:#30234d;color:#d8b4fe}
  .done-toggle { position:absolute; right:.8rem; bottom:.72rem; display:grid; width:1.35rem; height:1.35rem; place-items:center; border:1px solid var(--line); border-radius:50%; background:var(--surface-solid); color:#16a34a; cursor:pointer; font-weight:900; }
  .go { color:var(--brand); font-weight:800; }
</style>
