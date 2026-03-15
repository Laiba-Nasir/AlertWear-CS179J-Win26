<script lang="ts">
    import { onMount, afterUpdate } from 'svelte';

    export let workers: Array<{
        tagId: number;
        fullName: string;
        x: number;
        y: number;
        status: 'ok' | 'warning' | 'alert';
    }> = [];

    export let anchors: Array<{
        anchorId: string;
        name: string;
        x: number;
        y: number;
        online: boolean;
    }> = [];

    let canvas: HTMLCanvasElement;
    let ctx: CanvasRenderingContext2D;

    const CANVAS_SIZE = 500;
    const PADDING = 40;
    const MAX_TRAIL = 20;

    const statusColors = {
        ok: '#22c55e',
        warning: '#f59e0b',
        alert: '#ef4444'
    };

    // Auto-calculate grid size from anchor positions with some padding
    function getGridSize(): { width: number; height: number; step: number } {
        if (anchors.length === 0) return { width: 10, height: 10, step: 2 };

        const maxX = Math.max(...anchors.map(a => a.x));
        const maxY = Math.max(...anchors.map(a => a.y));

        // Add 10% padding beyond the furthest anchor
        const width = Math.ceil(Math.max(maxX, 1) * 1.1);
        const height = Math.ceil(Math.max(maxY, 1) * 1.1);

        // Calculate nice grid step (1m for small areas, 2m for medium, 5m for large)
        const maxDim = Math.max(width, height);
        let step = 1;
        if (maxDim > 15) step = 5;
        else if (maxDim > 8) step = 2;

        return { width, height, step };
    }

    // Trail history: tagId -> array of {x, y, status}
    const trails: Map<number, Array<{ x: number; y: number; status: string }>> = new Map();

    function updateTrails() {
        workers.forEach((worker) => {
            if (!trails.has(worker.tagId)) {
                trails.set(worker.tagId, []);
            }
            const trail = trails.get(worker.tagId)!;
            const last = trail[trail.length - 1];

            if (!last || last.x !== worker.x || last.y !== worker.y) {
                trail.push({ x: worker.x, y: worker.y, status: worker.status });
                if (trail.length > MAX_TRAIL) trail.shift();
            }
        });
    }

    function draw() {
        if (!ctx) return;

        const { width: gridW, height: gridH, step } = getGridSize();
        const scaleX = CANVAS_SIZE / gridW;
        const scaleY = CANVAS_SIZE / gridH;

        const totalW = CANVAS_SIZE + PADDING * 2;
        const totalH = CANVAS_SIZE + PADDING * 2;

        // Update canvas size if needed
        canvas.width = totalW;
        canvas.height = totalH;

        // Clear canvas
        ctx.fillStyle = '#1e293b';
        ctx.fillRect(0, 0, totalW, totalH);

        // Draw grid lines
        ctx.strokeStyle = '#334155';
        ctx.lineWidth = 1;

        // Vertical grid lines
        for (let i = 0; i <= gridW; i += step) {
            const x = i * scaleX + PADDING;
            ctx.beginPath();
            ctx.moveTo(x, PADDING);
            ctx.lineTo(x, CANVAS_SIZE + PADDING);
            ctx.stroke();
        }

        // Horizontal grid lines
        for (let i = 0; i <= gridH; i += step) {
            const y = CANVAS_SIZE + PADDING - i * scaleY;
            ctx.beginPath();
            ctx.moveTo(PADDING, y);
            ctx.lineTo(CANVAS_SIZE + PADDING, y);
            ctx.stroke();
        }

        // Draw axis labels
        ctx.fillStyle = '#64748b';
        ctx.font = '12px system-ui';

        // X-axis labels
        ctx.textAlign = 'center';
        for (let i = 0; i <= gridW; i += step) {
            ctx.fillText(`${i}m`, i * scaleX + PADDING, CANVAS_SIZE + PADDING + 20);
        }

        // Y-axis labels
        ctx.textAlign = 'right';
        for (let i = 0; i <= gridH; i += step) {
            ctx.fillText(`${i}m`, PADDING - 8, CANVAS_SIZE + PADDING - i * scaleY + 4);
        }

        // Draw anchors
        anchors.forEach((anchor) => {
            const x = anchor.x * scaleX + PADDING;
            const y = CANVAS_SIZE + PADDING - anchor.y * scaleY;

            ctx.fillStyle = anchor.online ? '#3b82f6' : '#64748b';
            ctx.fillRect(x - 12, y - 12, 24, 24);

            ctx.fillStyle = '#ffffff';
            ctx.font = 'bold 10px system-ui';
            ctx.textAlign = 'center';
            ctx.fillText(anchor.anchorId, x, y + 4);
        });

        // Draw worker trails (fading dots)
        trails.forEach((trail, tagId) => {
            const worker = workers.find(w => w.tagId === tagId);
            if (!worker || trail.length < 2) return;

            const color = statusColors[worker.status] || statusColors.ok;

            trail.forEach((point, i) => {
                if (i === trail.length - 1) return;

                const x = point.x * scaleX + PADDING;
                const y = CANVAS_SIZE + PADDING - point.y * scaleY;
                const opacity = (i + 1) / trail.length * 0.5;
                const radius = 4 + (i / trail.length) * 3;

                ctx.beginPath();
                ctx.arc(x, y, radius, 0, Math.PI * 2);
                ctx.fillStyle = color + Math.round(opacity * 255).toString(16).padStart(2, '0');
                ctx.fill();
            });
        });

        // Draw workers (colored circles)
        workers.forEach((worker) => {
            const x = worker.x * scaleX + PADDING;
            const y = CANVAS_SIZE + PADDING - worker.y * scaleY;

            ctx.beginPath();
            ctx.arc(x, y, 18, 0, Math.PI * 2);
            ctx.fillStyle = statusColors[worker.status];
            ctx.fill();

            ctx.beginPath();
            ctx.arc(x, y, 14, 0, Math.PI * 2);
            ctx.fillStyle = '#1e293b';
            ctx.fill();

            ctx.fillStyle = '#ffffff';
            ctx.font = 'bold 10px system-ui';
            ctx.textAlign = 'center';
            ctx.fillText(`W${worker.tagId}`, x, y + 4);
        });
    }

    onMount(() => {
        ctx = canvas.getContext('2d')!;
        draw();
    });

    afterUpdate(() => {
        if (ctx) {
            updateTrails();
            draw();
        }
    });
