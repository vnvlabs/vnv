class ApexScatterChartDirective(ApexChartDirective):
    required_arguments = 0
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = False
    option_spec = {
        'xdata': str,
        'ydata': str,
        'label': str,
        'title': str,
    }

    def getData(self):
        return jmes_jinja_zip({"x": self.options.get("xdata", '`[]`'), "y": self.options.get("ydata", "`[]`")})
    def getContent(self):
        return f'''
        {{
          "series": [
            {{
                "name": "{self.options.get("label", "Series 1")}",
                "data": {self.getData()}
            }}
          ],
          "chart": {{
                 "width" : "100%",
                "height" : "100%",  
             "type" : "scatter", 
             "zoom": {{
                 "enabled": true,
                 "type": "xy"
             }}
          }},
          "xaxis": {{
              "tickAmount": 10
          }},
          "yaxis": {{
            "tickAmount": 7
          }}
       }}
       '''

