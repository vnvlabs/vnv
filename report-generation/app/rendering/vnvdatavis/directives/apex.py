import json
import os.path

import jsonschema
from flask import render_template_string
from sphinx.errors import ExtensionError

from .charts import JsonChartDirective
from collections.abc import MutableMapping

def dict_pop(d):

    if isinstance(d,dict):
        pops = []
        for k,v in d.items():
            if isinstance(v, dict) and "kind" in v:
                pops.append(k)
            else:
                dict_pop(v)
        for i in pops:
            d.pop(i)

    elif isinstance(d,list):
        pops = []
        for v in d:
            if isinstance(v, dict) and "kind" in v:
                pops.append(v)
            else:
                dict_pop(v)
        for i in pops:
            d.remove(i)

try:
    apex_schema
except:
    with open(os.path.join(os.path.dirname(__file__), "apex-schema.json")) as f:
        apex_schema = json.load(f)
    dict_pop(apex_schema)

def apex_post_process(text, data, file, schema = apex_schema):
  try:
    # Extract all the trace definitions -- trace.x = scatter trace.y = line
    # Turn it into an object
    rdata = {}
    options = json.loads(text)

    class DictOrOpt:
        def __init__(self,parent, key):
            self.parent = parent
            self.key = key

    apex_opts = {}
    for k, v in options.items():
        value = json.loads(render_template_string(v,data=data,file=file))
        a = k.split('.')
        last = apex_opts
        lastParent = None
        lastKey = None
        for i in a:

            if isinstance(last,dict):
               if i not in last:
                   last[i] = DictOrOpt(last, i)

               lastParent = last
               lastKey = i
               last = last[i]

            elif isinstance(last,list) and i.isnumeric():

               ind = int(i)
               while len(last) <= ind:
                   last.append(DictOrOpt(last, len(last)))

               lastParent = last
               lastKey = ind
               last = last[ind]
            elif isinstance(last,list):
                raise ExtensionError("Non numeric tag passed to array.")
            elif isinstance(last,DictOrOpt):

                if i.isnumeric():
                    ind = int(i)
                    a = []
                    while len(a) <= ind:
                        a.append(DictOrOpt(a, len(a)))

                    last.parent[last.key] = a

                    lastParent = a
                    lastKey = ind
                    last = a[ind]
                else:
                    a = {}
                    a[i] = DictOrOpt(a,i)
                    last.parent[last.key] = a

                    lastParent = a
                    lastKey = i
                    last = a[i]
            else:
                raise ExtensionError("What")

        lastParent[lastKey] = value
    if schema:
        try:
            jsonschema.validate(apex_opts,schema)
        except Exception as e:
            apex_opts["errors"] = str(e)

    return json.dumps(apex_opts)

  except Exception as e:
      print(e)

class ApexOptionsDict(MutableMapping):
    """A dictionary that calls a function when a requested key does not
    exist in the dictionary"""

    def __init__(self):
        self.store = {
            "height": str,
            "width": str,
        }

    def __getitem__(self, key):
        if key in self.store:
            return self.store[key]
        return str

    def __setitem__(self, key, value):
        self.store[key] = value

    def __delitem__(self, key):
        del self.store[key]

    def __iter__(self):
        return iter(self.store)

    def __len__(self):
        return len(self.store)


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

class ApexChartDirective(JsonChartDirective):
    script_template = '''
         <div>
           <div id="{uid}" class='vnv-table'></div>
          
           <div class="apex-error-main" id="{uid}_errors" 
               style="color:orangered; font-size:30px; width:40px; height:40px; position:absolute; top:3px; right:3px; cursor:pointer; display:none">
            <i onclick="$('#{uid}_error_message').toggle()" class="feather icon-alert-triangle" ></i>
           </div>
           <div class="card" id ="{uid}_error_message" style="display:none; position:absolute; margin:20px; padding=20px; z-index:1000; top:43px; right:43px;"></div>   
         </div> 
          <script>
          $(document).ready(function() {{
            var chart = new ApexCharts(document.getElementById("{uid}"), {loading});
            chart.render();
            
            url = "/directives/updates/{uid}/{{{{data.getFile()}}}}/{{{{data.getAAId()}}}}{context}"
            update_now(url, "{uid}", 1000, function(config) {{
                z = JSON.parse(config)
                chart.updateOptions(z) 
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

    def getHtml(self, id_, uid):
        return f'''
          <div class="{self.options.get("class", "")}" style="width:{self.options.get("width", "100%")}; height:{self.options.get("height", "100%")};">{self.getScript().format(
            id_=id_,
            config=self.getContent(),
            uid=uid,
            loading=json.dumps(loading),
            context=self.getContext()
        )}</div>
        '''

    def register(self):
        return self.getContent()


class ApexDirec(ApexChartDirective):
    required_arguments = 0
    optional_arguments = 0
    file_argument_whitespace = False
    has_content = False
    option_spec = ApexOptionsDict()

    def getContext(self):
        return "?context=apex"

    def register(self):
        return self.getContent()

    def getRawContent(self):
        return json.dumps(self.options)


def setup(sapp):
    sapp.add_directive("vnv-apex", ApexDirec)
    sapp.add_directive("vnv-apex-raw", ApexChartDirective)
