import hashlib
import json
import os
import re
import uuid

import docutils
from docutils.nodes import SkipNode
from sphinx.directives import optional_int
from sphinx.util import nested_parse_with_titles
from sphinx.util.docutils import SphinxDirective

from .jmes import get_target_node

vnv_directives = {}


class VnVForNode(docutils.nodes.General, docutils.nodes.Element):

    @staticmethod
    def visit_node(visitor, node):
        visitor.body.append(f'''<div id="{node["uid"]}">''')
        node["start"] = len(visitor.body)
        visitor.body.append(f'''{{ %for {node["value"]} in {node["range"]} %}}''')

    @staticmethod
    def depart_node(visitor, node):
       visitor.body.append( f'''{{% endfor %}}''')

       #Write the update html file.
       r = "".join(visitor.body[node["start"]:])
       with open(os.path.join(the_app.config.update_dir, node["uid"] + ".html"), 'w') as f:
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


VnVForNode.NODE_VISITORS = {
    'html': (VnVForNode.visit_node, VnVForNode.depart_node)
}


class VnVForDirective(SphinxDirective):
    required_arguments = 1
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = True
    option_spec = {
        "variable" : str,
        "start" : str,
        "end" :   str,
        "step" :  str
    }

    def getContent(self, val):
        return re.sub('{{(.*?)}}',lambda x: jmes_jinja_query_raw( x.group(1)), val)
    def getO(self,val,defa):
        return self.getContent(self.options.get(val,defa))

    def get_range(self):
        return f'''range({self.getO("start","`0`")},{self.getO("end","`5`")},{self.getO("step","`1`")})'''

    def run(self):
        uid = uuid.uuid4().hex
        target, target_id = get_target_node(self)
        block = VnVForNode(
                           range=self.get_range(),
                           value=self.options.get("variable","i"),
                           uid=uid,
                           start=0
                          )
        nested_parse_with_titles(self.state, self.content, block)
        return [target, block]


class VnVForInDirective(VnVForDirective):
    option_spec = {
        "variable" : str,
        "values" : str
    }

    def get_range(self):
        return self.getO("values","`['0','1','2']`")

class VnVForInDirectiveRaw(VnVForDirective):
    option_spec = {
        "variable" : str,
        "values" : str
    }

    def get_range(self):
        return f'''`{self.options.get("values",[])}`'''

class VnVForDirectiveRaw(VnVForDirective):
    option_spec = {
        "variable": str,
        "start": int,
        "end": int,
        "step": int
    }

    def get_range(self):
        return f'''`{
          json.dumps(
            range(self.options.get("start",0),
                  self.options.get("end",4),
                  self.options.get("step",1)
                  )
        )
        }`'''


vnv_directives["vnv-for-range"] = VnVForDirective
vnv_directives["vnv-for-range-raw"] = VnVForDirectiveRaw
vnv_directives["vnv-for-in"] = VnVForInDirective
vnv_directives["vnv-for-in-raw"] = VnVForInDirectiveRaw

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
