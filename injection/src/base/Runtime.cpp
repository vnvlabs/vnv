
/** @file Runtime.cpp Implementation of the Runtime class as defined in
 * base/Runtime.h"
 **/

#include "base/Runtime.h"

#include <unistd.h>
#include <stdlib.h>

#include <iostream>

#include "shared/DistUtils.h"
#include "shared/Provenance.h"
#include "shared/Utilities.h"
#include "base/parser/JsonSchema.h"
#include "base/stores/ActionStore.h"
#include "base/stores/CommunicationStore.h"
#include "base/stores/InjectionPointStore.h"
#include "base/stores/OptionsParserStore.h"
#include "base/stores/OutputEngineStore.h"
#include "base/stores/SamplerStore.h"
#include "base/stores/TestStore.h"
#include "base/stores/UnitTestStore.h"
#include "base/stores/WorkflowStore.h"
#include "common-interfaces/all.h"
#include "interfaces/IAction.h"
#include "interfaces/Injection.h"
#include "dist/DistUtils.h"
#include "shared/exceptions.h"

using namespace VnV;

INJECTION_OPTIONS(VNVPACKAGENAME, getBaseOptionsSchema().dump().c_str(), void)
{
  RunTime::instance().getRunTimeOptions()->fromJson(config);
  engine->Put("version", 1.0);
  return NULL;
}

void RunTime::resetStore() { stores.clear(); }

namespace
{

  ICommunicator_ptr getComm(VnV_Comm comm)
  {
    return CommunicationStore::instance().getCommunicator(comm);
  }
} // namespace

bool RunTime::loadPlugin(std::string libraryPath, std::string packageName)
{
  try
  {
    auto it = plugins.find(libraryPath);
    if (it == plugins.end())
    {
      void *dllib = DistUtils::loadLibrary(libraryPath);
      if (dllib != nullptr)
      {
        registrationCallBack reg = DistUtils::searchLibrary(dllib, VNV_GET_REGISTRATION + packageName);
        if (reg != nullptr)
        {
          VnV_Debug(VNVPACKAGENAME, "Loading Plugin %s with library %s", packageName.c_str(), libraryPath.c_str());

          runTimePackageRegistration(packageName, reg);
          return true;
        }
      }
      else
      {
        VnV_Debug(VNVPACKAGENAME, "Could Not Load Plugin %s with library %s", packageName.c_str(), libraryPath.c_str());
        return false;
      }
    }
    return true;
  }
  catch (std::exception &e)
  {
    throw INJECTION_EXCEPTION("Error Loading Plugin: Library not found: %s", libraryPath.c_str());
    return false;
  }
}

int RunTime::registerCleanUpAction(std::function<void(ICommunicator_ptr)> action)
{
  int i = cleanupActionCounter++;
  cleanupActions[i] = action;
  return i;
}

void RunTime::makeLibraryRegistrationCallbacks(std::map<std::string, std::string> packageNames)
{
  for (auto it : packageNames)
  {
    loadPlugin(it.second, it.first);
  }

  std::string home_dir = DistUtils::getEnvironmentVariable("HOME", StringUtils::random(5));

  std::ifstream ifs(home_dir + "/.vnv");
  if (ifs.good()) {
    try{
      json conf = json::parse(ifs);

      for (auto &it : conf["plugins"].items())
      {
        if (!loadPlugin(it.value(), it.key()))
        {
          VnV_Warn(VNVPACKAGENAME, "Could not load plugin: %s", it.key().c_str());
        }
      }
    } catch(...) {
      //std::cout << "Could not load the Plugins from ~/.vnv" << std::endl;
    }
  }
}

bool RunTime::useAsciiColors() { return terminalSupportsAsciiColors; }

void RunTime::logUnhandled(std::string name, std::string id, NTV &args)
{
  VnV_Info(VNVPACKAGENAME, "Name: %s", name.c_str());
  VnV_Info(VNVPACKAGENAME, "ID: %s", id.c_str());
  for (auto &it : args)
  {
    VnV_Info(VNVPACKAGENAME, "%s : (%s)", it.first.c_str(), it.second.first.c_str());
  }
}

