
#include "c-interfaces/json-interface.h"

using nlohmann::json;

bool VnV_check(c_json json) {
    return json.data != NULL;
}

json VnV_castJson(c_json jsonRaw) {
    return *((json*) jsonRaw.data);
}

bool VnV_is_object(c_json json) {
    if (VnV_check(json))
       return VnV_castJson(json).is_object();
    return false;
}
bool VnV_is_null(c_json json) {
    if (VnV_check(json))
        return VnV_castJson(json).is_null();
    return false;
}
bool VnV_is_array(c_json json) {
    if (VnV_check(json))
         return VnV_castJson(json).is_array();
    return false;
}
bool VnV_is_number(c_json json) {
    if (VnV_check(json))
        return VnV_castJson(json).is_number();
    return false;
}
bool VnV_is_string(c_json json) {
    if (VnV_check(json))
        return VnV_castJson(json).is_string();
    return false;
}
bool VnV_is_boolean(c_json json) {
    if (VnV_check(json))
        return VnV_castJson(json).is_boolean();
    return false;
}
bool VnV_is_number_float(c_json json) {
    if (VnV_check(json))
        return VnV_castJson(json).is_number_float();
    return false;
}
bool VnV_is_number_unsigned(c_json json) {
    if (VnV_check(json))
        return VnV_castJson(json).is_number_unsigned();
    return false;
}
bool VnV_is_number_integer(c_json json) {
    if (VnV_check(json))
        return VnV_castJson(json).is_number_integer();
    return false;
}

bool VnV_getArraySize(c_json json, unsigned int *result) {
    if (VnV_is_array(json)) {
        *result =  VnV_castJson(json).size();
        return true;
    }
    return false;
}

bool VnV_getArrayIndex(c_json json, unsigned int index, c_json *result) {

    if (VnV_is_array(json)) {
        unsigned int x = 0;
        if ( VnV_getArraySize(json, &x) && index < x ) {
           *result = {&(VnV_castJson(json).at(index))};
           return true;
        }
    }
    return false;
}

bool VnV_getObjectByKey(c_json json, char* key, c_json* result) {
    if (VnV_is_object(json) ) {
        std::string keyStr = key;
        auto it = VnV_castJson(json).find(keyStr);
        if (it != VnV_castJson(json).end()) {
           *result = {&(it.value())};
           return true;
        }
    }
    return false;
}

bool VnV_hasKey(c_json json, char* key) {
    return (VnV_is_object(json) && VnV_castJson(json).contains(key));
}

bool VnV_getAllKeys(c_json json, char***keys, unsigned long* size) {
    if (VnV_is_object(json)) {
       *size = VnV_castJson(json).size();
       char** k = (char**) malloc(sizeof(char*)*(*size));
       int count = 0;
       for ( auto it : VnV_castJson(json).items()) {
           k[count] = (char*) malloc(sizeof(char)*(it.key().length()+2));
           strcpy(k[count++], it.key().c_str());
       }
       *keys = k;
       return true;
    }
    return false;
}

bool VnV_freeAllKeys(c_json json, char***keys, int* size) {
    for (int i = 0; i < *size; i++) {
        free((*keys)[i]);
    }
    free(*keys);
    return true;
}

// C Interface
bool VnV_getString(c_json json, char** result) {
    if (VnV_is_string(json)) {
        std::string s = VnV_castJson(json).get<std::string>();
        *result = (char*) malloc(sizeof(char)*(s.length()+1));
        strcpy(*result, s.c_str());
        return true;
    }
    return false;
}

bool VnV_freeString(c_json json, char** ptr) {
   free(*ptr);
   return true;
}


bool VnV_getInt(c_json json, int* result) {
    if (VnV_is_number_integer(json)) {
        *result = VnV_castJson(json).get<int>();
        return true;
    }
    return false;
}
bool VnV_getUnsignedInt(c_json json, unsigned int *result) {
    if (VnV_is_number_unsigned(json)) {
        *result = VnV_castJson(json).get<unsigned int>();
        return true;
    }
    return false;
}
bool VnV_getFloat(c_json json, float *result) {
    if (VnV_is_number(json)) {
         *result = VnV_castJson(json).get<float>();
        return true;
    }
    return false;
}

bool VnV_getBoolean(c_json json, bool *result) {
  if (VnV_is_boolean(json)) {
      *result = VnV_castJson(json).get<bool>();
      return true;
  }
  return false;
}

json VnV::asJson(c_json json) {
    return VnV_castJson(json);
}
