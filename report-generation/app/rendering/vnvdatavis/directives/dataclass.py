import json
from flask import render_template
from sphinx.errors import ExtensionError

import pygments
from pygments.lexers.data import JsonLexer
from pygments.formatters.html import HtmlFormatter

# Fake jme
from . import fakejmes as jmespath

def render_vnv_template(template, data, file, id=None):
    if id is None:
        return render_template(template, data=DataClass(data, data.getId(), file))
    return render_template(template, data=DataClass(data, id, file))


class DataClass:
    statsMethods = ["min", "max", "avg","str"]

    def __init__(self, data, id_, file):
        self.data = data
        self.id_ = id_
        self.file = file
        self.query_cache = {}

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

    def query(self, text):
        """Return the jmes query result"""
        if text in self.query_cache:
            return self.query_cache[text]

        try:
            res = self._compile(text).search(self.data)
            self.query_cache[text] = res
            return res

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

        a = self.query(text)
        r =  json.dumps(a, cls=jmespath.VnVJsonEncoder)
        return r

    def query_join(self, text):
        """Return the jmes query as a string"""

        a = self.query(text)
        try:
            return "".join([str(aa) for aa in a])
        except:
            return str(a)

    def query_str_array(self, text):
        """Return the jmes query as a string"""

        try:
            a = json.loads(text)
            if isinstance(a,list):
                x = []
                for e in a:
                    x.append(str(self.query(e)))
                return "".join(x)
        except:
            pass

        try:
            return str(self.query(text))
        except:
            return str(text)

    def codeblock(self, text):
        """Return highlighted json html for the resulting jmes query"""
        j = self.query_str(text)
        return pygments.highlight(
            j, JsonLexer(), HtmlFormatter(), outfile=None)

    def getFile(self):
        return self.file

    def getAAId(self):
        return self.data.getId()