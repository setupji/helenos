#
# Copyright (c) 2006 Martin Decky
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

# Just for this Makefile. Sub-makes will run in parallel if requested.
.NOTPARALLEL:

CSCOPE = cscope
CHECK = tools/check.sh
CONFIG = tools/config.py
AUTOTOOL = tools/autotool.py
SANDBOX = autotool

CONFIG_RULES = HelenOS.config

COMMON_MAKEFILE = Makefile.common
COMMON_HEADER = common.h
COMMON_HEADER_PREV = $(COMMON_HEADER).prev

CONFIG_MAKEFILE = Makefile.config
CONFIG_HEADER = config.h

.PHONY: all precheck cscope cscope_parts autotool config_auto config_default config distclean clean check releasefile release

all: $(COMMON_MAKEFILE) $(COMMON_HEADER) $(CONFIG_MAKEFILE) $(CONFIG_HEADER)
	cp -a $(COMMON_HEADER) $(COMMON_HEADER_PREV)
	$(MAKE) -r -C kernel PRECHECK=$(PRECHECK)
	$(MAKE) -r -C uspace PRECHECK=$(PRECHECK)
	$(MAKE) -r -C boot PRECHECK=$(PRECHECK)

precheck: clean
	$(MAKE) -r all PRECHECK=y

cscope:
	find abi kernel boot uspace -type f -regex '^.*\.[chsS]$$' | xargs $(CSCOPE) -b -k -u -f$(CSCOPE).out

cscope_parts:
	find abi -type f -regex '^.*\.[chsS]$$' | xargs $(CSCOPE) -b -k -u -f$(CSCOPE)_abi.out
	find kernel -type f -regex '^.*\.[chsS]$$' | xargs $(CSCOPE) -b -k -u -f$(CSCOPE)_kernel.out
	find boot -type f -regex '^.*\.[chsS]$$' | xargs $(CSCOPE) -b -k -u -f$(CSCOPE)_boot.out
	find uspace -type f -regex '^.*\.[chsS]$$' | xargs $(CSCOPE) -b -k -u -f$(CSCOPE)_uspace.out

# Pre-integration build check
check: $(CHECK)
ifdef JOBS 
	$(CHECK) -j $(JOBS)
else
	$(CHECK)
endif

# Autotool (detects compiler features)

autotool $(COMMON_MAKEFILE) $(COMMON_HEADER): $(CONFIG_MAKEFILE)
	$(AUTOTOOL)
	-[ -f $(COMMON_HEADER_PREV) ] && diff -q $(COMMON_HEADER_PREV) $(COMMON_HEADER) && mv -f $(COMMON_HEADER_PREV) $(COMMON_HEADER)

# Build-time configuration

config_default $(CONFIG_MAKEFILE) $(CONFIG_HEADER): $(CONFIG_RULES)
ifeq ($(HANDS_OFF),y)
	$(CONFIG) $< hands-off $(PROFILE)
else
	$(CONFIG) $< default $(PROFILE)
endif

config: $(CONFIG_RULES)
	$(CONFIG) $<

random-config: $(CONFIG_RULES)
	$(CONFIG) $< random

# Release files

releasefile: all
	$(MAKE) -r -C release releasefile

release:
	$(MAKE) -r -C release release

# Cleaning

distclean: clean
	rm -f $(CSCOPE).out $(COMMON_MAKEFILE) $(COMMON_HEADER) $(COMMON_HEADER_PREV) $(CONFIG_MAKEFILE) $(CONFIG_HEADER) tools/*.pyc tools/checkers/*.pyc release/HelenOS-*

clean:
	rm -fr $(SANDBOX)
	$(MAKE) -r -C kernel clean
	$(MAKE) -r -C uspace clean
	$(MAKE) -r -C boot clean

-include Makefile.local
