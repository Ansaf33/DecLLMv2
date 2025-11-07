#include <stdio.h>  // For printf
#include <stdint.h> // For uint16_t

// Function: print_tag_text
void print_tag_text(uint16_t param_1) {
    if (param_1 == 0xc4a5) {
        printf("");
    } else if (param_1 == 0x8825) {
        printf("GPS Info");
    } else if (param_1 == 0x8769) {
        printf("EXIF IFD Pointer");
    } else if (param_1 == 0x8298) {
        printf("Copyright");
    } else if (param_1 >= 0x100 && param_1 <= 0x13f) {
        switch (param_1) {
            case 0x100: printf("ImageWidth"); break;
            case 0x101: printf("ImageLength"); break;
            case 0x102: printf("BitsPerSample"); break;
            case 0x103: printf("Compression"); break;
            case 0x106: printf("PhotometricInterpretation"); break;
            case 0x10e: printf("ImageDescription"); break;
            case 0x10f: printf("Make"); break;
            case 0x110: printf("Model"); break;
            case 0x111: printf("StripOffsets"); break;
            case 0x112: printf("Orientation"); break;
            case 0x115: printf("SamplesPerPixel"); break;
            case 0x116: printf("RowsPerStrip"); break;
            case 0x117: printf("StripByteCounts"); break;
            case 0x11a: printf("XResolution"); break;
            case 0x11b: printf("YResolution"); break;
            case 0x11c: printf("PlanarConfiguration"); break;
            case 0x128: printf("ResolutionUnit"); break;
            case 0x12d: printf("TransferFunction"); break;
            case 0x131: printf("Software"); break;
            case 0x132: printf("DateTime"); break;
            case 0x13b: printf("Artist"); break;
            case 0x13e: printf("WhitePoint"); break;
            case 0x13f: printf("PrimaryChromaticities"); break;
        }
    } else if (param_1 >= 0x201 && param_1 <= 0x214) {
        switch (param_1) {
            case 0x201: printf("InterchangeFormat"); break;
            case 0x202: printf("InterchangeFormatLength"); break;
            case 0x211: printf("YCbCrCoefficients"); break;
            case 0x212: printf("YCbCrSubSampling"); break;
            case 0x213: printf("YCbCrPositioning"); break;
            case 0x214: printf("ReferenceBlackWhite"); break;
        }
    }
}