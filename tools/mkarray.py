#!/usr/bin/env python
#
# Copyright (c) 2011 Martin Decky
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# - Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
# - Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# - The name of the author may not be used to endorse or promote products
#   derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
"""
Binary inline data packer
"""

import sys
import os
import zlib
import zipfile
import binascii

def usage(prname):
	"Print usage syntax"
	print("%s [--deflate] <DESTINATION> <LABEL> <AS_PROLOG> <SECTION> [SOURCE ...]" % prname)

def arg_check():
	if (len(sys.argv) < 5):
		usage(sys.argv[0])
		sys.exit()

def deflate(data):
	"Compress using deflate algorithm (without any headers)"
	return zlib.compress(data, 9)[2:-4]

def chunks(string, length):
	"Produce string chunks"
	for start in range(0, len(string), length):
		yield string[start:start + length]

def main():
	arg_check()

	if sys.argv[1] == "--deflate":
		sys.argv.pop(1)
		arg_check()
		compress = True
	else:
		compress = False

	dest = sys.argv[1]
	label = sys.argv[2]
	as_prolog = sys.argv[3]
	section = sys.argv[4]

	timestamp = (1980, 1, 1, 0, 0, 0)

	header_ctx = []
	desc_ctx = []
	size_ctx = []
	data_ctx = []

	src_cnt = 0

	archive = zipfile.ZipFile("%s.zip" % dest, "w", zipfile.ZIP_STORED)

	for src in sys.argv[5:]:
		basename = os.path.basename(src)
		plainname = os.path.splitext(basename)[0]
		symbol = basename.replace(".", "_")

		print("%s -> %s" % (src, symbol))

		src_in = open(src, "rb")
		src_data = src_in.read()
		src_in.close()

		length = len(src_data)

		if compress:
			src_data = deflate(src_data)
			src_fname = os.path.basename("%s.deflate" % src)
			zipinfo = zipfile.ZipInfo(src_fname, timestamp)
			archive.writestr(zipinfo, src_data)
		else:
			src_fname = src

		if sys.version_info < (3,):
			src_data = bytearray(src_data)

		length_out = len(src_data)

		header_ctx.append("extern uint8_t %s[];" % symbol)
		header_ctx.append("extern size_t %s_size;" % symbol)

		data_ctx.append(".globl %s" % symbol)
		data_ctx.append(".balign 8")
		data_ctx.append(".size %s, %u" % (symbol, length_out))
		data_ctx.append("%s:" % symbol)
		data_ctx.append("\t.incbin \"%s\"\n" % src_fname)

		desc_field = []
		desc_field.append("\t{")
		desc_field.append("\t\t.name = \"%s\"," % plainname)
		desc_field.append("\t\t.addr = (void *) %s," % symbol)
		desc_field.append("\t\t.size = %u," % length_out)
		desc_field.append("\t\t.inflated = %u," % length)

		if compress:
			desc_field.append("\t\t.compressed = true")
		else:
			desc_field.append("\t\t.compressed = false")

		desc_field.append("\t}")

		desc_ctx.append("\n".join(desc_field))

		size_ctx.append("size_t %s_size = %u;" % (symbol, length_out))

		src_cnt += 1

	data = ''
	data += '/*\n'
	data += ' * AUTO-GENERATED FILE, DO NOT EDIT!!!\n'
	data += ' * Generated by: tools/mkarray.py\n'
	data += ' */\n\n'
	data += "#ifndef %sS_H_\n" % label.upper()
	data += "#define %sS_H_\n\n" % label.upper()
	data += "#include <stddef.h>\n"
	data += "#include <stdint.h>\n"
	data += "#include <stdbool.h>\n\n"
	data += "#define %sS  %u\n\n" % (label.upper(), src_cnt)
	data += "typedef struct {\n"
	data += "\tconst char *name;\n"
	data += "\tvoid *addr;\n"
	data += "\tsize_t size;\n"
	data += "\tsize_t inflated;\n"
	data += "\tbool compressed;\n"
	data += "} %s_t;\n\n" % label
	data += "extern %s_t %ss[];\n\n" % (label, label)
	data += "\n".join(header_ctx)
	data += "\n\n"
	data += "#endif\n"
	zipinfo = zipfile.ZipInfo("%s.h" % dest, timestamp)
	archive.writestr(zipinfo, data)

	data = ''
	data += '/*\n'
	data += ' * AUTO-GENERATED FILE, DO NOT EDIT!!!\n'
	data += ' * Generated by: tools/mkarray.py\n'
	data += ' */\n\n'
	data += as_prolog
	data += "%s\n\n" % section
	data += "\n".join(data_ctx)
	data += "\n"
	zipinfo = zipfile.ZipInfo("%s.s" % dest, timestamp)
	archive.writestr(zipinfo, data)

	data = ''
	data += '/*\n'
	data += ' * AUTO-GENERATED FILE, DO NOT EDIT!!!\n'
	data += ' * Generated by: tools/mkarray.py\n'
	data += ' */\n\n'
	data += "#include \"%s.h\"\n\n" % dest
	data += "%s_t %ss[] = {\n" % (label, label)
	data += ",\n".join(desc_ctx)
	data += "\n"
	data += "};\n\n"
	data += "\n".join(size_ctx)
	data += "\n"
	zipinfo = zipfile.ZipInfo("%s_desc.c" % dest, timestamp)
	archive.writestr(zipinfo, data)

	archive.close()

if __name__ == '__main__':
	main()
