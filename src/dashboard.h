#pragma once

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>VitalGuard | Patient Monitor</title>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@300;400;600;700&family=JetBrains+Mono:wght@500&display=swap" rel="stylesheet">
    <style>
        :root {
            --bg-color: #050510;
            --card-bg: rgba(20, 20, 45, 0.7);
            --accent-primary: #00f2ff;
            --accent-success: #00ff88;
            --accent-warning: #ffaa00;
            --accent-danger: #ff4444;
            --text-main: #e0e0f0;
            --text-muted: #8888aa;
            --glass-border: rgba(255, 255, 255, 0.1);
            --glow-color: rgba(0, 242, 255, 0.3);
        }

        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Inter', sans-serif;
            background-color: var(--bg-color);
            background-image: 
                radial-gradient(circle at 10% 20%, rgba(0, 242, 255, 0.05) 0%, transparent 40%),
                radial-gradient(circle at 90% 80%, rgba(0, 255, 136, 0.05) 0%, transparent 40%);
            color: var(--text-main);
            min-height: 100vh;
            padding: 20px;
            overflow-x: hidden;
        }

        header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 30px;
            padding: 0 10px;
        }

        .logo {
            display: flex;
            align-items: center;
            gap: 12px;
            font-weight: 700;
            font-size: 1.5rem;
            letter-spacing: -0.5px;
            color: var(--accent-primary);
        }

        .logo-icon {
            width: 32px;
            height: 32px;
            background: var(--accent-primary);
            border-radius: 8px;
            display: flex;
            align-items: center;
            justify-content: center;
            color: var(--bg-color);
        }

        .system-status {
            display: flex;
            align-items: center;
            gap: 8px;
            font-size: 0.85rem;
            color: var(--text-muted);
            background: var(--card-bg);
            padding: 8px 16px;
            border-radius: 20px;
            border: 1px solid var(--glass-border);
        }

        .status-dot {
            width: 8px;
            height: 8px;
            background: var(--accent-success);
            border-radius: 50%;
            box-shadow: 0 0 10px var(--accent-success);
            animation: pulse 2s infinite;
        }

        @keyframes pulse {
            0% { opacity: 1; transform: scale(1); }
            50% { opacity: 0.4; transform: scale(1.2); }
            100% { opacity: 1; transform: scale(1); }
        }

        .main-grid {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 20px;
            max-width: 1200px;
            margin: 0 auto;
        }

        .card {
            background: var(--card-bg);
            backdrop-filter: blur(12px);
            border: 1px solid var(--glass-border);
            border-radius: 24px;
            padding: 24px;
            transition: transform 0.3s ease, box-shadow 0.3s ease;
            position: relative;
            overflow: hidden;
        }

        .card:hover {
            transform: translateY(-5px);
            box-shadow: 0 10px 30px rgba(0,0,0,0.5);
            border-color: rgba(255, 255, 255, 0.2);
        }

        .card-header {
            display: flex;
            justify-content: space-between;
            align-items: flex-start;
            margin-bottom: 15px;
        }

        .card-title {
            color: var(--text-muted);
            font-size: 0.75rem;
            font-weight: 600;
            text-transform: uppercase;
            letter-spacing: 1px;
        }

        .card-icon {
            font-size: 1.2rem;
            opacity: 0.8;
        }

        .card-value {
            font-family: 'JetBrains Mono', monospace;
            font-size: 3.5rem;
            font-weight: 700;
            line-height: 1;
            margin-bottom: 5px;
        }

        .card-unit {
            font-size: 1rem;
            color: var(--text-muted);
            margin-left: 4px;
        }

        .card-footer {
            font-size: 0.85rem;
            margin-top: 15px;
            display: flex;
            align-items: center;
            gap: 6px;
        }

        /* Specific Card Styles */
        .card.ecg { grid-column: span 3; }
        .card.hr { border-left: 4px solid var(--accent-danger); }
        .card.spo2 { border-left: 4px solid var(--accent-primary); }
        .card.fluid { border-left: 4px solid var(--accent-success); }

        .card.fluid.critical {
            border-left-color: var(--accent-danger);
            animation: alert-border 1s infinite alternate;
        }

        @keyframes alert-border {
            from { border-left-width: 4px; }
            to { border-left-width: 12px; }
        }

        .chart-container {
            width: 100%;
            height: 250px;
            background: rgba(0,0,0,0.3);
            border-radius: 16px;
            margin-top: 10px;
            position: relative;
        }

        canvas {
            width: 100%;
            height: 100%;
        }

        .ecg-grid {
            position: absolute;
            top: 0; left: 0; width: 100%; height: 100%;
            background-image: 
                linear-gradient(rgba(255,255,255,0.05) 1px, transparent 1px),
                linear-gradient(90deg, rgba(255,255,255,0.05) 1px, transparent 1px);
            background-size: 20px 20px;
            pointer-events: none;
        }

        /* Fluid Level Indicator */
        .fluid-bar-container {
            width: 100%;
            height: 8px;
            background: rgba(255,255,255,0.1);
            border-radius: 4px;
            margin-top: 15px;
        }

        .fluid-bar {
            height: 100%;
            background: var(--accent-success);
            border-radius: 4px;
            width: 100%;
            transition: width 0.5s ease, background 0.3s ease;
            box-shadow: 0 0 15px var(--accent-success);
        }

        .fluid-bar.low {
            background: var(--accent-danger);
            box-shadow: 0 0 15px var(--accent-danger);
            width: 15%;
        }

        /* Overlay Alert */
        #alert-overlay {
            position: fixed;
            top: 0; left: 0; width: 100%; height: 100%;
            background: rgba(255, 68, 68, 0.15);
            display: none;
            pointer-events: none;
            z-index: 1000;
            animation: flash 0.5s infinite alternate;
        }

        @keyframes flash {
            from { opacity: 0; }
            to { opacity: 1; }
        }

        @media (max-width: 900px) {
            .main-grid { grid-template-columns: 1fr 1fr; }
            .card.ecg { grid-column: span 2; }
        }

        @media (max-width: 600px) {
            .main-grid { grid-template-columns: 1fr; }
            .card.ecg { grid-column: span 1; }
        }
    </style>
