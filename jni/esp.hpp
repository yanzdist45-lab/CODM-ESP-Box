#pragma once
#include <cstdint>
struct Vector3 { float x,y,z; };
void esp_set_unity_base(uintptr_t);
void esp_render(int,int);
