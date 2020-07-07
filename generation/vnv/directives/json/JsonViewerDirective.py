#!/usr/bin/env python
# -*- coding: utf-8 -*-
import json as json_loader
import os

from docutils.parsers.rst import Directive
from docutils.parsers.rst import directives
from docutils import nodes
from sphinx.errors import ExtensionError

from ..jmes import JmesDirective


def get_new_id():
    VnVJsonViewerDirective.idCount += 1
    return "vnv_jsonviewer_{}".format(VnVJsonViewerDirective.idCount)


def convert_to_bool(mess):
    if mess is None or mess in ["False", "false", "0", "f", "F"]:
        return False
    elif mess in ["True", "true", "1", "t", "T"]:
        return True
    raise RuntimeError("Non boolean options requested")


def cust_com(strf):
    return directives.choice(strf, ("dark", "light"))


class VnVJsonViewerDirective(Directive):
    required_arguments = 0
    optional_arguments = 1
    final_argument_whitespace = True
    has_content = True
    idCount = 0
    option_spec = {
        "width": int,
        "height": int,
        "open": int,
        "hoverPreviewEnabled": convert_to_bool,
        "hoverPreviewArrayCount": directives.positive_int,
        "hoverPreviewFieldCount": directives.positive_int,
        "theme": cust_com,
        "animateOpen": convert_to_bool,
        "animateClose": convert_to_bool,
        "useToJSON": convert_to_bool,
        "style": directives.unchanged
    }

    script_template = '''
    <div id="{id}" class='vnv_jsonviewer' width="{width}" height="{height}" style='{style}'></div> 
    <script> function {id}_func() {{ 
     var open = {open};
     var data = JSON.parse('{data}');
     var config = JSON.parse('{config}'); 
     var table = new JSONFormatter(data,open, config); 
     document.getElementById('{id}').appendChild(table.render()); }} 
     {id}_func();
    </script> '''

    def get_html(self, data):
        print(data)
        print(self.options)
        style = self.options.get("style", "")

        return self.script_template.format(
            id=get_new_id(),
            data=data,
            config=json_loader.dumps(self.options),
            open=self.options.get("open", 1),
            height=self.options.get("height", 400),
            width=self.options.get("width", 400),
            style=style
        )

    def run(self):
        env = self.state.document.settings.env

        if len(self.arguments) > 0:
            jmes = "".join(self.arguments)
            curr_node = env.vnv_current_node
            next_node = JmesDirective.getJMESNode(curr_node, jmes)
            html = self.get_html(next_node.getValue())
        else:
            html = self.get_html("".join(self.content))

        return [nodes.raw('', html, format='html')]


def on_environment_ready(app):
    # Make sure the javascript is available.
    js_dir = os.path.join(os.path.abspath(os.path.dirname(__file__)), "data")
    js_file = "json-formatter.js"

    if not os.path.isdir(js_dir):
        raise ExtensionError("Cannot file data directory")
    if not os.path.exists(os.path.join(js_dir, js_file)):
        raise ExtensionError("Cannot find Chart.min.js")

    app.config.html_static_path.append(js_dir)
    app.add_js_file(js_file)


def setup(app):
    app.add_directive("vnv-jsonviewer", VnVJsonViewerDirective)
    app.connect("builder-inited", on_environment_ready)
