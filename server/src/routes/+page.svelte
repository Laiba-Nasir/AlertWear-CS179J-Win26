<script lang="ts">
    import { onMount } from 'svelte';
    import { invalidateAll } from '$app/navigation';
    import Map from '$lib/components/Map.svelte';
    import WorkerCard from '$lib/components/WorkerCard.svelte';
    import AlertLog from '$lib/components/AlertLog.svelte';

    interface Worker {
        tagId: number;
        fullName: string;
        empId: string;
        x: number;
        y: number;
        gasResistance: number;
        iaq: number;
        iaqLabel: string;
        temperature: number | null;
        humidity: number | null;
        pressure: number | null;
        panic: boolean;
        status: 'ok' | 'warning' | 'alert' | 'panic';
    }

    interface Anchor {
        anchorId: string;
        name: string;
        x: number;
        y: number;
        online: boolean;
    }

    export let data: {
        workers: Worker[];
        anchors: Anchor[];
        alerts: any[];
    };

    onMount(() => {
        const interval = setInterval(() => {
            invalidateAll();
        }, 1000);

        return () => clearInterval(interval);
    });

    $: hasPanic = data.workers.some((w) => w.status === 'panic');
    $: hasAlert = data.workers.some((w) => w.status === 'alert');
    $: hasWarning = data.workers.some((w) => w.status === 'warning');
    $: panicWorkers = data.workers.filter((w) => w.status === 'panic');
    $: onlineAnchors = data.anchors.filter((a) => a.online).length;

    // Alert sounds
    let alertAudio: HTMLAudioElement;
    let alertPlaying = false;
    let muted = false;

    function playWarningBeep() {
        if (muted) return;
        try {
            const audioCtx = new (window.AudioContext || (window as any).webkitAudioContext)();
            const oscillator = audioCtx.createOscillator();
            const gainNode = audioCtx.createGain();

            oscillator.connect(gainNode);
            gainNode.connect(audioCtx.destination);

            oscillator.type = 'sine';
            oscillator.frequency.value = 880;

            gainNode.gain.setValueAtTime(0, audioCtx.currentTime);
            gainNode.gain.linearRampToValueAtTime(0.3, audioCtx.currentTime + 0.05);
            gainNode.gain.linearRampToValueAtTime(0, audioCtx.currentTime + 0.3);

            oscillator.start(audioCtx.currentTime);
            oscillator.stop(audioCtx.currentTime + 0.3);
        } catch (e) {
            console.error('Warning beep failed:', e);
        }
    }

    onMount(() => {
        alertAudio = new Audio('https://actions.google.com/sounds/v1/alarms/alarm_clock.ogg');
        alertAudio.loop = true;
    });

    $: if ((hasPanic || hasAlert) && !alertPlaying && alertAudio && !muted) {
        alertAudio.play();
        alertPlaying = true;
    } else if (!hasPanic && !hasAlert && alertPlaying && alertAudio) {
        alertAudio.pause();
        alertAudio.currentTime = 0;
        alertPlaying = false;
    }

    let warningInterval: ReturnType<typeof setInterval> | null = null;

    $: if (hasWarning && !hasAlert && !hasPanic && !muted) {
        if (!warningInterval) {
            playWarningBeep();
            warningInterval = setInterval(playWarningBeep, 10000);
        }
    } else {
        if (warningInterval) {
            clearInterval(warningInterval);
            warningInterval = null;
        }
    }

    function muteAll() {
        muted = true;
        if (alertAudio) {
            alertAudio.pause();
            alertAudio.currentTime = 0;
        }
        alertPlaying = false;
        if (warningInterval) {
            clearInterval(warningInterval);
            warningInterval = null;
        }
    }

    function unmuteAll() {
        muted = false;
    }
</script>

<svelte:head>
    <title>Dashboard - VERTEX</title>
</svelte:head>