void RunTime::writeSpecification(std::string filename, bool quit)
{
  std::ofstream ofs(filename);
  if (ofs.good())
  {
    ofs << getFullJsonSchema().dump(4);
  }
  if (quit)
  {
    std::exit(0);
  }
}

void dumpSpecification(bool quit)
{
  std::cout << "===START SCHEMA DUMP===" << std::endl;
  std::cout << RunTime::instance().getFullJsonSchema().dump(4);
  std::cout << "===END SCHEMA_DUMP===" << std::endl;
  if (quit)
  {
    std::exit(0);
  }
}

nlohmann::json RunTime::getFullJsonSchema()
{
  json packageJson = json::object();
  for (auto it : jsonCallbacks)
  {
    packageJson[it.first] = json::parse((it.second)());
  }

  json main = json::parse(getVVSchema().dump());
  json &defs = main["definitions"];

  defs["options"] = OptionsParserStore::instance().schema(packageJson);

  defs["injectionPoints"] = InjectionPointStore::instance().schema(packageJson);
  defs["test"] = TestStore::instance().schema(packageJson);

  defs["sampler"] = SamplerStore::instance().schema(packageJson);

  defs["outputEngine"] = OutputEngineStore::instance().schema(packageJson);
  defs["actions"] = ActionStore::instance().schema(packageJson);
  defs["unittest"] = UnitTestStore::instance().schema(packageJson);
  defs["workflows"] = WorkflowStore::instance().schema(packageJson);

  json j = json::parse(jsonCallbacks[mainPackageName]());
  defs["executable"] = j["Executables"];
  return main;
}

nlohmann::json RunTime::getFullJson()
{
  json main = json::object();

  for (auto &package : jsonCallbacks)
  {
    json j = json::parse(package.second());

    for (auto type : j.items())
    {
      // Add all the options and stuff
      json &mj = JsonUtilities::getOrCreate(main, type.key(), JsonUtilities::CreateType::Object);

      if (type.key() == "Options")
      {
        json jf = OptionsParserStore::instance().getSchema(package.first);
        jf["docs"] = type.value();
        mj[package.first] = jf;
      }
      else if (type.key() == "DataType" || type.key() == "Files")
      {
        for (auto &entry : type.value().items())
        {
          mj[package.first + ":" + entry.key()] = entry.value();
        }
      }
      else if (type.key() == "Introduction" || type.key() == "Conclusion")
      {
        if (package.first == mainPackageName)
        {
          mj["docs"] = type.value();
        }
      }
      else if (type.key().compare("InjectionPoints") == 0)
      {
        for (auto &entry : type.value().items())
        {
          if (InjectionPointStore::instance().registered(package.first, entry.key()))
          {
            mj[package.first + ":" + entry.key()] = entry.value();
          }
        }
      }
      else if (type.key().compare("Tests") == 0)
      {
        for (auto &entry : type.value().items())
        {
          if (InjectionPointStore::instance().registeredTest(package.first, entry.key()))
          {
            mj[package.first + ":" + entry.key()] = entry.value();
          }
        }
      }
      else if (type.key().compare("UnitTests") == 0)
      {
        if (info.unitTestInfo.runUnitTests)
        {
          for (auto &entry : type.value().items())
          {
            mj[package.first + ":" + entry.key()] = entry.value();
          }
        }
      }
      else if (type.key().compare("Actions") == 0)
      {
        for (auto &entry : type.value().items())
        {
          if (ActionStore::instance().registeredAction(package.first, entry.key()))
          {
            mj[package.first + ":" + entry.key()] = entry.value();
          }
          else
          {
          }
        }
      }
      else if (type.key().compare("Workflows") == 0)
      {
        for (auto &entry : type.value().items())
        {
          if (WorkflowStore::instance().registeredWorkflow(package.first, entry.key()))
          {
            mj[package.first + ":" + entry.key()] = entry.value();
          }
        }
      }
      else if (type.key().compare("CodeBlocks") == 0)
      {
        for (auto &entry : type.value().items())
        {
          mj[package.first + ":" + entry.key()] = entry.value();
        }
      }
    }
  }
  main.merge_patch(template_patch);
  return main;
}

