#!/usr/bin/env python
# -*- coding: utf-8 -*-
import jmespath
import json as jsonLoader
import os
import shutil
import uuid
import csv

from docutils.parsers.rst import Directive
from docutils import nodes
from ..jmes import JmesDirective
from ...generate import configs


class TableNode(nodes.General, nodes.Element):
     children = ()

     local_attributes = ('backrefs','config','filename')
     idCount = 0;

     def getNewId(self):
          TableNode.idCount += 1
          return "vnv_table_{}".format(TableNode.idCount)


     script_template ='''
           <div id="{id}" class='vnv-table' width="{width}" height="{height}"></div>
           <script>
             function {id}_func() {{
               const obj = JSON.parse('{config}')
               var table = new Tabulator("#{id}", obj);
               table.setData({data})
             }}
             {id}_func()
           </script>
           '''

     def getHtml(self, url, width="400", height="400"):
          id = self.getNewId()
          return self.script_template.format(
               id=id,
               height=height,
               data="'" + url + "'",
               width=width,
               config=jsonLoader.dumps(self.attributes['config'])
          )

     def set(self,filetype, filename, config):
          try:
             self.attributes['filetype'] = filetype
             self.attributes['filename'] = filename
             self.attributes['config'] = jsonLoader.loads("".join(config))
          except:
             raise RuntimeError("Invalid Json")



class VnVTableDirective(Directive):
     required_arguments = 1
     optional_arguements = 1
     final_argument_whitespace = True
     has_content = True

     def run(self):
          node = TableNode("")
          node.set(self.arguments[0], "".join(self.arguments[1]), self.content)
          return [node]

def process_table_nodes(app, doctree, docname):
     #Create the directory if not already exists.
     env = app.builder.env

     for node in doctree.traverse(TableNode):

        parents = env.vnv_all_jmes.get(docname,[])
        srcDir = os.path.join(app.srcdir,os.path.dirname(docname))
        filepath = JmesDirective.getFilePath(node.attributes['filename'], parents, srcDir)

        writer = None if node.attributes['filetype'] != "csv" else convertCsvToJson

        print("WRITER : " , writer, node.attributes['filetype'])

        url = JmesDirective.getUrl(filepath, app.outdir, env, writer=writer)
        html = node.getHtml(url)
        node.replace_self([nodes.raw('',html,format='html')])

def convertCsvToJson(readPath, writePath):
     print("HERE")
     with open(readPath,'r') as f:
          a = [ row for row in csv.DictReader(f, skipinitialspace=True)]
          with open(writePath,'w') as f:
               f.write(jsonLoader.dumps(a))


def on_environment_ready(app):
     ## Make sure the javascript is available.
     js_dir = os.path.join(os.path.abspath(os.path.dirname(__file__)),"data")
     js_files = ["js/tabulator.min.js"]
     css_files = ["css/bootstrap/tabulator_bootstrap4.min.css", "css/bootstrap/tabulator_bootstrap4.min.css.map"]

     app.config.html_static_path.append(js_dir)
     for js_file in js_files:
        app.add_js_file(js_file)
     for css_file in css_files:
          app.add_css_file(css_file)


def setup(app):
     app.add_node(TableNode)
     app.add_directive("vnv-table", VnVTableDirective)
     app.connect("doctree-resolved",process_table_nodes)
     app.connect("builder-inited", on_environment_ready)