</head>
<body>
    <div id="alert-overlay"></div>

    <header>
        <div class="logo">
            <div class="logo-icon">✚</div>
            VitalGuard
        </div>
        <div class="system-status">
            <div class="status-dot" id="ws-status-dot"></div>
            <span id="ws-status-text">Connecting...</span>
        </div>
    </header>

    <div class="main-grid">
        <!-- ECG Waveform -->
        <div class="card ecg">
            <div class="card-header">
                <div class="card-title">Live ECG Signal</div>
                <div class="card-icon" style="color: var(--accent-success)">⚡</div>
            </div>
            <div class="chart-container">
                <div class="ecg-grid"></div>
                <canvas id="ecgCanvas"></canvas>
            </div>
            <div class="card-footer" style="color: var(--text-muted)">
                Sampling rate: 250Hz | Lead II
            </div>
        </div>

        <!-- Heart Rate -->
        <div class="card hr">
            <div class="card-header">
                <div class="card-title">Heart Rate</div>
                <div class="card-icon">❤️</div>
            </div>
            <div>
                <span class="card-value" id="hr-val">--</span>
                <span class="card-unit">BPM</span>
            </div>
            <div class="card-footer">
                <span style="color: var(--accent-success)">●</span> Rhythm: Sinus
            </div>
        </div>

        <!-- SpO2 -->
        <div class="card spo2">
            <div class="card-header">
                <div class="card-title">Oxygen Saturation</div>
                <div class="card-icon">💧</div>
            </div>
            <div>
                <span class="card-value" id="spo2-val">--</span>
                <span class="card-unit">%</span>
            </div>
            <div class="card-footer">
                <span style="color: var(--accent-success)">●</span> Perfusion: High
            </div>
        </div>

        <!-- Fluid Level -->
        <div class="card fluid" id="fluid-card">
            <div class="card-header">
                <div class="card-title">Infusion Status</div>
                <div class="card-icon">🧪</div>
            </div>
            <div>
                <span class="card-value" id="fluid-text" style="font-size: 2rem;">OK</span>
            </div>
            <div class="fluid-bar-container">
                <div class="fluid-bar" id="fluid-bar"></div>
            </div>
            <div class="card-footer" id="fluid-footer">
                Level is sufficient
            </div>
        </div>
    </div>

    <script>
        const canvas = document.getElementById('ecgCanvas');
        const ctx = canvas.getContext('2d');
        let width, height;

        function resize() {
            width = canvas.parentElement.clientWidth;
            height = canvas.parentElement.clientHeight;
            canvas.width = width;
            canvas.height = height;
        }
        window.addEventListener('resize', resize);
        resize();

        // ECG Plotting variables
        const ecgData = new Float32Array(300);
        ecgData.fill(height / 2);
        let xPos = 0;

        function drawECG() {
            ctx.clearRect(0, 0, width, height);
            
            ctx.beginPath();
            ctx.strokeStyle = '#00ff88';
            ctx.lineWidth = 2;
            ctx.lineJoin = 'round';
            ctx.lineCap = 'round';

            const step = width / ecgData.length;
            
            for(let i = 0; i < ecgData.length; i++) {
                const x = i * step;
                const y = ecgData[i];
                if(i === 0) ctx.moveTo(x, y);
                else ctx.lineTo(x, y);
            }
            ctx.stroke();

            // Glow effect
            ctx.shadowBlur = 10;
            ctx.shadowColor = '#00ff88';
            ctx.stroke();
            ctx.shadowBlur = 0;
        }

        function updateECG(newValue) {
            // Map raw ADC (0-4095) to canvas height
            const mapped = height - (newValue / 4095) * height;
            for(let i = 0; i < ecgData.length - 1; i++) {
                ecgData[i] = ecgData[i + 1];
            }

            ecgData[ecgData.length - 1] = mapped;
            drawECG();
        }

        // WebSocket Integration
        let socket;
        function connectWS() {
            const proto = location.protocol === 'https:' ? 'wss:' : 'ws:';
            socket = new WebSocket(`${proto}//${location.host}/ws`);

            socket.onopen = () => {
                document.getElementById('ws-status-dot').style.background = 'var(--accent-success)';
                document.getElementById('ws-status-text').textContent = 'Live Connection';
            };

            socket.onclose = () => {
                document.getElementById('ws-status-dot').style.background = 'var(--accent-danger)';
                document.getElementById('ws-status-text').textContent = 'Disconnected';
                setTimeout(connectWS, 2000);
            };

            socket.onmessage = (event) => {
                try {
                    const data = JSON.parse(event.data);
                    
                    if (data.ecg !== undefined) updateECG(data.ecg);
                    if (data.hr !== undefined) document.getElementById('hr-val').textContent = data.hr;
                    if (data.spo2 !== undefined) document.getElementById('spo2-val').textContent = data.spo2;
                    
                    if (data.fluid !== undefined) {
                        const fluidCard = document.getElementById('fluid-card');
                        const fluidBar = document.getElementById('fluid-bar');
                        const fluidText = document.getElementById('fluid-text');
                        const fluidFooter = document.getElementById('fluid-footer');
                        const overlay = document.getElementById('alert-overlay');

                        if (data.fluid) {
                            fluidCard.classList.remove('critical');
                            fluidBar.classList.remove('low');
                            fluidText.textContent = "OK";
                            fluidFooter.textContent = "Level is sufficient";
                            overlay.style.display = 'none';
                        } else {
                            fluidCard.classList.add('critical');
                            fluidBar.classList.add('low');
                            fluidText.textContent = "LOW";
                            fluidFooter.textContent = "REFILL IMMEDIATELY!";
                            overlay.style.display = 'block';
                        }
                    }
                } catch (e) {
                    console.error("Data error", e);
                }
            };
        }

        // Start connection
        connectWS();

        // For demo/dev purposes - can be removed when actual HW is used
        // setInterval(() => {
        //     updateECG(2000 + Math.random() * 500);
        // }, 40);

    </script>
</body>
</html>
)rawliteral";