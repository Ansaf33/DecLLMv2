#include <stdio.h>   // Required for printf
#include <stdint.h>  // Required for uint16_t (standard fixed-width integer type)

// Function: print_tag_text
void print_tag_text(uint16_t param_1) {
  if (param_1 == 0xc4a5) {
    printf("");
  }
  else if (param_1 < 0xc4a6) {
    if (param_1 == 0x8825) {
      printf("GPS Info");
    }
    else if (param_1 < 0x8826) {
      if (param_1 == 0x8769) {
        printf("EXIF IFD Pointer");
      }
      else if (param_1 < 0x876a) {
        if (param_1 == 0x8298) {
          printf("Copyright");
        }
        else if (param_1 < 0x8299) {
          if (param_1 < 0x140) {
            if (0xff < param_1) {
              switch(param_1) {
              case 0x100:
                printf("ImageWidth");
                break;
              case 0x101:
                printf("ImageLength");
                break;
              case 0x102:
                printf("BitsPerSample");
                break;
              case 0x103:
                printf("Compression");
                break;
              case 0x106:
                printf("PhotometricInterpretation");
                break;
              case 0x10e:
                printf("ImageDescription");
                break;
              case 0x10f:
                printf("Make");
                break;
              case 0x110:
                printf("Model");
                break;
              case 0x111:
                printf("StripOffsets");
                break;
              case 0x112:
                printf("Orientation");
                break;
              case 0x115:
                printf("SamplesPerPixel");
                break;
              case 0x116:
                printf("RowsPerStrip");
                break;
              case 0x117:
                printf("StripByteCounts");
                break;
              case 0x11a:
                printf("XResolution");
                break;
              case 0x11b:
                printf("YResolution");
                break;
              case 0x11c:
                printf("PlanarConfiguration");
                break;
              case 0x128:
                printf("ResolutionUnit");
                break;
              case 0x12d:
                printf("TransferFunction");
                break;
              case 0x131:
                printf("Software");
                break;
              case 0x132:
                printf("DateTime");
                break;
              case 0x13b:
                printf("Artist");
                break;
              case 0x13e:
                printf("WhitePoint");
                break;
              case 0x13f:
                printf("PrimaryChromaticities");
                break;
              }
            }
          }
          else if ((param_1 < 0x215) && (0x200 < param_1)) {
            switch(param_1) {
            case 0x201:
              printf("InterchangeFormat");
              break;
            case 0x202:
              printf("InterchangeFormatLength");
              break;
            case 0x211:
              printf("YCbCrCoefficients");
              break;
            case 0x212:
              printf("YCbCrSubSampling");
              break;
            case 0x213:
              printf("YCbCrPositioning");
              break;
            case 0x214:
              printf("ReferenceBlackWhite");
              break;
            }
          }
        }
      }
    }
  }
  return;
}

// Minimal main function to make the code compilable and runnable
int main() {
    printf("Testing print_tag_text:\n");
    printf("Tag 0xc4a5: "); print_tag_text(0xc4a5); printf("[empty string]\n");
    printf("Tag 0x8825: "); print_tag_text(0x8825); printf("\n");
    printf("Tag 0x8769: "); print_tag_text(0x8769); printf("\n");
    printf("Tag 0x8298: "); print_tag_text(0x8298); printf("\n");
    printf("Tag 0x100:  "); print_tag_text(0x100); printf("\n");
    printf("Tag 0x13f:  "); print_tag_text(0x13f); printf("\n");
    printf("Tag 0x201:  "); print_tag_text(0x201); printf("\n");
    printf("Tag 0x214:  "); print_tag_text(0x214); printf("\n");
    printf("Tag 0x0:    "); print_tag_text(0x0); printf("[no output]\n");
    printf("Tag 0x1ff:  "); print_tag_text(0x1ff); printf("[no output]\n");
    printf("Tag 0xffff: "); print_tag_text(0xffff); printf("[no output]\n");
    return 0;
}