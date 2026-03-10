<script lang="ts">
    import { page } from '$app/stores';
    import { writable } from 'svelte/store';

    $: isLoginPage = $page.url.pathname === '/login';

    // Dark mode store
    export const darkMode = writable(false);
    let isDark = false;

    function toggleTheme() {
        isDark = !isDark;
        darkMode.set(isDark);
        document.documentElement.setAttribute('data-theme', isDark ? 'dark' : 'light');
    }
</script>


{#if isLoginPage}
    <slot />
{:else}
    <div class="app">
        <aside class="sidebar">
            <div class="sidebar-brand">
                <img src="/logo.png" alt="VERTEX" class="logo" />
            </div>

            <nav class="sidebar-nav">
                <a href="/" class="nav-item" class:active={$page.url.pathname === '/'}>
                    <svg class="nav-icon" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor">
                        <path d="M3 3h8v8H3V3zm0 10h8v8H3v-8zm10-10h8v8h-8V3zm0 10h8v8h-8v-8z"/>
                    </svg>
                    <span>Dashboard</span>
                </a>

                <div class="nav-group">
                    <p class="nav-group-title">Administration</p>
                    <a href="/admin/workers" class="nav-item" class:active={$page.url.pathname === '/admin/workers'}>
                        <svg class="nav-icon" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor">
                            <path d="M16 11c1.66 0 2.99-1.34 2.99-3S17.66 5 16 5c-1.66 0-3 1.34-3 3s1.34 3 3 3zm-8 0c1.66 0 2.99-1.34 2.99-3S9.66 5 8 5C6.34 5 5 6.34 5 8s1.34 3 3 3zm0 2c-2.33 0-7 1.17-7 3.5V19h14v-2.5c0-2.33-4.67-3.5-7-3.5zm8 0c-.29 0-.62.02-.97.05 1.16.84 1.97 1.97 1.97 3.45V19h6v-2.5c0-2.33-4.67-3.5-7-3.5z"/>
                        </svg>
                        <span>Workers</span>
                    </a>
                    <a href="/admin/anchors" class="nav-item" class:active={$page.url.pathname === '/admin/anchors'}>
                        <svg class="nav-icon" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor">
                            <path d="M1 9l2 2c2.97-2.97 7.03-2.97 10 0l2-2C11.56 5.56 6.44 5.56 1 9zm8 8l3 3 3-3c-1.65-1.66-4.34-1.66-6 0zm-4-4l2 2c1.1-1.1 2.6-1.77 4.26-1.77S14.9 13.9 16 15l2-2C15.14 10.14 12.67 9 12 9c-.67 0-3.14 1.14-5 3H8z"/>
                        </svg>
                        <span>Anchors</span>
                    </a>
                </div>

                <a href="/debug" class="nav-item" class:active={$page.url.pathname === '/debug'}>
                    <svg class="nav-icon" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor">
                        <path d="M9.4 16.6L4.8 12l4.6-4.6L8 6l-6 6 6 6 1.4-1.4zm5.2 0l4.6-4.6-4.6-4.6L16 6l6 6-6 6-1.4-1.4z"/>
                    </svg>
                    <span>Debug</span>
                </a>
            </nav>

            <div class="sidebar-footer">
                <button class="nav-item theme-btn" on:click={toggleTheme}>
                    <svg class="nav-icon" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor">
                        {#if isDark}
                            <path d="M12 7c-2.76 0-5 2.24-5 5s2.24 5 5 5 5-2.24 5-5-2.24-5-5-5zM2 13h2c.55 0 1-.45 1-1s-.45-1-1-1H2c-.55 0-1 .45-1 1s.45 1 1 1zm18 0h2c.55 0 1-.45 1-1s-.45-1-1-1h-2c-.55 0-1 .45-1 1s.45 1 1 1zM11 2v2c0 .55.45 1 1 1s1-.45 1-1V2c0-.55-.45-1-1-1s-1 .45-1 1zm0 18v2c0 .55.45 1 1 1s1-.45 1-1v-2c0-.55-.45-1-1-1s-1 .45-1 1zM5.99 4.58c-.39-.39-1.03-.39-1.41 0-.39.39-.39 1.03 0 1.41l1.06 1.06c.39.39 1.03.39 1.41 0 .38-.39.39-1.03 0-1.41L5.99 4.58zm12.37 12.37c-.39-.39-1.03-.39-1.41 0-.39.39-.39 1.03 0 1.41l1.06 1.06c.39.39 1.03.39 1.41 0 .39-.39.39-1.03 0-1.41l-1.06-1.06zm1.06-12.37l-1.06 1.06c-.39.39-.39 1.03 0 1.41.39.39 1.03.39 1.41 0l1.06-1.06c.39-.39.39-1.03 0-1.41-.38-.39-1.03-.39-1.41 0zM7.05 18.36l-1.06 1.06c-.39.39-.39 1.03 0 1.41.39.39 1.03.39 1.41 0l1.06-1.06c.39-.39.39-1.03 0-1.41-.38-.39-1.03-.39-1.41 0z"/>
                        {:else}
                            <path d="M12 3c-4.97 0-9 4.03-9 9s4.03 9 9 9 9-4.03 9-9c0-.46-.04-.92-.1-1.36-.98 1.37-2.58 2.26-4.4 2.26-2.98 0-5.4-2.42-5.4-5.4 0-1.81.89-3.42 2.26-4.4-.44-.06-.9-.1-1.36-.1z"/>
                        {/if}
                    </svg>
                    <span>{isDark ? 'Light Mode' : 'Dark Mode'}</span>
                </button>

                <a href="/login" class="nav-item logout-item">
                    <svg class="nav-icon" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor">
                        <path d="M17 7l-1.41 1.41L18.17 11H8v2h10.17l-2.58 2.58L17 17l5-5-5-5zM4 5h8V3H4c-1.1 0-2 .9-2 2v14c0 1.1.9 2 2 2h8v-2H4V5z"/>
                    </svg>
                    <span>Logout</span>
                </a>
            </div>
        </aside>

        <div class="main-content">
            <slot />
        </div>
    </div>
{/if}

<style>
    :global(*) {
        margin: 0;
        padding: 0;
        box-sizing: border-box;
    }

    :global(:root) {
        --bg-primary: #f1f5f9;
        --bg-card: #ffffff;
        --bg-nav: #0f172a;
        --bg-status: #e2e8f0;
        --text-primary: #1e293b;
        --text-secondary: #475569;
        --text-muted: #64748b;
        --border: rgba(0, 0, 0, 0.1);
        --shadow: 0 1px 3px rgba(0, 0, 0, 0.1);
        --sidebar-width: 240px;
    }

    :global([data-theme="dark"]) {
        --bg-primary: #0f172a;
        --bg-card: #1e293b;
        --bg-nav: #020617;
        --bg-status: #1e293b;
        --text-primary: #f1f5f9;
        --text-secondary: #cbd5e1;
        --text-muted: #94a3b8;
        --border: rgba(255, 255, 255, 0.1);
        --shadow: 0 1px 3px rgba(0, 0, 0, 0.3);
    }

    :global(body) {
        font-family: system-ui, -apple-system, sans-serif;
        background: var(--bg-primary);
        color: var(--text-primary);
    }

    .app {
        display: flex;
        min-height: 100vh;
    }

    /* ── SIDEBAR ── */
    .sidebar {
        width: var(--sidebar-width);
        background: #0f172a;
        display: flex;
        flex-direction: column;
        position: fixed;
        top: 0;
        left: 0;
        bottom: 0;
        z-index: 100;
        overflow: hidden;
    }

    /* White brand area — mix-blend-mode:multiply makes logo's
       white background invisible while preserving its colors */
    .sidebar-brand {
        background: #ffffff;
        padding: 14px 20px;
        display: flex;
        align-items: center;
        justify-content: center;
        border-bottom: 3px solid #3b82f6;
        flex-shrink: 0;
    }

    .logo {
        height: 52px;
        width: auto;
        mix-blend-mode: multiply;
        display: block;
    }

    /* ── NAV ── */
    .sidebar-nav {
        flex: 1;
        padding: 16px 10px;
        display: flex;
        flex-direction: column;
        gap: 2px;
        overflow-y: auto;
    }

    .nav-item {
        display: flex;
        align-items: center;
        gap: 10px;
        padding: 10px 12px;
        border-radius: 8px;
        color: #94a3b8;
        text-decoration: none;
        font-size: 14px;
        font-weight: 500;
        transition: background 0.15s, color 0.15s;
        cursor: pointer;
        background: none;
        border: none;
        width: 100%;
        text-align: left;
        line-height: 1;
    }

    .nav-item:hover {
        background: #1e293b;
        color: #e2e8f0;
    }

    .nav-item.active {
        background: #1d4ed8;
        color: #ffffff;
    }

    .nav-icon {
        width: 18px;
        height: 18px;
        flex-shrink: 0;
    }

    .nav-group {
        margin-top: 10px;
    }

    .nav-group-title {
        font-size: 10px;
        font-weight: 700;
        text-transform: uppercase;
        letter-spacing: 0.1em;
        color: #475569;
        padding: 4px 12px 6px;
    }

    /* ── FOOTER ── */
    .sidebar-footer {
        padding: 10px;
        border-top: 1px solid #1e293b;
        display: flex;
        flex-direction: column;
        gap: 2px;
        flex-shrink: 0;
    }

    .logout-item {
        color: #f87171;
    }

    .logout-item:hover {
        background: rgba(248, 113, 113, 0.12);
        color: #fca5a5;
    }

    /* ── MAIN CONTENT ── */
    .main-content {
        margin-left: var(--sidebar-width);
        flex: 1;
        min-height: 100vh;
        background: var(--bg-primary);
    }
</style>
