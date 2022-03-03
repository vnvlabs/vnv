import hashlib
import json
import os
import re
import uuid

import docutils
import flask
from docutils.nodes import SkipNode
from sphinx.directives import optional_int
from sphinx.util import nested_parse_with_titles
from sphinx.util.docutils import SphinxDirective


from .jmes import get_target_node, jmes_jinja_query, jmes_jinja_if_query

vnv_directives = {}


class VnVForNode(docutils.nodes.General, docutils.nodes.Element):


    @staticmethod
    def visit_node(visitor, node):
        visitor.body.append(f'''
            <div id="{node["uid"]}_start" hidden ></div>
            <div id="{node["uid"]}_end" hidden ></div>
            <script>
            $(document).ready(function(){{
                
                url = "/directives/updates/{node["uid"]}/{{{{data.getFile()}}}}/{{{{data.getAAId()}}}}?context=for"
                update_now(url, "{node["uid"]}_start", 1000, function(config) {{
                    var nodes = $('#{node["uid"]}_start') 
                    var nodee = $('#{node["uid"]}_end')[0]
                    while (nodes.next()[0] != nodee) {{ nodes.next().remove(); }}
                    nodes.after(config)  
                }})
            
            }})
            </script>
        ''')


    @staticmethod
    def depart_node(visitor, node):
        pass


VnVForNode.NODE_VISITORS = {
    'html': (VnVForNode.visit_node, VnVForNode.depart_node)
}


class VnVForDirective(SphinxDirective):
    required_arguments = 0
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = True
    option_spec = {
        "start" : str,
        "end" : str,
        "step" : str,
        "values" : str,
        "variable" : str,
        "newlines" : int
    }

    def process_options(self):
        return re.sub('{{(.*?)}}', lambda x: jmes_jinja_query( x.group(1)), json.dumps(self.options))

    def get_json(self):
        return { "options" : self.process_options(), "content" : "\n".join(self.content) }

    @staticmethod
    def render_var(var,data, func=None):
        a = flask.render_template_string(var, data=data)
        if func is not None:
            return func(a)
        return a

    @staticmethod
    def extract_range(opts,data) :

        v = VnVForDirective.render_var(opts.get("variable", "i"), data)
        if "values" in opts:
            r = json.loads(flask.render_template_string(opts.get("values"), data=data))
        else:
            s = VnVForDirective.render_var(opts.get("start", "0"), data, int)
            e = VnVForDirective.render_var(opts.get("end", "10"), data, int)
            st = VnVForDirective.render_var(opts.get("step", "1"), data, int)
            r = range(s, e, st)
        return r,v

    @staticmethod
    def substitute(r, v, content):

        def subs(x, i, n, r):
            if x.group(1) == "index":
                return str(n)
            elif x.group(1) == "first":
                return "1" if n == 0 else "0"
            elif x.group(1) == "first":
                return "1" if len(r) == n + 1 else "0"
            return str(i)

        pcontent = []
        patt = re.compile("\\${" + v + "_?(.*?)?}")
        for n, i in enumerate(r):
            pcontent.append(re.sub(patt, lambda x: subs(x, i, n, r), content))

        return pcontent


    @staticmethod
    def post_process(cont, data, file):

        j = json.loads(cont)
        content = j["content"]
        opts = json.loads(j["options"])
        r,v = VnVForDirective.extract_range(opts,data)
        pcontent = VnVForDirective.substitute(r,v ,content)
        tname = file.render_to_string( ("\n" * opts.get("newlines",2)).join(pcontent))
        return flask.render_template_string(tname, data=data, file=file)

    def run(self):
        cont = json.dumps(self.get_json())
        uid = hashlib.md5(cont.encode()).hexdigest()
        with open(os.path.join(the_app.config.update_dir, uid),'w') as f:
            f.write(cont)

        target, target_id = get_target_node(self)
        block = VnVForNode(uid=uid)
        return [target, block]


vnv_directives["vnv-for"] = VnVForDirective
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
