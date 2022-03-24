import json

from sphinx.util.docutils import SphinxDirective

from .charts import VnVChartNode
from .jmes import get_target_node

self_access_default = {

    "Development" : {
        "description" : "Improving scientific software development—a process of writing, maintaining, and extending code—in order to increase software productivity, quality, and sustainability, as key aspects of advancing overall scientific productivity.",
        "subgoals" : {
            "Revision Control" : {
                 "description" : "Approaches for managing changes to files (source code, documentation, data) as well as maintaining their history and attribution, especially across multiple contributors and branches of development (also known as version control).",
                 "scores" : ["Not Currently Being Used", "Code is checked in to a revision control system, such as Git","Developers make use of pull requests and multiple branches, etc.","Has a documented workflow for merging code for quality assurance and releases"],
                 "score" : 0
            },
            "Code Reviews" : {
                "description" : "The act of consciously and systematically convening with one's fellow programmers to check each other's code for mistakes, and has been repeatedly shown to accelerate and streamline the process of software development like few other practices can.",
                "scores" :   ["Not Currently Being Used","Ad-hoc code reviews","Regular code reviews (e.g., weekly meetings)","Documented process for code reviews to be automated in workflow (e.g., via pull requests)"],
                "score" : 0
            },
            "Issue Tracking" : {
                "description" : "The process of capturing, reporting, tracking, and managing information about issues related to software. Issues include bugs, feature requests, missing documentation, and other problems and/or requirements.",
                "scores" :   ["Not Currently Being Used","Ad-hoc issue tracking","Dedicated issue tracking system consistently being used","Documented process for how issue tracking is done (e.g. Kanban board, assigned responsibilities)"],
                "score" : 0
            },
            "Deployment" : {
                "description" : "Approaches for versioning, packaging, releasing, and deploying software, documentation, or data for users to then obtain, install, and use.",
                "scores" :  ["Not Currently Being Used","Manual deployment (e.g., using a script or checklist)","Deployment as part of development workflow with manual intervention","Continuous deployment (or deployment combined with continuous integration)"],
                "score" : 0
            },
            "Documentation" : {
                "description" : "Creating, maintaining, and hosting quality documentation (written text or illustration) about the use, operation, or design of software.",
                "scores" : ["Not Currently Being Used","Informally maintained or ad-hoc text files","Code and documentation are cross-referenced and updated when committed to repository","Integrated with the software release workflow"],
                "score" : 0
            }
        }
    },
    "Planning" : {
        "description" : "The software development life cycle is the process of dividing software development work into distinct phases to improve design, product management, and project management.",
        "subgoals" : {
            "Development Process" : {
                 "description" : "Approaches for managing changes to files (source code, documentation, data) as well as maintaining their history and attribution, especially across multiple contributors and branches of development (also known as version control).",
                 "scores" : ["Not Currently Being Used", "Has development process but it is based on ad-hoc rules","Employs a basic iterative development process","Follows a responsive, flexible development methodology (e.g. Agile)"],

                 "score" : 0
            },
            "Contribution Management" : {
                "description" : "Tracking and managing contributions to software.",
                "scores" : ["Not Currently Being Used","Development guidelines include requirements gathering","Formal requirements gathering is undertaken as part of the project","Formally tracked and documented through a requirements management system"],
                "score" : 0
            },
            "Requirements Analysis" : {
                "description" : "Statements about what functions a software product should perform, including any constraints under which it should operate but avoiding as much as possible implementation details.",
                "scores" :   ["Not Currently Being Used","Development guidelines include design in the process","A modeling language is employed for key aspects of the project","Visual modeling using a graphical representation to capture design"],
                "score" : 0
            },
            "Software Design" : {
                "description" : "Major considerations in designing software to meet its scientific objectives and sustainability goals. Software design usually follows from requirements specification, and involves problem solving and planning a software solution.",
                "scores" :   ["Not Currently Being Used","Development guidelines include requirements gathering","Formal requirements gathering is undertaken as part of the project","Formally tracked and documented through a requirements management system"],

                "score" : 0
            },
            "Onboarding" : {
                "description" : "The process of integrating a new employee into an organization or a new team member into a team.",
                "scores" :   ["Not Currently Being Used","Consistent onboarding process is followed","Minimally documented onboarding process","Fully documented onboarding process is used for all personnel changes"],
                "score" : 0
            },
            "Offboarding" : {
                "description" : "The process of transferring employee knowledge to the broader team or organization.",
                "scores" :   ["Not Currently Being Used","Consistent offboarding process is followed","Minimally documented offboarding process","Fully documented offboarding process is used for all personnel changes"],
                "score" : 0
            }
        }
    },
     "Performance" : {
        "description" : "Improving strategies for writing scientific software that is efficient, scalable, and portable—from laptops to emerging extreme-scale architectures—while preserving other code quality metrics such as correctness and usability.",
        "subgoals" : {
            "Testing" : {
                 "description" : "Determining the speed, scalability, reliability, and stability of an application under a variety of workloads.",
                 "scores" :   ["Not Currently Being Used", "Conducts performance tests","Automated performance tests are run regularly","Uses performance profiling tools to guide performance-related improvements"],
                 "score" : 0
            },
            "Regression" : {
                 "description" : "Undertaking testing that exercises performance capabilities combined with plans for how to make practical use of that performance data",
                 "scores" :["Not Currently Being Used","A testable primary use case and set of measurements are established","The performance history of the tests are tracked over time","Performance tracking is run and reviewed at regular intervals on relevant platforms"],
                 "score" : 0
            },
            "Portability" : {
                 "description" : "Software exhibiting similar performance across multiple platforms, with the time to solution reflecting efficient utilization of computational resources on each platform.",
                 "scores" :["Not Currently Being Used","Utilize a standard parallel programming model (e.g. MPI)","Utilize a programming model designed for portability (e.g. OpenACC, OpenMP)","Re-writing the application to utilize an advanced portability library (e.g. Kokkos)"],
                 "score" : 0
            }
        }
    },
     "Reliability" : {
        "description" : "Improving methods of testing and verification to ensure that software is robust and produces reliable results, thereby helping to ensure the integrity of research and enable collaboration across teams.",
        "subgoals" : {
            "Testing" : {
                 "description" : "Software exhibiting similar performance across multiple platforms, with the time to solution reflecting efficient utilization of computational resources on each platform.",
                 "scores" :  ["Not Currently Being Used", "Comparison used to create system-level no-change tests","Unit testing for refactored and new code","Continuous integration"],
                 "score" : 0
            },
            "Reproducability" : {
                 "description" : "Any effort whose goal is to increase trustworthiness and reuse of computational capabilities and results and to ensure correctness.",
                 "scores" : ["Not Currently Being Used","Publication of code","Inclusion of data when code is published","The entire provenance of the application and its execution environment is published"],
                 "score" : 0
            },
            "Verification and Validation" : {
                 "description" : "Any effort whose goal is to increase trustworthiness of the results when compared to the physical requirements.",
                 "scores" : ["Not Currently Being Used","Eyeball Verification","Use the Verification and Validation toolkit ","Automated Verification and validation pipeline"],
                 "score" : 0
            }
        }
    }
}

