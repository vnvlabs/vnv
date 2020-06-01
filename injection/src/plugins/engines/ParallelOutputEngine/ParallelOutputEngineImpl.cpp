/** @file vv-debug-engine.cpp **/

#include "plugins/engines/ParallelOutputEngine/ParallelOutputEngineImpl.h"
#include "c-interfaces/Logging.h"
#include <iostream>
#include <mpi.h>
#include <unistd.h>

using namespace VnV;

static json __parallel_engine_schema__ =
		R"(
{
	"$schema": "http://json-schema.org/draft-07/schema#",
	"$id": "http://rnet-tech.net/vv-debug-schema.json",
	"title": "Parallel Engine Input Schema",
	"description": "Schema for the parallel engine",
    "type": "object"
}
)"_json;

ParallelEngine::ParallelEngine(ParallelEngineWrapper *wrapper) : wrapper(wrapper){
}

void ParallelEngine::Put(std::string variableName, double &value) {
    std::string str = std::to_string(value);
    Put(variableName, str);
}
void ParallelEngine::Put(std::string variableName, int &value) {
    std::string str = std::to_string(value);
    Put(variableName, str);
}
void ParallelEngine::Put(std::string variableName, float &value) {
    std::string str = std::to_string(value);
    Put(variableName, str);
}
void ParallelEngine::Put(std::string variableName, long &value) {
    std::string str = std::to_string(value);
    Put(variableName, str);
}

void ParallelEngine::Put(std::string variableName, std::string &value) {
	wrapper->getRouter()->send(variableName, value);
}

void ParallelEngine::Log(const char *package, int stage, std::string level,
		std::string message) {
	if (wrapper->getRouter()->isRoot()) {
		std::string s = getIndent(stage);
		printf("%s[%s:%s]: %s\n", s.c_str(), package, level.c_str(),
				message.c_str());
	}
}

void ParallelEngine::Define(VariableEnum type, std::string variableName) {
	printf("PARALLEL ENGINE DEFINE %s = %s\n", variableName.c_str(),
			VariableEnumFactory::toString(type).c_str());
}

ParallelEngineWrapper::ParallelEngineWrapper() {
}

json ParallelEngineWrapper::getConfigurationSchema() {
	return __parallel_engine_schema__;
}

void ParallelEngineWrapper::finalize() {
	VnV_Info("PARALLEL ENGINE: FINALIZE");
}

void ParallelEngineWrapper::set(json &config) {
	printf("PARALLEL ENGINE WRAPPER Init with file %s\n",
			config.dump().c_str());
	engine = new ParallelEngine(this);
	router = new Router();
}

void ParallelEngineWrapper::injectionPointEndedCallBack(std::string id,
		InjectionPointType type, std::string stageVal) {
	if (engine) {
		printf("PARALLEL ENGINE End Injection Point %s : %s \n", id.c_str(),
				InjectionPointTypeUtils::getType(type, stageVal).c_str());
		router->forward();
	} else {
		throw "Engine not initialized";
	}
}

void ParallelEngineWrapper::injectionPointStartedCallBack(std::string id,
		InjectionPointType type, std::string stageVal) {
	if (engine) {
		printf("PARALLEL ENGINE Start Injection Point %s : %s \n", id.c_str(),
				InjectionPointTypeUtils::getType(type, stageVal).c_str());
	} else {
		throw "Engine not initialized";
	}
}

void ParallelEngineWrapper::testStartedCallBack(std::string testName) {
	if (engine) {
		printf("PARALLEL ENGINE Start Test %s \n", testName.c_str());
	} else {
		throw "Engine not initialized";
	}
}

void ParallelEngineWrapper::testFinishedCallBack(bool result_) {
	if (engine) {
		printf("PARALLEL ENGINE Stop Test. Test Was Successful-> %d\n",
				result_);
	} else {
		throw "Engine not initialized";
	}
}

void ParallelEngineWrapper::unitTestStartedCallBack(std::string unitTestName) {
	if (engine) {
		printf("PARALLEL ENGINE START UNIT TEST: %s\n", unitTestName.c_str());
	} else {
		throw "Engine not initialized";
	}
}

void ParallelEngineWrapper::unitTestFinishedCallBack(IUnitTest *tester) {
	if (engine) {
		printf("Test Results\n");
		bool suiteSuccessful = true;
        for ( auto it : tester->getResults() ) {
            printf("\t%s : %s\n", std::get<0>(it).c_str(), std::get<2>(it) ? "Successful" : "Failed");
            if (!std::get<2>(it)) {
                printf("\t\t%s\n", std::get<1>(it).c_str());
                suiteSuccessful = false;
           }
        }
		printf("PARALLEL ENGINE Test Suite Completed : %s\n",
				(suiteSuccessful) ? "Successfully" : "Unsuccessfully");
	} else {
		throw "Engine Not Initialized";
	}
}

IOutputEngine* ParallelEngineWrapper::getOutputEngine() {
	return engine;
}

Router * ParallelEngineWrapper::getRouter() {
    return router;
}

OutputEngineManager* ParallelEngineBuilder() {
	return new ParallelEngineWrapper();
}