RunTimeOptions *RunTime::getRunTimeOptions() { return &runTimeOptions; }

std::shared_ptr<InjectionPoint> RunTime::getNewInjectionPoint(VnV_Comm comm, std::string pname, std::string id,
                                                              struct VnV_Function_Sig pretty, InjectionPointType type,
                                                              NTV &args)
{
  if (runTests)
  {

    ActionStore::instance().injectionPointStart(getComm(comm), pname, id);

    std::shared_ptr<InjectionPoint> ipd =
        InjectionPointStore::instance().getNewInjectionPoint(pname, id, pretty, type, args);
    if (ipd != nullptr)
    {
      ipd->setInjectionPointType(type, "Begin");
      return ipd;
    }
    else if (runTimeOptions.logUnhandled)
    {
      logUnhandled(pname, id, args);
    }
  }
  return nullptr;
}

std::shared_ptr<InjectionPoint> RunTime::getExistingInjectionPoint(std::string pname, std::string id,
                                                                   InjectionPointType type, std::string stageId)
{
  if (runTests)
  {
    std::shared_ptr<InjectionPoint> ipd = InjectionPointStore::instance().getExistingInjectionPoint(pname, id, type);

    if (type == InjectionPointType::End)
    {
      ActionStore::instance().injectionPointEnd();
    }
    else
    {
      ActionStore::instance().injectionPointIter(stageId);
    }

    if (ipd != nullptr)
    {
      ipd->setInjectionPointType(type, stageId);
      return ipd;
    }
  }

  return nullptr;
}

void RunTime::injectionPoint_begin(VnV_Comm comm, std::string pname, std::string id, struct VnV_Function_Sig pretty,
                                   std::string fname, int line, const DataCallback &callback, NTV &args)
{
  if (runTests)
  {

    read_stdout();

    auto it = getNewInjectionPoint(comm, pname, id, pretty, InjectionPointType::Begin, args);
    if (it != nullptr)
    {
      it->setComm(getComm(comm));
      it->run(fname, line, callback);
    }
  }
}

void RunTime::injectionPoint_iter(std::string pname, std::string id, std::string stageId, std::string fname, int line,
                                  const DataCallback &callback)
{
  if (runTests)
  {

    read_stdout();

    auto it = getExistingInjectionPoint(pname, id, InjectionPointType::Iter, stageId);
    if (it != nullptr)
    {
      it->run(fname, line, callback);
    }
  }
}

void RunTime::injectionPoint_end(std::string pname, std::string id, std::string fname, int line,
                                 const DataCallback &callback)
{

  if (runTests)
  {
    read_stdout();
    auto it = getExistingInjectionPoint(pname, id, InjectionPointType::End, "End");
    if (it != nullptr)
    {
      it->run(fname, line, callback);
    }
  }
}

void RunTime::declarePackageJson(std::string pname, vnvFullJsonStrCallback callback)
{
  jsonCallbacks.insert(std::make_pair(pname, callback));
}

RunTime &RunTime::instance(bool reset)
{
  static RunTime store;
  if (reset)
  {
    store = RunTime();
  }
  return store;
}
RunTime &RunTime::instance() { return instance(false); }
RunTime &RunTime::reset() { return instance(true); }

RunTime::RunTime()
{
  // Set the workflow name and job ids.
  workflowName_ = DistUtils::getEnvironmentVariable("VNV_WORKFLOW_ID", StringUtils::random(5));
  workflowJob_ = StringUtils::random(5);
  start = std::chrono::steady_clock::now();
}

void RunTime::registerLogLevel(std::string packageName, std::string name, std::string color)
{
  if (runTests)
  {
    logger.registerLogLevel(packageName, name, color, true);
  }
}

