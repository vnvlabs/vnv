import hashlib
import os
import re
import uuid

import docutils
from docutils.nodes import SkipNode
from sphinx.directives import optional_int
from sphinx.util import nested_parse_with_titles
from sphinx.util.docutils import SphinxDirective

import Directory
from app.base.blueprints import files as dddd

from app.rendering.vnvdatavis.directives.jmes import get_target_node, jmes_jinja_query, jmes_jinja_if_query

vnv_directives = {}


class VnVIfNode(docutils.nodes.General, docutils.nodes.Element):

    @staticmethod
    def visit_node(visitor, node):
        visitor.body.append(f'''<div id="{node["uid"]}">''')
        node["start"] = len(visitor.body)
        visitor.body.append(f'''{jmes_jinja_if_query(node["query"])}''')

    @staticmethod
    def depart_node(visitor, node):
       visitor.body.append( f'''{{% endif %}}''')

       #Write the update html file.
       r = "".join(visitor.body[node["start"]:])
       with open(os.path.join(Directory.UPDATE_DIR, node["uid"] + ".html"), 'w') as f:
          f.write(r)

       visitor.body.append(f'''
                  </div>
                  <script>
                    url = "/directives/updates/{node["uid"]}/{{{{data.getFile()}}}}/{{{{data.getAAId()}}}}"
                    update_soon(url, "{node["uid"]}", 1000, function(config) {{
                        $('#{node["uid"]}).html(config)
                    }})
                  </script>
       ''')


VnVIfNode.NODE_VISITORS = {
    'html': (VnVIfNode.visit_node, VnVIfNode.depart_node)
}


class VnVIfDirective(SphinxDirective):
    required_arguments = 1
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = True
    option_spec = {}

    def run(self):
        uid = uuid.uuid4().hex
        target, target_id = get_target_node(self)
        block = VnVIfNode(query=" ".join(self.arguments), uid=uid, start=0)
        nested_parse_with_titles(self.state, self.content, block)
        return [target, block]


vnv_directives["vnv-if"] = VnVIfDirective

try:
    the_app
except NameError:
    the_app = None


def setup(sapp):
    global the_app
    the_app = sapp

    sapp.add_node(VnVIfNode, **VnVIfNode.NODE_VISITORS)

    for key, value in vnv_directives.items():
        sapp.add_directive(key, value)
