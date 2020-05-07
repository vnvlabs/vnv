#!/usr/bin/env python
# -*- coding: utf-8 -*-
import jmespath
import json as jsonLoader
import os
import shutil
import uuid

from docutils.parsers.rst import Directive
from docutils import nodes
from ..jmes import JmesDirective
from ...generate import configs


class ParaviewNode(nodes.General, nodes.Element):
     children = ()

     local_attributes = ('backrefs','content','filename')

     idCount = 0;
     def getNewId(self):
          ParaviewNode.idCount += 1
          return "vnv-paraview-{}".format(ParaviewNode.idCount)

     script_template ='''
           <div id={id} class='vtkjs-volume-viewer' data-url="{url}" width={width} height={height}></div>
     '''

     def set(self,filename):
          self.attributes['filename'] = filename

     def getHtml(self, url  ):
          return self.script_template.format(
                   id=self.getNewId(),
                   url=url,
                   width="400",
                   height="400"
          )

class VnVParaviewDirective(Directive):
     required_arguments = 1
     optional_arguements = 0
     final_argument_whitespace = True
     has_content = False

     def run(self):
          node = ParaviewNode("")
          node.set(" ".join(self.arguments))
          return [node]

def process_paraview_nodes(app, doctree, docname):
     #Create the directory if not already exists.
     env = app.builder.env

     for node in doctree.traverse(ParaviewNode):
          parents = env.vnv_all_jmes.get(docname,[])
          srcDir = os.path.join(app.srcdir,os.path.dirname(docname))
          filepath = JmesDirective.getFilePath(node.attributes['filename'], parents, srcDir)
          url = JmesDirective.getUrl(filepath, app.outdir, env)
          html = node.getHtml(url)
          node.replace_self([nodes.raw('',html,format='html')])

def on_environment_ready(app):
     ## Make sure the javascript is available.
     js_dir = os.path.join(os.path.abspath(os.path.dirname(__file__)),"data")
     js_files = ["volumeViewer.js"]

     if not os.path.isdir(js_dir):
          raise ExtensionError("Cannot file data directory")
     for f in js_files:
        if not os.path.exists(os.path.join(js_dir,f)):
          raise ExtensionError("Cannot find {}".format(f))

     app.config.html_static_path.append(js_dir)
     for js_file in js_files:
        app.add_js_file(js_file)

def setup(app):
     app.add_node(ParaviewNode)
     app.add_directive("vnv-paraview", VnVParaviewDirective)
     app.connect("doctree-resolved",process_paraview_nodes)
     app.connect("builder-inited", on_environment_ready)



