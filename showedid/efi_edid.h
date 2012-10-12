#ifndef EFI_EDID_HEADER
#define EFI_EDID_HEADER  1

#include <efi.h>

#define EFI_EDID_ACTIVE_PROTOCOL_GUID \
   (EFI_GUID) {0xbd8c1056, 0x9f36, 0x44ec, {0x92,0xa8,0xa6,0x33,0x7f,0x81,0x79,0x86}}

#define EFI_EDID_DISCOVERED_PROTOCOL_GUID \
   (EFI_GUID) {0x1c0c34f6, 0xd380, 0x41fa, {0xa0,0x49,0x8a,0xd0,0x6c,0x1a,0x66,0xaa}}

typedef struct {
    UINT8   Header[8];                        //EDID header "00 FF FF FF FF FF FF 00"
    UINT16  ManufactureName;                  //EISA 3-character ID
    UINT16  ProductCode;                      //Vendor assigned code
    UINT32  SerialNumber;                     //32-bit serial number
    UINT8   WeekOfManufacture;                //Week number
    UINT8   YearOfManufacture;                //Year
    UINT8   EdidVersion;                      //EDID Structure Version
    UINT8   EdidRevision;                     //EDID Structure Revision
    UINT8   VideoInputDefinition;
    UINT8   MaxHorizontalImageSize;           //cm
    UINT8   MaxVerticalImageSize;             //cm
    UINT8   DisplayGamma;
    UINT8   DpmSupport;
    UINT8   RedGreenLowBits;                  //Rx1 Rx0 Ry1 Ry0 Gx1 Gx0 Gy1Gy0
    UINT8   BlueWhiteLowBits;                 //Bx1 Bx0 By1 By0 Wx1 Wx0 Wy1 Wy0
    UINT8   RedX;                             //Red-x Bits 9 - 2
    UINT8   RedY;                             //Red-y Bits 9 - 2
    UINT8   GreenX;                           //Green-x Bits 9 - 2
    UINT8   GreenY;                           //Green-y Bits 9 - 2
    UINT8   BlueX;                            //Blue-x Bits 9 - 2
    UINT8   BlueY;                            //Blue-y Bits 9 - 2
    UINT8   WhiteX;                           //White-x Bits 9 - 2
    UINT8   WhiteY;                           //White-x Bits 9 - 2
    UINT8   EstablishedTimings[3];
    UINT8   StandardTimingIdentification[16];
    UINT8   DescriptionBlock1[18];
    UINT8   DescriptionBlock2[18];
    UINT8   DescriptionBlock3[18];
    UINT8   DescriptionBlock4[18];
    UINT8   ExtensionFlag;                    //Number of (optional) 128-byte EDID extension blocks to follow
    UINT8   Checksum;
} EDID_DATA_BLOCK;

typedef struct {
    UINT32  SizeOfEdid;
    UINT8   *Edid;
} EFI_EDID_ACTIVE_PROTOCOL;

typedef struct {
    UINT32  SizeOfEdid;
    UINT8   *Edid;
} EFI_EDID_DETECTED_PROTOCOL;

