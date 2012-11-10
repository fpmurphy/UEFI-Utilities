#
#  Makefile for listcerts.efi.
#

TARGET     = listcerts.efi
SOURCES    = asn1_ber_decoder.c oid_registry.c x509.c
OBJS       = asn1_ber_decoder.o oid_registry.o x509.o

PREFIX     := /usr
ARCH       := $(shell uname -m | sed s,i[3456789]86,ia32,)
HOSTARCH   = $(shell uname -m | sed s,i[3456789]86,ia32,)
LDFLAGS	   = -nostdlib

CC         = gcc
AS         = as
LD         = ld.bfd
AR         = ar
RM         = rm -f
RANLIB     = ranlib
OBJCOPY    = objcopy
FORMAT     = efi-app-$(ARCH)

ifeq ($(ARCH), ia32)
  LIBDIR := $(PREFIX)/lib
  ifeq ($(HOSTARCH), x86_64)
    ARCH3264 := -m32
  endif
endif

ifeq ($(ARCH), x86_64)
  CFLAGS += -mno-red-zone -DEFI_FUNCTION_WRAPPER
  LIBDIR := $(PREFIX)/lib64
  ifeq ($(HOSTARCH), ia32)
    ARCH3264 := -m64
  endif
endif

INCDIR	   =  -I/usr/include/efi -I/usr/include/efi/$(ARCH) -I/usr/include/efi/protocol
CPPFLAGS   = -DCONFIG_$(ARCH)
CFLAGS	   += -O2 -fpic -Wall -fshort-wchar -fno-strict-aliasing -fno-merge-constants
LDFLAGS	   += -T $(LIBDIR)/gnuefi/elf_$(HOSTARCH)_efi.lds -shared -Bsymbolic /usr/lib64/gnuefi/crt0-efi-$(ARCH).o -L /usr/lib64
LOADLIBES  = -lefi -lgnuefi $(shell $(CC) -print-libgcc-file-name)


all: $(TARGET)

%.efi: %.so
	$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel \
		-j .rela -j .reloc --target=$(FORMAT) $*.so $@

%.so: %.o $(OBJS) 
	$(LD) $(LDFLAGS) $^ -o $@ $(LOADLIBES)
	nm -D $@ | grep ' U ' && exit 1 || exit 0

%.o: %.c 
	$(CC) $(INCDIR) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

x509.c x509.h: x509.asn1 asn1com Makefile
	./asn1com x509.asn1 x509.c x509.h

asn1com: asn1_compiler.c asn1_ber_bytecode.h Makefile
	$(CC) -Wall -O2 -o $@ asn1_compiler.c

oid_registry_data.h: oid_registry.h
	./build_oid_registry_data.py

clean:
	$(RM) $(TARGET) asn1com *.so *.o x509.c x509.h
