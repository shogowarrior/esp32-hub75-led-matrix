#!/usr/bin/env python3

import os
import utils

def run_backend_server(return_process=False):
    """Run backend server in watch mode."""
    os.chdir(os.path.join(os.path.dirname(__file__), '..', 'backend'))

    utils.kill_server("backend")
    utils.install_deps()

    return utils.run_server(
        command=['npx', 'tsx', 'watch', 'backend.ts'],
        return_process=return_process,
        watch_mode=True
    )

if __name__ == "__main__":
    run_backend_server()