// most of these defines come straight out of NetBSD edid source code
// #define CHECK_BIT(var,pos)                ((var) & (1<<(pos)))
#define CHECK_BIT(var,pos)                ((var) & (1<<(pos)) == (1<<(pos)))
#define EDID_COMBINE_HI_8LO( hi, lo )     ((((unsigned)hi) << 8) | (unsigned)lo )
#define EDID_LENGTH                       0x80
#define EDID_VIDEO_INPUT_DFP1_COMPAT      0x01
#define EDID_VIDEO_INPUT_LEVEL(x)         (((x) & 0x60) >> 5)
#define EDID_DPMS_ACTIVE_OFF              (1 << 5)
#define EDID_DPMS_SUSPEND                 (1 << 6)
#define EDID_DPMS_STANDBY                 (1 << 7)
#define EDID_STD_TIMING_HRES(ptr)         ((((ptr)[0]) * 8) + 248)
#define EDID_STD_TIMING_VFREQ(ptr)        ((((ptr)[1]) & 0x3f) + 60)
#define EDID_STD_TIMING_RATIO(ptr)        ((ptr)[1] & 0xc0)
#define EDID_BLOCK_IS_DET_TIMING(ptr)     ((ptr)[0] | (ptr)[1])
#define EDID_DET_TIMING_DOT_CLOCK(ptr)    (((ptr)[0] | ((ptr)[1] << 8)) * 10000)
#define EDID_HACT_LO(ptr)                 ((ptr)[2])
#define EDID_HBLK_LO(ptr)                 ((ptr)[3])
#define EDID_HACT_HI(ptr)                 (((ptr)[4] & 0xf0) << 4)
#define EDID_HBLK_HI(ptr)                 (((ptr)[4] & 0x0f) << 8)
#define EDID_DET_TIMING_HACTIVE(ptr)      (EDID_HACT_LO(ptr) | EDID_HACT_HI(ptr))
#define EDID_DET_TIMING_HBLANK(ptr)       (EDID_HBLK_LO(ptr) | EDID_HBLK_HI(ptr))
#define EDID_VACT_LO(ptr)                 ((ptr)[5])
#define EDID_VBLK_LO(ptr)                 ((ptr)[6])
#define EDID_VACT_HI(ptr)                 (((ptr)[7] & 0xf0) << 4)
#define EDID_VBLK_HI(ptr)                 (((ptr)[7] & 0x0f) << 8)
#define EDID_DET_TIMING_VACTIVE(ptr)      (EDID_VACT_LO(ptr) | EDID_VACT_HI(ptr))
#define EDID_DET_TIMING_VBLANK(ptr)       (EDID_VBLK_LO(ptr) | EDID_VBLK_HI(ptr))
#define EDID_HOFF_LO(ptr)                 ((ptr)[8])
#define EDID_HWID_LO(ptr)                 ((ptr)[9])
#define EDID_VOFF_LO(ptr)                 ((ptr)[10] >> 4)
#define EDID_VWID_LO(ptr)                 ((ptr)[10] & 0xf)
#define EDID_HOFF_HI(ptr)                 (((ptr)[11] & 0xc0) << 2)
#define EDID_HWID_HI(ptr)                 (((ptr)[11] & 0x30) << 4)
#define EDID_VOFF_HI(ptr)                 (((ptr)[11] & 0x0c) << 2)
#define EDID_VWID_HI(ptr)                 (((ptr)[11] & 0x03) << 4)
#define EDID_DET_TIMING_HSYNC_OFFSET(ptr) (EDID_HOFF_LO(ptr) | EDID_HOFF_HI(ptr))
#define EDID_DET_TIMING_HSYNC_WIDTH(ptr)  (EDID_HWID_LO(ptr) | EDID_HWID_HI(ptr))
#define EDID_DET_TIMING_VSYNC_OFFSET(ptr) (EDID_VOFF_LO(ptr) | EDID_VOFF_HI(ptr))
#define EDID_DET_TIMING_VSYNC_WIDTH(ptr)  (EDID_VWID_LO(ptr) | EDID_VWID_HI(ptr))
#define EDID_HSZ_LO(ptr)                  ((ptr)[12])
#define EDID_VSZ_LO(ptr)                  ((ptr)[13])
#define EDID_HSZ_HI(ptr)                  (((ptr)[14] & 0xf0) << 4)
#define EDID_VSZ_HI(ptr)                  (((ptr)[14] & 0x0f) << 8)
#define EDID_DET_TIMING_HSIZE(ptr)        (EDID_HSZ_LO(ptr) | EDID_HSZ_HI(ptr))
#define EDID_DET_TIMING_VSIZE(ptr)        (EDID_VSZ_LO(ptr) | EDID_VSZ_HI(ptr))
#define EDID_DET_TIMING_HBORDER(ptr)      ((ptr)[15])
#define EDID_DET_TIMING_VBORDER(ptr)      ((ptr)[16])
#define EDID_DET_TIMING_FLAGS(ptr)        ((ptr)[17])
#define EDID_DET_TIMING_VSOBVHSPW(ptr)    ((ptr)[11])

#endif 
