#!/usr/bin/env python
# -*- coding: utf-8 -*-
import jmespath
import json as jsonLoader
import os

from docutils.parsers.rst import Directive
from docutils import nodes
from ..jmes import RootNodeVisitor

class chart(nodes.General, nodes.Element):
     children = ()

     local_attributes = ('backrefs','content')

     idCount = 0;
     def getNewId(self):
          chart.idCount += 1
          return "vnv-chart-{}".format(chart.idCount)

     script_template ='''
     <canvas id = "{id}" width={width} height ={height}></canvas>
     <script>
     const json = '{json}'
     const obj = JSON.parse(json)
     var ctx = document.getElementById('{id}');
     var myChart = new Chart(ctx, obj);
     </script>
     '''

     def getHtml(self):
          return self.content

     def setHtml(self, jsonObj, width="400", height="400"):
         self.content = self.script_template.format(
         id=self.getNewId(),
         json=jsonLoader.dumps(jsonObj),
         width=width,
         height=height
       )


class VnVChartDirective(Directive):
     required_arguments = 0
     optional_arguements = 0
     file_argument_whitespace = True
     has_content = True

     def run(self):
          node = chart("")
          cont = "".join(self.content)
          print(cont)
          node.setHtml(jsonLoader.loads(cont))
          return [node]

def process_chart_nodes(app, doctree, docname):
     # Convert all chart nodes to raw html.
     for node in doctree.traverse(chart):
        html = node.getHtml()
        node.replace_self([nodes.raw('',html,format="html")])

def on_environment_ready(app):
     ## Make sure the javascript is available.
     js_dir = os.path.join(os.path.abspath(os.path.dirname(__file__)),"data")
     js_file = "Chart.min.js"

     if not os.path.isdir(js_dir):
          raise ExtensionError("Cannot file data directory")
     if not os.path.exists(os.path.join(js_dir,js_file)):
          raise ExtensionError("Cannot find Chart.min.js")

     app.config.html_static_path.append(js_dir)
     app.add_js_file(js_file)

def setup(app):
     app.add_node(chart)
     app.add_directive("vnv-chart", VnVChartDirective)
     app.connect("doctree-resolved",process_chart_nodes)
     app.connect("builder-inited", on_environment_ready)


