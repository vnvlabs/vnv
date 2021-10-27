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

# Fake jmes
import app.rendering.fakejmes as jmespath
from app.base.blueprints import files as dddd


def render_vnv_template(template, data, file):
    return render_template(template, data=DataClass(data, data.getId(), file))


class DataClass:
    statsMethods = ["min", "max", "avg","str"]

    def __init__(self, data, id_, file):
        self.data = data
        self.id_ = id_
        self.file = file

    def _compile(self, expr):
        try:
            return jmespath.compile(expr)
        except Exception as e:
            raise ExtensionError("Invalid Jmes Path")

    def mquery(self,meth, query):
        if meth == "str":
            return self.query_str(query)
        elif meth == "codeblock":
            return self.codeblock(query)
        elif meth == "json":
            return self.query_json(query)
        elif meth == "":
            return self.query(query)
        else:
            return f"todo:{meth}({query})"

    def query(self, text) -> str:
        """Return the jmes query result"""
        if (text == "Data.TotalTime"):
            a = self._compile('TotalTime').search(self.data)
            print(a)
            return str(a[0])
        try:
            return self._compile(text).search(self.data)
        except Exception as e:
            print(e)
            return ""

    def query_str(self, text):
        """Return the jmes query as a string"""
        return str(self.query(text))

    def query_percent(self, curr, min, max):
        acurr = self.query(curr)
        amin = self.query(min)
        amax = self.query(max)
        return 100 * ( acurr / (amax - amin ) )

    def query_zip(self, text):
        vals = {}
        a = json.loads(text)
        if len(a) > 0:

            for i in a:
                a[i] = self.query(a[i])
            res = [dict(zip(a, t)) for t in zip(*a.values())]
            return json.dumps(res)
        else:
            return []

        return json.dumps(ret)

    def query_json(self, text):
        """Return the jmes query as a string"""
        return json.dumps(self.query(text), cls=jmespath.VnVJsonEncoder)

    def codeblock(self, text):
        """Return highlighted json html for the resulting jmes query"""
        j = self.query_str(text)
        return pygments.highlight(
            j, JsonLexer(), HtmlFormatter(), outfile=None)

    def getFile(self):
        return self.file

    def getAAId(self):
        return self.data.getId()