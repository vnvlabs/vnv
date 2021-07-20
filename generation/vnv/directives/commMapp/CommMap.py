import os

from docutils.parsers.rst import Directive, directives
from docutils import nodes

from ...generate.configs import add_js_files
import json as jsonLoader


class VnVCommMapDirective(Directive):
    required_arguments = 0
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = False
    idCount = 0

    option_spec = {
        "width": directives.unchanged,
        "height": directives.unchanged,
    }

    def getNewId(self):
        VnVCommMapDirective.idCount += 1
        return "vnv-commMap-{}".format(VnVCommMapDirective.idCount)

    script_name = "commMap.js"
    css_name = "commMap.css"

    script_template = '''
     <svg id="{id}" width="{width}" height="{height}" worldsize={worldsize}></svg>
     <script> 
     const json_commMap = `{json}`
     const obj_commMap = JSON.parse(json_commMap)
     drawGraph(obj_commMap, "{id}") 
     </script>
     '''

    def getHtml(self, commMap, worldsize):
        id_ = self.getNewId()
        return self.script_template.format(
            id=id_,
            worldsize=worldsize,
            json=commMap,
            width=self.options.get("width", "600"),
            height=self.options.get("height", "400")
        )

    def run(self):
        env = self.state.document.settings.env
        commMap = env.vnv_current_reader.get().getCommMap()
        worldsize = env.vnv_current_reader.get().getWorldSize()
        if worldsize > 1:
            return [nodes.raw('', self.getHtml(commMap,worldsize), format="html")]
        return []




def on_environment_ready(app):
    # Make sure the javascript is available.
    js_dir = os.path.join(os.path.abspath(os.path.dirname(__file__)), "data")
    js_files = ["commMap.js", "d3.js", "commMap.css"]
    add_js_files(app.outdir, js_files, js_dir)
    app.add_js_file("js/d3.js")
    app.add_js_file("js/commMap.js")
    app.add_css_file("js/commMap.css")

def setup(app):
    app.add_directive("vnv-comm-map", VnVCommMapDirective)
    app.connect("builder-inited", on_environment_ready)
