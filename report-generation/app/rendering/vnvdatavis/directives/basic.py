import base64
import json
import os
import re
import uuid

import docutils.nodes
from docutils.nodes import SkipNode
from docutils.parsers.rst import directives
from flask import render_template
from sphinx.directives import optional_int
from sphinx.util.docutils import SphinxDirective

from app.rendering.vnvdatavis.directives.charts import VnVChartNode
from app.rendering.vnvdatavis.directives.jmes import jmes_jinja_query_str, jmes_jinga_stat, DataClass, \
    jmes_jinja_codeblock, jmes_jinja_query, get_target_node, jmes_jinja_query_json

vnv_directives = {}
vnv_roles = {}

################ ADD A BUNCH OF ROLES TO TAKE STATISTICS OF JMES RESTULT #
def process_query(text, stats_function, tag="span"):
    r = jmes_jinga_stat(text, stats_function)
    html = f'''
                <{tag} data-o="on" 
                  data-f="{{{{data.getFile()}}}}" 
                  data-i="{{{{data.getAAId()}}}}" 
                  data-m="{stats_function}"
                  data-t="{tag}"
                  data-q="{base64.urlsafe_b64encode(text.encode('ascii')).decode('ascii')}">
                  {jmes_jinga_stat(text, stats_function)}
                </{tag}>
            '''
    return [VnVChartNode(html=html)], []


def get_stats_role(stats_function):
    def role(name, rawtext, text, lineno, inliner, options={}, content=[]):
        return process_query(text, stats_function)

    return role



class JmesStringDirective(SphinxDirective):
    required_arguments = 1
    optional_arguments = 0
    final_argument_whitespace = True
    option_spec = {}
    has_content = False

    def run(self):
        return process_query(" ".join(self.arguments), "str")



class JsonCodeBlockDirective(SphinxDirective):
    has_content = False
    required_arguments = 1
    optional_arguments = 0
    final_argument_whitespace = True
    option_spec = {}

    def run(self):
        return process_query(" ".join(self.arguments), "codeblock", tag="div")

class JsonImageDirective(SphinxDirective):
    required_arguments = 1
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = False
    options_spec = {
        "height": optional_int,
        "width": optional_int,
        "alt": directives.unchanged
    }

    script_tempate = '''
    <image  id="{id_} class="vnv_image" src="{src}" alt="{alt}" width="{width}" height="{height}"/>
    '''

    def getHtml(self, id_, content):
        return self.script_template.format(
            id_=id_,
            height=self.options.get("height", 400),
            width=self.options.get("width", ),
            src=content,
            alt=self.options.get("alt", "")
        )

    def run(self):
        j = jmes_jinja_query_str(" ".join(self.arguments))
        target, target_id = get_target_node(self)
        block = VnVChartNode(html=self.getHtml(target_id, j))
        return [target, block]

vnv_directives["vnv-image"] = JsonImageDirective
vnv_directives["vnv-code"] = JsonCodeBlockDirective
vnv_directives["vnv-print"] = JmesStringDirective
vnv_roles["vnv"] = get_stats_role("str")
for f in DataClass.statsMethods:
    vnv_roles[f"vnv-{f}"] = get_stats_role(f)

def setup(sapp):

    for key, value in vnv_roles.items():
        sapp.add_role(key, value)
    for key, value in vnv_directives.items():
        sapp.add_directive(key, value)