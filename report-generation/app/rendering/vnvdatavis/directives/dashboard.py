import base64
import json
import os
import re
import uuid

import docutils.nodes
from sphinx.directives import optional_int
from sphinx.util import nodes, nested_parse_with_titles
from sphinx.util.docutils import SphinxDirective

from app.rendering.vnvdatavis.directives.jmes import get_target_node

class VnVDashBoardNode(docutils.nodes.General, docutils.nodes.Element):

    @staticmethod
    def visit_node(visitor, node):
        visitor.body.append(node["start"])

    @staticmethod
    def depart_node(visitor, node):
        visitor.body.append(node["end"])

VnVDashBoardNode.NODE_VISITORS = {
        'html': (VnVDashBoardNode.visit_node, VnVDashBoardNode.depart_node)
    }


class VnVDashBoardDirective(SphinxDirective):

    required_arguments = 0
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = True

    options_spec = {
        "height": optional_int,
        "width": optional_int,
        "base-width" : optional_int,
        "base-height" : optional_int,
        "margin-h" : optional_int,
        "margin-v" : optional_int
    }

    def o(self, key, value):
        return self.options.get(key,value)

    def initialize(self, id):
        return f'''
        <script>
            $(document).ready(function() {{
                
                $("#{id} ul").gridster({{
                     widget_margins: [{self.o("margin-h",10)}, {self.o("margin-v",10)}],
                     widget_base_dimensions: [{self.o("base-width",140)},{self.o("base-height",140)}]
                 }});
            }});
        </script>
        '''

    def start(self, id_):
        return f'''
            <div id="{id_}" 
                 class="gridster", 
                 style="width:{self.o("width","unset")}; height:{self.o("height","unset")};"
            > <ul>
        '''

    def end(self, id_):
        return f'''
            </ul>
            </div>
            {self.initialize(id_)}
        '''

    def run(self):
        uid = str(uuid.uuid4().hex)
        target, target_id = get_target_node(self)
        block = VnVDashBoardNode(start=self.start(uid), end=self.end(uid))
        nested_parse_with_titles(self.state,self.content,block)
        return [target, block]


class VnVDashBoardWidgetDirective(SphinxDirective):
    required_arguments = 0
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = True

    options_spec = {
        "row": optional_int,
        "col": optional_int,
        "size-x": optional_int,
        "size-y": optional_int
    }

    def o(self, key, value):
        return self.options.get(key, value)

    def start(self, id_):
        return f'''
           <li data-row="{self.o("row",1)}" 
               data-col="{self.o("col",1)}" 
               data-sizex="{self.o("size-x",1)}"
               data-sizey="{self.o("size-y",1)}"
           >
        '''

    def end(self, id_):
        return f'''</li>'''

    def run(self):
        uid = str(uuid.uuid4().hex)
        target, target_id = get_target_node(self)
        block = VnVDashBoardNode(start=self.start(uid), end=self.end(uid))
        nested_parse_with_titles(self.state, self.content, block)
        return [target, block]

def setup(sapp):
    sapp.add_node(VnVDashBoardNode, **VnVDashBoardNode.NODE_VISITORS)
    sapp.add_directive("vnv-dashboard", VnVDashBoardDirective)
    sapp.add_directive("vnv-db-widget", VnVDashBoardWidgetDirective)
