import hashlib
import json
import os
import re
import uuid

import docutils
from docutils.nodes import SkipNode
from sphinx.directives import optional_int
from sphinx.errors import ExtensionError
from sphinx.util import nested_parse_with_titles
from sphinx.util.docutils import SphinxDirective

from .jmes import get_target_node

vnv_directives = {}


class VnVForNode(docutils.nodes.General, docutils.nodes.Element):

    @staticmethod
    def visit_node(visitor, node):
        pass

    @staticmethod
    def depart_node(visitor, node):
        pass


VnVForNode.NODE_VISITORS = {
    'html': (VnVForNode.visit_node, VnVForNode.depart_node)
}


class VnVForDirective(SphinxDirective):
    required_arguments = 1
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = True
    option_spec = {
        "variable": str,
        "start": int,
        "end": int,
        "step": int,
        "newline" : int
    }

    def get_range(self):
        return range(self.options.get("start", 0), self.options.get("end", 5), self.options.get("step"), 1)

    def run(self):
        target, target_id = get_target_node(self)
        block = VnVForNode()
        a = []
        for i in self.get_range():
            p = re.compile(f"\${{{self.options.get('variable', 'i')}}}")
            for ii in self.content:
                a.append(re.sub(p, i), ii)
            a.append("\n" * self.options.get("newline",2 ))

        nested_parse_with_titles(self.state, a, block)
        return [target, block]


def json_array(str):
    try:
        return json.loads(str)
    except:
        raise ExtensionError()


class VnVForInDirective(VnVForDirective):
    option_spec = {
        "variable": str,
        "values": json_array
    }

    def get_range(self):
        return self.getO("values", [0, 1, 2, 3, 4])

vnv_directives["vnv-for-range"] = VnVForDirective
vnv_directives["vnv-for-in"] = VnVForInDirective

try:
    the_app
except NameError:
    the_app = None


def setup(sapp):
    global the_app
    the_app = sapp

    sapp.add_node(VnVForNode, **VnVForNode.NODE_VISITORS)

    for key, value in vnv_directives.items():
        sapp.add_directive(key, value)
