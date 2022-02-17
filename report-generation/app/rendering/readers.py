import csv
import datetime
import hashlib
import json
import os
import urllib.request
from pathlib import Path

import docutils
import markdown as markdown
import flask
import pygments
from flask import render_template
from pygments.lexers import guess_lexer_for_filename

from app import Directory


def getPath(filename, exten=None):
    uu = hashlib.sha1(filename.encode()).hexdigest()
    if exten is None:
        ext = filename.split(".")
        if len(ext) > 1:
            uu += "." + ext[-1]
    else:
        uu += "." + exten

    d = Directory.STATIC_FILES_DIR
    p = os.path.join(d, uu)
    return p, uu


def getUID(filename, exten=None):
    p, uu = getPath(filename, exten=exten)
    if not os.path.exists(p):
        os.symlink(filename, p)
    return uu


def render_image(filename, **kwargs):
    return f"<img class='card' src='/temp/files/{getUID(filename)}' style='max-width:100%;'>"


def render_html(filename, **kwargs):
    return f"<iframe class='card' src='/temp/files/{getUID(filename)}' style='width: 100%;height:80vh;'>"


def render_pdf(filename, **kwargs):
    return f"<iframe class='card' src='/temp/files/{getUID(filename)}' style='width: 100%;height:80vh;'>"


def render_glvis(filename, **kwargs):
    path = urllib.request.pathname2url(f"/temp/files/{getUID(filename)}")
    return f"<iframe class='card' src='/static/glvis/index.html?stream={path}' style='width: 100%;height:80vh;'>"


def render_vti(filename, **kwargs):
    path = urllib.request.pathname2url(f"/temp/files/{getUID(filename)}")
    return f"<iframe class='card' src='/static/volume/index.html?fileURL={path}' style='width: 100%;height:80vh;'>"


def render_rst(filename, **kwargs):
    with open(filename, 'r') as f:
        d = f.read()
    p, u = getPath(filename, exten="html")

    with open(p, "w") as ff:
        ff.write(docutils.core.publish_string(d, writer_name='html5').decode())

    return f"<iframe src='/static/files/{u}' style='width: 100%;height:80vh;'>"


def render_markdown(filename, **kwargs):
    with open(filename, 'r') as f:
        return f"<div>{markdown.markdown(f.read())}</div>"


def render_code(filename, **kwargs):

    h = kwargs.get('highlightline')
    hl_lines = [int(h)] if h is not None else kwargs.get('highlightlines',[])

    with open(filename, 'r') as f:
        d = f.read()

        form = pygments.formatters.html.HtmlFormatter(
            linenos=True, hl_lines=hl_lines, style="colorful", cssclass="vnvhigh")

        try:
            lex = guess_lexer_for_filename(filename, d, stripnl=False)
            return pygments.highlight(d, lex, form)
        except:
            return pygments.highlight(d, pygments.lexers.get_lexer_by_name("bash", stripnl=False), form)

def render_csv(filename, **kwargs):
    divid = "table-table"
    with open(filename, 'r') as f:
        reader = csv.DictReader(f)
        tabledata = [r for r in reader]
        columndata = [{'title': a, "field": a} for a in tabledata[0]]

    return f'''
    <div id='{divid}' style='width:100%'></div>
    <script>
       var tabledata = JSON.parse('{json.dumps(tabledata)}')
       var columndata = JSON.parse('{json.dumps(columndata)}')
       var table = new Tabulator("#{divid}", {{
 	   height:205,
 	   data:tabledata,
 	   columns:columndata,
 	   layout:"fitColumns"

 	}});
 	</script>
    '''


ext_map = {
    ".jpeg": "image",
    ".jpg": "image",
    ".png": "image",
    ".gif": "image",
    ".svg": "image",
    ".md": "markdown"
}


def has_reader(reader):
    return f'render_{reader}' in globals()


def get_reader(ext):
    if has_reader(ext[1:]):
        return ext[1:]

    if ext in ext_map:
        return ext_map[ext]
    return "code"

class LocalFile:
    def __init__(self, abspath, vnvfileid, connection, reader=None, **kwargs):

        self.inputpath = abspath

        self.vnvfileid = vnvfileid
        self.connection = connection
        self.setInfo()
        self.reader = reader if reader is not None else get_reader(self.ext)

        self.breadcrumb = None
        self.iconStr = None
        self.exists_ = None
        self.children_ = None
        self.download_ = None
        self.is_dir_ = None
        self.root_ = None
        self.options = kwargs

    def render_reader(self):
        if has_reader(self.reader):
            a = globals()[f'render_{self.reader}'](self.download(), **self.options)
            return a
        else:
            return "<div> Reader is not implemented yet. Sorry</div>"

    def setHighlightLines(self, hl_lines):
        self.highlight = hl_lines

    def has_option(self, key):
        return key in self.options

    def get_option(self,key, default=None):
        return self.options.get(key,default)

    def setInfo(self):
        self.abspath, self.dir, self.name, self.ext, self.size, self.lastMod, self.lastModStr = self.connection.info(self.inputpath)

    def getVnVFileId(self):
        return self.vnvfileid

    def url(self):
        return urllib.request.pathname2url(self.abspath)

    def is_dir(self):
        if self.is_dir_ is None:
            self.is_dir_ = self.connection.is_dir(self.abspath)
        return self.is_dir_

    def connected(self):
        return self.connection.connected()

    def exists(self):
        if self.exists_ is None:
            self.exists_ = self.connection.exists(self.abspath)
        return self.exists_

    def children(self):
        if self.children_ is None:
            self.children_ = [LocalFile(i, self.vnvfileid, self.connection) for i in
                              self.connection.children(self.abspath)]
        return self.children_

    def download(self):
        if self.download_ is None:
            self.download_ = self.connection.download(self.abspath)
        return self.download_

    def render(self, modal=""):

        if self.is_dir():
            return render_template("files/directory.html", file=self, modal=modal)

        try:
            return self.render_reader()
        except Exception as e:
            return f"<div>{str(e)}</div>"

    def icon(self):
        return "folder" if self.is_dir() else "file"

    def crumb(self):
        if self.breadcrumb is None:
            cc = self.connection.crumb(self.dir)
            self.breadcrumb = [LocalFile(i, self.vnvfileid, self.connection) for i in cc]
            self.breadcrumb.append(self)
        return self.breadcrumb

    def root(self):
        if self.root_ is None:
            self.root_ = self.connection.root()
        return self.root_