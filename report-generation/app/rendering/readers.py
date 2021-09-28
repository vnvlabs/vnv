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


def getPath(filename, exten=None):
    uu = hashlib.sha1(filename.encode()).hexdigest()
    if exten is None:
        ext = filename.split(".")
        if len(ext) > 1:
            uu += "." + ext[-1]
    else:
        uu += "." + exten
    p = os.path.join(flask.current_app.config.root_path, "static", "files", uu)
    return p, uu


def getUID(filename, exten=None):
    p, uu = getPath(filename, exten=exten)
    if not os.path.exists(p):
        os.symlink(filename, p)
    return uu


def render_image(filename):
    return f"<img class='card' src='/static/files/{getUID(filename)}' style='max-width:89vw;'>"


def render_html(filename):
    return f"<iframe class='card' src='/static/files/{getUID(filename)}' style='width: 100%;height:80vh;'>"


def render_pdf(filename):
    return f"<iframe class='card' src='/static/files/{getUID(filename)}' style='width: 100%;height:80vh;'>"


def render_glvis(filename):
    path = urllib.request.pathname2url(f"/static/files/{getUID(filename)}")
    return f"<iframe class='card' src='/static/glvis/index.html?stream={path}' style='width: 100%;height:80vh;'>"


def render_vti(filename):
    path = urllib.request.pathname2url(f"/static/files/{getUID(filename)}")
    return f"<iframe class='card' src='/static/volume/index.html?fileURL={path}' style='width: 100%;height:80vh;'>"


def render_rst(filename):
    with open(filename, 'r') as f:
        d = f.read()
    p, u = getPath(filename, exten="html")

    with open(p, "w") as ff:
        ff.write(docutils.core.publish_string(d, writer_name='html5').decode())

    return f"<iframe src='/static/files/{u}' style='width: 100%;height:80vh;'>"


def render_markdown(filename):
    with open(filename, 'r') as f:
        return f"<div>{markdown.markdown(f.read())}</div>"


def render_code(filename):
    with open(filename, 'r') as f:
        d = f.read()
        lex = guess_lexer_for_filename(filename, d, stripall=True)
        form = pygments.formatters.html.HtmlFormatter(
            linenos=True, style="colorful", noclasses=True)
        return pygments.highlight(d, lex, form)


def render_csv(filename):
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
    ".jpeg" : "image",
    ".jpg" : "image",
    ".png" : "image",
    ".gif" : "image",
    ".svg" : "image",
    ".md" : "markdown"
}

def get_reader(filename):

    if os.path.exists(filename) and Path(filename).is_dir():
        return "directory"

    ext = os.path.splitext(filename)[1]
    if has_reader(ext[1:]):
        return ext[1:]

    if ext in ext_map:
        return ext_map[ext]
    return "code"


class LocalFile:
    def __init__(self, abspath, reader=None):

        self.abspath = abspath
        self.dir = os.path.dirname(abspath)
        self.name = os.path.basename(abspath)
        self.reader = reader if reader is not None else get_reader(self.abspath)
        if len(self.abspath):
            self.size = os.lstat(abspath).st_size
            self.lastMod = os.lstat(abspath).st_mtime
            value = datetime.datetime.fromtimestamp(self.lastMod)
            self.lastModStr = (value.strftime('%Y-%m-%d %H:%M:%S'))

    def url(self):
        return urllib.request.pathname2url(self.abspath)

    def children(self):
        c = []
        for i in os.listdir(self.abspath):
            ap = os.path.join(self.abspath,i)
            c.append(LocalFile(ap))
        return c

    def render(self):
        try:
            return render_reader(self.abspath, self.reader)
        except Exception as e :
            return f"<div>{str(e)}</div>"

    def icon(self):
        return "folder" if Path(self.abspath).is_dir() else "file"

    def crumb(self):
        c =  os.path.normpath(self.dir).split(os.path.sep)
        cc = os.path.abspath(os.sep)
        loc = []
        for i in c:
            if len(i):
                cc = os.path.join(cc,i)
                loc.append(LocalFile(cc))
        loc.append(self)
        return loc

def render_directory(filename):
    if len(filename) == 0 :
        filename = os.path.abspath(os.sep)

    if os.path.exists(filename) and Path(filename).is_dir():
        return render_template("files/directory.html", file=LocalFile(os.path.abspath(filename)))


def has_reader(reader):
    return f'render_{reader}' in globals()


def render_reader(filename, reader):
    if has_reader(reader):
        a = globals()[f'render_{reader}'](filename)
        return a
    else:
        return "<div> Reader is not implemented yet. Sorry</div>"
