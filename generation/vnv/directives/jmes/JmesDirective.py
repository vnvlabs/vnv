#!/usr/bin/env python
# -*- coding: utf-8 -*-
import jmespath
import os
import uuid
import shutil

from docutils.parsers.rst import Directive
from docutils import nodes
from ..utils import JmesSearch
from ..utils.ModifiedFunctions import savedNodes


''' Save a node with a variable name'''


class VnVSaveDirective(Directive):
    required_arguments = 2
    optional_arguments = 0
    final_argument_whitespace = True
    has_content = False

    def run(self):
        env = self.state.document.settings.env
        key = self.arguments[0]
        jmes = " ".join(self.arguments[1:])
        node = JmesSearch.getJMESNode(env.vnv_current_node, jmes)
        savedNodes[key] = node
        return []


''' Save a node with a variable name'''


class VnVDeleteDirective(Directive):
    required_arguments = 1
    optional_arguments = 0
    final_argument_whitespace = True
    has_content = False

    def run(self):
        key = self.arguments[0]
        del savedNodes[key]
        return []


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
        jmes = " ".join(self.arguments)
        currNode = env.vnv_current_node
        nextNode = JmesSearch.getJMESNode(currNode, jmes)
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
    result = str(JmesSearch.getJMESNode(env.vnv_current_node, jmes))
    node = nodes.Text(result, result)
    return [node], []


def setup(app):
    app.add_directive("vnv-str", VnVStringWriter)
    app.add_directive("vnv-save", VnVSaveDirective)
    app.add_directive("vnv-del", VnVDeleteDirective)
    app.add_role("vnv", vnv_data_role)
