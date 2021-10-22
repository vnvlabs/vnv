from app.rendering.vnvdatavis.directives.utils import JsonChartDirective

vnv_directives = {}

class PlotlyChartDirective(JsonChartDirective):
    script_template = '''
            <div id="{id_}" style="width:"{width}"; height:"{height}"></div>
            <script>
            {{
              const obj = JSON.parse('{config}')
              Plotly.newPlot('{id_}',obj['data'],obj['layout']);
              
              url = "/directives/updates/{uid}/{{{{data.getFile()}}}}/{{{{data.getAAId()}}}}"
              update_soon(url, "{id_}_container", 1000, function(config) {{
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
            update_soon(url, "{id_}_container", 1000, function(config) {{
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
               update_soon(url, "{id_}_container", 1000, function(config) {{
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
            update_soon(url, "{id_}_container", 3000, function(config) {{
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
           update_soon(url, "{id_}_container", 3000, function(config) {{
                var data = JSON.parse(config);
                var tree = new JSONFormatter(data['data'], true ,data['config']);
                document.getElementById('{id_}').innerHTML=''
                document.getElementById('{id_}').appendChild(tree.render());
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

def setup(sapp):
    for key, value in vnv_directives.items():
        sapp.add_directive(key, value)


