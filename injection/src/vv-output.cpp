
#include "injection.h"



void VV_registerEngine(std::string name, engine_register_ptr r) { 
    (InjectionPointBaseFactory::registeredEngines)[name] = r; 
}

IVVOutputEngine::~IVVOutputEngine(){}
