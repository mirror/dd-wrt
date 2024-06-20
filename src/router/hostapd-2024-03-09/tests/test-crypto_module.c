/*
 * crypto module tests - test program
 * Copyright (c) 2022, Glenn Strauss <gstrauss@gluelogic.com>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "utils/includes.h"
#include "utils/module_tests.h"
#include "crypto/crypto_module_tests.c"

int main(int argc, char *argv[])
{
	return crypto_module_tests();
}
