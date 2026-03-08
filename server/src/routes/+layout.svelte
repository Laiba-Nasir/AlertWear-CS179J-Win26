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
        <nav class="navbar">
            <div class="nav-brand">
                <img src="/logo.png" alt="VERTEX" class="logo" />
            </div>
            
            <div class="nav-links">
                <a href="/" class:active={$page.url.pathname === '/'}>
                    Dashboard
                </a>
                <a href="/admin/workers" class:active={$page.url.pathname.startsWith('/admin')}>
                    Admin
                </a>
                <a href="/debug" class:active={$page.url.pathname === '/debug'}>
                    Debug
                </a>
            </div>

            <div class="nav-user">
                <button class="theme-toggle" on:click={toggleTheme}>
                    {isDark ? '☀️' : '🌙'}
                </button>
                <a href="/login" class="logout-btn">Logout</a>
            </div>
        </nav>

        <main>
            <slot />
        </main>
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
        --bg-nav: #1e293b;
        --bg-status: #e2e8f0;
        --text-primary: #1e293b;
        --text-secondary: #475569;
        --text-muted: #64748b;
        --border: rgba(0, 0, 0, 0.1);
        --shadow: 0 1px 3px rgba(0, 0, 0, 0.1);
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
        min-height: 100vh;
        display: flex;
        flex-direction: column;
    }

    .navbar {
        background: var(--bg-nav);
        color: white;
        padding: 0 24px;
        height: 60px;
        display: flex;
        align-items: center;
        justify-content: space-between;
        position: sticky;
        top: 0;
        z-index: 100;
    }

    .nav-brand {
        display: flex;
        align-items: center;
    }

    .logo {
        height: 59px;
        width: auto;
    }

    .nav-links {
        display: flex;
        gap: 8px;
    }

    .nav-links a {
        color: #94a3b8;
        text-decoration: none;
        padding: 8px 16px;
        border-radius: 6px;
        font-size: 14px;
        font-weight: 500;
        transition: all 0.2s;
    }

    .nav-links a:hover {
        color: white;
        background: #334155;
    }

    .nav-links a.active {
        color: white;
        background: #3b82f6;
    }

    .nav-user {
        display: flex;
        align-items: center;
        gap: 10px;
    }

    .theme-toggle {
        background: #334155;
        border: none;
        padding: 8px 12px;
        border-radius: 6px;
        cursor: pointer;
        font-size: 16px;
        transition: background 0.2s;
    }

    .theme-toggle:hover {
        background: #475569;
    }

    .logout-btn {
        background: #334155;
        padding: 8px 16px;
        border-radius: 6px;
        font-size: 13px;
        color: #94a3b8;
        text-decoration: none;
        transition: all 0.2s;
    }

    .logout-btn:hover {
        background: #475569;
        color: white;
    }

    main {
        flex: 1;
    }
</style>