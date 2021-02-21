#pragma once
#ifndef VEC3_H
#define VEC3_H

#include <wren.h>
#include <string.h>
#include <stdio.h>

typedef struct vec3 {
    float v[3];
} vec3;

void vec3Allocate(WrenVM* vm)
{
    vec3** v = (vec3**)wrenSetSlotNewForeign(vm,0, 0, sizeof(vec3*));
    //const char* path = wrenGetSlotString(vm, 1);
    *v = (vec3*)malloc(sizeof(vec3));

    printf("Vec3 constructor\n");
}

void vec3Finalize(void* data)
{
    vec3**v = ((vec3**) data);

    if (*v != NULL)
    {
        printf("Vec3 destructor\n");
        free(*v);
        *v = NULL;
    }
}

void vec3_set_x(WrenVM *vm)
{
    vec3** v = (vec3**)wrenGetSlotForeign(vm, 0);

    // Make sure the file is still open.
    if (*v == NULL)
    {
        wrenSetSlotString(vm, 0, "Vec3 is invalid");
        wrenAbortFiber(vm, 0);
        return;
    }
    printf("Vec3 set x\n");
    double value = wrenGetSlotDouble(vm, 1);
    (*v)->v[0] = (float)value;
}

void vec3_get_x(WrenVM *vm)
{
    vec3** v = (vec3**)wrenGetSlotForeign(vm, 0);

    // Make sure the file is still open.
    if (*v == NULL)
    {
        wrenSetSlotString(vm, 0, "Vec3 is invalid");
        wrenAbortFiber(vm, 0);
        return;
    }
    printf("Vec3 get x\n");
    wrenSetSlotDouble(vm, 0, (double)(*v)->v[0]);
}

void vec3_to_string(WrenVM *vm)
{
    vec3** v = (vec3**)wrenGetSlotForeign(vm, 0);

    // Make sure the file is still open.
    if (*v == NULL)
    {
        wrenSetSlotString(vm, 0, "Vec3 is invalid");
        wrenAbortFiber(vm, 0);
        return;
    }
    char vec3str[128];
    sprintf(&vec3str, "[%5.5f, %5.5f, %5.5f]", (*v)->v[0], (*v)->v[1], (*v)->v[2]);
    wrenSetSlotString(vm, 0, &vec3str);
}

bool bind_vec3_class(WrenVM *vm, WrenForeignClassMethods *methods, const char* module, const char* className)
{
    if (strcmp(className, "Vec3") == 0)
    {
        methods->allocate = vec3Allocate;
        methods->finalize = vec3Finalize;
        return true;
    }
    return false;
}

WrenForeignMethodFn bind_vec3_methods(WrenVM* vm,
                           const char* moduleName,
                           const char* className,
                           bool isStatic,
                           const char* signature)
{
    if (strcmp(className, "Vec3") == 0)
    {
        if (!isStatic && strcmp(signature, "x(_)") == 0)
        {
            return vec3_set_x;
        }

        if (!isStatic && strcmp(signature, "x()") == 0)
        {
            return vec3_get_x;
        }

        if (!isStatic && strcmp(signature, "toString()") == 0)
        {
            return vec3_to_string;
        }
    }

    // Unknown method.
    return NULL;
}

#endif