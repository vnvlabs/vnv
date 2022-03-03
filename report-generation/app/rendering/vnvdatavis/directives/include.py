import os

from flask import render_template_string, render_template

from .charts import JsonChartDirective


def post_process_include(text, data, file ):
    fileName = render_template_string(text, data=data, file=file)
    path = file.connection.download(fileName)

    if os.path.exists(path):
        with open(path,'r') as f:
            # Convert RST to HTML Flask Template
            rendered_template = file.render_to_string(f.read())

            # Convert Flask Template to Full HTML
            config = render_template_string(rendered_template, data=data)

            # Chuck it inside an iframe
            return render_template("viewers/rst_render.html", content=config);

    return "Could not open File " + fileName


class IncludeRSTDirective(JsonChartDirective):

    required_arguments = 1
    file_argument_whitespace = True
    has_content = False

    script_template = '''
          <div id="{uid}" class='vnv-table'>
             Loading...
          </div>
          <script>
          $(document).ready(function() {{
            
            url = "/directives/updates/{uid}/{{{{data.getFile()}}}}/{{{{data.getAAId()}}}}?context=include"
            update_now(url, "{uid}", 1000, function(config) {{
                $('#{uid}').html(config)
            }})
          }})
          </script>
        '''

    def getRawContent(self):
        return "\n".join(self.arguments)

    def getHtml(self, id_, uid):
        return f'''
          <div class="{self.options.get("class", "")}" style="width:{self.options.get("width", "100%")}; height:{self.options.get("height", "100%")};">{
                self.getScript().format(uid=uid)}
          </div>
        '''
def setup(sapp):
    sapp.add_directive("vnv-include-rst", IncludeRSTDirective)
