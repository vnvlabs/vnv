#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import unicode_literals
from .directives import VnVDirectives
from .generate import Generate
from .generate.basic_tree import VnVTreeGenerator
__version__ = "0.0.1"

import shutil
import sys
import textwrap

from sphinx.cmd.quickstart import *

from pathlib import Path

# Straight copy from sphinx-quickstart main method, execept
# it returns the dict so we can modify.


def vnv_main(argv: List[str] = sys.argv[1:]) -> dict:
    sphinx.locale.setlocale(locale.LC_ALL, '')
    sphinx.locale.init_console(os.path.join(package_dir, 'locale'), 'sphinx')

    if not color_terminal():
        nocolor()

    # parse options
    parser = get_parser()
    try:
        args = parser.parse_args(argv)
    except SystemExit as err:
        return err.code

    d = vars(args)
    # delete None or False value
    d = {k: v for k, v in d.items() if v is not None}

    # handle use of CSV-style extension values
    d.setdefault('extensions', [])
    for ext in d['extensions'][:]:
        if ',' in ext:
            d['extensions'].remove(ext)
            d['extensions'].extend(ext.split(','))

    try:
        if 'quiet' in d:
            if not {'project', 'author'}.issubset(d):
                print(__('"quiet" is specified, but any of "project" or '
                         '"author" is not specified.'))
                return 1

        if {'quiet', 'project', 'author'}.issubset(d):
            # quiet mode with all required params satisfied, use default
            d.setdefault('version', '')
            d.setdefault('release', d['version'])
            d2 = DEFAULTS.copy()
            d2.update(d)
            d = d2

            if not valid_dir(d):
                print()
                print('Error: specified path is not a directory, or sphinx'
                      ' files already exist.')
                print('sphinx-quickstart only generate into a empty directory.'
                      ' Please specify a new root path.')
                return 1
        else:
            ask_user(d)
    except (KeyboardInterrupt, EOFError):
        print()
        print('[Interrupted.]')
        return 130  # 128 + SIGINT

    for variable in d.get('variables', []):
        try:
            name, value = variable.split('=')
            d[name] = value
        except ValueError:
            print(__('Invalid template variable: %s') % variable)

    generate(d, overwrite=False, templatedir=args.templatedir)
    return d


def main():

    reader = sys.argv[1]
    filename = sys.argv[2]

    # Call sphinx quick-start to get us going.
    d = vnv_main(sys.argv[3:])
    print(d)
    srcPath = os.path.join(d['path'], 'source') if d['sep'] else d['path']

    confFile = os.path.join(srcPath, "conf.py")
    indexFile = os.path.join(srcPath, "index.rst")
    with open(indexFile, 'r') as file:
        filedata = file.read()

    # Replace the target string
    replaceText = ".. vnv-read:: {reader} {filename}\n    :title: VnV Simulation report\n    :commmap: 1\n\n\n .. toctree::".format(
        reader=reader, filename=filename)
    filedata = filedata.replace('.. toctree::', replaceText)

    # Write the file out again
    with open(indexFile, 'w') as file:
        file.write(filedata)

    theme = '''
    try:
       import sphinx_rtd_theme
       html_theme = 'sphinx_rtd_theme'
    except:
       pass
    '''
    d = Path(os.path.dirname(os.path.abspath(__file__)))
    with open(confFile, 'r') as file:
        fileData = file.read()

    with open(confFile, 'w') as file:
        file.write("import os\n")
        file.write("import sys\n\n")
        file.write(fileData)
        file.write('\n\nsys.path.append("{}")\n\n'.format(d))
        file.write('\n\nextensions.append("vnv")\n\n')
        file.write(textwrap.dedent(theme))


def setup(app):
    Generate.setup(app)  # Setup the generator extensions
    VnVDirectives.setup(app)  # Setup the ResT extensions
    VnVTreeGenerator.setup(app)
    return {"version": __version__}


if __name__ == "__main__":
    main()
