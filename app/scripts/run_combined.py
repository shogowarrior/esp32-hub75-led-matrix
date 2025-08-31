#!/usr/bin/env python3

import threading
from run_frontend import run_frontend_server
from run_backend import run_backend_server

def stream_output(process, prefix):
    """Stream output from a process with a prefix."""
    for line in iter(process.stdout.readline, b''):
        if line:
            print(f"{prefix}: {line.decode().strip()}")

def run_servers():
    """Run both backend and frontend servers with combined output."""
    print("Starting servers...")
    servers = [
        ("[BACKEND]", run_backend_server(return_process=True)),
        ("[FRONTEND]", run_frontend_server(return_process=True))
    ]

    # Create and start output streaming threads
    threads = [
        threading.Thread(
            target=stream_output,
            args=(process, prefix),
            daemon=True
        )
        for prefix, process in servers
    ]

    for thread in threads:
        thread.start()

    try:
        for _, process in servers:
            process.wait()
    except KeyboardInterrupt:
        print("\nShutting down servers...")
        for _, process in servers:
            process.terminate()
            process.wait()
        print("Servers stopped.")

if __name__ == "__main__":
    run_servers()