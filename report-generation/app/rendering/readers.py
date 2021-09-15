import csv
import hashlib
import json
import os
import urllib.request

import docutils
import markdown as markdown
import flask
import pygments
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


def has_reader(reader):
    return f'render_{reader}' in globals()


def render_reader(filename, reader):
    if has_reader(reader):
        a = globals()[f'render_{reader}'](filename)
        print(a)
        return a
    else:
        return "<div> Reader is not implemented yet. Sorry</div>"


print(render_csv('/home/ben/te/test.csv'))
