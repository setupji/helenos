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
 * @brief Utilities for convenient manipulation with ARM registers.
 */

#ifndef KERN_arm64_REGUTILS_H_
#define KERN_arm64_REGUTILS_H_

#ifndef __ASM__
#if defined(KERNEL) || defined(BOOT)
#include <stddef.h>
#else
#include <stdint.h>
#endif

#define SPECIAL_REG_GEN_READ(name) \
	static inline uintptr_t name##_read(void) \
	{ \
		uintptr_t res; \
		asm volatile ( \
			"mrs %[res], " #name \
			: [res] "=r" (res) \
		); \
		return res; \
	}

#define SPECIAL_REG_GEN_WRITE(name) \
	static inline void name##_write(uintptr_t regn) \
	{ \
		asm volatile ( \
			"msr " #name ", %[regn]\n" \
			"isb\n" \
			:: [regn] "r" (regn) \
		); \
	}

#else /* __ASM__ */

#define SPECIAL_REG_GEN_READ(name)
#define SPECIAL_REG_GEN_WRITE(name)

#endif /* __ASM__*/

/* CurrentEL */
SPECIAL_REG_GEN_READ(CurrentEL)
#define CURRENT_EL_EL0  0x0
#define CURRENT_EL_EL1  0x4
#define CURRENT_EL_EL2  0x8
#define CURRENT_EL_EL3  0xc

/* DAIF */
SPECIAL_REG_GEN_READ(DAIF)
SPECIAL_REG_GEN_WRITE(DAIF)
#define DAIF_IRQ_SHIFT  7  /* I flag */
#define DAIF_IRQ_FLAG  (1 << DAIF_IRQ_SHIFT)

/* ELR */
SPECIAL_REG_GEN_WRITE(ELR_EL1)

/* ESR */
SPECIAL_REG_GEN_READ(ESR_EL1)
#define ESR_EC_SHIFT  26
#define ESR_EC_MASK  (0x3f << ESR_EC_SHIFT)

/** Exception from SVC instruction execution. */
#define ESR_EC_SVC  0x15

/** Instruction abort from Lower Exception level. */
#define ESR_EC_IA_LOWER_EL  0x20
/** Data abort from Lower Exception level. */
#define ESR_EC_DA_LOWER_EL  0x24
/** Data abort from Current Exception level. */
#define ESR_EC_DA_CURRENT_EL  0x25

/** Instruction/data Fault Status Code. */
#define ESR_IDFSC_SHIFT  0
#define ESR_IDFSC_MASK  (0x3f << ESR_IDFSC_SHIFT)

/** Instruction/data abort, translation fault, zeroth level. */
#define ESR_IDA_IDFSC_TF0  0x4
/** Instruction/data abort, translation fault, first level. */
#define ESR_IDA_IDFSC_TF1  0x5
/** Instruction/data abort, translation fault, second level. */
#define ESR_IDA_IDFSC_TF2  0x6
/** Instruction/data abort, translation fault, third level. */
#define ESR_IDA_IDFSC_TF3  0x7

/** Data abort, Write not Read. */
#define ESR_DA_WNR_SHIFT  6
#define ESR_DA_WNR_FLAG  (1 << ESR_DA_WNR_SHIFT)

/* FAR */
SPECIAL_REG_GEN_READ(FAR_EL1)

/* SCTLR */
#define SCTLR_M_SHIFT  0  /* M flag */
#define SCTLR_M_FLAG  (1 << SCTLR_M_SHIFT)

/* SP */
SPECIAL_REG_GEN_WRITE(SP_EL0)

/* SPSR */
SPECIAL_REG_GEN_READ(SPSR_EL1)
SPECIAL_REG_GEN_WRITE(SPSR_EL1)
#define SPSR_MODE_MASK  0x1f
#define SPSR_MODE_ARM64_EL0T  0x00  /* ARM64, Exception Level 0, SP_EL0 */

/* TPIDR */
SPECIAL_REG_GEN_WRITE(TPIDR_EL0)

/* TTBR */
SPECIAL_REG_GEN_WRITE(TTBR0_EL1)
SPECIAL_REG_GEN_WRITE(TTBR1_EL1)
#define TTBR0_ASID_SHIFT  48

/* VBAR */
SPECIAL_REG_GEN_WRITE(VBAR_EL1)

/* TLBI VAE1IS and TLBI ASIDE1IS parameter. */
#define TLBI_ASID_SHIFT  48

#endif

/** @}
 */