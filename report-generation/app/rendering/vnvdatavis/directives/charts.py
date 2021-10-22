import os
import uuid

import docutils
from docutils.nodes import SkipNode
from sphinx.directives import optional_int
from sphinx.util.docutils import SphinxDirective

from app.rendering.vnvdatavis.directives.jmes import get_target_node, get_update_dir, jmes_jinja_query

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
        "height": optional_int,
        "width": optional_int
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
        return self.getScript().format(
            id_=id_,
            height=self.options.get("height", 400),
            width=self.options.get("width", 400),
            config=self.getContent(),
            uid=uid
        )

    def updateRegistration(self):
        r = self.register()
        if r is not None:
            uid = str(uuid.uuid4().hex)
            with open(os.path.join(get_update_dir(), uid + ".html"), 'w') as f:
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
            <div id="{id_}" style="width:"{width}"; height:"{height}"></div>
            <script>
            {{
              const obj = JSON.parse('{config}')
              Plotly.newPlot('{id_}',obj['data'],obj['layout']);
              
              url = "/directives/updates/{uid}/{{{{data.getFile()}}}}/{{{{data.getAAId()}}}}"
              update_soon(url, "{id_}", 1000, function(config) {{
                var xx = JSON.parse(config)
                Plotly.update('{id_}',xx['data'],xx['layout']);
              }})
                            
            }}
            </script>
            '''

    def register(self):
        return self.getContent()


class ApexChartDirective(JsonChartDirective):
    script_template = '''
          <div id="{id_}" class='vnv-table' width="{width}" height="{height}"></div>
          <script>
          {{
            const obj = JSON.parse(`{config}`)
            var chart = new ApexCharts(document.querySelector("#{id_}"), obj);
            chart.render();
            
            url = "/directives/updates/{uid}/{{{{data.getFile()}}}}/{{{{data.getAAId()}}}}"
            update_soon(url, "{id_}", 1000, function(config) {{
                chart.updateOptions(JSON.parse(config)) 
            }})
            
          }}
          </script>
        '''

    def register(self):
        return self.getContent()


class GChartChartDirective(JsonChartDirective):
    script_template = '''
            <div id="{id_}" style="width:"{width}"; height:"{height}"></div>
            <script>
            {{
              const json = `{config}`
              const obj = JSON.parse(`{config}`)
              obj['containerId'] = '{id_}'

              google.charts.load('current');
              
              google.charts.setOnLoadCallback(drawVisualization);
              function drawVisualization() {{
               var wrapper = new google.visualization.ChartWrapper(obj);
               wrapper.draw();
              
               url = "/directives/updates/{uid}/{{{{data.getFile()}}}}/{{{{data.getAAId()}}}}"
               update_soon(url, "{id_}", 1000, function(config) {{
                 var xx = JSON.parse(config)
                 wrapper.setOptions(xx);
                 wrapper.draw()
               }})
            }}
            </script>
        '''

    def register(self):
        return self.getContent()


class ChartJsChartDirective(JsonChartDirective):
    script_template = '''
           <div id="{id_}_container" width="{width}" height="{height}">
              <canvas id="{id_}"></canvas>
           </div>
           <script>
           {{
             const obj = JSON.parse(`{config}`)
             var ctx = document.getElementById('{id_}');
             var myChart = new Chart(ctx, obj);
             
             url = "/directives/updates/{uid}/{{{{data.getFile()}}}}/{{{{data.getAAId()}}}}"
             update_soon(url, "{id_}_container", 1000, function(config) {{
                myChart.config = JSON.parse(config)
                myChart.update()  
             }})
           }}
           </script>
           '''

    def register(self):
        return self.getContent()


class TableChartDirective(JsonChartDirective):
    script_template = '''
         <div id="{id_}" class='vnv-table' width="{width}" height="{height}"></div>
         <script>
         {{
             const obj = JSON.parse(`{config}`)
             var table = new Tabulator("#{id_}", obj);
         
            url = "/directives/updates/{uid}/{{{{data.getFile()}}}}/{{{{data.getAAId()}}}}"
            update_soon(url, "{id_}", 3000, function(config) {{
.               var table = new Tabulator("#{id_}", JSON.parse(config));
            }})
         }}
         </script>
         '''

    def register(self):
        return self.getContent()


class TreeChartDirective(JsonChartDirective):
    script_template = '''
        <div id="{id_}" class='vnv_jsonviewer' width="{width}" height="{height}"></div>
        <script>
        {{
           var data = JSON.parse(`{config}`);
           var tree = new JSONFormatter(data['data'], true ,data['config']);
           document.getElementById('{id_}').appendChild(tree.render());
           
           url = "/directives/updates/{uid}/{{{{data.getFile()}}}}/{{{{data.getAAId()}}}}"
           update_now(url, "{id_}", 3000, function(config) {{
                var data = JSON.parse(config);
                var tree = new JSONFormatter(data['data'], true ,data['config']);
                document.getElementById('{id_}').innerHTML=''
                document.getElementById('{id_}').appendChild(tree.render());
           }});    
        }}
           
        </script> '''

    def register(self):
        return self.getContent()


<<<<<<< HEAD:report-generation/app/rendering/vnvdatavis/directives/general.py
class TerminalDirective(JsonChartDirective):
    script_template = '''
            <div id="{id_}" class='card vnv_terminalviewer' style="height:800px;">
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

    script_template1 = '''
        <div id="{id_}" class='vnv_terminalviewer' style="height:800px;"></div>
        <script>
        {{
            $(document).ready(function() {{
               const term = new Terminal({{
                    cursorBlink: false,
                    macOptionIsMeta: true,
                    scrollback: 9999999,
                }});
                const fit = new FitAddon.FitAddon();
                term.loadAddon(fit);
                term.open(document.getElementById("{id_}"));
                term.resize(180,40)
                fit.fit();
                url = "/directives/updates/{uid}/{{{{data.getFile()}}}}/{{{{data.getAAId()}}}}"
                update_now(url, "{id_}", 3000, function(config) {{
                      term.clear()
                      config = config.replace(/\\n/g,"\\r\\n")
                      term.write(config) ;
                }});    
           }});
        }}

        </script> '''

    def register(self):
        return self.getContent()


vnv_nodes.append(VnVChartNode)
=======
>>>>>>> origin/add_dashboard:report-generation/app/rendering/vnvdatavis/directives/charts.py
vnv_directives["vnv-apex"] = ApexChartDirective
vnv_directives["vnv-plotly"] = PlotlyChartDirective
vnv_directives["vnv-gchart"] = GChartChartDirective
vnv_directives["vnv-chart"] = ChartJsChartDirective
vnv_directives["vnv-table"] = TableChartDirective
vnv_directives["vnv-tree"] = TreeChartDirective
vnv_directives["vnv-terminal"] = TerminalDirective


def setup(sapp):
    sapp.add_node(VnVChartNode, VnVChartNode.NODE_VISITORS)
    for key, value in vnv_directives.items():
        sapp.add_directive(key, value)


