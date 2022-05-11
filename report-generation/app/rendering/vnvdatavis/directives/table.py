import json

from sphinx.directives import optional_int

from .charts import TableChartDirective
from .jmes import jmes_jinja_query_json


def jmes_query(x):
    return x


def json_array(x):
    return json.loads(x)


class QuickTableChartDirective(TableChartDirective):
    required_arguments = 0
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = False
    option_spec = {
        "width": optional_int,
        "height": optional_int,
        "names": json_array,
        "fields": json_array,
        "data": jmes_query,
        "title": str
    }

    def columns(self):
        names = self.options.get("names", [])
        fields = self.options.get("fields", [])

        r = []
        for n, i in enumerate(names):
            r.append(f'''{{title:"{i}", field:"{fields[n]}"}}''')

        return ",".join(r)

    def register(self):
        return jmes_jinja_query_json(self.options.get("data", "`[]`"))

    def getHtml(self, id_, uid):
        return f'''
         <div class='card' style='padding:20px;'>
            <div id="v-{id_}" class='vnv-table' width="{self.options.get("width", 400)}" height="{self.options.get("height", 400)}"></div>
         </div>   
            <script>
              $(document).ready(function() {{
                var table = new Tabulator("#v-{id_}", {{
                      layout : "fitDataStretch",
                      "title" : "{self.options.get("title", "VnV Table")}",
                      "columns": [{self.columns()}]
                }});
                
                var url = "/directives/updates/{uid}/{{{{data.getFile()}}}}/{{{{data.getAAId()}}}}"
                update_now(url, "v-{id_}", 3000, function(config) {{
                  table.setData(JSON.parse(config))
                }})
                
            }})
            </script>
            '''


class DataTableChartDirective(TableChartDirective):
    required_arguments = 0
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = False
    option_spec = {
        "width": str,
        "height": str,
        "query": str,
        "title": str
    }

    def register(self):
        return self.getContent()

    def getRawContent(self):
        return json.dumps(self.options)

    def register(self):
        return jmes_jinja_query_json(self.options.get("data", "`[]`"))

    def getHtml(self, id_, uid):
        return f'''
         <div class='card' style='padding:20px;'>
            <div id="{uid}" 
                 class='vnv-table' 
                 style="width:{self.options.get("width", "auto")};height:{self.options.get("height", "auto")}">
            </div> 
         </div>   
          <script>
             $(document).ready(function() {{
                var table = new Tabulator("#{uid}", {{
                      layout : "fitDataStretch",
                      "title" : "{self.options.get("title", "VnV Table")}",
                      "columns": [{self.columns()}]
             }});

             var url = "/directives/updates/{uid}/{{{{data.getFile()}}}}/{{{{data.getAAId()}}}}"
             update_now(url, "{id_}", 3000, function(config) {{
                table.setData(JSON.parse(config))
             }})

            }})
            </script>
            '''


def setup(sapp):
    sapp.add_directive("vnv-quick-table", QuickTableChartDirective)
