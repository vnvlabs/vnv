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
import os

from sphinx.cmd.quickstart import *
from sphinx.cmd.build import main as sphinx_build
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

            while not valid_dir(d):
                print('quickstart only generate into a empty directory.'
                      ' Please specify a new root path.')
                np = input("Enter a new path to use when generating reports (leave empty to abort): ")
                if len(np) == 0:
                    return 1
                else:
                    d["path"] = np
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

def quickstart(reader: str, 
               file: str, 
               **kwargs
              ):

    author = kwargs.get("author", str(os.getlogin()))
    title = kwargs.get("title", "VnV Report")
    version = kwargs.get("version","0")
    release = kwargs.get("release","0")
    path = kwargs.get("path",".")
    builder = kwargs.get("builder","html")
    build_dir = kwargs.get("builder_dir", "_build")
    build_dir = os.path.join(path, build_dir)

    main(reader,file,"-q", "-a" , author, "-p", title, "-r", release, "-v", release, path )        
    build(builder,path,build_dir)


def build(builder, source_dir=".", build_dir="_build"):
    sphinx_build(["-M", builder, source_dir, build_dir])


def main(reader:str, filename:str, *sphinx_args):

    # Call sphinx quick-start to get us going.
    d = vnv_main(sphinx_args)
    if not isinstance(d,dict):
        sys.exit(1)

    srcPath = os.path.join(d['path'], 'source') if d['sep'] else d['path']

    confFile = os.path.join(srcPath, "conf.py")
    indexFile = os.path.join(srcPath, "index.rst")
    makefile = os.path.join(srcPath,"Makefile")
    with open(indexFile, 'r') as file:
        filedata = file.read()

    # Replace the target string
    replaceText = ".. vnv-read:: {reader} {filename}\n    :title: VnV Simulation report\n    :commmap: 1\n\n\n.. toctree::".format(
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

def serve(directory=os.getcwd(), port=8000, bind=None):
    import http.server as server
    import webbrowser as wb
    import threading



    def run(HandlerClass, ServerClass, port, bind, running):
        ServerClass.address_family, addr = server._get_best_family(bind, port)
        HandlerClass.protocol_version = "HTTP/1.0"
        with ServerClass(addr, HandlerClass) as httpd:
            host, port = httpd.socket.getsockname()[:2]
            url_host = f'[{host}]' if ':' in host else host
            print(
                f"Serving HTTP on {host} port {port} "
                f"(http://{url_host}:{port}/) ..."
            )
            while running():
                httpd.handle_request()
            print("Quitting")

    def serve_forever(handler_class,claz,port,bind, running):
        run(handler_class,claz,port,bind,running)

    handler_class = server.partial(server.SimpleHTTPRequestHandler,directory=directory)

    # ensure dual-stack is not disabled; ref #38907
    class DualStackServer(server.ThreadingHTTPServer):
        timeout = 1
        def server_bind(self):
            # suppress exception when protocol is IPv4
            with server.contextlib.suppress(Exception):
                self.socket.setsockopt(
                    socket.IPPROTO_IPV6, socket.IPV6_V6ONLY, 0)
            return super().server_bind()

    running = True
    def isRunning():
        return running

    try:
        th = threading.Thread(target=serve_forever, args=(handler_class,DualStackServer,port,bind, isRunning))
        th.start()
        wb.open("http://localhost:" + str(port) )
        th.join()
    except (KeyboardInterrupt, SystemExit):
        print('\n! Received keyboard interrupt, quitting threads.\n')
        running = False
        

def setup(app):
    Generate.setup(app)  # Setup the generator extensions
    VnVDirectives.setup(app)  # Setup the ResT extensions
    VnVTreeGenerator.setup(app)
    return {"version": __version__}


if __name__ == "__main__":
    if len(sys.argv) == 3:
        main(sys.argv[1],sys.argv[2], [])
    elif len(sys.argv) > 3:
        main(sys.argv[1],sys.argv[2], sys.argv[3:])
    else:
        print("Invalid usage -- You must provide at least a reader and file.")
