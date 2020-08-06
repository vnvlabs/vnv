#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
import shutil

from docutils.parsers.rst import Directive, directives
from docutils import nodes

from ..utils.JmesSearch import getFilePath, getUrl
from ...generate import configs
from ...generate.HtmlToPng import getIFrame
from ...generate.configs import add_js_files

header_template = '''
<script src='/_static/js/iframeResizer.contentWindow.min.js'></script>
<script src='/_static/js/volumeViewer.js'></script>
'''

script_template = '''
    <div id='{id}' width="100%" height="100%" class='vtkjs-volume-viewer' data-url='{url}'
      data-w='{width}' data-h={height}
    ></div>
'''


def getHtml(url, width="100%", height="500"):
    html = script_template.format(
        id=VnVParaviewDirective.getNewId(),
        url=url,
        width=width,
        height=height)
    return getIFrame(html, width=width, header=header_template)


class VnVParaviewDirective(Directive):
    idCount = 0
    required_arguments = 1
    optional_arguments = 0
    final_argument_whitespace = True
    has_content = False
    option_spec = {
        "width": directives.unchanged,
        "height": directives.unchanged
    }
    idCount = 0

    @staticmethod
    def getNewId():
        VnVParaviewDirective.idCount += 1
        return "vnv-paraview-{}".format(VnVParaviewDirective.idCount)

    def run(self):
        env = self.state.document.settings.env
        app = configs.getApp()
        src_dir = os.path.join(app.srcdir, os.path.dirname(env.docname))

        file_path = getFilePath(
            " ".join(
                self.arguments),
            env.vnv_current_node,
            src_dir)
        url = getUrl(file_path, app.outdir, env)
        width = str(self.options.get("width", "100%"))
        height = str(self.options.get("height", "100%"))
        iframe = getHtml(url, width=width, height=height)
        return [nodes.raw('', iframe, format='html')]


def on_environment_ready(app):
    # Make sure the javascript is available.
    js_dir = os.path.join(os.path.abspath(os.path.dirname(__file__)), "data")
    js_files = ["volumeViewer.js"]
    add_js_files(app.outdir, js_files, js_dir)


def setup(app):
    app.add_directive("vnv-vti-viewer", VnVParaviewDirective)
    app.connect("builder-inited", on_environment_ready)
