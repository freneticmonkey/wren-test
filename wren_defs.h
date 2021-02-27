#pragma once
#ifndef WREN_DEFS_H
#define WREN_DEFS_H

#include <wren.h>

//*v = (type *)malloc(sizeof(type));

#define wren_bind_foreign_alloc_begin(type, name)                               \
  void type##_allocate(WrenVM *vm)                                              \
  {                                                                             \
    type *v = (type *)wrenSetSlotNewForeign(vm, 0, 0, sizeof(type));            \
    if (v != NULL)                                                              \
    {

#define wren_bind_foreign_string_alloc(name)                                    \
        v->name = NULL;

#define wren_bind_foreign_alloc_end()                                           \
    }                                                                           \
}                                                                               

#define wren_bind_foreign_final_begin(type, name)                               \
void type##_finalize(void* data)                                                \
{                                                                               \
    type *v = (type*)data;                                                      \
                                                                                \
    if (v != NULL)                                                              \
    {

#define wren_bind_foreign_string_final(name)                                    \
        if (v->name != NULL)                                                    \
            free(v->name);

// free(*v);
// *v = NULL;
#define wren_bind_foreign_final_end(type, name)                                 \
    }                                                                           \
}                                                                               \
                                                                                \
bool bind_##type##_class(WrenVM *vm,                                            \
                     WrenForeignClassMethods *methods,                          \
                     const char* module,                                        \
                     const char* class_name)                                    \
{                                                                               \
    if (strcmp(class_name, #name) == 0)                                         \
    {                                                                           \
        methods->allocate = type##_allocate;                                    \
        methods->finalize = type##_finalize;                                    \
        return true;                                                            \
    }                                                                           \
    return false;                                                               \
}

#define wren_bind_foreign_class(type, methods)                  \
    bind_##type##_class(vm, &methods, module, className);


#define wren_define_accessor_int(type, field_name, field_var)   \
void type##_set_##field_name(WrenVM *vm)                        \
{                                                               \
    type* v = (type*)wrenGetSlotForeign(vm, 0);                 \
                                                                \
    if (*v == NULL)                                             \
    {                                                           \
        wrenSetSlotString(vm, 0, #type " is invalid");          \
        wrenAbortFiber(vm, 0);                                  \
        return;                                                 \
    }                                                           \
    double value = wrenGetSlotDouble(vm, 1);                    \
    (*v)->field_var = (int)value;                               \
}                                                               \
                                                                \
void type##_get_##field_name(WrenVM *vm)                        \
{                                                               \
    type* v = (type*)wrenGetSlotForeign(vm, 0);                 \
                                                                \
    if (v == NULL)                                              \
    {                                                           \
        wrenSetSlotString(vm, 0, #type " is invalid");          \
        wrenAbortFiber(vm, 0);                                  \
        return;                                                 \
    }                                                           \
    wrenSetSlotDouble(vm, 0, (double)(v->field_var) );          \
}

#define wren_define_accessor_float(type, field_name, field_var)     \
void type##_set_##field_name(WrenVM *vm)                            \
{                                                                   \
    type* v = (type*)wrenGetSlotForeign(vm, 0);                     \
                                                                    \
    if (v == NULL)                                                  \
    {                                                               \
        wrenSetSlotString(vm, 0, #type " is invalid");              \
        wrenAbortFiber(vm, 0);                                      \
        return;                                                     \
    }                                                               \
    double value = wrenGetSlotDouble(vm, 1);                        \
    v->field_var = (float)value;                                    \
}                                                                   \
                                                                    \
void type##_get_##field_name(WrenVM *vm)                            \
{                                                                   \
    type* v = (type*)wrenGetSlotForeign(vm, 0);                     \
                                                                    \
    if (v == NULL)                                                  \
    {                                                               \
        wrenSetSlotString(vm, 0, #type " is invalid");              \
        wrenAbortFiber(vm, 0);                                      \
        return;                                                     \
    }                                                               \
    wrenSetSlotDouble(vm, 0, (double)(v->field_var) );              \
}

#define wren_define_accessor_bool(type, field_name, field_var)  \
void type##_set_##field_name(WrenVM *vm)                        \
{                                                               \
    type* v = (type*)wrenGetSlotForeign(vm, 0);                 \
                                                                \
    if (v == NULL)                                              \
    {                                                           \
        wrenSetSlotString(vm, 0, #type " is invalid");          \
        wrenAbortFiber(vm, 0);                                  \
        return;                                                 \
    }                                                           \
    v->field_var = wrenGetSlotBool(vm, 1);                      \
}                                                               \
                                                                \
void type##_get_##field_name(WrenVM *vm)                        \
{                                                               \
    type* v = (type*)wrenGetSlotForeign(vm, 0);                 \
                                                                \
    if (v == NULL)                                              \
    {                                                           \
        wrenSetSlotString(vm, 0, #type " is invalid");          \
        wrenAbortFiber(vm, 0);                                  \
        return;                                                 \
    }                                                           \
    wrenSetSlotBool(vm, 0, v->field_var);                       \
}

#define wren_define_accessor_string(type, field_name, field_var)                \
void type##_set_##field_name(WrenVM *vm)                                        \
{                                                                               \
    type* v = (type*)wrenGetSlotForeign(vm, 0);                                 \
                                                                                \
    if (v == NULL)                                                              \
    {                                                                           \
        wrenSetSlotString(vm, 0, #type " is invalid");                          \
        wrenAbortFiber(vm, 0);                                                  \
        return;                                                                 \
    }                                                                           \
    const char *str_value = wrenGetSlotString(vm, 1);                           \
                                                                                \
    char *string = v->field_var;                                                \
    if (string != NULL)                                                         \
    {                                                                           \
        free(string);                                                           \
        string = NULL;                                                          \
    }                                                                           \
    size_t length = strlen(str_value);                                          \
    v->field_var = (char*)malloc(sizeof(char) * length + 1);                    \
    string = v->field_var;                                                      \
    if (string != NULL)                                                         \
    {                                                                           \
        strncpy(string, str_value, length);                                     \
        string[length] = '\0';                                                  \
    }                                                                           \
}                                                                               \
                                                                                \
void type##_get_##field_name(WrenVM *vm)                                        \
{                                                                               \
    type* v = (type*)wrenGetSlotForeign(vm, 0);                                 \
                                                                                \
    if (v == NULL)                                                              \
    {                                                                           \
        wrenSetSlotString(vm, 0, #type " is invalid");                          \
        wrenAbortFiber(vm, 0);                                                  \
        return;                                                                 \
    }                                                                           \
    if (v->field_var == NULL)                                                   \
    {                                                                           \
        wrenSetSlotString(vm, 0, #type " has invalid '" #field_var "' value");  \
        wrenAbortFiber(vm, 0);                                                  \
        return;                                                                 \
    }                                                                           \
    wrenSetSlotString(vm, 0, v->field_var);                                     \
}

//    *f = (field_type*)malloc(sizeof(field_type));                               

#define wren_define_accessor_foreign(type, module, class_name, field_name, field_type, field_var)  \
void type##_set_##field_name(WrenVM *vm)                                            \
{                                                                                   \
    type *v = (type *)wrenGetSlotForeign(vm, 0);                                    \
                                                                                    \
    if (v == NULL) {                                                                \
        wrenSetSlotString(vm, 0, #type " is invalid");                              \
        wrenAbortFiber(vm, 0);                                                      \
        return;                                                                     \
    }                                                                               \
    field_type *value = (field_type *)wrenGetSlotForeign(vm, 1);                    \
    if (value != NULL)                                                              \
    {                                                                               \
        memcpy(&(v->field_var), value, sizeof(field_type) );                        \
    }                                                                               \
}                                                                                   \
                                                                                    \
void type##_get_##field_name(WrenVM *vm)                                            \
{                                                                                   \
    type *v = (type *)wrenGetSlotForeign(vm, 0);                                    \
                                                                                    \
    if (v == NULL) {                                                                \
        wrenSetSlotString(vm, 0, #type " is invalid");                              \
        wrenAbortFiber(vm, 0);                                                      \
        return;                                                                     \
    }                                                                               \
    wrenGetVariable(vm, #module, #class_name, 0);                                   \
    field_type* f = (field_type*)wrenSetSlotNewForeign(vm,0, 0, sizeof(field_type));\
                                                                                    \
    if (f != NULL)                                                                  \
    {                                                                               \
        memcpy(f, &(v->field_var), sizeof(field_type));                             \
    }                                                                               \
}

#define wren_bind_methods_begin(type, name)             \
WrenForeignMethodFn bind_##type##_methods(WrenVM* vm,   \
                           const char* moduleName,      \
                           const char* class_name,      \
                           bool isStatic,               \
                           const char* signature)       \
{                                                       \
    if (strcmp(class_name, #name) == 0)                 \
    {

#define wren_bind_accessor(type, field_name)                            \
    if (!isStatic && strcmp(signature, #field_name "(_)") == 0) return type##_set_##field_name; \
    if (!isStatic && strcmp(signature, #field_name "()") == 0) return type##_get_##field_name;  

#define wren_bind_method(wren_signature, func)  \
    if (!isStatic && strcmp(signature, wren_signature) == 0) return func;

#define wren_bind_methods_end() \
    }                           \
    return NULL;                \
}

#define wren_bind_foriegn_methods(type)                      \
    WrenForeignMethodFn type##_func = bind_##type##_methods( \
        vm,                                                  \
        moduleName,                                          \
        className,                                           \
        isStatic,                                            \
        signature);                                          \
    if (type##_func != NULL)                                 \
        return type##_func;



#endif