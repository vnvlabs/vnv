#!/usr/bin/env python
# -*- coding: utf-8 -*-
import jmespath
import json
import os
import uuid
import shutil

import sphinx
from docutils.parsers.rst import Directive
from docutils.statemachine import ViewList
from sphinx.util import docutils
from ..nodes.VnVNodes import get_node_by_id
from docutils import nodes
from . import RootNodeVisitor


def getUrl(filepath, outDir, env, writer=None):
    if not hasattr(env, "vnv_copied_files"):
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
            url = os.path.join('_static/files', str(uuid.uuid4()) + ext)
            while os.path.exists(os.path.join(outDir, url)):
                url = os.path.join('_static/files', str(uuid.uuid4()) + ext)
            env.vnv_copied_files[filepath] = url

        fname = os.path.join(outDir, url)
        if not os.path.exists(fname):
            if not os.path.exists(os.path.dirname(fname)):
                os.makedirs(os.path.dirname(fname))

            if writer is None:
                shutil.copy(filepath, fname)
            else:
                writer(filepath, fname)
        return "/" + url  # Make it relative to the static dir.


# Get a raw file path
def getFilePath(filename, node=None, srcdir=None):
    if filename.startswith("http://") or filename.startswith("https://") or filename.startswith("ftp://"):
        return filename

    if filename.startswith("vnv:"):
        jmes = filename[4:]
        return str(getJMESNode(node, jmes))

    elif filename.startswith("." + os.path.sep) or filename.startswith(".." + os.path.sep):
        if srcdir is None:
            raise RuntimeError("No source dir available for relative file reference.")
        return os.path.join(srcdir, filename)
    else:
        return filename


def getJMESNode(node, jmesString):
    expression = jmespath.compile(jmesString)
    result = RootNodeVisitor.search(expression, node)
    return result


'''
 All content is parsed as if the this element was given by the jmes string in the arguments.
'''


class VnVIdDirective(Directive):
    required_arguments = 1
    optional_arguements = 0
    final_argument_whitespace = True
    option_spec = {}
    has_content = True

    def run(self):
        env = self.state.document.settings.env
        # Save the current id
        currNode = env.vnv_current_node

        # Set the new Id.
        env.vnv_current_node = getJMESNode(" ".join(self.arguments))

        # Parse the content with the updated node.
        node = docutils.nodes.paragraph()
        result = ViewList(self.content.splitlines(), source="")
        sphinx.util.nodes.nested_parse_with_titles(self.state, result, node)

        env.vnv_current_node = currNode
        return [node]


'''
  .. vnvToString:: <jmes-string>

  will call toString on the template for the method.
'''


class VnVStringWriter(Directive):
    required_arguments = 0
    optional_arguments = 1
    final_argument_whitespace = True
    option_spec = {
    }
    has_content = False

    def run(self):
        env = self.state.document.settings.env
        jmes = "".join(self.arguments)
        currNode = env.vnv_current_node
        nextNode = getJMESNode(currNode, jmes)
        rawT = nextNode.__str__()
        node = nodes.Text(rawT, rawT)
        return [node]


'''
Inline role for calling the toString() method for a given node. The toString
method is text. There is no nested parsing going on here.

:vnv:`jmes-string`

'''


def vnv_data_role(role, rawtext, jmes, lineno, inliner,
                  options={}, content=[]):
    env = inliner.document.settings.env
    result = str(getJMESNode(env.vnv_current_node, jmes))
    node = nodes.Text(result, result)
    return [node], []


def setup(app):
    app.add_directive("vnv-id", VnVIdDirective)
    app.add_directive("vnv-str", VnVStringWriter)
    app.add_role("vnvData", vnv_data_role)
