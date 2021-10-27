from sphinx.util.docutils import SphinxDirective

from app.rendering.vnvdatavis.directives.charts import VnVChartNode
from app.rendering.vnvdatavis.directives.jmes import get_target_node


class PSIPDirective(SphinxDirective):

    PSIP_SCHEMA_URI="http://localhost:5000/static/assets/psip/schema.json"
    required_arguments = 1
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = False
    option_spec = {
        "height": str,
        "width": str,
        "class" : str
    }

    def register(self):
        return None

    def getScript(self): return self.script_template

    def getHtml(self):

        return f'''
          <div class="{self.options.get("class","card")}" style="width:{self.options.get("width","100%")}; height:{self.options.get("height" , "100%")};">
               {{%with schema="{PSIPDirective.PSIP_SCHEMA_URI}" %}}
               {{%with impl= data.query_str('{" ".join(self.arguments)}') %}}
               {{%include 'psip/index.html' %}}
               {{%endwith%}}
               {{%endwith%}} 
         </div>
        '''

    def run(self):
        target, target_id = get_target_node(self)
        block = VnVChartNode(html=self.getHtml())
        return [target, block]


    def register(self):
        return self.getContent()


def setup(sapp):
    sapp.add_directive("vnv-psip", PSIPDirective)