</script>

<div class="map-container">
    <canvas
        bind:this={canvas}
        width={CANVAS_SIZE + PADDING * 2}
        height={CANVAS_SIZE + PADDING * 2}
    ></canvas>
    <div class="legend">
        <div class="legend-item">
            <div class="anchor-icon"></div>
            <span>Anchor</span>
        </div>
        <div class="legend-item">
            <div class="worker-icon ok"></div>
            <span>OK</span>
        </div>
        <div class="legend-item">
            <div class="worker-icon warning"></div>
            <span>Warning</span>
        </div>
        <div class="legend-item">
            <div class="worker-icon alert"></div>
            <span>Alert</span>
        </div>
        <div class="legend-item">
            <div class="trail-icon"></div>
            <span>Trail</span>
        </div>
    </div>
</div>

<style>
    .map-container {
        display: flex;
        flex-direction: column;
        gap: 16px;
    }

    canvas {
        border-radius: 8px;
    }

    .legend {
        display: flex;
        gap: 24px;
        justify-content: center;
    }

    .legend-item {
        display: flex;
        align-items: center;
        gap: 8px;
        font-size: 14px;
        color: #94a3b8;
    }

    .anchor-icon {
        width: 16px;
        height: 16px;
        background: #3b82f6;
        border-radius: 2px;
    }

    .worker-icon {
        width: 16px;
        height: 16px;
        border-radius: 50%;
    }

    .worker-icon.ok {
        background: #22c55e;
    }

    .worker-icon.warning {
        background: #f59e0b;
    }

    .worker-icon.alert {
        background: #ef4444;
    }

    .trail-icon {
        width: 16px;
        height: 16px;
        border-radius: 50%;
        background: radial-gradient(circle, rgba(34, 197, 94, 0.5) 0%, rgba(34, 197, 94, 0.1) 100%);
    }
</style>