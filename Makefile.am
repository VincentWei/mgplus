SUBDIRS = src include m4 cmake

COMM_HDRS = mgplusconfig.h

# The directory where the include files will be installed
libmgplusconfigdir = $(includedir)/mgplus

# Which header files to install
libmgplusconfig_HEADERS = $(COMM_HDRS)

EXTRA_DIST = INSTALL README autogen.sh

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = mgplus.pc
