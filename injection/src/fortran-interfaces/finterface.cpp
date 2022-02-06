// NOLINTBEGIN(bugprone*)
#include <stdio.h>
#include <stdlib.h>

#include <map>
#include <string>
#include <vector>

#include "base/Runtime.h"

// Initialize the thing
namespace {

class InitCtx {
 public:
  std::string package;
  std::string filename;
  std::vector<std::string> args;
  registrationCallBack callback;
  InitCtx(const char* p, const char* f, registrationCallBack c) : package(p), filename(f), callback(c) {}
};

class PointCtx {
 public:
  int world;
  std::string package, name;
  std::map<std::string, std::pair<std::string, void*>> parameters;
  PointCtx(int w, std::string p, std::string n) : world(w), package(p), name(n) {}
};

class PlugCtx : public PointCtx {
 public:
  VnV_Iterator iterator;
  int once = 1;
  PlugCtx(int w, std::string p, std::string n, int o = 1) : PointCtx(w, p, n), once(o) { iterator.data = NULL; }
};

}  // namespace

extern "C" {

void* vnv_init_start_x(const char* package, const char* fname, registrationCallBack* callback) {
  InitCtx* a = new InitCtx(package, fname, *callback);
  return a;
}

void vnv_add_arg_x(void** ctx, char* s) {
  InitCtx* c = (InitCtx*)(*ctx);
  c->args.push_back(s);
}

void vnv_init_end_x(void** ctx) {
  InitCtx* c = (InitCtx*)(*ctx);

  std::vector<char*> cstrings;
  cstrings.reserve(c->args.size());

  for (auto& s : c->args) cstrings.push_back(&s[0]);

  int argc = c->args.size();
  char** argv = cstrings.data();
  VnV_init(c->package.c_str(), &argc, &argv, c->filename.c_str(), c->callback);
  delete (c);
}

void vnv_finalize_x(const char* s) { VnV_finalize(); }

void vnv_declare_type_parameter_x(void** ctx, const char* name, const char* typeInfo, void* ptr) {
  PointCtx* c = (PointCtx*)(*ctx);
  c->parameters[name] = {typeInfo, ptr};
}

// Scalars
#define SCALARS X(integer, int) X(float, float) X(double, double) X(string, const char)
#define X(NAME, TYPE)                                                                \
  void vnv_declare_##NAME##_parameter_x(void** ctx, const char* name, TYPE* value) { \
    vnv_declare_type_parameter_x(ctx, name, #TYPE, (void*)value);                    \
  }
SCALARS
#undef X

// Arrays of scalars
#define X(NAME, TYPE)                                                                          \
  void vnv_declare_##NAME##_array_x(void** ctx, const char* name, TYPE* value, size_t* size) { \
    vnv_declare_type_parameter_x(ctx, name, #TYPE, (void*)value);                              \
  }
SCALARS
#undef X

// Matrix of scalars
#define X(NAME, TYPE)                                                                                          \
  void vnv_declare_##NAME##_matrix_x(void** ctx, const char* name, TYPE** value, size_t* dim0, size_t* dim1) { \
    vnv_declare_type_parameter_x(ctx, name, #TYPE, (void*)value);                                              \
  }
SCALARS
#undef X

// Get parameters of a certain type from the options object -- this is in leiu of a processing
// VnV::RawJsonObject* rj = (VnV::RawJsonObject*)VnV::RunTime::instance().getOptionsObject(package); \
// return rj.value(json::json_pointer(parameter), *def);                                             \

#define X(NAME, TYPE) \
  TYPE vnv_get_##NAME##_parameter_x(const char* package, const char* parameter, TYPE* def) { return *def; }
SCALARS
#undef X

const char* vnv_get_str_parameter_x(const char* package, const char* parameter) {
  // VnV::RawJsonObject* rj = (VnV::RawJsonObject*)VnV::RunTime::instance().getOptionsObject(package);
  // std::string v = rj->j.value(json::json_pointer(parameter), "__not_found__");
  // if (v.compare("__not_found__") == 0) {
  //  return nullptr;
  //}

  // They need to free this once they are done with it.
  // char* res = (char*)malloc((v.size() + 1) * sizeof(char));
  // strcpy(res, v.c_str());
  // return res;
  return "GGGG";  // uncomment once I merge the new options interface.
}

#undef SCALARS

void* vnv_loop_init_x(int world, const char* package, const char* fname) {
  return (void*)new PointCtx(world, package, fname);
}

void vnv_loop_begin_x(void** ctx) {
  PointCtx* c = (PointCtx*)(*ctx);
  VnV_Comm comm = c->world ? VWORLD : VSELF;
  VnV::RunTime::instance().injectionPoint_begin(comm, c->package, c->name, {"gfortran", ""}, "", -1, NULL,
                                                c->parameters);
}

void vnv_loop_iter_x(void** ctx, const char* iter) {
  PointCtx* c = (PointCtx*)(*ctx);
  VnV::RunTime::instance().injectionPoint_iter(c->package, c->name, iter, "", -1);
}

void vnv_loop_end_x(void** ctx) {
  PointCtx* c = (PointCtx*)(*ctx);
  VnV::RunTime::instance().injectionPoint_end(c->package, c->name, "", -1);
  delete (c);
}

void* vnv_point_init_x(int world, const char* package, const char* fname) {
  PointCtx* a = new PointCtx(world, package, fname);
  return a;
}

void vnv_point_run_x(void** ctx) {
  PointCtx* c = (PointCtx*)(*ctx);
  VnV_Comm comm = c->world ? VWORLD : VSELF;
  VnV::RunTime::instance().injectionPoint(comm, c->package, c->name, {"gfortran", ""}, "", -1, NULL, c->parameters);
}

void* vnv_plug_init_x(int world, const char* package, const char* fname) {
  PlugCtx* a = new PlugCtx(world, package, fname);
  return a;
}

int vnv_plug_run_x(void** ctx) {
  PlugCtx* c = (PlugCtx*)(*ctx);
  VnV_Comm comm = c->world ? VWORLD : VSELF;

  if (c->iterator.data == NULL) {
    c->iterator = VnV::RunTime::instance().injectionPlug(comm, c->package, c->name, {"gfortran", ""}, "", -1, NULL,
                                                         c->parameters);
  }
  auto res = VnV::RunTime::instance().injectionPlugRun(&(c->iterator));

  if (res == 0) {
    delete c;
  }
  return res;
}

void* vnv_iterator_init_x(int world, const char* package, const char* fname, int* once) {
  PlugCtx* a = new PlugCtx(world, package, fname, *once);
  return a;
}

int vnv_iterator_run_x(void** ctx) {
  PlugCtx* c = (PlugCtx*)(*ctx);
  VnV_Comm comm = c->world ? VWORLD : VSELF;

  if (c->iterator.data == NULL) {
    c->iterator = VnV::RunTime::instance().injectionIteration(comm, c->package, c->name, {"gfortran", ""}, "", -1, NULL,
                                                              c->parameters, c->once);
  }
  auto res = VnV::RunTime::instance().injectionIterationRun(&(c->iterator));

  if (res == 0) {
    delete c;
  }
  return res;
}

void vnv_log_x(int world, const char* level, const char* package, const char* message) {
  VnV_Comm comm = world ? VWORLD : VSELF;
  VnV::RunTime::instance().log(comm, package, level, message);
}

void vnv_file_x(int world, int input, const char* package, const char* name, const char* filename, const char* reader) {
  VnV_Comm comm = world ? VWORLD : VSELF;
  VnV::RunTime::instance().registerFile(comm, package, name, input, filename, reader);
}
}

// NOLINTEND(bugprone*)