{#if hasPanic}
    <div class="alert-banner panic">
        <span>🆘 EMERGENCY — {panicWorkers.map(w => w.fullName).join(', ')} activated panic button!</span>
        {#if !muted}
            <button class="mute-btn" on:click={muteAll}>🔇 Mute</button>
        {:else}
            <button class="mute-btn unmute" on:click={unmuteAll}>🔊 Unmute</button>
        {/if}
    </div>
{:else if hasAlert}
    <div class="alert-banner critical">
        <span>🚨 HAZARD ALERT — IAQ > 200 — Evacuate or investigate immediately!</span>
        {#if !muted}
            <button class="mute-btn" on:click={muteAll}>🔇 Mute</button>
        {:else}
            <button class="mute-btn unmute" on:click={unmuteAll}>🔊 Unmute</button>
        {/if}
    </div>
{:else if hasWarning}
    <div class="alert-banner warning">
        <span>⚠️ AIR QUALITY WARNING — IAQ > 150 — Monitor conditions</span>
        {#if !muted}
            <button class="mute-btn" on:click={muteAll}>🔇 Mute</button>
        {:else}
            <button class="mute-btn unmute" on:click={unmuteAll}>🔊 Unmute</button>
        {/if}
    </div>
{/if}

<div class="dashboard">
    <div class="status-bar">
        <span class="dot green"></span>
        Live | {data.workers.length} Workers | {onlineAnchors}/{data.anchors.length} Anchors Online
    </div>

    <div class="content">
        <div class="map-section">
            <h2>Facility Map</h2>
            <Map workers={data.workers} anchors={data.anchors} />
        </div>

        <div class="workers-section">
            <h2>Workers ({data.workers.length})</h2>
            <div class="workers-list">
                {#if data.workers.length === 0}
                    <p class="no-data">Waiting for worker data...</p>
                {:else}
                    {#each data.workers as worker (worker.tagId)}
                        <WorkerCard {worker} />
                    {/each}
                {/if}
            </div>
        </div>
    </div>

    <div class="alert-section">
        <AlertLog alerts={data.alerts} />
    </div>
</div>

<style>
    .alert-banner {
        color: white;
        padding: 12px;
        font-weight: bold;
        display: flex;
        justify-content: center;
        align-items: center;
        gap: 16px;
    }

    .alert-banner.panic {
        background: #7c3aed;
        animation: flash-panic 0.5s infinite;
    }

    .alert-banner.critical {
        background: #ef4444;
        animation: flash-critical 1s infinite;
    }

    .alert-banner.warning {
        background: #f59e0b;
        animation: flash-warning 2s infinite;
    }

    .mute-btn {
        background: rgba(255, 255, 255, 0.2);
        color: white;
        border: 1px solid rgba(255, 255, 255, 0.4);
        padding: 6px 14px;
        border-radius: 6px;
        cursor: pointer;
        font-size: 13px;
        font-weight: 600;
    }

    .mute-btn:hover {
        background: rgba(255, 255, 255, 0.3);
    }

    .mute-btn.unmute {
        background: rgba(255, 255, 255, 0.35);
    }

    @keyframes flash-panic {
        0%, 100% { opacity: 1; }
        50% { opacity: 0.5; }
    }

    @keyframes flash-critical {
        0%, 100% { opacity: 1; }
        50% { opacity: 0.7; }
    }

    @keyframes flash-warning {
        0%, 100% { opacity: 1; }
        50% { opacity: 0.85; }
    }

    .dashboard {
        min-height: calc(100vh - 60px);
        background: var(--bg-primary);
    }

    .status-bar {
        display: flex;
        align-items: center;
        gap: 8px;
        padding: 12px 24px;
        background: var(--bg-status);
        font-size: 14px;
        color: var(--text-secondary);
    }

    .dot {
        width: 10px;
        height: 10px;
        border-radius: 50%;
    }

    .dot.green {
        background: #22c55e;
        animation: pulse-dot 2s infinite;
    }

    @keyframes pulse-dot {
        0%, 100% { opacity: 1; box-shadow: 0 0 0 0 rgba(34, 197, 94, 0.4); }
        50% { opacity: 0.8; box-shadow: 0 0 0 6px rgba(34, 197, 94, 0); }
    }

    .content {
        display: flex;
        gap: 24px;
        padding: 24px;
        max-width: 1400px;
        margin: 0 auto;
    }

    .map-section {
        flex: 1;
        background: var(--bg-card);
        border-radius: 12px;
        padding: 20px;
        box-shadow: var(--shadow);
    }

    .workers-section {
        width: 320px;
        background: var(--bg-card);
        border-radius: 12px;
        padding: 20px;
        box-shadow: var(--shadow);
        max-height: calc(100vh - 180px);
        overflow-y: auto;
    }

    h2 {
        margin: 0 0 16px 0;
        font-size: 18px;
        color: var(--text-primary);
    }

    .workers-list {
        display: flex;
        flex-direction: column;
        gap: 12px;
    }

    .no-data {
        color: var(--text-muted);
        font-style: italic;
        text-align: center;
        padding: 20px;
    }

    .alert-section {
        padding: 0 24px 24px;
        max-width: 1400px;
        margin: 0 auto;
    }
</style>