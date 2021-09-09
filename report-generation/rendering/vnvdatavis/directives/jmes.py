import json
import os
import re

import docutils.nodes
from docutils.nodes import SkipNode
from docutils.parsers.rst import directives
from flask import render_template
from sphinx.directives import optional_int
from sphinx.directives.code import CodeBlock
from sphinx.errors import ExtensionError
from sphinx.util import nodes
from sphinx.util.docutils import SphinxDirective
import pygments
from pygments.lexers.data import JsonLexer
from pygments.formatters.html import HtmlFormatter

### Fake jmes
import rendering.fakejmes as jmespath




def get_target_node(directive):
    serial_no = directive.env.new_serialno("ccb")
    target_id = f"vnv-{serial_no}"
    targetnode = docutils.nodes.target('', '', ids=[target_id])
    return targetnode, target_id


def render_vnv_template(template, data):
    return render_template(template, data=DataClass(data))

class DataClass:
    statsMethods = ["min", "max", "avg"]

    def __init__(self, data):
        self.data = data;

    def _compile(self, expr):
        try:
            return jmespath.compile(expr)
        except  Exception as e:
            raise ExtensionError("Invalid Jmes Path")

    def query(self, text) -> str:
        """Return the jmes query result"""
        if (text == "Data.TotalTime"):
            a =  self._compile('TotalTime').search(self.data)
            print(a)
            return str(a[0])

        return self._compile(text).search(self.data)

    def query_str(self, text):
        """Return the jmes query as a string"""
        return str(self.query(text))

    def query_json(self, text):
        """Return the jmes query as a string"""
        return json.dumps(self.query(text))


    def codeblock(self, text):
        """Return highlighted json html for the resulting jmes query"""
        j = self.query_str(text)
        return pygments.highlight(j, JsonLexer(), HtmlFormatter(), outfile=None)

    def tree(self, text, options):
        """Return a table containing the json data provided"""
        opts = json.loads(options)

        return "<div> DATA WILL BE HERE </div>"

    def stat_str(self, text, meth):
        """Return statstics for a jmes query"""
        return "STATS"


def jmes_jinja_query(text):
    if jmespath.compile(text):
        return "{{ data.query('" + text + "') }}"
    else:
        raise ExtensionError("Invalid jmes path query")


def jmes_jinja_query_str(text):
    if jmespath.compile(text):
        return "{{ data.query_str('" + text + "')}}"
    else:
        raise ExtensionError("Invalid jmes path query")

def jmes_jinja_query_json(text):
    if jmespath.compile(text):
        return "{{ data.query_json('" + text + "')}}"
    else:
        raise ExtensionError("Invalid jmes path query")


def jmes_jinja_codeblock(text):
    if jmespath.compile(text):
        return "{{ data.codeblock('" + text + "')}}"
    else:
        raise ExtensionError("Invalid jmes path query")


def jmes_jinga_tree(text, options):
    if jmespath.compile(text):
        opts = json.dumps(options)
        return f"{{{{ data.table('{text}','{opts}') }}}}"
    else:
        raise ExtensionError("Invalid jmes path query")


def jmes_jinga_stat(text, meth):
    if jmespath.compile(text) and meth in DataClass.statsMethods:
        return f"{{{{ data.stat('{text}','{meth}') }}}}"
    else:
        raise ExtensionError("Invalid jmes path query")



