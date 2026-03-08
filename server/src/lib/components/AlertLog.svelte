<script lang="ts">
    export let alerts: {
        tagId: number;
        fullName: string;
        empId: string;
        type: 'gas' | 'panic';
        gasResistance: number;
        iaq: number;
        timestamp: string;
    }[] = [];

    function formatTime(timestamp: string): string {
        return new Date(timestamp).toLocaleTimeString('en-US', {
            hour12: false,
            hour: '2-digit',
            minute: '2-digit',
            second: '2-digit'
        });
    }

    function formatDate(timestamp: string): string {
        return new Date(timestamp).toLocaleDateString('en-US', {
            month: 'short',
            day: 'numeric'
        });
    }

    function exportCSV() {
        const header = 'Timestamp,Tag ID,Name,Employee ID,Type,Gas Resistance (kOhm),IAQ\n';
        const rows = alerts.map(a =>
            `${a.timestamp},${a.tagId},${a.fullName},${a.empId},${a.type},${a.gasResistance},${a.iaq}`
        ).join('\n');

        const blob = new Blob([header + rows], { type: 'text/csv' });
        const url = URL.createObjectURL(blob);
        const link = document.createElement('a');
        link.href = url;
        link.download = `vertex-alerts-${new Date().toISOString().slice(0, 10)}.csv`;
        link.click();
        URL.revokeObjectURL(url);
    }
</script>

<div class="alert-log">
    <div class="log-header">
        <h3>Alert History ({alerts.length})</h3>
        {#if alerts.length > 0}
            <button class="export-btn" on:click={exportCSV}>📥 Export CSV</button>
        {/if}
    </div>

    <div class="log-list">
        {#if alerts.length === 0}
            <p class="empty">No alerts recorded.</p>
        {:else}
            {#each alerts.slice(0, 50) as alert}
                <div class="log-entry {alert.type}">
                    <div class="entry-icon">
                        {alert.type === 'panic' ? '🆘' : '🔴'}
                    </div>
                    <div class="entry-info">
                        <span class="entry-name">W{alert.tagId} - {alert.fullName}</span>
                        <span class="entry-detail {alert.type}">
                            {#if alert.type === 'panic'}
                                PANIC BUTTON ACTIVATED
                            {:else}
                                IAQ: {alert.iaq} | Gas: {alert.gasResistance.toFixed(1)} kΩ
                            {/if}
                        </span>
                    </div>
                    <div class="entry-time">
                        <span class="time">{formatTime(alert.timestamp)}</span>
                        <span class="date">{formatDate(alert.timestamp)}</span>
                    </div>
                </div>
            {/each}
        {/if}
    </div>
</div>

<style>
    .alert-log {
        background: var(--bg-card, #ffffff);
        border-radius: 12px;
        padding: 20px;
        box-shadow: var(--shadow, 0 1px 3px rgba(0, 0, 0, 0.1));
    }

    .log-header {
        display: flex;
        justify-content: space-between;
        align-items: center;
        margin-bottom: 16px;
    }

    h3 {
        margin: 0;
        font-size: 16px;
        color: var(--text-primary, #1e293b);
    }

    .export-btn {
        background: #3b82f6;
        color: white;
        border: none;
        padding: 6px 14px;
        border-radius: 6px;
        font-size: 12px;
        font-weight: 600;
        cursor: pointer;
        transition: background 0.2s;
    }

    .export-btn:hover {
        background: #2563eb;
    }

    .log-list {
        max-height: 300px;
        overflow-y: auto;
        display: flex;
        flex-direction: column;
        gap: 8px;
    }

    .empty {
        color: var(--text-muted, #64748b);
        font-style: italic;
        text-align: center;
        padding: 20px;
    }

    .log-entry {
        display: flex;
        align-items: center;
        gap: 12px;
        padding: 10px 12px;
        background: rgba(239, 68, 68, 0.05);
        border-left: 3px solid #ef4444;
        border-radius: 6px;
    }

    .log-entry.panic {
        background: rgba(124, 58, 237, 0.05);
        border-left-color: #7c3aed;
    }

    .entry-icon {
        font-size: 16px;
    }

    .entry-info {
        flex: 1;
        display: flex;
        flex-direction: column;
        gap: 2px;
    }

    .entry-name {
        font-weight: 600;
        font-size: 13px;
        color: var(--text-primary, #1e293b);
    }

    .entry-detail {
        font-size: 12px;
        color: #ef4444;
        font-weight: 500;
    }

    .entry-detail.panic {
        color: #7c3aed;
    }

    .entry-time {
        display: flex;
        flex-direction: column;
        align-items: flex-end;
        gap: 2px;
    }

    .time {
        font-size: 12px;
        font-weight: 600;
        color: var(--text-secondary, #475569);
    }

    .date {
        font-size: 11px;
        color: var(--text-muted, #64748b);
    }
</style>