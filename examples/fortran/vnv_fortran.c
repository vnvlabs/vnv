#include <stdio.h>
#include <stdlib.h>

void vnv_declare_type_parameter_x(void* ctx, const char*name, const char* typeInfo, void* ptr) {}


// Scalars
#define SCALARS X(integer,int) X(float,float) X(double,double) X(string, const char)
#define X(NAME,TYPE)\
void vnv_declare_##NAME##_parameter_x(void* ctx, const char* name, TYPE* value) {} 
SCALARS
#undef X

//Arrays of scalars
#define X(NAME,TYPE) \
void vnv_declare_##NAME##_array_x(void* ctx, const char* name, TYPE* value, size_t *size) {}
SCALARS
#undef X

//Matrix of scalars
#define X(NAME,TYPE) \
void vnv_declare_##NAME##_matrix_x(void* ctx, const char* name, TYPE** value, size_t *dim0, size_t *dim1) {}
SCALARS
#undef X

//Get parameters of a certain type from the options object -- this is in leiu of a processing 
#define X(NAME,TYPE) \
TYPE vnv_get_##NAME##_parameter_x(const char* package, const char* parameter, TYPE* def) {return *def;}
SCALARS
#undef X
const char* vnv_get_str_parameter_x(const char* package, const char* parameter) {return "GGGG";}

#undef SCALARS


void vnv_init_start_x(const char* package, const char*fname) {
    printf("Hello INit %s %s\n", package, fname);
}

void vnv_add_arg_x(const char* s) {
    printf("Hello Arg %s\n", s);
}

void vnv_init_end_x() {
    printf("Hello End Initalizatuib \n");
}

void vnv_finalize_x(const char* s) {
    printf("Hello Finalize %s\n", s);
}

typedef struct  {
    const char* package;
    const char* name;
    int keepGoing;
} loopcontext;


void* vnv_loop_init_x(const char* package, const char*fname) {
    printf("Hello Loop INit %s %s\n", package, fname);
    loopcontext* ctx = (loopcontext*) malloc(sizeof(loopcontext));
    ctx->package = package;
    ctx->name = fname;
    return (void*) ctx;
}

void vnv_loop_begin_x(void* ctx) {
    // TODO
}

void vnv_loop_iter_x(void* ctx, const char* iter) {
    // TODO
}

void vnv_loop_end_x(void* ctx) {
    loopcontext* lctx = (loopcontext*) ctx;
    // TODO
    free(ctx);
}

void* vnv_point_init_x(const char* package, const char*fname) {
    printf("Hello Loop INit %s %s\n", package, fname);
    loopcontext* ctx = (loopcontext*) malloc(sizeof(loopcontext));
    ctx->package = package;
    ctx->name = fname;
    return (void*) ctx;
}

void vnv_point_run_x(void* ctx) {
    loopcontext* lctx = (loopcontext*) ctx;
    // TODO
    free(ctx);
}


void* vnv_plug_init_x(const char* package, const char*fname) {
    loopcontext* ctx = (loopcontext*) malloc(sizeof(loopcontext));
    ctx->package = package;
    ctx->name = fname;
    return (void*) ctx;
}

int vnv_plug_run_x(void* ctx) {
    loopcontext* lctx = (loopcontext*) ctx;
    // TODO    
    if (!lctx->keepGoing) { free(ctx); return 0; }
    return lctx->keepGoing;
}


void* vnv_iterator_init_x(const char* package, const char*fname) {
    loopcontext* ctx = (loopcontext*) malloc(sizeof(loopcontext));
    ctx->package = package;
    ctx->name = fname;
    return (void*) ctx;
}

int vnv_iterator_run_x(void* ctx) {
    loopcontext* lctx = (loopcontext*) ctx;
    // TODO
    if (!lctx->keepGoing) { free(ctx); return 0; }
    return lctx->keepGoing;
}




