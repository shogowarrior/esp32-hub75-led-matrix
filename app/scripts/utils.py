#!/usr/bin/env python3

import os
import subprocess
import psutil

def kill_server(server_type):
    """Kill server processes by type (frontend/backend)."""
    if server_type == "frontend":
        patterns = ["npx serve -l 8080", "serve -l 8080", "serve dist"]
        port = 8080
    elif server_type == "backend":
        patterns = ["npx tsx backend.ts", "tsx backend.ts", "node server.js", "node server"]
        port = 3001
    else:
        raise ValueError(f"Unknown server type: {server_type}")

    def _kill_server(patterns, port, server_name):
        """Kill server processes by patterns and port."""
        for proc in psutil.process_iter(['pid', 'name', 'cmdline']):
            try:
                # Kill by patterns
                cmdline = proc.info.get('cmdline')
                if cmdline:
                    cmdline_str = ' '.join(cmdline)
                    if any(p in cmdline_str for p in patterns):
                        print(f"Killing {server_name} process: {cmdline_str}")
                        proc.kill()
                        continue

                # Kill by port
                if proc.net_connections():
                    for conn in proc.net_connections():
                        if conn.laddr.port == port:
                            print(f"Killing {server_name} process on port {port}: {proc.info.get('cmdline')}")
                            proc.kill()
                            break
            except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
                pass

    _kill_server(patterns, port, server_type)

def install_deps():
    """Install Node.js dependencies if needed."""
    if not os.path.exists('node_modules'):
        print("Installing dependencies...")
        subprocess.run(['npm', 'install'], check=True)

def build_frontend():
    """Build frontend using npm."""
    print("Building frontend...")
    frontend_path = os.path.join(os.path.dirname(__file__), '..', 'frontend')
    subprocess.run(['npm', 'run', 'build'], cwd=frontend_path, check=True)

def run_server(command=None, port=None, directory="dist", return_process=False, cwd=None, watch_mode=False):
    """Run a server (command or simple web server)."""
    cmd = command or ['npx', 'serve', '-l', str(port), directory]
    mode_msg = " in watch mode" if watch_mode else ""

    print(f"Starting {'server' if command else 'web server'} on port {port}{mode_msg}..." if port else
          f"Starting server{mode_msg}: {cmd}")

    if return_process:
        return subprocess.Popen(cmd, cwd=cwd)
    subprocess.run(cmd, cwd=cwd, check=True)
