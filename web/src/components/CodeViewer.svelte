<script lang="ts">
  export let code = '';
  let copied = false;
  async function copy() {
    try {
      await navigator.clipboard.writeText(code);
      copied = true;
      setTimeout(() => copied = false, 1400);
    } catch {
      copied = false;
    }
  }
</script>

<div class="code-view">
  <button class="copy-btn" type="button" on:click={copy}>{copied ? '已复制 ✓' : '复制 C++'}</button>
  <pre><code>{code}</code></pre>
</div>