void RunTime::registerFile(VnV_Comm comm, std::string packageName, std::string name, int input, std::string reader, std::string ifilename, std::string ofilename)
{
  if (runTests)
  {
    read_stdout();
    OutputEngineStore::instance().getEngineManager()->file(getComm(comm), packageName, name, input, reader, ifilename, ofilename);
  }
}

void RunTimeOptions::callback(json &j) { RunTime::instance().runTimeOptions.fromJson(j); }

void RunTimeOptions::fromJson(json &j)
{
  if (j.contains("logUnhandled"))
  {
    logUnhandled = j["logUnhandled"].get<bool>();
  }
}

void RunTime::getFullSchema(std::string filename) {}

void RunTime::writeRunInfoFile()
{
  // WorkflowName is fixed for for a particular "Job"
  // Workflow Job is unique to this example.

  std::string f = "vnv_" + workflowName() + "_" + workflowJob() + ".runInfo";

  std::string filename = DistUtils::join({workflowDir_, f}, 0777, false);
  std::ofstream ofs(filename);
  if (ofs.good())
  {
    json rinfo = json::object();
    rinfo["workflow"] = workflowName();
    rinfo["name"] = workflowJob();
    rinfo["engine"] = OutputEngineStore::instance().getRunInfo();
    rinfo["alias"] = DistUtils::getEnvironmentVariable("VNV_RUN_ALIAS", workflowJob());
    ofs << rinfo.dump(3);
    ofs.close();
  }
}

void RunTime::loadRunInfo(RunInfo &info, registrationCallBack callback)
{
  initializedCount++;

  if (initializedCount == 1)
  {
    // Set up the logger. This occurs as early as possible to allow log messages
    // to be caught int the registration objects.
    if (info.logInfo.on)
    {
      logger.setLog(info.logInfo.engine, info.logInfo.type, info.logInfo.filename);

      for (auto it : info.logInfo.logs)
      {
        logger.setLogLevel(it.first, it.second);
      }
      for (auto it : info.logInfo.blackList)
      {
        logger.addToBlackList(it);
      }
    }

    if (!info.workflowDir.empty())
    {
      workflowDir_ = info.workflowDir;
    }

    // Pull out the template patch. This is the user provides specification file
    // that should be merged into the final specificiation.
    template_patch = info.template_overrides;

    // Register VnV.
    runTimePackageRegistration(VNV_STR(VNVPACKAGENAME), INJECTION_REGISTRATION_PTR(VNVPACKAGENAME));

    // Register the Executable.
    if (callback != nullptr)
    {
      runTimePackageRegistration(mainPackageName, *callback);
    }

    // Now we register VnVs communicator. The VnV communicator is set to be the
    // same as the communicator for the executable.
    // CommunicationStore::instance().copySettings(mainPackageName,VNVPACKAGENAME_S);
  }

  // Register the plugins specified in the input file.
  makeLibraryRegistrationCallbacks(info.additionalPlugins);

  // Set up the engine
  ICommunicator_ptr world = CommunicationStore::instance().worldComm();
  logger.setRank(world->Rank());

  if (initializedCount == 1)
  {
    if (!OutputEngineStore::instance().isInitialized())
    {
      VnV_Debug(VNVPACKAGENAME, "Configuring The Output Engine");
      try
      {
        OutputEngineStore::instance().setEngineManager(world, info.engineInfo.engineType, info.engineInfo.engineConfig);
      }
      catch (VnVExceptionBase &e)
      {
        VnV_Error(VNVPACKAGENAME,
                  "Error Initializing Engine: What happens next will depend on the 'onEngineInitializationFailed' "
                  "parameter.");

        // TODO Allow user to decide what happens here.
        VnV_Error(VNVPACKAGENAME, "Aborting");
        std::abort();
      }
      VnV_Debug(VNVPACKAGENAME, "Output Engine Configuration Successful");
    }
  }

  // Process the configs (wait until now because it allows loaded test libraries
  // to register options objects.

  processToolConfig(info.pluginConfig, command_line_vector, world);

  if (info.schemaDump)
  {
    dumpSpecification(info.schemaQuit);
  }

  // Write the run info file -- This contains all the info needed to launch the reader.
  writeRunInfoFile();

  VnV_Debug(VNVPACKAGENAME,
            "Validating Json Test Configuration Input and converting to TestConfig "
            "objects");

  for (auto it : info.injectionPoints)
  {

    SamplerConfig sampler(it.second.sampler.package,it.second.sampler.name,it.second.package,it.second.name,it.second.sampler.config);
    auto x = TestStore::instance().validateTests(it.second.tests);
    InjectionPointStore::instance().addInjectionPoint(it.second.package, it.second.name, it.second.runInternal, it.second.templateName, x, sampler);
  }

  if (info.runAll)
  {
    auto x = TestStore::instance().validateTests(info.runAll_tests);
    InjectionPointStore::instance().runAll(x);
  }

  jobManager = WorkflowStore::instance().buildJobManager(mainPackageName, workflowName(), info.workflowInfo.workflows);

  OutputEngineStore::instance().getEngineManager()->sendInfoNode(world, getFullJson(), getProv().toJson(), workflowName_, workflowJob());
  
  if (info.actionInfo.run) {

     for (auto it : info.actionInfo.actions) {
        if (it.run) {
          try {
            ActionStore::instance().addAction(it.package, it.name, it.config);
          } catch (...) {
            VnV_Error(VNVPACKAGENAME, "Action %s:%s does not exist -- Ignoring ", it.package.c_str(), it.name.c_str());
          }
        }
      }
    }


  // Debug info printing all the information.
  printRunTimeInformation();

  auto fp = OutputEngineStore::instance().getFilePath();
  if ( fp.length() > 0 ) {
    std::string reportName = info.name;
    std::string description = info.description;
    if (reportName.length() == 0 ) {
       reportName = "Untitled Report ";
       reportName += workflowName();
    }
    registerReport(reportName.c_str(), fp.c_str(), description);
  }
  
}

