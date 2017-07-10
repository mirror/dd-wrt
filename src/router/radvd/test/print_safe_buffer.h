
#pragma once

#include "../config.h"
#include "../radvd.h"

size_t snprint_safe_buffer(char *s, size_t size, struct safe_buffer const *sb);
void print_safe_buffer(struct safe_buffer const *sb);
