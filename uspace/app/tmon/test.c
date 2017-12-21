/*
 * Copyright (c) 2017 Petr Manek
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** @addtogroup tmon
 * @{
 */
/**
 * @file
 * USB transfer debugging.
 */

#include <stdio.h>
#include <devman.h>
#include <str_error.h>
#include <usbdiag_iface.h>
#include "commands.h"
#include "resolve.h"

#define NAME "tmon"
#define MAX_PATH_LENGTH 1024

#define DEFAULT_CYCLES  1024
#define DEFAULT_SIZE    65432

static int resolve_and_test(int argc, char *argv[], int (*test)(devman_handle_t, int, size_t)) {
	devman_handle_t fun = -1;

	if (argc == 0) {
		if (tmon_resolve_default(&fun))
			return 1;
	} else if (argc == 1) {
		if (tmon_resolve_named(argv[0], &fun))
			return 1;
	} else {
		printf(NAME ": Too many arguments provided.\n");
		return 1;
	}

	int rc;
	char path[MAX_PATH_LENGTH];
	if ((rc = devman_fun_get_path(fun, path, sizeof(path)))) {
		printf(NAME ": Error resolving path of device with handle %ld. %s\n", fun, str_error(rc));
		return 1;
	}

	printf("Using device: %s\n", path);

	// TODO: Read options here.

	return test(fun, DEFAULT_CYCLES, DEFAULT_SIZE);
}

static int stress_intr_in(devman_handle_t fun, int cycles, size_t size) {
	async_sess_t *sess = usbdiag_connect(fun);
	async_exch_t *exch = async_exchange_begin(sess);

	int rc = usbdiag_stress_intr_in(exch, cycles, size);
	int ec = 0;

	if (rc) {
		printf(NAME ": %s\n", str_error(rc));
		ec = 1;
	}

	async_exchange_end(exch);
	usbdiag_disconnect(sess);
	return ec;
}

static int stress_intr_out(devman_handle_t fun, int cycles, size_t size) {
	async_sess_t *sess = usbdiag_connect(fun);
	async_exch_t *exch = async_exchange_begin(sess);

	int rc = usbdiag_stress_intr_out(exch, cycles, size);
	int ec = 0;

	if (rc) {
		printf(NAME ": %s\n", str_error(rc));
		ec = 1;
	}

	async_exchange_end(exch);
	usbdiag_disconnect(sess);
	return ec;
}

static int stress_bulk_in(devman_handle_t fun, int cycles, size_t size) {
	async_sess_t *sess = usbdiag_connect(fun);
	async_exch_t *exch = async_exchange_begin(sess);

	int rc = usbdiag_stress_bulk_in(exch, cycles, size);
	int ec = 0;

	if (rc) {
		printf(NAME ": %s\n", str_error(rc));
		ec = 1;
	}

	async_exchange_end(exch);
	usbdiag_disconnect(sess);
	return ec;
}

static int stress_bulk_out(devman_handle_t fun, int cycles, size_t size) {
	async_sess_t *sess = usbdiag_connect(fun);
	async_exch_t *exch = async_exchange_begin(sess);

	int rc = usbdiag_stress_bulk_out(exch, cycles, size);
	int ec = 0;

	if (rc) {
		printf(NAME ": %s\n", str_error(rc));
		ec = 1;
	}

	async_exchange_end(exch);
	usbdiag_disconnect(sess);
	return ec;
}

int tmon_stress_intr_in(int argc, char *argv[])
{
	return resolve_and_test(argc, argv, stress_intr_in);
}

int tmon_stress_intr_out(int argc, char *argv[])
{
	return resolve_and_test(argc, argv, stress_intr_out);
}

int tmon_stress_bulk_in(int argc, char *argv[])
{
	return resolve_and_test(argc, argv, stress_bulk_in);
}

int tmon_stress_bulk_out(int argc, char *argv[])
{
	return resolve_and_test(argc, argv, stress_bulk_out);
}

/** @}
 */