VnVProv RunTime::getProv() { return *prov; }

void RunTime::registerReport(const char* reportName, const char* filename, std::string description) {

  std::string home_dir = DistUtils::getEnvironmentVariable("HOME", StringUtils::random(5));
  json conf;
  try {
    std::ifstream ifs(home_dir + "/.vnv");
    conf = json::parse(ifs);
  } catch (...) {
    conf = json::object();
  }
  
  if (conf.find("reports") == conf.end()) {
    conf["reports"] = json::object();
  }
  conf["reports"][reportName] = filename;

  try {
    std::ofstream ofs(home_dir + "/.vnv");
    ofs << conf.dump(4);
  } catch (...) {
    std::cout << "Error writing the registration file" << std::endl;
  }
  return; 

}


// Cant overload the name because "json" can be a "string".
bool RunTime::InitFromJson(const char *packageName, int *argc, char ***argv, json &config, InitDataCallback icallback,
                           registrationCallBack callback)
{
  mainPackageName = packageName;

  DistUtils::libData lb;
  DistUtils::getAllLinkedLibraryData(&lb);
  prov.reset(new VnV::VnVProv(*argc, *argv, configFile, config));
  prov->setLibraries(lb);

  for (int i = 0; i < *argc; i++)
  {
    command_line_vector.push_back((*argv)[i]);
  }

  JsonParser parser;
  try
  {
    info = parser.parse(config, argc, *argv);
  }
  catch (VnVExceptionBase e)
  {
    std::cerr << "VnV Initialization Failed during input file validation. \n";
    std::cerr << e.what() << std::endl;
    std::abort();
  }

  bool exitStatus = configure(packageName, info, callback);

  if (exitStatus != 0)
  {
    return true;
  }

  if (runTests) {

    if (info.cmdline.contains("dump"))
    {
        dumpSpecification(false);
    }
    
    if (info.cmdline.contains("qdump")){
       dumpSpecification(false);
    }
    
    if (info.cmdline.contains("fdump")){
       writeSpecification(info.cmdline["fdump"].get<std::string>(), false);
    }
    
    if (info.cmdline.contains("qfdump")){
       writeSpecification(info.cmdline["qfdump"].get<std::string>(), true);
    }
    

    prov.reset(new VnV::VnVProv(*argc, *argv, configFile, config));

    VnV_Comm comm = CommunicationStore::instance().world();

    // Make the callback to the initialization function.
    auto engine = OutputEngineStore::instance().getEngineManager();
    engine->initializationStartedCallBack(CommunicationStore::instance().worldComm(), mainPackageName);
    icallback(comm, engine->getOutputEngine());
    engine->initializationEndedCallBack(mainPackageName);
    configure_stdout();

    /**
     * @title VnV Application Profiling Loop.
     * @description Initialization of the VnV Loop
     * @instructions Use this to track updates for the entire application.
     * @param runTests A bool indicating if run Tests is turned on
     *
     * This injection point is called at the end of the VnVInit function. This is a
     * looped injection point with no interesting parameters passed in. This
     * injection point exists soley as a mechanism for profiling the given
     * application between the VnVInit and VnVFinalize functions.
     *
     */
    INJECTION_LOOP_BEGIN(VNVPACKAGENAME, comm, initialization, VNV_NOCALLBACK, runTests);
  }

  return false;
}

