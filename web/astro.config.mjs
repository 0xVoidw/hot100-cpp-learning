import { defineConfig } from 'astro/config';
import svelte from '@astrojs/svelte';
import tailwindcss from '@tailwindcss/vite';

export default defineConfig({
  site: 'https://0xvoidw.github.io',
  // GitHub Pages 项目站点的基础路径。
  base: '/hot100-cpp-learning',
  output: 'static',
  outDir: '../site',
  integrations: [svelte()],
  vite: {
    plugins: [tailwindcss()],
  },
});
