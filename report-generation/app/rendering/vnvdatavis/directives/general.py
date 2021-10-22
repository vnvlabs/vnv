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
from sphinx.directives.code import CodeBlock
from sphinx.errors import ExtensionError
from sphinx.util import nodes
from sphinx.util.docutils import SphinxDirective
import pygments
from pygments.lexers.data import JsonLexer
from pygments.formatters.html import HtmlFormatter

from app.base.blueprints import files as dddd

vnv_directives = {}
vnv_roles = {}
vnv_nodes = []
the_app = None

from app.rendering.vnvdatavis.directives.jmes import jmes_jinja_query_str, jmes_jinga_stat, DataClass, \
    jmes_jinja_codeblock, jmes_jinja_query, get_target_node, jmes_jinja_query_json


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


def jmes_text_node(text, meth):
    result = meth(text)
    return [VnVChartNode(html=result)]


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


vnv_roles["vnv"] = get_stats_role("str")
for f in DataClass.statsMethods:
    vnv_roles[f"vnv-{f}"] = get_stats_role(f)


class JmesStringDirective(SphinxDirective):
    required_arguments = 1
    optional_arguments = 0
    final_argument_whitespace = True
    option_spec = {}
    has_content = False

    def run(self):
        return process_query(" ".join(self.arguments), "str")


vnv_directives["vnv-print"] = JmesStringDirective


class JsonCodeBlockDirective(SphinxDirective):
    has_content = False
    required_arguments = 1
    optional_arguments = 0
    final_argument_whitespace = True
    option_spec = {}

    def run(self):
        return process_query(" ".join(self.arguments), "codeblock", tag="div")


vnv_directives["vnv-code"] = JsonCodeBlockDirective


class JsonChartDirective(SphinxDirective):

    def get_update_dir(self):
        dir = os.path.join(os.path.dirname(dddd.__file__), "templates", "renders", str(the_app.config.vnv_file), "updates")
        if not os.path.exists(dir):
            os.makedirs(dir)
        return dir

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
vnv_directives["vnv-apex"] = ApexChartDirective
vnv_directives["vnv-plotly"] = PlotlyChartDirective
vnv_directives["vnv-gchart"] = GChartChartDirective
vnv_directives["vnv-chart"] = ChartJsChartDirective
vnv_directives["vnv-table"] = TableChartDirective
vnv_directives["vnv-tree"] = TreeChartDirective
vnv_directives["vnv-terminal"] = TerminalDirective


root = os.path.join("app", "static", "assets")
js_root = os.path.join(root, "js", "plugins")
css_root = os.path.join(root, "css", "plugins")

vnv_js_assets = []
vnv_css_assets = []

vnv_js_assets.append(os.path.join(js_root, "apexcharts.min.js"))
vnv_css_assets.append(os.path.join(css_root, "apexcharts.css"))
vnv_js_assets.append(os.path.join(js_root, "plotly.min.js"))
vnv_js_assets.append(os.path.join(js_root, "gcharts.min.js"))
vnv_js_assets.append(os.path.join(js_root, "Chart.min.js"))
vnv_css_assets.append(os.path.join(css_root, "Chart.min.css"))
vnv_js_assets.append(os.path.join(js_root, "tabulator.min.js"))
vnv_css_assets.append(os.path.join(css_root, "tabulator.min.css"))
vnv_js_assets.append(os.path.join(js_root, "json-formatter.umd.js"))
vnv_css_assets.append(os.path.join(css_root, "json-formatter.css"))


#####################  Image #######################

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


def setup(sapp):
    for node in vnv_nodes:
        sapp.add_node(node, **node.NODE_VISITORS)
    for key, value in vnv_roles.items():
        sapp.add_role(key, value)
    for key, value in vnv_directives.items():
        sapp.add_directive(key, value)
    for file in vnv_js_assets:
        sapp.add_js_file(file)
    for file in vnv_css_assets:
        sapp.add_css_file(file)

    global the_app
    the_app = sapp
