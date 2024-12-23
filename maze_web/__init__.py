""" A web interface for generating mazes """

import os
import subprocess
from typing import Any, Optional, Union

from flask import Flask, render_template, send_from_directory, send_file, request

from .link import LinkHeader, Link, LinkParam

Headers = dict[str, str]
Response = Union[tuple[str, int], tuple[Any, int, Headers]]

# A mapping of out_formats to mime types
mimetypes: dict[str, str] = {
    'png': 'image/png',
    'text': 'text/plain',
}

APP = Flask(__name__)

# Load file based configuration overrides if present
if os.path.exists(os.path.join(os.getcwd(), 'config.py')):
    APP.config.from_pyfile(os.path.join(os.getcwd(), 'config.py'))
else:
    APP.config.from_pyfile(os.path.join(os.getcwd(), 'config.env.py'))

APP.secret_key = APP.config['SECRET_KEY']

@APP.route('/', methods=['GET'])
@APP.route('/<out_format>', methods=['GET'])
def _png(out_format: str = 'png') -> Response:
    """ Generates a new maze

    Query params:
      - rows: the number of rows
      - cols: the number of columns
      - seed: plaintext seed to use
      - path_len: the maximum length of the path, defaulting to no limit (0)

    Path params:
      - out_format: the format to return

    """
    rows = request.args.get('rows', APP.config['DEFAULT_SIZE'])
    cols = request.args.get('cols', APP.config['DEFAULT_SIZE'])
    seed = request.args.get('seed', None)
    path_len = request.args.get('path_len', 0)

    valid_out_formats = set(subprocess.check_output([
        APP.config['EXEC_PATH'],
        '--print-valid-formats',
        ]
    ).decode('utf8').split())
    if out_format not in valid_out_formats:
        return f'out_format {out_format} must be one of {valid_out_formats}', 404

    try:
        print([
                APP.config['EXEC_PATH'],
                '--rows', rows,
                '--cols', cols,
                '--path-len', path_len,
                '-f', APP.config['OUT_PATH'],
                '--format', out_format,
            ] + (['--seed', seed] if seed else []))

        subprocess.run([
                APP.config['EXEC_PATH'],
                '--rows', str(rows),
                '--cols', str(cols),
                '--path-len', str(path_len),
                '-f', APP.config['OUT_PATH'],
                '--format', out_format,
            ] + (['--seed', seed] if seed else []),
            check=True,
            capture_output=True,
        )
    except subprocess.CalledProcessError as err:
        # uh so it broke and we should handle that, but that can happen later
        return f'something went wrong, sorry ({err.returncode})\n\n{err.stderr}', 500

    commit_hash = get_commit()
    if commit_hash:
        headers = {
            'X-Version': commit_hash,
            'Link': str(LinkHeader(
                Link('https://github.com/mxmeinhold/maze-generator',
                    LinkParam('rel', 'latest-version'),
                    LinkParam('rel', 'edit')
                ),
                Link(f'https://github.com/mxmeinhold/maze-generator/tree/{commit_hash}',
                    LinkParam('rel', 'version-history')
                )
            )),
        }
    else:
        headers = {
            'Link': str(LinkHeader(
                Link('https://github.com/mxmeinhold/maze-generator',
                    LinkParam('rel', 'latest-version'),
                    LinkParam('rel', 'edit')
                ),
            )),
        }

    return send_file(
        APP.config['OUT_PATH'],
        mimetype=mimetypes.get(out_format, 'application/octet-stream'),
        max_age=(3600 if seed else 0),
    ), 200, headers


@APP.route('/_version', methods=['GET'])
def version() -> Response:
    """ Return the plaintext version, and related headers  """
    commit_hash = get_commit()
    if commit_hash:
        return commit_hash, 200, {
            'X-Version': commit_hash,
            'Link': str(LinkHeader(
                Link('https://github.com/mxmeinhold/maze-generator',
                    LinkParam('rel', 'latest-version'),
                    LinkParam('rel', 'edit')
                ),
                Link(f'https://github.com/mxmeinhold/maze-generator/tree/{commit_hash}',
                    LinkParam('rel', 'version-history')
                )
            )),
            'Content-Type': 'text/plain',
            }

    return 'could not determine version', 500, {
        'Link': str(LinkHeader(
            Link('https://github.com/mxmeinhold/maze-generator',
                LinkParam('rel', 'latest-version'),
                LinkParam('rel', 'edit')
            ),
        )),
        'Content-Type': 'text/plain',
    }

def get_commit() -> Optional[str]:
    """ Attempt to discover the commit sha  """
    try:
        return subprocess.check_output(['git', 'rev-parse', '--short', 'HEAD']) \
                                .strip() \
                                .decode('utf-8')
    # pylint: disable=bare-except
    except:
        return None
