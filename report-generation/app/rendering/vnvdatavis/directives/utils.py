
from docutils.nodes import SkipNode
import os
import re
import uuid

import docutils.nodes
from docutils.nodes import SkipNode
from sphinx.directives import optional_int
from sphinx.util.docutils import SphinxDirective
from app.base.blueprints import files as dddd
from app.rendering.vnvdatavis.directives.utils import VnVChartNode

root = os.path.join("app", "static", "assets")
js_root = os.path.join(root, "js", "plugins")
css_root = os.path.join(root, "css", "plugins")

vnv_directives = {}
vnv_roles = {}
vnv_nodes = []
the_app = None

from app.rendering.vnvdatavis.directives.jmes import jmes_jinja_query_str, jmes_jinga_stat, DataClass, \
    jmes_jinja_codeblock, jmes_jinja_query, get_target_node, jmes_jinja_query_json




class VnVChartNode(docutils.nodes.General, docutils.nodes.Element):

    @staticmethod
    def visit_node(visitor, node):
        visitor.body.append(node["html"])
        raise SkipNode

    @staticmethod
    def depart_node(visitor, node):
        pass

VnVChartNode.NODE_VISITORS = {
    'html': (VnVChartNode.visit_node, VnVChartNode.depart_node)
}


def get_update_dir():
    dir = os.path.join(os.path.dirname(dddd.__file__), "templates", "renders", str(the_app.config.vnv_file), "updates")
    if not os.path.exists(dir):
        os.makedirs(dir)
    return dir

class JsonChartDirective(SphinxDirective):

    registration = {}
    required_arguments = 0
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = True
    options_spec = {
        "height": optional_int,
        "width": optional_int
    }

    def register(self):
        return None

    def getScript(self): return self.script_template

    def getContent(self):
        return re.sub(
            '{{(.*?)}}',
            lambda x: jmes_jinja_query(
                x.group(1)),
            "\n".join(
                self.content))

    def getHtml(self, id_, uid):
        return self.getScript().format(
            id_=id_,
            height=self.options.get("height", 400),
            width=self.options.get("width", 400),
            config=self.getContent(),
            uid=uid
        )

    def updateRegistration(self):
        r = self.register()
        if r is not None:
            uid = str(uuid.uuid4().hex)
            with open(os.path.join(get_update_dir(), uid + ".html"), 'w') as f:
                f.write(r)

            return uid
        return -1

    def run(self):
        uid = self.updateRegistration()
        target, target_id = get_target_node(self)
        block = VnVChartNode(html=self.getHtml(target_id, uid))
        return [target, block]

def setup(sapp):
    global the_app
    the_app = sapp

    sapp.add_node(VnVChartNode, **VnVChartNode.NODE_VISITORS)

