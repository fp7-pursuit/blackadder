PSIRP_MAJOR = 0
PSIRP_MINOR = 2

sbin_PROGRAMS  = placonf gen_cert platest perftest

INCLUDES = -I../include
INCLUDES += -I/usr/local/include

placonf_SOURCES = ec_io.c gnb.c ec.c ec_hw.c placonf.c
placonf_CFLAGS = $(INCLUDES)
placonf_LDFLAGS = -lpla -lcrypto -L.

gen_cert_SOURCES = 
gen_cert:
	@./generate-certificate
	@touch gen_cert

platest_SOURCES = platest.cpp
platest_CFLAGS = $(INCLUDES)
platest_LDFLAGS = -lpla -L.

perftest_SOURCES = perftest.c ec.c ec_hw.c gnb.c
perftest_CFLAGS = $(INCLUDES)
perftest_LDFLAGS = -lgmp -lcrypto -L/usr/local/lib -L.

lib_LTLIBRARIES = libpla.la
libpla_la_LDFLAGS = -version-info $(PSIRP_MAJOR):$(PSIRP_MINOR) -lpthread -lm -lgmp -lcrypto -L/usr/local/lib
libpla_la_CFLAGS = $(INCLUDES)
libpla_la_SOURCES = libpla.c libpla_io.c libpla_connections.c libpla_cert.c gnb.c ec.c ec_hw.c \
	ec_io.c 

