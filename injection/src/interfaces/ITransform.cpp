
#include "interfaces/ITransform.h"

using namespace VnV;

void* ITransform::Transform(std::string outputType, std::string inputType, void* ptr ) {
    if ( inputType.compare(outputType) == 0 ) {
        return ptr;
    } else {
        throw "Input Type Does not match output type when using Default Transform.";
    }
}

ITransform::ITransform(){};
ITransform::~ITransform(){};
