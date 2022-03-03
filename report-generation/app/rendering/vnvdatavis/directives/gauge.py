
from .jmes import jmes_jinja_percentage
from .apex import ApexChartDirective


class ApexGaugeDirective(ApexChartDirective):
    required_arguments = 0
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = False
    option_spec = {
        'min': str,
        'max': str,
        'curr': str,
        'title': str
    }

    def g(self):
        mi = self.options.get("min", "`0`")
        cu = self.options.get("curr", "`50`")
        ma = self.options.get("max", "`100`");
        return jmes_jinja_percentage(cu,mi,ma)

    def getContent(self):
        return f'''
         {{
          "series": [{self.g()}],
          "chart": {{
            "type": "radialBar",
            "width" : "100%",
            "height" : "100%"
          }},
          "plotOptions": {{
            "radialBar": {{
              "hollow": {{
                "size": "70%"
              }}
            }}
          }},
          "labels": ["{self.options.get("title","VnV Gauge Chart")}"]
        }}
        '''

def setup(sapp):
    sapp.add_directive("vnv-gauge", ApexGaugeDirective)
