#pragma once
#ifndef VEC3_H
#define VEC3_H

#include <wren.h>
#include <string.h>
#include <stdio.h>

#include "wren_defs.h"

typedef struct vec3 {
    float v[3];
} vec3;

wren_bind_foreign_alloc_begin(vec3, Vec3)
wren_bind_foreign_alloc_end()

wren_bind_foreign_final_begin(vec3, Vec3)
wren_bind_foreign_final_end(vec3, Vec3)

wren_define_accessor_float(vec3, x, v[0])
wren_define_accessor_float(vec3, y, v[1])
wren_define_accessor_float(vec3, z, v[2])

void vec3_to_string(WrenVM *vm)
{
    vec3* v = (vec3*)wrenGetSlotForeign(vm, 0);

    // Make sure the file is still open.
    if (v == NULL)
    {
        wrenSetSlotString(vm, 0, "Vec3 is invalid");
        wrenAbortFiber(vm, 0);
        return;
    }
    char vec3str[128];
    sprintf(&vec3str[0], "[%5.5f, %5.5f, %5.5f]", v->v[0], v->v[1], v->v[2]);
    wrenSetSlotString(vm, 0, &vec3str[0]);
}

wren_bind_methods_begin(vec3, Vec3)
    wren_bind_accessor(vec3, x)
    wren_bind_accessor(vec3, y)
    wren_bind_accessor(vec3, z)
    wren_bind_method("toString()", vec3_to_string)
wren_bind_methods_end()

typedef struct node {
    char * name;
} node;

wren_bind_foreign_alloc_begin(node, Node)
wren_bind_foreign_string_alloc(name)
wren_bind_foreign_alloc_end()

wren_bind_foreign_final_begin(node, Node)
	wren_bind_foreign_string_final(name)
wren_bind_foreign_final_end(node, Node)

wren_define_accessor_string(node, name, name)

wren_bind_methods_begin(node, Node)
    wren_bind_accessor(node, name)
wren_bind_methods_end()

typedef struct transform {
    vec3 pos;
} transform;

wren_bind_foreign_alloc_begin(transform, Transform)
wren_bind_foreign_alloc_end()

wren_bind_foreign_final_begin(transform, Transform)
wren_bind_foreign_final_end(transform, Transform)

//wren_define_accessor_foreign(transform, pos, vec3, pos)

void transform_set_pos(WrenVM *vm) {
  transform *v = (transform*)wrenGetSlotForeign(vm, 0);
  if (v == NULL) {
    wrenSetSlotString(vm, 0,
                      "transform"
                      " is invalid");
    wrenAbortFiber(vm, 0);
    return;
  }
  vec3 *value = (vec3 *)wrenGetSlotForeign(vm, 1);
  if (value != NULL) {
    memcpy(&v->pos, value, sizeof(vec3));
  }
}
void transform_get_pos(WrenVM *vm) {
  transform *v = (transform *)wrenGetSlotForeign(vm, 0);
  if (v == NULL) {
    wrenSetSlotString(vm, 0,
                      "transform"
                      " is invalid");
    wrenAbortFiber(vm, 0);
    return;
  }
  // TODO: Figure out how to push the Vec3 class into Slot 0
  vec3 *f = (vec3*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(vec3));
  //f = (vec3 *)malloc(sizeof(vec3));
  if (f != NULL) {
    memcpy(f, &v->pos, sizeof(vec3));
  }
}

wren_bind_methods_begin(transform, Transform)
    wren_bind_accessor(transform, pos)
wren_bind_methods_end()

#endif