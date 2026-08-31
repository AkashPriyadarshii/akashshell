document.addEventListener('DOMContentLoaded', () => {
    // 1. Intersection Observer for Scroll Reveals
    const observer = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                entry.target.classList.add('is-visible');
            }
        });
    }, {
        threshold: 0.08,
        rootMargin: '0px 0px -40px 0px'
    });

    document.querySelectorAll('.reveal').forEach((el) => {
        observer.observe(el);
    });

    // 2. Interactive Split-View Terminal Tab Switcher
    const tabData = {
        processes: {
            cmd: 'sys.processes',
            args: '| sort-by cpu desc',
            flags: '| take 3',
            table: `
                <table class="table-output">
                    <thead>
                        <tr>
                            <th>PID</th>
                            <th>NAME</th>
                            <th style="text-align: right;">CPU %</th>
                            <th style="text-align: right;">MEM</th>
                        </tr>
                    </thead>
                    <tbody>
                        <tr>
                            <td>4102</td>
                            <td style="color: var(--ink);">rustc.exe</td>
                            <td class="col-num" style="color: var(--accent);">34.2%</td>
                            <td class="col-num">840 MB</td>
                        </tr>
                        <tr>
                            <td>1894</td>
                            <td style="color: var(--ink);">cargo.exe</td>
                            <td class="col-num" style="color: var(--accent);">18.7%</td>
                            <td class="col-num">120 MB</td>
                        </tr>
                        <tr>
                            <td>8420</td>
                            <td style="color: var(--ink);">akashshell.exe</td>
                            <td class="col-num" style="color: var(--accent);">0.1%</td>
                            <td class="col-num">12 MB</td>
                        </tr>
                    </tbody>
                </table>`
        },
        filesystem: {
            cmd: 'fs.scan',
            args: './crates --depth 2',
            flags: '| where size > 10kb | take 3',
            table: `
                <table class="table-output">
                    <thead>
                        <tr>
                            <th>PATH</th>
                            <th>TYPE</th>
                            <th style="text-align: right;">SIZE</th>
                            <th style="text-align: right;">MODIFIED</th>
                        </tr>
                    </thead>
                    <tbody>
                        <tr>
                            <td style="color: var(--ink);">akashshell-core/lib.rs</td>
                            <td><span class="col-bool-true">Rust</span></td>
                            <td class="col-num" style="color: var(--accent);">14.2 KB</td>
                            <td class="col-num">2m ago</td>
                        </tr>
                        <tr>
                            <td style="color: var(--ink);">akashshell-pty/lib.rs</td>
                            <td><span class="col-bool-true">Rust</span></td>
                            <td class="col-num" style="color: var(--accent);">11.8 KB</td>
                            <td class="col-num">12m ago</td>
                        </tr>
                        <tr>
                            <td style="color: var(--ink);">akashshell-tui/lib.rs</td>
                            <td><span class="col-bool-true">Rust</span></td>
                            <td class="col-num" style="color: var(--accent);">10.4 KB</td>
                            <td class="col-num">1h ago</td>
                        </tr>
                    </tbody>
                </table>`
        },
        json: {
            cmd: 'http.get',
            args: 'https://api.internal/metrics',
            flags: '| select status latency ok',
            table: `
                <table class="table-output">
                    <thead>
                        <tr>
                            <th>ENDPOINT</th>
                            <th>STATUS</th>
                            <th style="text-align: right;">LATENCY</th>
                            <th style="text-align: right;">HEALTH</th>
                        </tr>
                    </thead>
                    <tbody>
                        <tr>
                            <td style="color: var(--ink);">/v1/evaluator</td>
                            <td>200 OK</td>
                            <td class="col-num" style="color: var(--accent);">0.18 ms</td>
                            <td class="col-num col-bool-true">true</td>
                        </tr>
                        <tr>
                            <td style="color: var(--ink);">/v1/conpty</td>
                            <td>200 OK</td>
                            <td class="col-num" style="color: var(--accent);">0.22 ms</td>
                            <td class="col-num col-bool-true">true</td>
                        </tr>
                        <tr>
                            <td style="color: var(--ink);">/v1/tui-render</td>
                            <td>200 OK</td>
                            <td class="col-num" style="color: var(--accent);">0.04 ms</td>
                            <td class="col-num col-bool-true">true</td>
                        </tr>
                    </tbody>
                </table>`
        }
    };

    const tabButtons = document.querySelectorAll('.tab-btn');
    const cmdEl = document.getElementById('cli-cmd');
    const argsEl = document.getElementById('cli-args');
    const flagsEl = document.getElementById('cli-flags');
    const tableContainer = document.getElementById('table-container');

    tabButtons.forEach(btn => {
        btn.addEventListener('click', () => {
            tabButtons.forEach(b => b.classList.remove('active'));
            btn.classList.add('active');

            const tabKey = btn.getAttribute('data-tab');
            const data = tabData[tabKey];
            if (data && cmdEl && argsEl && flagsEl && tableContainer) {
                cmdEl.textContent = data.cmd;
                argsEl.textContent = data.args;
                flagsEl.textContent = data.flags;
                tableContainer.innerHTML = data.table;
            }
        });
    });
});

// 3. One-click Command Copy
window.copyInstallCmd = function() {
    const text = 'git clone https://github.com/AkashPriyadarshii/akashshell.git && cd akashshell && cargo run --release';
    navigator.clipboard.writeText(text).then(() => {
        const btn = document.getElementById('copy-btn');
        if (btn) {
            btn.innerHTML = '<i class="ph-bold ph-check"></i> Copied!';
            setTimeout(() => {
                btn.innerHTML = '<i class="ph ph-copy"></i> Copy';
            }, 2000);
        }
    }).catch(err => {
        console.error('Failed to copy: ', err);
    });
};

