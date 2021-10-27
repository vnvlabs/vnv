import os
import re
import uuid

import docutils
from docutils.nodes import SkipNode
from sphinx.directives import optional_int
from sphinx.util.docutils import SphinxDirective
from app.base.blueprints import files as dddd


from app.rendering.vnvdatavis.directives.jmes import get_target_node,jmes_jinja_query

vnv_directives = {}


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


class JsonChartDirective(SphinxDirective):

    registration = {}
    required_arguments = 0
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = True
    options_spec = {
        "height": str,
        "width": str,
        "class" : str
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
        return f'''
          <div class="{self.options.get("class","")}" style="width:{self.options.get("width","100%")}; height:{self.options.get("height" , "100%")};">{self.getScript().format(
            id_=id_,

            config=self.getContent(),
            uid=uid
        )}</div>
        '''


    def get_update_dir(self):
        dir = os.path.join(os.path.dirname(dddd.__file__), "templates", "renders", str(the_app.config.vnv_file),
                           "updates")
        if not os.path.exists(dir):
            os.makedirs(dir)
        return dir

    def updateRegistration(self):
        r = self.register()
        if r is not None:
            uid = str(uuid.uuid4().hex)
            with open(os.path.join(self.get_update_dir(), uid + ".html"), 'w') as f:
                f.write(r)

            return uid
        return -1

    def run(self):
        uid = self.updateRegistration()
        target, target_id = get_target_node(self)
        block = VnVChartNode(html=self.getHtml(target_id, uid))
        return [target, block]


class PlotlyChartDirective(JsonChartDirective):
    script_template = '''
            <div id="{id_}" style="width:"100%"; height:"100%"></div>
            <script>
            $(document).ready(function() {{
              const obj = JSON.parse('{config}')
              Plotly.newPlot('{id_}',obj['data'],obj['layout']);
              
              url = "/directives/updates/{uid}/{{{{data.getFile()}}}}/{{{{data.getAAId()}}}}"
              update_soon(url, "{id_}_container", 1000, function(config) {{
                var xx = JSON.parse(config)
                Plotly.update('{id_}',xx['data'],xx['layout']);
              }})
                            
            }})
            </script>
            '''

    def register(self):
        return self.getContent()


class ApexChartDirective(JsonChartDirective):
    script_template = '''
          <div id="{id_}" class='vnv-table'></div>
          <script>
          $(document).ready(function() {{
            const obj = JSON.parse(`{config}`)
            var chart = new ApexCharts(document.querySelector("#{id_}"), obj);
            chart.render();
            
            url = "/directives/updates/{uid}/{{{{data.getFile()}}}}/{{{{data.getAAId()}}}}"
            update_soon(url, "{id_}_container", 1000, function(config) {{
                chart.updateOptions(JSON.parse(config)) 
            }})
            
          }})
          </script>
        '''

    def register(self):
        return self.getContent()


class GChartChartDirective(JsonChartDirective):
    script_template = '''
            <div id="{id_}" style="width:"100%"; height:"100%"></div>
            <script>
              $(document).ready(function() {{
              const json = `{config}`
              const obj = JSON.parse(`{config}`)
              obj['containerId'] = '{id_}'

              google.charts.load('current');
              
              google.charts.setOnLoadCallback(drawVisualization);
              function drawVisualization() {{
               var wrapper = new google.visualization.ChartWrapper(obj);
               wrapper.draw();
              
               url = "/directives/updates/{uid}/{{{{data.getFile()}}}}/{{{{data.getAAId()}}}}"
               update_soon(url, "{id_}_container", 1000, function(config) {{
                 var xx = JSON.parse(config)
                 wrapper.setOptions(xx);
                 wrapper.draw()
               }})
            }})
            </script>
        '''

    def register(self):
        return self.getContent()


class ChartJsChartDirective(JsonChartDirective):
    script_template = '''
           <div id="{id_}_container" width="100%" height="100%">
              <canvas id="{id_}"></canvas>
           </div>
           <script>
             $(document).ready(function() {{
             const obj = JSON.parse(`{config}`)
             var ctx = document.getElementById('{id_}');
             var myChart = new Chart(ctx, obj);
             
             url = "/directives/updates/{uid}/{{{{data.getFile()}}}}/{{{{data.getAAId()}}}}"
             update_soon(url, "{id_}_container", 1000, function(config) {{
                myChart.config = JSON.parse(config)
                myChart.update()  
             }})
           }})
           </script>
           '''

    def register(self):
        return self.getContent()


class TableChartDirective(JsonChartDirective):
    script_template = '''
         <div id="{id_}" class='vnv-table' width="100%" height="100%"></div>
         <script>
         $(document).ready(function() 
         {{
             const obj = JSON.parse(`{config}`)
             var table = new Tabulator("#{id_}", obj);
         
            url = "/directives/updates/{uid}/{{{{data.getFile()}}}}/{{{{data.getAAId()}}}}"
            update_soon(url, "{id_}_container", 3000, function(config) {{
               var table = new Tabulator("#{id_}", JSON.parse(config));
            }})
         }});
         </script>
         '''

    def register(self):
        return self.getContent()


class TreeChartDirective(JsonChartDirective):
    script_template = '''
        <div id="{id_}" class='vnv_jsonviewer' width="100%" height="100%"></div>
        <script>
          $(document).ready(function() {{
           var data = JSON.parse(`{config}`);
           var tree = new JSONFormatter(data['data'], true ,data['config']);
           document.getElementById('{id_}').appendChild(tree.render());
           
           url = "/directives/updates/{uid}/{{{{data.getFile()}}}}/{{{{data.getAAId()}}}}"
           update_soon(url, "{id_}_container", 3000, function(config) {{
                var data = JSON.parse(config);
                var tree = new JSONFormatter(data['data'], true ,data['config']);
                document.getElementById('{id_}').innerHTML=''
                document.getElementById('{id_}').appendChild(tree.render());
           }});    
        }})
           
        </script> '''

    def register(self):
        return self.getContent()

class TerminalDirective(JsonChartDirective):
    script_template = '''
            <div id="{id_}" class='card vnv_terminalviewer' style="height:100%;">
            </div>
            <script>
            {{
                $(document).ready(function() {{
                    url = "/directives/updates/{uid}/{{{{data.getFile()}}}}/{{{{data.getAAId()}}}}"
                    update_now(url, "{id_}", 3000, function(config) {{
                          $(document.getElementById("{id_}")).html("<pre class='term'>" + config + "</pre>")
                    }});    
               }});
            }}
            </script> '''

    def register(self):
        return self.getContent()


vnv_directives["vnv-apex"] = ApexChartDirective
vnv_directives["vnv-plotly"] = PlotlyChartDirective
vnv_directives["vnv-gchart"] = GChartChartDirective
vnv_directives["vnv-chart"] = ChartJsChartDirective
vnv_directives["vnv-table"] = TableChartDirective
vnv_directives["vnv-tree"] = TreeChartDirective
vnv_directives["vnv-terminal"] = TerminalDirective

try:
    the_app
except NameError:
    the_app = None


def setup(sapp):

    global the_app
    the_app = sapp

    sapp.add_node(VnVChartNode, **VnVChartNode.NODE_VISITORS)

    for key, value in vnv_directives.items():
        sapp.add_directive(key, value)


