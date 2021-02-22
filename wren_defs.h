#pragma once
#ifndef WREN_DEFS_H
#define WREN_DEFS_H

#include <wren.h>


#define wren_bind_foreign(type, name)                                           \
void type##_allocate(WrenVM* vm)                                                \
{                                                                               \
    type** v = (type**)wrenSetSlotNewForeign(vm,0, 0, sizeof(type*));      \
    *v = (type*)malloc(sizeof(type));                                       \
                                                                                \
    printf(#type " constructor\n");                                             \
}                                                                               \
                                                                                \
void type##_finalize(void* data)                                                \
{                                                                               \
    type**v = ((type**) data);                                              \
                                                                                \
    if (*v != NULL)                                                             \
    {                                                                           \
        printf(#type " destructor\n");                                          \
        free(*v);                                                               \
        *v = NULL;                                                              \
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
}                                                                               \

// TODO: add other slot types

#define wren_define_accessor_float(type, field_name, field_var)   \
void  type##_set_##field_name(WrenVM *vm)                         \
{                                                       \
    type** v = (type**)wrenGetSlotForeign(vm, 0);   \
                                                        \
    if (*v == NULL)                                     \
    {                                                   \
        wrenSetSlotString(vm, 0, #type " is invalid");  \
        wrenAbortFiber(vm, 0);                          \
        return;                                         \
    }                                                   \
    double value = wrenGetSlotDouble(vm, 1);            \
    (*v)->field_var = (float)value;                   \
}                                                       \
                                                        \
void type##_get_##field_name(WrenVM *vm)                \
{                                                       \
    type** v = (type**)wrenGetSlotForeign(vm, 0);   \
                                                        \
    if (*v == NULL)                                     \
    {                                                   \
        wrenSetSlotString(vm, 0, #type " is invalid");  \
        wrenAbortFiber(vm, 0);                          \
        return;                                         \
    }                                                   \
    wrenSetSlotDouble(vm, 0, (double)(*v)->field_var);  \
}


#define wren_bind_methods_begin(type, name)         \
WrenForeignMethodFn bind_vec3_methods(WrenVM* vm,   \
                           const char* moduleName,  \
                           const char* class_name,  \
                           bool isStatic,           \
                           const char* signature)   \
{                                                   \
    if (strcmp(class_name, #name) == 0)             \
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



#endif