void RunTime::read_stdout()
{
  if (stdout_configured)
  {
    int n = -1;
    std::string ss = "";
    while ((n = read(stdout_pipefd[0], buf, 100)) > 0)
    {
      write(stdout_bk, buf, n);
      std::string s(buf, n);
      ss += s;
    }
    if (ss.size() > 0)
    {
      OutputEngineStore::instance().getEngineManager()->write_stdout(ss, false);
    }
    ss = "";
    while ((n = read(stderr_pipefd[0], buf, 100)) > 0)
    {
      write(stderr_bk, buf, n);
      std::string s(buf, n);
      ss += s;
    }
    if (ss.size() > 0)
    {
      OutputEngineStore::instance().getEngineManager()->write_stdout(ss, true);
    }
  }
}

void RunTime::configure_stdout()
{
  pipe2(stdout_pipefd, O_NONBLOCK);
  pipe2(stderr_pipefd, O_NONBLOCK);

  stdout_bk = dup(fileno(stdout));
  stderr_bk = dup(fileno(stderr));

  dup2(stdout_pipefd[1], fileno(stdout));
  dup2(stderr_pipefd[1], fileno(stderr));

  stdout_configured = true;
}

void RunTime::unconfigure_stdout()
{
  if (stdout_configured)
  {
    dup2(stdout_bk, fileno(stdout));
    dup2(stderr_bk, fileno(stderr));

    close(stdout_bk);
    close(stderr_bk);

    close(stdout_pipefd[0]);
    close(stdout_pipefd[1]);
    close(stderr_pipefd[0]);
    close(stderr_pipefd[1]);
  }
  stdout_configured = false;
}

std::string getOverrideFileName() {
  std::string fname = DistUtils::getEnvironmentVariable("VNV_INPUT_FILE", "");

  if (fname.length() > 0)
  {
    if (fname.compare("__off__") == 0) {
      return VNV_DEFAULT_INPUT_FILE_OFF;
    } else if (fname.compare("__on__") == 0) {
      return VNV_DEFAULT_INPUT_FILE_ON;
    }else if (fname.compare("__all__") == 0) {
      return VNV_DEFAULT_INPUT_FILE_ALL;
    }
    return fname;
  } 
  return "";
}


