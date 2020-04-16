
/** @file OutputEngineStore.cpp Implementation of the OutputEngineStore as defined in
 * base/OutputEngineStore.h"
 **/

#include "base/OutputReaderStore.h"
#include "c-interfaces/Logging.h"
#include "json-schema.hpp"
#include "base/exceptions.h"

using namespace VnV;
using nlohmann::json;
using nlohmann::json_schema::json_validator;

OutputReaderStore::OutputReaderStore() {}

OutputReaderStore& OutputReaderStore::instance() {
  static OutputReaderStore reader;
  return reader;
}

void OutputReaderStore::registerReader(std::string name,
                                 reader_register_ptr* engine_ptr) {
  registeredReaders.insert(std::make_pair(name, engine_ptr));
}

void OutputReaderStore::registerTreeGenerator(std::string name,
                                 tree_gen_register_ptr* engine_ptr) {
  registeredGenerators.insert(std::make_pair(name, engine_ptr));
}


std::unique_ptr<Reader::ITreeGenerator> OutputReaderStore::getTreeGenerator(std::string generator) {
    auto it = registeredGenerators.find(generator);
    if (it != registeredGenerators.end()) {
         return std::unique_ptr<Reader::ITreeGenerator>(it->second());
    }
    throw VnVExceptionBase("No Generator Exists with that name");
}

std::unique_ptr<Reader::IReader> OutputReaderStore::getReader(std::string reader) {
     auto it = registeredReaders.find(reader);
     if (it != registeredReaders.end()) {
          return std::unique_ptr<Reader::IReader>(it->second());
     }
     throw VnVExceptionBase("No Reader Exists");
}

void VnV::registerReader(std::string name, reader_register_ptr r) {
    OutputReaderStore::instance().registerReader(name, r);
}

void VnV::registerTreeGenerator(std::string name, tree_gen_register_ptr r) {
    OutputReaderStore::instance().registerTreeGenerator(name,r);
}

