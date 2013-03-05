TARGET     = showhotkeys.efi
SRCS       = showhotkeys.c efilink.c

SRCDIR     = .
PREFIX     := /usr
HOSTARCH   = $(shell uname -m | sed s,i[3456789]86,ia32,)
ARCH       := $(shell uname -m | sed s,i[3456789]86,ia32,)
INCDIR     = -I.
CPPFLAGS   = -DCONFIG_$(ARCH)
ASFLAGS    = $(ARCH3264)
LDFLAGS    = -nostdlib
INSTALL    = install

CC         = gcc
AS         = as
LD         = ld.bfd
AR         = ar
RANLIB     = ranlib
OBJCOPY    = objcopy

ifeq ($(ARCH), ia32)
  LIBDIR := $(PREFIX)/lib
  ifeq ($(HOSTARCH), x86_64)
    ARCH3264 := -m32
  endif
endif

ifeq ($(ARCH), x86_64)
  CFLAGS += -mno-red-zone
  LIBDIR := $(PREFIX)/lib64
  ifeq ($(HOSTARCH), ia32)
    ARCH3264 := -m64
  endif
endif

OBJS = $(SRCS:.c=.o)

FORMAT = efi-app-$(HOSTARCH)
LDFLAGS = -nostdlib -T $(LIBDIR)/gnuefi/elf_$(HOSTARCH)_efi.lds -shared -Bsymbolic $(LIBDIR)/gnuefi/crt0-efi-$(HOSTARCH).o -L$(LIBDIR)
LIBS = -lefi -lgnuefi $(shell $(CC) -print-libgcc-file-name)
CCLDFLAGS =
CFLAGS = -I/usr/include/efi/ -I/usr/include/efi/$(HOSTARCH)/ -I/usr/include/efi/protocol -fpic -fshort-wchar -fno-reorder-functions -fno-strict-aliasing -fno-merge-constants -mno-red-zone -Wimplicit-function-declaration


all : $(TARGET)

.PHONY: all clean install

%.efi: %.so
	$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel \
		-j .rela -j .reloc --target=$(FORMAT) $*.so $@

$(TARGET:.efi=.so): $(OBJS) 
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(INCDIR) $(CFLAGS) $(CPPFLAGS) -D__UEFI__ -c $< -o $@

clean : 
	@rm -rf *.o *.a *.so $(TARGET)
