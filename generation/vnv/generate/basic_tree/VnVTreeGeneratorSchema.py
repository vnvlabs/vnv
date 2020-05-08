#!/usr/bin/env python
# -*- coding: utf-8 -*-
import json
from jsonschema import validate


vnvTreeSchema = {
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "properties": {
    "title": {
      "type": "string"
    },
    "templates" : {
       "type" : "object",
       "aditionalProperties" : {
           "type" : "object",
           "additionalProperties" : {
                "type" : "object",
                "additionalProperties" : {
                   "type" : "object",
                   "oneOf" : [ {
                      "properties" : {
                         "file" : {"type" : "string" },
                      },
                      "required" : ["file"],
                      "additionalProperties" : False
                      },{
                      "properties" : {
                         "text" : {"type" : "string" },
                      },
                      "required" : ["text"],
                      "additionalProperties" : False
                      }
                   ]
               }
          }
       }
    },
    "bib": {
      "type": "object",
      "properties": {
        "Author": {
          "type": "string"
        },
        "Authors": {
          "type": "string"
        },
        "Organization": {
          "type": "string"
        },
        "Contact": {
          "type": "string"
        },
        "Address": {
          "type": "string"
        },
        "Version": {
          "type": "string"
        },
        "Status": {
          "type": "string"
        },
        "Date": {
          "type": "string"
        },
        "Copyright": {
          "type": "string"
        },
        "Abstract": {
          "type": "string"
        },
        "Dedication": {
          "type": "string"
        }
      },
      "additionalProperties": False
    },
    "beforeRootNode": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "filename": {
            "type": "string"
          },
          "text": {
            "type": "string"
          }
        }
      }
    },
    "afterRootNode": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "filename": {
            "type": "string"
          },
          "text": {
            "type": "string"
          }
        }
      }
    },
    "rootNode": {
      "type": "object",
      "properties": {
        "title": {
          "type": "string"
        },
        "filename": {
          "type": "string"
        },
        "infoNode": {
          "$ref": "#/definitions/infoNode"
        },
        "unitTests": {
          "$ref": "#/definitions/UnitTestsNode"
        },
        "children": {
          "$ref": "#/definitions/ChildrenNode"
        }
      }
    }
  },
  "definitions": {
    "infoNode": {
      "type": "object",
      "properties": {
        "filename": {
          "type": "string"
        },
        "display": {
          "type": "boolean"
        },
        "title": {
          "type": "string"
        },
        "template": {
          "type": "string"
        }
      }
    },
    "UnitTestsNode": {
      "type": "object",
      "properties": {
        "filename": {
          "type": "string"
        },
        "display": {
          "type": "boolean"
        },
        "title": {
          "type": "string"
        },
        "packages": {
          "type": "object",
          "additonalProperties": {
            "$ref": "#/definitions/PackageUnitsTestNode"
          }
        }
      }
    },
    "PackageUnitTestsNode": {
      "type": "object",
      "properties": {
        "filename": {
          "type": "string"
        },
        "display": {
          "type": "boolean"
        },
        "title": {
          "type": "string"
        },
        "unitTests": {
          "type": "object",
          "additonalProperties": {
            "$ref": "#/definitions/UnitTestNode"
          }
        }
      }
    },
    "ChildrenNode": {
      "type": "object",
      "properties": {
        "filename": {
          "type": "string"
        },
        "title": {
          "type": "string"
        },
        "display": {
          "type": "boolean"
        },
        "children": {
          "type": "object",
          "additonalProperties": {
            "anyOf": [
              {
                "$ref": "#/definitions/LogNode"
              },
              {
                "$ref": "#/definitions/InjectionPointNode"
              }
            ]
          }
        }
      },
      "UnitTestNode": {
        "type": "object",
        "properties": {
          "filename": {
            "type": "string"
          },
          "title": {
            "type": "string"
          },
          "display": {
            "type": "boolean"
          },
          "children": {
            "$ref": "#/defintitions/ChildrenNode"
          },
          "template": {
            "type": "string"
          }
        }
      },
      "LogNode": {
        "type": "object",
        "properties": {
          "display": {
            "type": "boolean"
          }
        }
      },
      "InjectionPointNode": {
        "type": "object",
        "properties": {
          "display": {
            "type": "boolean"
          },
          "children": {
            "$ref": "#/definitions/ChildrenNode"
          },
          "tests": {
            "$ref": "#/definitions/TestsNode"
          }
        }
      },
      "TestNode": {
        "type": "object",
        "properties": {
          "filename": {
            "type": "string"
          },
          "title": {
            "type": "string"
          },
          "display": {
            "type": "boolean"
          },
          "template": {
            "type": "string"
          },
          "children" : {
             "$ref" : "#/definitions/ChildrenNode"
          }
        }
      },
      "TestsNode": {
        "type": "object",
        "properties": {
          "filename": {
            "type": "string"
          },
          "display": {
            "type": "boolean"
          },
          "title": {
            "type": "string"
          },
          "children": {
            "type": "object",
            "additonalProperties": {
              "$ref": "#/definitions/TestNode"
            }
          }
        }
      }
    }
  }
}

defaultIntro = {
"title" : "<title>",
"filename" : "index",
"bib" : {
"Author" : "<author>",
"Organization" : "<organization>",
"Contact" : "<contact>",
"Address" : "<address>",
"Version" : "<version>",
"Status" : "<status>",
"Date" : "<date>",
"Copyright" : "<copyright>",
"Abstract" : "<abstract>",
"Dedication" : "<dedication>"
},
"beforeRootNode" : [
{
 "filename" : "Introduction",
 "text" : "VnV Simulation Report"
}
],
"afterRootNode" : [
{
 "filename" : "Conclusion",
 "text" : "End VnV Simulation Report"
},
],
"rootNode" : {
"title" : "VnV Root Node",
"filename" : "RootNode",
"infoNode" : {
  "title" : "Runtime Information",
  "filename" : "Information",
  "display" : True,
  "text" : "Title: :vnv:`.Title`\nDate: :vnv:`.Date`"
},
"unitTests": {
  "title" : "Runtime Unit Tests",
  "filename" : "UnitTests",
  "display" : True,
  "packages" : {
  }
},
"children":{
  "title" : "Runtime Children",
  "filename" : "Children",
  "display" : True,
  "children" : {

  }

}

}
}

vnvStarterTemplate = '''
.. T documentation master file, created by
   sphinx-quickstart (using VnV Tree) on {date}.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to T's documentation!
=============================

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   {rootNode}

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
'''




def get():
     return json.loads(json.dumps(vnvTreeSchema))

def validateJson(input):
    validate(instance=input, schema=get())
