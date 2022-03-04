import base64
import json
import os
import re
import uuid

import docutils.nodes

from sphinx.util import nested_parse_with_titles
from sphinx.util.docutils import SphinxDirective

from .charts import VnVChartNode
from .dataclass import DataClass
from .jmes import jmes_jinja_query_str, jmes_jinga_stat, \
    jmes_jinja_codeblock, jmes_jinja_query, get_target_node, jmes_jinja_query_json

vnv_directives = {}
vnv_roles = {}


def math_role(name, rawtext, text, lineno, inliner, options={}, content=[]):
    html = f"{{% raw %}} \( {text} \) {{% endraw %}}"
    return [VnVChartNode(html=html)], []


class VnVMathDirective(SphinxDirective):
    required_arguments = 0
    optional_arguments = 0
    final_argument_whitespace = True
    option_spec = {}
    has_content = True

    def run(self):
        target, target_id = get_target_node(self)
        cont = "\n".join(self.content)
        html = f"""<div class="math notranslate nohighlight">
                   {{% raw %}} \[ {cont} \] {{% endraw %}}
                   </div>
                """
        return [target, VnVChartNode(html=html)]



def setup(sapp):
    sapp.add_role("vnvmath", math_role)
    sapp.add_directive("vnv-math", VnVMathDirective)
