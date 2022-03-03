import hashlib
import os
import re
import uuid

import docutils
from docutils.nodes import SkipNode
from sphinx.directives import optional_int
from sphinx.util import nested_parse_with_titles
from sphinx.util.docutils import SphinxDirective

from .jmes import get_target_node, jmes_jinja_query, jmes_jinja_if_query

vnv_directives = {}


class VnVMainSliderNode(docutils.nodes.General, docutils.nodes.Element):

    @staticmethod
    def visit_node(visitor, node):
        visitor.body.append(f"""<div id="{node["uid"]}" class="carousel slide card " style=" align-items:center; " data-ride="carousel"><div id="{node["uid"]}_inner" class="carousel-inner" style="width:80%;">""")

    @staticmethod
    def depart_node(visitor, node):
       visitor.body.append( f'''
        </div>
        <a class="carousel-control-prev" href="#{node["uid"]}" role="button" data-slide="prev"><span class="carousel-control-prev-icon" aria-hidden="true"></span><span class="sr-only">Previous</span></a>
        <a class="carousel-control-next" href="#{node["uid"]}" role="button" data-slide="next"><span class="carousel-control-next-icon" aria-hidden="true"></span><span class="sr-only">Next</span></a>
        </div>
      ''')


class VnVSliderItemNode(docutils.nodes.General, docutils.nodes.Element):

    @staticmethod
    def visit_node(visitor, node):
        visitor.body.append(f"""<div class="carousel-item {node["active"]}" style="text-align:center;">""")

    @staticmethod
    def depart_node(visitor, node):
        visitor.body.append(f'''</div>''')

VnVMainSliderNode.NODE_VISITORS = {
    'html': (VnVMainSliderNode.visit_node, VnVMainSliderNode.depart_node)
}
VnVSliderItemNode.NODE_VISITORS = {
    'html': (VnVSliderItemNode.visit_node, VnVSliderItemNode.depart_node)
}


class VnVSliderDirective(SphinxDirective):
    required_arguments = 0
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = True
    option_spec = {}

    slider_count = 0

    def run(self):
        uid = uuid.uuid4().hex
        target, target_id = get_target_node(self)
        block = VnVMainSliderNode(uid=uid)
        nested_parse_with_titles(self.state, self.content, block)
        return [target, block]



class VnVSliderItemDirective(SphinxDirective):
    required_arguments = 0
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = True
    option_spec = {
        "active" : int
    }

    def run(self):
        uid = uuid.uuid4().hex
        target, target_id = get_target_node(self)
        block = VnVSliderItemNode(uid=uid, active=("active" if self.options.get("active",False) else ""))
        nested_parse_with_titles(self.state, self.content, block)
        return [target, block]

vnv_directives["vnv-slider"] = VnVSliderDirective
vnv_directives["vnv-slider-item"] = VnVSliderItemDirective

try:
    the_app
except NameError:
    the_app = None


def setup(sapp):
    global the_app
    the_app = sapp

    sapp.add_node(VnVMainSliderNode, **VnVMainSliderNode.NODE_VISITORS)
    sapp.add_node(VnVSliderItemNode, **VnVSliderItemNode.NODE_VISITORS)

    for key, value in vnv_directives.items():
        sapp.add_directive(key, value)
