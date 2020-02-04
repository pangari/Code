include ../Build/macros.tmpl
APP=Proxy

EXTRA_LDFLAGS=-static

SRCS = $(wildcard *.c) \
		$(wildcard aes/*.c) \
		$(wildcard fish/*.c) \
		$(wildcard ice/*.c) \
		$(wildcard lzo/*.c) \
		$(wildcard tea/*.c) \

LIBS=

include $(WORKROOT)/Build/app.rules
