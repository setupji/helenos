/*
 * Copyright (c) 2015 Petr Pavlu
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

/** @addtogroup arm64
 * @{
 */
/** @file
 * @brief ARM64 FPU context.
 */

#include <arch/regutils.h>
#include <fpu_context.h>

/** Initialize FPU functionality. */
void fpu_init(void)
{
	/* REVISIT */
	/* Mask all exception traps. */
}

/** Enable FPU instructions. */
void fpu_enable(void)
{
	CPACR_EL1_write((CPACR_EL1_read() & ~CPACR_FPEN_MASK) |
	    (CPACR_FPEN_TRAP_NONE << CPACR_FPEN_SHIFT));
}

/** Disable FPU instructions. */
void fpu_disable(void)
{
	CPACR_EL1_write((CPACR_EL1_read() & ~CPACR_FPEN_MASK) |
	    (CPACR_FPEN_TRAP_ALL << CPACR_FPEN_SHIFT));
}