bool RunTime::InitFromFile(const char *packageName, int *argc, char ***argv, InitDataCallback icallback, registrationCallBack callback)
{

  configFile = getOverrideFileName();
  if (configFile.size() == 0 || configFile.compare( VNV_DEFAULT_INPUT_FILE_OFF) == 0)
  {
    json j = json::object();
    j["runTests"] = false;
    return InitFromJson(packageName, argc, argv, j, icallback, callback);
  }

  else if (configFile.compare(VNV_DEFAULT_INPUT_FILE_ON) == 0)
  {
    json j = json::object();
    j["runTests"] = true;
    return InitFromJson(packageName, argc, argv, j, icallback, callback);
  }
  
  else if (configFile.compare(VNV_DEFAULT_INPUT_FILE_ALL) == 0)
  {
    json j = json::object();
    j["runTests"] = true;
    j["injectionPoints"] = json::object();
    j["injectionPoints"]["runAll"] = true;
    return InitFromJson(packageName, argc, argv, j, icallback, callback);
  }

  std::ifstream fstream(configFile);
  if (fstream.good())
  {
    json mainJson = JsonUtilities::load(configFile);
    return InitFromJson(packageName, argc, argv, mainJson, icallback, callback);
  }
  else
  {
    std::cout << "Could not find input file " << configFile << std::endl;
    std::cout << "Running without VnV" << std::endl;
    json j = json::object();
    j["runTests"] = false;
    return InitFromJson(packageName, argc, argv, j, icallback, callback);
  }
}

bool RunTime::configure(std::string packageName, RunInfo info, registrationCallBack callback)
{
  try
  {
    if (info.cmdline.contains("on") || DistUtils::getEnvironmentVariable("VNV_ON","0").compare("1")== 0) {
      info.runTests = true;
    }
    runTests = info.runTests;



    if (runTests)
    {
      auto commW = CommunicationStore::instance().worldComm();

      loadRunInfo(info, callback);

      // Run any workflows listed in the application
      if (jobManager)
        jobManager->run(commW, true);

      if (info.unitTestInfo.runUnitTests)
      {
        runUnitTests(VnV_Comm_World(), info.unitTestInfo);

        if (info.unitTestInfo.exitAfterTests)
        {
          return 1; // Exit because user asked to exit after unit tests
        }
      }

      ActionStore::instance().initialize(commW);
    }
    else if (info.error)
    {
      return 2; // Exit because there was an error.
    }

    return 0;
  }
  catch (std::exception e)
  {
    std::cout << e.what() << std::endl;
  }
  catch (const char *e)
  {
    std::cout << e << std::endl;
  }
  catch (...)
  {
  }
  return 2;
}

void RunTime::processToolConfig(json config, std::vector<std::string> &cmdline, ICommunicator_ptr world)
{
  OptionsParserStore::instance().parse(config, cmdline, world);
}

void RunTime::runTimePackageRegistration(std::string packageName, registrationCallBack reg)
{
  auto it = plugins.find(packageName);
  if (it == plugins.end())
  {
    VnV_Debug(VNVPACKAGENAME, "Registering a new Package %s", packageName.c_str());
    plugins.insert(packageName);
    reg(); // could be recursive.
  }
}

long RunTime::currentTime()
{
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
      .count();
}

bool RunTime::Finalize()
{
  if (runTests)
  {
    auto comm = CommunicationStore::instance().worldComm();

    INJECTION_LOOP_END(VNVPACKAGENAME, initialization, VNV_NOCALLBACK);

    ActionStore::instance().finalize(comm);

    // Run any workflow jobs marked for execution after the workflow finishs.,
    if (jobManager)
      jobManager->run(comm, false);

    read_stdout();
    unconfigure_stdout();

    auto engine = OutputEngineStore::instance().getEngineManager();
    if (engine)
      engine->finalize(comm, currentTime());

    // Call any cleanup actions that were registered.
    for (auto &it : cleanupActions)
    {
      it.second(comm);
    }
  }

  CommunicationStore::instance().Finalize();

  resetStore();
  return true;
}

bool RunTime::isRunTests() { return runTests; }

void RunTime::log(VnV_Comm comm, std::string pname, std::string level, std::string message, va_list args)
{
  read_stdout();
  logger.log_c(comm, pname, level, message, args);
}
void RunTime::log(VnV_Comm comm, std::string pname, std::string level, std::string message)
{
  read_stdout();
  logger.log(comm, pname, level, message);
}

void RunTime::runUnitTests(VnV_Comm comm, UnitTestInfo info)
{
  UnitTestStore::instance().runAll(comm, info);
}

void RunTime::printRunTimeInformation()
{
  InjectionPointStore::instance().print();
  ActionStore::instance().print();
}

std::string RunTime::getPackageName() { return mainPackageName; }
