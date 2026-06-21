import { defineConfig } from 'vite'

// On GitHub Pages the site is served from a sub-path
// (https://<user>.github.io/LOOP-WHILE-GOTO-Simulator/), so the CI build sets
// GITHUB_PAGES=true to emit asset URLs under that base. Locally we keep '/'.
export default defineConfig({
  base: process.env.GITHUB_PAGES ? '/LOOP-WHILE-GOTO-Simulator/' : '/',
})
