#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
import shutil

from docutils.parsers.rst import Directive, directives
from docutils import nodes

from ...generate import configs
from ...generate.HtmlToPng import getIFrame
from ...generate.configs import add_js_files

script_template = '''
<div id="{id}" width="100%" height="100%" class='vtkjs-function-viewer'
data-scale="{scale}" data-res="{res}" data-func='{func}'></div>
'''

header_template = '''
<script src='/_static/js/iframeResizer.contentWindow.min.js'></script>
<script src='/_static/js/functionViewer.js'></script>
'''


def getHtml(id_, func, scale=1, res=230, width="100%", height="400px"):
    html = script_template.format(id=id_, scale=scale, res=res, func=func)
    return getIFrame(html, width=width, header=header_template)


class VnVVtkFunctionDirective(Directive):
    idCount = 0
    required_arguments = 0
    optional_arguments = 0
    final_argument_whitespace = True
    has_content = True
    option_spec = {
        "width": directives.unchanged,
        "height": directives.unchanged,
        "scale": float,
        "res": directives.positive_int
    }
    idCount = 0

    @staticmethod
    def getNewId():
        VnVVtkFunctionDirective.idCount += 1
        return "vnv-function-{}".format(VnVVtkFunctionDirective.idCount)

    def run(self):
        env = self.state.document.settings.env
        app = configs.getApp()
        width = str(self.options.get("width", "100%"))
        height = str(self.options.get("height", "500px"))
        func = "\n".join(self.content)
        id_ = self.getNewId()
        scale = self.options.get("scale", 1)
        res = self.options.get('res', 230)
        iframe = getHtml(
            id_,
            func,
            width=width,
            height=height,
            scale=scale,
            res=res)
        return [nodes.raw('', iframe, format='html')]


def on_environment_ready(app):
    # Make sure the javascript is available.
    js_dir = os.path.join(os.path.abspath(os.path.dirname(__file__)), "data")
    js_files = ["functionViewer.js"]
    add_js_files(app.outdir, js_files, js_dir)


def setup(app):
    app.add_directive("vnv-function-viewer", VnVVtkFunctionDirective)
    app.connect("builder-inited", on_environment_ready)
