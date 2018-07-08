/*
 * Copyright (c) 2008 Pavel Rimsky
 * Copyright (c) 2017 Jiri Svoboda
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

/** @file Sun4v console driver
 */

#include <as.h>
#include <async.h>
#include <ddf/log.h>
#include <ddi.h>
#include <errno.h>
#include <io/chardev_srv.h>
#include <stdbool.h>
#include <thread.h>

#include "sun4v-con.h"

static void sun4v_con_connection(ipc_callid_t, ipc_call_t *, void *);

#define POLL_INTERVAL  10000

/*
 * Kernel counterpart of the driver pushes characters (it has read) here.
 * Keep in sync with the definition from
 * kernel/arch/sparc64/src/drivers/niagara.c.
 */
#define INPUT_BUFFER_SIZE  ((PAGE_SIZE) - 2 * 8)

typedef volatile struct {
	uint64_t write_ptr;
	uint64_t read_ptr;
	char data[INPUT_BUFFER_SIZE];
} __attribute__((packed)) __attribute__((aligned(PAGE_SIZE))) *input_buffer_t;

/* virtual address of the shared buffer */
static input_buffer_t input_buffer;

static int sun4v_con_read(chardev_srv_t *, void *, size_t, size_t *);
static int sun4v_con_write(chardev_srv_t *, const void *, size_t, size_t *);

static chardev_ops_t sun4v_con_chardev_ops = {
	.read = sun4v_con_read,
	.write = sun4v_con_write
};

static void sun4v_con_putchar(sun4v_con_t *con, uint8_t data)
{
	(void) con;
	(void) data;
}

/** Add sun4v console device. */
int sun4v_con_add(sun4v_con_t *con, sun4v_con_res_t *res)
{
	ddf_fun_t *fun = NULL;
	int rc;

	con->res = *res;
	input_buffer = (input_buffer_t) AS_AREA_ANY;

	fun = ddf_fun_create(con->dev, fun_exposed, "a");
	if (fun == NULL) {
		ddf_msg(LVL_ERROR, "Error creating function 'a'.");
		rc = ENOMEM;
		goto error;
	}

	chardev_srvs_init(&con->cds);
	con->cds.ops = &sun4v_con_chardev_ops;
	con->cds.sarg = con;

	ddf_fun_set_conn_handler(fun, sun4v_con_connection);

	rc = physmem_map(res->base, 1, AS_AREA_READ | AS_AREA_WRITE,
	    (void *) &input_buffer);
	if (rc != EOK) {
		ddf_msg(LVL_ERROR, "Error mapping memory: %d", rc);
		goto error;
	}

	rc = ddf_fun_bind(fun);
	if (rc != EOK) {
		ddf_msg(LVL_ERROR, "Error binding function 'a'.");
		goto error;
	}

	return EOK;
error:
	/* XXX Clean up thread */

	if (input_buffer != (input_buffer_t) AS_AREA_ANY)
		physmem_unmap((void *) input_buffer);

	if (fun != NULL)
		ddf_fun_destroy(fun);

	return rc;
}

/** Remove sun4v console device */
int sun4v_con_remove(sun4v_con_t *con)
{
	return ENOTSUP;
}

/** Msim console device gone */
int sun4v_con_gone(sun4v_con_t *con)
{
	return ENOTSUP;
}

/** Read from Sun4v console device */
static int sun4v_con_read(chardev_srv_t *srv, void *buf, size_t size,
    size_t *nread)
{
	size_t p;
	uint8_t *bp = (uint8_t *) buf;
	char c;

	while (input_buffer->read_ptr == input_buffer->write_ptr)
		fibril_usleep(POLL_INTERVAL);

	p = 0;
	while (p < size && input_buffer->read_ptr != input_buffer->write_ptr) {
		c = input_buffer->data[input_buffer->read_ptr];
		input_buffer->read_ptr =
		    ((input_buffer->read_ptr) + 1) % INPUT_BUFFER_SIZE;
		bp[p++] = c;
	}

	*nread = p;
	return EOK;
}

/** Write to Sun4v console device */
static int sun4v_con_write(chardev_srv_t *srv, const void *data, size_t size,
    size_t *nwr)
{
	sun4v_con_t *con = (sun4v_con_t *) srv->srvs->sarg;
	size_t i;
	uint8_t *dp = (uint8_t *) data;

	for (i = 0; i < size; i++)
		sun4v_con_putchar(con, dp[i]); 

	*nwr = size;
	return EOK;
}

/** Character device connection handler. */
static void sun4v_con_connection(ipc_callid_t iid, ipc_call_t *icall,
    void *arg)
{
	sun4v_con_t *con = (sun4v_con_t *) ddf_dev_data_get(
	    ddf_fun_get_dev((ddf_fun_t *) arg));

	chardev_conn(iid, icall, &con->cds);
}

/** @}
 */
