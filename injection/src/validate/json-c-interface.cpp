/*
 * JSON schema validator for JSON for modern C++
 *
 * Copyright (c) 2016-2019 Patrick Boettcher <p@yai.se>.
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <iostream>
#include "validate/json-c-interface.hpp"
#include "shared/json.hpp"
using nlohmann::json;

#define CJSON_OK 0
#define CJSON_INVALID_TYPE 1
#define CJSON_INVALID_DATA 2
#define CJSON_ARRAY_INDEX_OUT_OF_BOUNDS 3
#define CJSON_OBJECT_KEY_OUT_OF_BOUNDS 4
#define CJSON_BUFFER_TO_SMALL 5

namespace {

json* cast(cjson* jsonRaw) {
  if (jsonRaw->data != nullptr)
    return (json*)jsonRaw->data;
  else
    return nullptr;
}

}  // namespace

extern "C" {

int cjson_ok(int err) { return err == 0 ? 1 : 0 ; }

#define X(CJT)                                   \
  int cjson_is_##CJT(cjson* json, int* value) { \
    if (auto a = cast(json)) {                   \
      *value = (a->is_##CJT()) ? 1 : 0 ;         \
      return CJSON_OK;                           \
    }                                            \
    return CJSON_INVALID_DATA;                   \
  }
CJSON_IS_TYPES
#undef X

int cjson_object_contains(cjson* json, const char* key, int* result) {
  auto a = cast(json);
  if (a->is_object()) {
    *result = (a->contains(key)) ? 1 : 0 ;
	return CJSON_OK;
  }
  return CJSON_INVALID_TYPE;
}

int cjson_object_key(cjson* json, int index, char* result, int max_size) {
  auto a = cast(json);
  if (a->is_object()) {
    int i = 0;
    for (auto it : a->items()) {
      if (i++ == index) {
        std::string s = it.key();
        strncpy(result, s.c_str(), max_size);
        if (s.size() + 1 > max_size) {
          return CJSON_BUFFER_TO_SMALL;
        }
        return CJSON_OK;
      }
    }
    return CJSON_ARRAY_INDEX_OUT_OF_BOUNDS;
  }
  return CJSON_INVALID_TYPE;
}

int cjson_object_size(cjson* json, int* size) {
  auto a = cast(json);
  if (a->is_object()) {
    *size = a->size();
    return CJSON_OK;
  }
  return CJSON_INVALID_TYPE;
}

int cjson_object_get(cjson* json, const char* key, cjson* result) {
  auto a = cast(json);
  if (a->is_object()) {
    if (!a->contains(key)) {
      return CJSON_OBJECT_KEY_OUT_OF_BOUNDS;
    }
    result->data = &((*a)[key]);
	return CJSON_OK;
  }
  return CJSON_INVALID_TYPE;
}

int cjson_array_size(cjson* json, int* size) {
  auto a = cast(json);
  if (a->is_array()) {
    *size = a->size();
    return CJSON_OK;
  }
  return CJSON_INVALID_TYPE;
}

int cjson_array_get(cjson* json, long index, cjson* result) {
  auto a = cast(json);
  if (a->is_array()) {
    if (index < 0 || index >= a->size()) {
      return CJSON_ARRAY_INDEX_OUT_OF_BOUNDS;
    }
    result->data = &((*a)[index]);
	return CJSON_OK;
  }
  return CJSON_INVALID_TYPE;
}

int cjson_double(cjson* json, double* value) {
  auto a = cast(json);
  if (a->is_number_float()) {
    *value = a->get<double>();
    return CJSON_OK;
  }
  return CJSON_INVALID_TYPE;
}

int cjson_boolean(cjson* json, int* value) {
  auto a = cast(json);
  if (a->is_boolean()) {
    *value = a->get<bool>() ? 1 : 0 ;
    return CJSON_OK;
  }
  return CJSON_INVALID_TYPE;
}
int cjson_long(cjson* json, long* value) {
  auto a = cast(json);
  if (a->is_number_integer()) {
    *value = a->get<long>();
    return CJSON_OK;
  }
  return CJSON_INVALID_TYPE;
}
int cjson_integer(cjson* json, int* value) {
  auto a = cast(json);
  if (a->is_number_integer()) {
    *value = a->get<int>();
    return CJSON_OK;
  }
  return CJSON_INVALID_TYPE;
}


int cjson_float(cjson* json, float* value) {
  auto a = cast(json);
  if (a->is_number_float()) {
    *value = a->get<float>();
    return CJSON_OK;
  }
  return CJSON_INVALID_TYPE;
}

int cjson_string(cjson* json, char* value, int max_size) {
  auto a = cast(json);
  if (a->is_string()) {
    std::string s = a->get<std::string>();
    strncpy(value, s.c_str(), max_size);
    if (s.size() + 1 > max_size) {
      return CJSON_BUFFER_TO_SMALL;
    }
    return CJSON_OK;
  }
  return CJSON_INVALID_TYPE;
}

int cjson_dump(cjson* json, int sep) {
  std::cout << cast(json)->dump(sep) << std::endl;
  return CJSON_OK;
}

const char* cjson_error_message(int err) {
  switch (err) {
  case CJSON_OK:
    return "Valid";
  case CJSON_INVALID_DATA:
    return "data object is null";
  case CJSON_INVALID_TYPE:
    return "json object is not of the correct type for this operation";
  case CJSON_OBJECT_KEY_OUT_OF_BOUNDS:
    return "key does not exist in json object";
  case CJSON_ARRAY_INDEX_OUT_OF_BOUNDS:
    return "array index out of bounds for json array";
  case CJSON_BUFFER_TO_SMALL:
    return "the buffer provided was to small to fit the string.";
  default:
    return "Unknown Error";
  }
}

}