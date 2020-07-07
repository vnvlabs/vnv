#!/usr/bin/env python
# -*- coding: utf-8 -*-
import jmespath
import json as jsonLoader
import os
import re
from docutils.parsers.rst import Directive
from docutils import nodes
from sphinx.errors import ExtensionError
from docutils.parsers.rst import directives
from ..utils.FakeDict import FakeDict
from ..jmes import RootNodeVisitor, JmesDirective
import re


class chart(nodes.General, nodes.Element):
    children = ()

    local_attributes = ('backrefs', 'content')


class JmesSearchTerm:
    def __init__(self, mess):
        self.content = mess


class RawJsonData:
    def __init__(self, mess):
        self.content = mess


def checkJmes(mess):
    if mess[0] == "$":
        return JmesSearchTerm(mess)
    try:
        return RawJsonData(jsonLoader.loads(mess))
    except Exception as e:
        raise RuntimeError("Invalid Data. Data should be jmes (starts with $) or raw Json.")


def jmesChecker(key):
    return checkJmes


class VnVChartDirective(Directive):
    required_arguments = 0
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = True
    idCount = 0

    option_spec = FakeDict(jmesChecker, **{
        "width": int,
        "height": int
    })

    def getNewId(self):
        VnVChartDirective.idCount += 1
        return "vnv-chart-{}".format(VnVChartDirective.idCount)

    script_template = '''
     <div id="{id}_container" width="{width}px" height={height}px>
     <canvas id="{id}"></canvas>
     </div>
     <script>
     const json = '{json}'
     const obj = JSON.parse(json)
     var ctx = document.getElementById('{id}');
     var myChart = new Chart(ctx, obj);
     </script>
     '''

    @staticmethod
    def format(message, args):
        print(message)
        print(args)

        res = '\\$\\$([a-zA-Z0-9_] *)\\$\\$'
        return re.sub(res, lambda x: jsonLoader.dumps(args[x.group(1)]), message)

    def process(self, node):
        # Process the content for any vnv directives.
        subs = {}
        # TODO ADD toJSON METHOD to DATA NODES.

        print(self.options)

        for sub in self.options.keys():
            if isinstance(self.options[sub], RawJsonData):
                subs[sub] = self.options[sub].content
            elif isinstance(self.options[sub], JmesSearchTerm):
                subs[sub] = JmesDirective.getJMESNode(node, self.options[sub].content).toJson()
        cont = "".join(self.content)
        return VnVChartDirective.format(cont, subs)

    def getHtml(self, jsonObj):
        return self.script_template.format(
            id=self.getNewId(),
            json=jsonLoader.dumps(jsonObj),
            width=self.options.get("width", 400),
            height=self.options.get("height", 400)
        )

    def run(self):
        env = self.state.document.settings.env
        if not hasattr(env, "vnv_current_node"):
            env.vnv_current_node = None
        cont = self.process(env.vnv_current_node)
        print(cont)
        html = self.getHtml(jsonLoader.loads(cont))
        return [nodes.raw('', html, format="html")]


def on_environment_ready(app):
    # Make sure the javascript is available.
    js_dir = os.path.join(os.path.abspath(os.path.dirname(__file__)), "data")
    js_file = "Chart.min.js"

    if not os.path.isdir(js_dir):
        raise ExtensionError("Cannot file data directory")
    if not os.path.exists(os.path.join(js_dir, js_file)):
        raise ExtensionError("Cannot find Chart.min.js")

    app.config.html_static_path.append(js_dir)
    app.add_js_file(js_file)


def setup(app):
    app.add_node(chart)
    app.add_directive("vnv-chart", VnVChartDirective)
    app.connect("builder-inited", on_environment_ready)
