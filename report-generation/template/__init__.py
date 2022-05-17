import uuid

from flask import Blueprint
from flask import render_template, url_for, request


#This is the minimal example of a vnv compatible blueprint that can be added into the main
#gui through the input file. The blueprint must match this template to ensure that it all
#works together with all the other blueprints.

#Step 0:

##### Step 1 -- CHANGE THIS NAME TO BE UNIQUE TO THIS BLUEPRINT
TEMPLATE_NAME="###TEMPLATENAME###"

# Blueprint must be named "blueprint"
blueprint = Blueprint(
    TEMPLATE_NAME,
    __name__,
    template_folder='templates',
    static_folder="static",
    url_prefix="/" + TEMPLATE_NAME
)

#Get the full route


def get_route(function, **kwargs):
    return url_for("base." + TEMPLATE_NAME + "." + function, **kwargs)

# Get the full url path
def get_url(path):
    return "/" + TEMPLATE_NAME + path

# Get the url path for a static file.
def get_static_raw(path):
    return url_for("base." + TEMPLATE_NAME + ".static", filename=path)

def get_template_file(filename):
    return get_file(TEMPLATE_NAME + "/" + filename)

# Render a file inside the VnV GUI
def get_file(path):
    return get_route("segment", content=path)



# VnV Will call this function. Whatever is entered here can be used
# globally in templates within the {{...}} commands.
def template_globals(global_template_dict):
    global_template_dict[TEMPLATE_NAME] = {
        "route" : get_route,
        "url" : get_url,
        "static" : get_static_raw,
        "file": get_file
    }

## Default route to render template file within the VnV Gui
@blueprint.route("fetch")
def segment():
   try:
    content = request.args.get("content")
    return render_template(content)
   except:
       return render_template("includes/page-not-found.html")
