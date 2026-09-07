<script lang="ts">
  import { onMount } from 'svelte';
  export let num: number;
  let done = false;
  onMount(() => {
    try { done = new Set<number>(JSON.parse(localStorage.getItem('hot100-done') ?? '[]')).has(num); } catch { done = false; }
  });
  function toggle() {
    const values = new Set<number>(JSON.parse(localStorage.getItem('hot100-done') ?? '[]'));
    done ? values.delete(num) : values.add(num);
    done = !done;
    localStorage.setItem('hot100-done', JSON.stringify([...values]));
  }
</script>
<button type="button" class:done on:click={toggle}>{done ? '✓ 已掌握' : '○ 标记为已掌握'}</button>
<style>
  button { border:1px solid var(--line); border-radius:999px; background:var(--surface); color:var(--muted); padding:.48rem .72rem; cursor:pointer; font-size:.8rem; font-weight:750; }
  button.done { border-color:#34d399; background:#ecfdf5; color:#15803d; }
  :global(.dark) button.done { background:#143323; color:#86efac; }
</style>
