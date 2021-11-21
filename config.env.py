import os
from pathlib import Path
import secrets

# Values in this file are loaded into the flask app instance. This file sources
# values from the environment if they exist, otherwise a set of defaults are
# used. Defaults may be overriden either by defining the environment variables,
# or by creating a `config.py` file that contains locally set secrets or config
# values.


# Defaults for flask configuration
IP = os.environ.get('IP', '127.0.0.1')
PORT = os.environ.get('PORT', 5000)
#SERVER_NAME = os.environ.get('SERVER_NAME', 'localhost:5000')
SECRET_KEY = os.environ.get('SESSION_KEY', default=''.join(secrets.token_hex(16)))

DEFAULT_SIZE = os.environ.get('MAZE_DEFAULT_SIZE', 50)
EXEC_PATH = str(Path(os.environ.get('MAZE_EXEC_PATH', './maze')).resolve())
OUT_PATH = Path(os.environ.get('MAZE_OUT_PATH', './maze.out')).resolve()
