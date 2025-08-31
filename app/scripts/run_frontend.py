#!/usr/bin/env python3

import os
import utils

def run_frontend_server(return_process=False):
    """Run frontend server in watch mode."""
    frontend_path = os.path.join(os.path.dirname(__file__), '..', 'frontend')
    os.chdir(frontend_path)

    utils.kill_server("frontend")
    utils.install_deps()

    # Run npm build directly in the frontend directory
    import subprocess
    subprocess.run(['npm', 'run', 'build'], check=True)

    return utils.run_server(
        port=8080,
        return_process=return_process,
        cwd=os.getcwd(),
        watch_mode=True
    )

if __name__ == "__main__":
    run_frontend_server()