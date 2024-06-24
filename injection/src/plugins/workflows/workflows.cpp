#include "base/stores/WorkflowStore.h"
#include "interfaces/IWorkflow.h"

/**
 * A simple job Creator that runs the example.
 *
 */
INJECTION_WORKFLOW(VNVPACKAGENAME, Simple, "{}") {
  
  auto config = json::object();
  auto scheduler = WorkflowStore::instance().getScheduler(VNVPACKAGENAME_S, "Bash", config);

  /****
   *
   * 1111 HGEGFSEG
   *
   */
  INJECTION_CREATE_JOB(job, "Sample Job 1", scheduler);
  job->addEnvironmentVariable("testEnv", "hello");
  job->setSetupScript("echo \"Hello World\"");
  job->setTeardownScript("echo \"Hello World from the Teardown script (hello=${testEnv})\"");
  job->setWorkingDirectory("/home/ben/source/vv/vv-neams/build/examples/dummy/executables");
  job->setRunScript("VNV_RUN_ALIAS=Sample1 ./sample.a");

  auto fconfig = json::object();
  // Job is successfull only if the exit code is zero.
  job->addSuccessRequirement(WorkflowStore::instance().getValidator(VNVPACKAGENAME_S, "ExitCode", fconfig));

  // Add a dependency
  fconfig["src"] = "hello there -- this text will be written to file ";
  fconfig["dest"] = "does_this_work.txt";
  fconfig["wdir"] = job->getWorkingDirectory();
  job->addDepdendency(WorkflowStore::instance().getValidator(VNVPACKAGENAME_S, "FileStaging", fconfig));

  /****
   *
   * 22222 HGEGFSEG
   *
   */
  INJECTION_CREATE_JOB(job1, "Sample Job 2", scheduler);
  job1->setRunScript("echo sdfsdfsdf ");
  job1->setWorkingDirectory("/home/ben/source/vv/vv-neams/build/examples/dummy/executables");

  // Only runs once job1 is completed with an exit status of zero.
  fconfig.clear();
  fconfig["job"] = "Sample Job 1";
  job1->addDepdendency(WorkflowStore::instance().getValidator(VNVPACKAGENAME_S, "ExitCode", fconfig));

  /****
   *
   * 333333 HGEGFSEG
   *
   */
  INJECTION_CREATE_JOB(job3, "Sample Job 3", scheduler);
  job3->setRunScript("echo sdfsdfsdf ");
  job3->setWorkingDirectory("/home/ben/source/vv/vv-neams/build/examples/dummy/executables");
  job3->setRunBeforeMainApplication(false);  // Make it run after the application.

  /****
   *
   * 4444 HGEGFSEG
   *
   */
  INJECTION_CREATE_JOB(job4, "Sample Job 4", scheduler);
  job4->setRunScript("echo sdfsdfsdf ");
  job4->setWorkingDirectory("/home/ben/source/vv/vv-neams/build/examples/dummy/executables");
  job4->setRunBeforeMainApplication(false);  // Make it run after the application.

  fconfig.clear();
  fconfig["job"] = "Sample Job 3";
  job4->addDepdendency(WorkflowStore::instance().getValidator(VNVPACKAGENAME_S, "ExitCode", fconfig));
}
