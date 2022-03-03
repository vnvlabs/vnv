import json

from .apex import apex_post_process, ApexChartDirective, ApexOptionsDict



def chartsjs_post_process(text, data, file):
    return apex_post_process(text,data,file,None)


loading = {
    "series": [70],
    "chart": {
          "height": 350,
          "type": "radialBar",
    },
    "plotOptions": {
      "radialBar": {
         "hollow": {
            "size": "70%",
         }
      },
    },
    "labels": ["Loading"],
}

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
         update_now(url, "{id_}_container", 1000, function(config) {{
            myChart.config = JSON.parse(config)
            myChart.update()  
         }})
       }})
       </script>
       '''

class ChartsJsChartDirective(ApexChartDirective):

    script_template = '''
         <div>
           <div id="{uid}_container" class='vnv-table'>
              <canvas id="{uid}"></canvas>
           </div>
    
           <div class="apex-error-main" id="{uid}_errors" 
               style="color:orangered; font-size:30px; width:40px; height:40px; position:absolute; top:3px; right:3px; cursor:pointer; display:none">
               <i onclick="$('#{uid}_error_message').toggle()" class="feather icon-alert-triangle" ></i>
           </div>
           <div class="card" id ="{uid}_error_message" style="display:none; position:absolute; margin:20px; padding=20px; z-index:1000; top:43px; right:43px;"></div>   
         </div> 
         <script>
           $(document).ready(function() {{
             var ctx = document.getElementById('{uid}');
             var myChart = new Chart(ctx, {loading});

             url = "/directives/updates/{uid}/{{{{data.getFile()}}}}/{{{{data.getAAId()}}}}{context}"
             update_now(url, "{uid}_container", 1000, function(config) {{
                 z = JSON.parse(config)
                 myChart.config = z 
                 myChart.update()  
                 if (z["errors"]) {{
                    $('#{uid}_errors').show()   
                    $('#{uid}_error_message').html(z["errors"])
                  }} else {{
                    $('#{uid}_errors').hide()   
                }}
             }})
          }})
          </script>
        '''

    def getContext(self):
        return ""

    def getLoading(self):
        return loading

    def register(self):
        return self.getContent()


class ChartsJsDirec(ChartsJsChartDirective):
    required_arguments = 0
    optional_arguments = 0
    file_argument_whitespace = False
    has_content = False
    option_spec = ApexOptionsDict()

    def getContext(self):
        return "?context=jscharts"

    def register(self):
        return self.getContent()

    def getRawContent(self):
        return json.dumps(self.options)


def setup(sapp):
    sapp.add_directive("vnv-charts-js-raw", ChartsJsChartDirective)
    sapp.add_directive("vnv-charts-js", ChartsJsDirec)
    sapp.add_directive("vnv-chart", ChartsJsChartDirective)

