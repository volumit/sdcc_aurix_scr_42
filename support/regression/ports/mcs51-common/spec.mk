# Common regression test specification for the mcs51 targets running with uCsim

# simulation timeout in seconds
SIM_TIMEOUT = 80

EMU_PORT_FLAG = -t32
EMU_FLAGS = -S in=$(DEV_NULL),out=-
PORT_BASE = mcs51-common

# path to uCsim
ifdef SDCC_BIN_PATH
  S51 = $(SDCC_BIN_PATH)/ucsim_51$(EXEEXT)
else
  ifdef UCSIM_DIR
    S51A = $(UCSIM_DIR)/s51.src/ucsim_51$(EXEEXT)
  else
    S51A = $(top_builddir)/sim/ucsim/s51.src/ucsim_51$(EXEEXT)
    S51B = $(top_builddir)/bin/ucsim_51$(EXEEXT)
  endif

  EMU = $(WINE) $(shell if [ -f $(S51A) ]; then echo $(S51A); else echo $(S51B); fi)

ifndef CROSSCOMPILING
  SDCCFLAGS += --nostdinc -I$(INC_DIR)/mcs51 -I$(top_srcdir)
  LINKFLAGS += --nostdlib -L$(LIBDIR)
endif
endif

ifdef CROSSCOMPILING
  DEV_NULL ?= NUL
  SDCCFLAGS += -I$(top_srcdir)
else
  DEV_NULL ?= /dev/null
endif

SDCCFLAGS += 
LINKFLAGS += -lscrtc4xx -lsdcc -llong -lint -lfloat -llonglong -lscrtc4xx -lsdcc
DEBUGFLAGS = NO
OBJEXT = .rel
BINEXT = .ihx

# otherwise `make` deletes testfwk.rel and `make -j` will fail
.PRECIOUS: $(PORT_CASES_DIR)/%$(OBJEXT)

# Required extras
EXTRAS = $(PORT_CASES_DIR)/testfwk$(OBJEXT) $(PORT_CASES_DIR)/support$(OBJEXT)
include $(srcdir)/fwk/lib/spec.mk
FWKLIB += $(PORT_CASES_DIR)/T2_isr$(OBJEXT)

SPEC_LIB = $(PORTS_DIR)/mcs51-common/fwk.a

_clean:
