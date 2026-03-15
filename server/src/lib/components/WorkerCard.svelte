<script lang="ts">
    export let worker: {
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
    };

    const statusLabels: Record<string, string> = {
        ok: 'OK',
        warning: 'Warning',
        alert: 'GAS ALERT',
        panic: 'EMERGENCY'
    };
</script>

<div class="card {worker.status}">
    <div class="header">
        <div class="avatar {worker.status}">W{worker.tagId}</div>
        <div class="info">
            <div class="name">{worker.fullName}</div>
            <div class="emp-id">{worker.empId}</div>
        </div>
    </div>

    {#if worker.panic && worker.status === 'panic'}
        <div class="panic-banner">🆘 PANIC BUTTON ACTIVATED</div>
    {/if}

    <div class="details">
        <div class="detail">
            <span class="label">Position</span>
            <span class="value">({worker.x}, {worker.y})</span>
        </div>
        <div class="detail">
            <span class="label">Air Quality (IAQ)</span>
            <span class="value gas-{worker.status}">{worker.iaq > 0 ? worker.iaq : 'N/A'} {worker.iaq > 0 ? `- ${worker.iaqLabel}` : ''}</span>
        </div>
        <div class="detail">
            <span class="label">Gas Resistance</span>
            <span class="value">{worker.gasResistance > 0 ? worker.gasResistance.toFixed(1) + ' kΩ' : 'N/A'}</span>
        </div>
        {#if worker.temperature !== null}
            <div class="detail">
                <span class="label">Temperature</span>
                <span class="value">{worker.temperature.toFixed(1)} °C</span>
            </div>
        {/if}
        {#if worker.humidity !== null}
            <div class="detail">
                <span class="label">Humidity</span>
                <span class="value">{worker.humidity.toFixed(1)} %</span>
            </div>
        {/if}
        {#if worker.pressure !== null}
            <div class="detail">
                <span class="label">Pressure</span>
                <span class="value">{worker.pressure.toFixed(1)} hPa</span>
            </div>
        {/if}
        <div class="detail">
            <span class="label">Status</span>
            <span class="status-badge {worker.status}">{statusLabels[worker.status]}</span>
        </div>
    </div>
</div>

<style>
    .card {
        background: var(--bg-card);
        border-radius: 8px;
        padding: 16px;
        border-left: 4px solid #22c55e;
    }

    .card.warning {
        border-left-color: #f59e0b;
        background: #fffbeb;
    }

    .card.alert {
        border-left-color: #ef4444;
        background: #fef2f2;
        animation: pulse 1s infinite;
    }

    .card.panic {
        border-left-color: #7c3aed;
        background: #f5f3ff;
        animation: pulse-panic 0.5s infinite;
    }

    @keyframes pulse {
        0%, 100% { opacity: 1; }
        50% { opacity: 0.8; }
    }

    @keyframes pulse-panic {
        0%, 100% { opacity: 1; }
        50% { opacity: 0.6; }
    }

    .header {
        display: flex;
        align-items: center;
        gap: 12px;
        margin-bottom: 12px;
    }

    .avatar {
        width: 40px;
        height: 40px;
        border-radius: 50%;
        display: flex;
        align-items: center;
        justify-content: center;
        font-weight: bold;
        font-size: 14px;
        color: white;
        background: #22c55e;
    }

    .avatar.warning { background: #f59e0b; }
    .avatar.alert { background: #ef4444; }
    .avatar.panic { background: #7c3aed; }

    .info { flex: 1; }

    .name {
        font-weight: 600;
        color: var(--text-primary);
    }

    .emp-id {
        font-size: 12px;
        color: var(--text-muted);
    }

    .panic-banner {
        background: #7c3aed;
        color: white;
        padding: 8px;
        border-radius: 6px;
        text-align: center;
        font-weight: bold;
        font-size: 13px;
        margin-bottom: 12px;
        animation: pulse-panic 0.5s infinite;
    }

    .details {
        display: flex;
        flex-direction: column;
        gap: 8px;
    }

    .detail {
        display: flex;
        justify-content: space-between;
        font-size: 13px;
    }

    .label { color: var(--text-muted); }

    .value {
        color: var(--text-primary);
        font-weight: 500;
    }

    .gas-warning { color: #f59e0b; }
    .gas-alert { color: #ef4444; font-weight: bold; }
    .gas-panic { color: #7c3aed; font-weight: bold; }

    .status-badge {
        padding: 2px 8px;
        border-radius: 12px;
        font-size: 11px;
        font-weight: 600;
    }

    .status-badge.ok { background: #dcfce7; color: #16a34a; }
    .status-badge.warning { background: #fef3c7; color: #d97706; }
    .status-badge.alert { background: #fee2e2; color: #dc2626; }
    .status-badge.panic { background: #ede9fe; color: #7c3aed; }
</style>