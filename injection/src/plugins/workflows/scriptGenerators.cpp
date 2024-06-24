

#include <chrono>

#include "shared/DistUtils.h"
#include "shared/Utilities.h"
#include "base/stores/WorkflowStore.h"
#include "interfaces/IWorkflow.h"

INJECTION_INJA_TEMPLATE(VNVPACKAGENAME, Template, R"({"type":"object","properties":{"children":{"type":"array"}}})",
R"(Roll Call:
## for child in children
  Hello {{ child }}.
## endfor    
)");

INJECTION_INJA_TEMPLATE(VNVPACKAGENAME, SimpleSlurmHeader,

                        R"(

{   
    "type" : "object",
    "properties" : {
        "jobName" : {"type" : "string"},
        "hello" : {"type" : "string" },
        "email" : {"type" : "string" },
        "tasks" : {"type" : "integer" },
        "memory" : {"type" : "integer" },
        "walltime" : {"type" : "string" },
        "stdout" : {"type" : "string" },    
        "modules" : {"type" : "array", "items" : {"type" : "string" } },
        "notify" : {"type" : "array", "items" : {"type" : "string" } }
    },
    "required" : ["name","hello","email","tasks","memory","walltime","stdout","modules","notify"]
}

)",
                        R"(

#!/bin/bash
#SBATCH --job-name={{jobName}}            # Job name
#SBATCH --mail-type={{join(notify,",")}}  # Mail events (NONE, BEGIN, END, FAIL, ALL)
#SBATCH --mail-user={{email}}             # Where to send mail	
#SBATCH --ntasks={{tasks}}                # Run on a single CPU
#SBATCH --mem={{memory}}gb                # Job memory request
#SBATCH --time={{walltime}}               # Time limit hrs:min:sec
#SBATCH --output={{stdout}}               # Standard output and error log

pwd; hostname; date

## for module in modules
   module load {{module}}
## endfor

echo {{ hello }})");