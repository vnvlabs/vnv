from sphinx.errors import ExtensionError
from sphinx.util.docutils import SphinxDirective

from .charts import VnVChartNode
from .jmes import get_target_node


def tcheck(a):
    if a in ["github","gitlab","local"]:
        return a
    raise ExtensionError("Invalid Type")


class IssuesDirective(SphinxDirective):



    required_arguments = 1
    file_argument_whitespace = True
    has_content = False
    option_spec = {
        "host": tcheck,
    }

    def get_include(self):
        return self.options.get("host","github")

    def getHtml(self):

        return f'''
          <div class="{self.options.get("class","")}" style="width:{self.options.get("width","100%")}; height:{self.options.get("height" , "100%")};">
               {{%with vnv_data= data.query_json('{" ".join(self.arguments)}') , readonly=True %}}
               {{%include 'issues/{self.get_include()}.html' %}}
               {{%endwith%}} 
         </div>
        '''

    def run(self):
        target, target_id = get_target_node(self)
        block = VnVChartNode(html=self.getHtml())
        return [target, block]

def setup(sapp):
    sapp.add_directive("vnv-issues", IssuesDirective)
