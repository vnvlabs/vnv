#!/usr/bin/env python
# -*- coding: utf-8 -*-
import jmespath
import json
import os
import uuid
import shutil

from docutils.parsers.rst import Directive
from ...generate.configs import getJmesSearchNode, getTextForNode
from docutils import nodes
from . import RootNodeVisitor

def getUrl(filepath, outDir, env, writer=None  ):

     if not hasattr(env,"vnv_copied_files"):
          env.vnv_copied_files = {}

     if filepath.startswith('http') or filepath.startswith('ftp'):
        return filepath
     else:

        if not os.path.exists(filepath):
           raise RuntimeError("File {} needed does not exist".format(filepath))

        if filepath in env.vnv_copied_files:
          url = env.vnv_copied_files[filepath]
        else:

          ext = os.path.splitext(filepath)[1]
          url = os.path.join('_static/files',str(uuid.uuid4()) + ext)
          while os.path.exists(os.path.join(outDir,url)):
               url = os.path.join('_static/files',str(uuid.uuid4()) + ext)
          env.vnv_copied_files[filepath] = url

        fname = os.path.join(outDir,url)
        if not os.path.exists(fname):
          if not os.path.exists(os.path.dirname(fname)):
             os.makedirs(os.path.dirname(fname))

          if writer is None:
             shutil.copy(filepath,fname)
          else:
             writer(filepath,fname)
        return "/" + url # Make it relative to the static dir.

#Get a raw file path
def getFilePath(filename, parents = None, srcdir = None):
     if filename.startswith("http://") or filename.startswith("https://") or filename.startswith("ftp://") :
          return filename

     if filename.startswith("vnv:"):
       jmes = filename[4:]
       return JmesDirective.getNodeText(parents, jmes)

     elif filename.startswith("." + os.path.sep) or filename.startswith(".." + os.path.sep):
          if srcdir is None:
               raise RuntimeError("No source dir available for relative file reference.")
          return os.path.join(srcdir, filename)
     else:
          return filename

def findNodeUsingJmes(jmesString):
     mainNode = getJmesSearchNode(jmesString)
     expression = jmespath.compile(jmesString)
     result = RootNodeVisitor.search(expression, mainNode)
     return result

def getNode(parents, childString):
     if parents is None:
          cparents = []
     else:
          cparents = parents[:]

     if childString[0] == "$" :
          if len(childString) == 1:
              return findNodeUsingJmes("".join(parents))
          else:
               return findNodeUsingJmes("".join(parents) + childString[1:])

     if childString[0] != ".": # Not relative, just use child String.
          return findNodeUsingJmes(childString)
     else:
         # Pop the first one, thats just means this directory.
         childString = childString[1:]
         # For any more dots, we pop a parent.
         while len(childString) and childString[0] == ".":
              # If we run out of parents, throw an error
              if len(cparents) == 0 :
                   raise RuntimeError("No more parent directories")
              cparents.pop()
              childString  = childString[1:]

         return findNodeUsingJmes("".join(cparents) + childString)

def getNodeText(parents, child):
     return getNode(parents,child).__str__()


class vnv(nodes.General, nodes.Element):
     children = ()

     local_attributes = ('backrefs','filename','jmes')

     def setExt(self, filename, jmes):
          self.attributes["filename"] = filename
          self.attributes["jmes"] = jmes


class VnVJmesPathDirective(Directive):
     required_arguments = 0
     optional_arguements = 0
     final_argument_whitespace = True
     option_spec = {}
     has_content = True

     def run(self):
          env = self.state.document.settings.env
          if not hasattr(env,"vnv_all_jmes"):
               env.vnv_all_jmes = {}
          p = json.loads("\n".join(self.content))
          env.vnv_all_jmes[env.docname] = p
          return []

def purge_all_jmes(app,env,docname):
     if not hasattr(env,"vnv_all_jmes"):
          return
     env.vnv_all_jmes.pop(docname, None)


def process_jmes_nodes(app, doctree, docname):
     env = app.builder.env
     if not hasattr(env, "vnv_all_jmes"):
          env.vnv_all_jmes = {}
     # Set the parent path for each jmes node.
     for node in doctree.traverse(vnv):
          parents = env.vnv_all_jmes.get(node.attributes["filename"],[])
          text = getNodeText(parents, node.attributes["jmes"])
          node.replace_self([nodes.Text(text,text)])

def vnv_data_role(role, rawtext, text, lineno, inliner,
                            options={}, content=[]):

     env = inliner.document.settings.env
     filename = env.docname
     jmes = text
     v = vnv('')
     v.setExt(filename,jmes)
     return [v] , []

def setup(app):

     app.add_node(vnv)
     app.add_directive("vnv-jmes-path", VnVJmesPathDirective)
     app.add_role("vnv", vnv_data_role)
     app.connect("env-purge-doc",purge_all_jmes)
     app.connect("doctree-resolved",process_jmes_nodes)

