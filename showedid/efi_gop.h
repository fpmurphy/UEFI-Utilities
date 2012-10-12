#ifndef EFI_GOP_HEADER
#define EFI_GOP_HEADER	1

typedef enum {
    EFI_GOT_RGBA8,
    EFI_GOT_BGRA8,
    EFI_GOT_BITMASK
} efi_gop_pixel_format_t;

typedef enum {
    EFI_BLT_VIDEO_FILL,
    EFI_BLT_VIDEO_TO_BLT_BUFFER,
    EFI_BLT_BUFFER_TO_VIDEO,
    EFI_BLT_VIDEO_TO_VIDEO,
    EFI_BLT_OPERATION_MAX
} efi_gop_blt_operation_t;

struct efi_gop_blt_pixel {
    UINT8 blue;
    UINT8 green;
    UINT8 red;
    UINT8 reserved;
};

struct efi_gop_pixel_bitmask {
    UINT32 r;
    UINT32 g;
    UINT32 b;
    UINT32 a;
};

struct efi_gop_mode_info {
    UINT32 version;
    UINT32 width;
    UINT32 height;
    efi_gop_pixel_format_t pixel_format;
    struct efi_gop_pixel_bitmask pixel_bitmask;
    UINT32 pixels_per_scanline;
};

struct efi_gop_mode {
    UINT32 max_mode;
    UINT32 mode;
    struct efi_gop_mode_info *info;
    UINTN info_size;
    UINT64 fb_base;
    UINTN fb_size;
};

/* Forward declaration.  */
struct efi_gop;

typedef EFI_STATUS
(*efi_gop_query_mode_t) (struct efi_gop *this,
			      UINT32 mode_number,
			      UINTN *size_of_info,
			      struct efi_gop_mode_info **info);

typedef EFI_STATUS
(*efi_gop_set_mode_t) (struct efi_gop *this,
			    UINT32 mode_number);

typedef EFI_STATUS
(*efi_gop_blt_t) (struct efi_gop *this,
		       void *buffer,
		       UINTN operation,
		       UINTN sx,
		       UINTN sy,
		       UINTN dx,
		       UINTN dy,
		       UINTN width,
		       UINTN height,
		       UINTN delta);

struct efi_gop {
    efi_gop_query_mode_t query_mode;
    efi_gop_set_mode_t set_mode;
    efi_gop_blt_t blt;
    struct efi_gop_mode *mode;
};

#endif
