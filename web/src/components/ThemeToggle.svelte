<script lang="ts">
  import { onMount } from 'svelte';

  let dark = false;
  function apply(value: boolean) {
    dark = value;
    document.documentElement.classList.toggle('dark', value);
    localStorage.setItem('hot100-theme', value ? 'dark' : 'light');
  }

  onMount(() => {
    const saved = localStorage.getItem('hot100-theme');
    apply(saved ? saved === 'dark' : window.matchMedia('(prefers-color-scheme: dark)').matches);
  });
</script>

<button class="theme-toggle" type="button" on:click={() => apply(!dark)} aria-label="切换深色模式" title="切换深色模式">
  <span>{dark ? '☀️' : '🌙'}</span>
</button>

<style>
  .theme-toggle { display:grid; width:2.45rem; height:2.45rem; place-items:center; border:1px solid var(--line); border-radius:.8rem; background:var(--surface); cursor:pointer; box-shadow:0 6px 18px rgba(48,55,95,.08); }
  .theme-toggle:hover { transform: translateY(-1px); border-color: var(--brand); }
</style>