ptc_default =  [
  {
                    "process" : "Start PSIP Journey",
                    "target" : "This PSIP Card focusing on integrating PSIP into your workflow.",
                    "score" : 0,
                    "scores" : [ "No PSIP Integration", "Some PSIP Integration" , "A lot of PSIP Integration", "Woah -- All the PSIP integration" ]
  }
]

def GET_DEFAULT_PSIP():
    return json.dumps({"sa":self_access_default, "ptc": ptc_default})

class PSIPDirective(SphinxDirective):

    PSIP_SCHEMA_URI="http://localhost:5000/static/assets/psip/schema.json"
    required_arguments = 1
    optional_arguments = 0
    file_argument_whitespace = True
    has_content = False
    option_spec = {
        "height": str,
        "width": str,
        "class" : str
    }

    def register(self):
        return None

    def getScript(self): return self.script_template

    def getHtml(self):

        return f'''
          <div class="{self.options.get("class","")}" style="width:{self.options.get("width","100%")}; height:{self.options.get("height" , "100%")};">
               {{%with initial_data= data.query_json('{" ".join(self.arguments)}'), readonly=True %}}
               
               {{%include 'psip/selfaccess.html'%}}
               <script>
                  a = {{{{initial_data|safe}}}}
                  set_psip_data(a,-1)
               </script>      
               {{%endwith%}} 
         </div>
        '''

    def run(self):
        target, target_id = get_target_node(self)
        block = VnVChartNode(html=self.getHtml())
        return [target, block]


    def register(self):
        return self.getContent()


def setup(sapp):
    sapp.add_directive("vnv-psip", PSIPDirective)
