include ../Build/macros.tmpl
APP=LogProcessor

EXTRA_LDFLAGS=-static
EXTRA_INCLUDES=-I./pcre/inc
EXTRA_DEFINES=-DHAVE_CONFIG_H

SRCS = $(wildcard *.C) \
		$(wildcard pcre/src/*.c) \
		$(wildcard pcre/tests/*.c) \

LIBS=

include $(WORKROOT)/Build/app.rules
