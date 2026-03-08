<script lang="ts">
    import { goto } from '$app/navigation';

    let username = '';
    let password = '';
    let error = '';
    let loading = false;

    async function handleLogin() {
        error = '';
        loading = true;

        try {
            const res = await fetch('/api/auth/login', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ username, password })
            });

            const data = await res.json();

            if (res.ok) {
                goto('/');
            } else {
                error = data.error || 'Login failed';
            }
        } catch (e) {
            error = 'An error occurred. Please try again.';
        } finally {
            loading = false;
        }
    }
</script>

<svelte:head>
    <title>Login - VERTEX</title>
</svelte:head>

<div class="login-page">
    <div class="login-card">
        <div class="logo-container">
            <img src="/logo.png" alt="VERTEX" class="logo" />
        </div>

        <p class="subtitle">Sign in to access the dashboard</p>

        {#if error}
            <div class="error-message">
                {error}
            </div>
        {/if}

        <form on:submit|preventDefault={handleLogin}>
            <div class="form-group">
                <label for="username">Username</label>
                <input
                    type="text"
                    id="username"
                    bind:value={username}
                    placeholder="admin"
                    required
                />
            </div>

            <div class="form-group">
                <label for="password">Password</label>
                <input
                    type="password"
                    id="password"
                    bind:value={password}
                    placeholder="••••••••"
                    required
                />
            </div>

            <button type="submit" class="btn-login" disabled={loading}>
                {loading ? 'Signing in...' : 'Sign In'}
            </button>
        </form>

        <p class="hint">
            Demo credentials: admin / Vertex@2026!
        </p>
    </div>
</div>

<style>
    .login-page {
        min-height: 100vh;
        display: flex;
        align-items: center;
        justify-content: center;
        background: linear-gradient(135deg, #1e293b 0%, #334155 100%);
        padding: 20px;
    }

    .login-card {
        background: white;
        padding: 40px;
        border-radius: 16px;
        width: 100%;
        max-width: 420px;
        box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3);
    }

    .logo-container {
        display: flex;
        justify-content: center;
        margin-bottom: 16px;
    }

    .logo {
        height: 150px;
        width: auto;
    }

    .subtitle {
        text-align: center;
        color: #64748b;
        margin: 0 0 24px 0;
        font-size: 14px;
    }

    .error-message {
        background: #fef2f2;
        color: #dc2626;
        padding: 12px 16px;
        border-radius: 8px;
        margin-bottom: 20px;
        font-size: 14px;
        text-align: center;
    }

    .form-group {
        margin-bottom: 20px;
    }

    .form-group label {
        display: block;
        margin-bottom: 8px;
        font-size: 14px;
        font-weight: 500;
        color: #475569;
    }

    .form-group input {
        width: 100%;
        padding: 12px 16px;
        border: 2px solid #e2e8f0;
        border-radius: 8px;
        font-size: 16px;
        transition: border-color 0.2s;
    }

    .form-group input:focus {
        outline: none;
        border-color: #3b82f6;
    }

    .form-group input::placeholder {
        color: #94a3b8;
    }

    .btn-login {
        width: 100%;
        padding: 14px;
        background: #3b82f6;
        color: white;
        border: none;
        border-radius: 8px;
        font-size: 16px;
        font-weight: 600;
        cursor: pointer;
        transition: background 0.2s;
    }

    .btn-login:hover:not(:disabled) {
        background: #2563eb;
    }

    .btn-login:disabled {
        background: #94a3b8;
        cursor: not-allowed;
    }

    .hint {
        text-align: center;
        color: #94a3b8;
        font-size: 12px;
        margin: 20px 0 0 0;
    }
</style>