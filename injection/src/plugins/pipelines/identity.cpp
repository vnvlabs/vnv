#include "VnV.h"

INJECTION_PIPELINE(VNVPACKAGENAME, identity, "{\"type\":\"object\"}") {
    return config.dump();
}