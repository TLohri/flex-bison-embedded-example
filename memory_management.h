#pragma once

#include <stddef.h>

void initialize();

void* my_alloc(size_t size);
void* my_realloc(void* ptr, size_t size);
void my_free(void* ptr);