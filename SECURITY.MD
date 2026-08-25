# SECURITY POLICY

## 🛡️ Supported Versions
Currently, `akashshell` is in active beta. Only the latest `main` branch is supported with security updates.

| Version | Supported          |
| ------- | ------------------ |
| v0.1.x  | :white_check_mark: |
| < v0.1  | :x:                |

## 🚨 Reporting a Vulnerability
Security is a top priority for the `akashshell` architecture. If you discover a vulnerability, **do not open a public issue.**

Please report security vulnerabilities directly to the project architect, **Akash Priyadarshii**, via a private GitHub Security Advisory on this repository.

### What to include in your report:
* Detailed, step-by-step instructions to reproduce the vulnerability.
* The expected vs. actual behavior.
* Potential attack vectors (e.g., pipeline injection, PTY buffer overflow, arbitrary code execution via Wasm plugins).
* Your environment (Windows 10 build, Windows 11 build, Rust version).

### Response SLA
We will triage the vulnerability within 48 hours. If the vulnerability is verified, a patch will be developed, tested, and pushed directly to `main` before a public CVE or advisory is published.
