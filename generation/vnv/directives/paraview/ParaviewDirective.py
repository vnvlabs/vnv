#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os

from docutils.parsers.rst import Directive
from docutils import nodes
from sphinx.errors import ExtensionError

from ..jmes import JmesDirective
from ...generate import configs

class ParaviewNode(nodes.General, nodes.Element):
    children = ()

    local_attributes = ('backrefs', 'content', 'filename')

script_template = '''
           <div id={id} class='vtkjs-volume-viewer' data-url="{url}" width={width} height={height}></div>
'''


def getHtml(url, idV, width="400", height="400"):
    return script_template.format(
        id=idV,
        url=url,
        width=width,
        height=height
    )


class VnVParaviewDirective(Directive):
    idCount = 0
    required_arguments = 1
    optional_arguments = 0
    final_argument_whitespace = True
    has_content = False
    option_spec = {
        "width": int,
        "height": int
    }
    idCount = 0;

    def getNewId(self):
        VnVParaviewDirective.idCount += 1
        return "vnv-paraview-{}".format(VnVParaviewDirective.idCount)

    def run(self):
        env = self.state.document.settings.env
        app = configs.getApp()
        src_dir = os.path.join(app.srcdir, os.path.dirname(env.docname))

        file_path = JmesDirective.getFilePath(" ".join(self.arguments), env.vnv_current_node, src_dir)
        url = JmesDirective.getUrl(file_path, app.outdir, env)
        width = str(self.options.get("width", 400))
        height = str(self.options.get("height", 400))
        html = getHtml(url, self.getNewId(), width=width, height=height)
        return [nodes.raw('', html, format='html')]


def on_environment_ready(app):
    # Make sure the javascript is available.
    js_dir = os.path.join(os.path.abspath(os.path.dirname(__file__)), "data")
    js_files = ["volumeViewer.js"]

    if not os.path.isdir(js_dir):
        raise ExtensionError("Cannot file data directory")
    for f in js_files:
        if not os.path.exists(os.path.join(js_dir, f)):
            raise ExtensionError("Cannot find {}".format(f))

    app.config.html_static_path.append(js_dir)
    for js_file in js_files:
        app.add_js_file(js_file)


def setup(app):
    app.add_directive("vnv-paraview", VnVParaviewDirective)
    app.connect("builder-inited", on_environment_ready)
