#include <stdio.h>  // Required for printf
#include <stdint.h> // Required for uint16_t (standard C type for unsigned short with fixed width)

// Function: print_xif_tag_text
void print_xif_tag_text(uint16_t param_1) {
  if (param_1 < 0xa421) {
    if (param_1 < 0xa401) {
      if (param_1 == 0xa302) {
        printf("CFAPattern");
      }
      else if (param_1 < 0xa303) {
        if (param_1 == 0xa301) {
          printf("SceneType");
        }
        else if (param_1 < 0xa302) {
          if (param_1 == 0xa300) {
            printf("FileSource");
          }
          else if ((param_1 < 0xa301) && (param_1 < 0xa218)) {
            if (param_1 < 0xa20b) {
              if (param_1 < 0xa005) {
                if (param_1 < 0xa000) {
                  if (param_1 < 0x9293) {
                    if (param_1 < 0x927c) {
                      if (param_1 < 0x9215) {
                        if (param_1 < 0x9201) {
                          if (param_1 == 0x9102) {
                            printf("CompressedBitsPerPixel");
                          }
                          else if (param_1 < 0x9103) {
                            if (param_1 == 0x9101) {
                              printf("ComponentsConfiguration");
                            }
                            else if (param_1 < 0x9102) {
                              if (param_1 == 0x9004) {
                                printf("DateTimeDigitized");
                              }
                              else if (param_1 < 0x9005) {
                                if (param_1 == 0x9003) {
                                  printf("DateTimeOriginal");
                                }
                                else if (param_1 < 0x9004) {
                                  if (param_1 == 0x9000) {
                                    printf("ExifVersion");
                                  }
                                  else if (param_1 < 0x9001) {
                                    if (param_1 == 0x8828) {
                                      printf("OECF");
                                    }
                                    else if (param_1 < 0x8829) {
                                      if (param_1 == 0x8827) {
                                        printf("ISOSpeedRatings");
                                      }
                                      else if (param_1 < 0x8828) {
                                        if (param_1 == 0x8824) {
                                          printf("SpectralSensitivity");
                                        }
                                        else if (param_1 < 0x8825) {
                                          if (param_1 == 0x8822) {
                                            printf("ExposureProgram");
                                          }
                                          else if (param_1 < 0x8823) {
                                            if (param_1 == 0x829a) {
                                              printf("ExposureTime");
                                            }
                                            else if (param_1 == 0x829d) {
                                              printf("FNumber");
                                            }
                                          }
                                        }
                                      }
                                    }
                                  }
                                }
                              }
                            }
                          }
                        }
                        else {
                          switch(param_1) {
                          case 0x9201:
                            printf("ShutterSpeedValue");
                            break;
                          case 0x9202:
                            printf("ApertureValue");
                            break;
                          case 0x9203:
                            printf("BrightnessValue");
                            break;
                          case 0x9204:
                            printf("ExposureBiasValue");
                            break;
                          case 0x9205:
                            printf("MaxApertureValue");
                            break;
                          case 0x9206:
                            printf("SubjectDistance");
                            break;
                          case 0x9207:
                            printf("MeteringMode");
                            break;
                          case 0x9208:
                            printf("LightSource");
                            break;
                          case 0x9209:
                            printf("Flash");
                            break;
                          case 0x920a:
                            printf("FocalLength");
                            break;
                          case 0x9214:
                            printf("SubjectArea");
                            break; // Added missing break for 0x9214, although it's the last case.
                          }
                        }
                      }
                    }
                    else {
                      switch(param_1) {
                      case 0x927c:
                        printf("MakeNote");
                        break;
                      case 0x9286:
                        printf("UserComment");
                        break;
                      case 0x9290:
                        printf("SubSecTime");
                        break;
                      case 0x9291:
                        printf("SubSecTimeOriginal");
                        break;
                      case 0x9292:
                        printf("SubSecTimeDigitized");
                        break; // Added missing break for 0x9292, although it's the last case.
                      }
                    }
                  }
                }
                else {
                  switch(param_1) {
                  case 0xa000:
                    printf("FlashpixVersion");
                    break;
                  case 0xa001:
                    printf("ColorSpace");
                    break;
                  case 0xa002:
                    printf("PixelXDimension");
                    break;
                  case 0xa003:
                    printf("PixelYDimension");
                    break;
                  case 0xa004:
                    printf("RelatedSoundFile");
                    break; // Added missing break for 0xa004, although it's the last case.
                  }
                }
              }
            }
            else {
              switch(param_1) {
              case 0xa20b:
                printf("FlashEnergy");
                break;
              case 0xa20c:
                printf("SpatialFrequencyResponse");
                break;
              case 0xa20e:
                printf("FocalPlaneXResolution");
                break;
              case 0xa20f:
                printf("FocalPlaneYResolution");
                break;
              case 0xa210:
                printf("FocalPlaneResolutionUnit");
                break;
              case 0xa214:
                printf("SubjectLocation");
                break;
              case 0xa215:
                printf("ExposureIndex");
                break;
              case 0xa217:
                printf("SensingMethod");
                break; // Added missing break for 0xa217, although it's the last case.
              }
            }
          }
        }
      }
    }
    else {
      switch(param_1) {
      case 0xa401:
        printf("CustomRendered");
        break;
      case 0xa402:
        printf("ExposureMode");
        break;
      case 0xa403:
        printf("WhiteBalance");
        break;
      case 0xa404:
        printf("DigitalZoomRatio");
        break;
      case 0xa405:
        printf("FocalLengthin35mmFilm");
        break;
      case 0xa406:
        printf("SceneCaptureType");
        break;
      case 0xa407:
        printf("GainControl");
        break;
      case 0xa408:
        printf("Contrast");
        break;
      case 0xa409:
        printf("Saturation");
        break;
      case 0xa40a:
        printf("Sharpness");
        break;
      case 0xa40b:
        printf("DeviceSettingDescription");
        break;
      case 0xa40c:
        printf("SubjectDistanceRange");
        break;
      case 0xa420:
        printf("ImageUniqueID");
        break; // Added missing break for 0xa420, although it's the last case.
      }
    }
  }
  return;
}

int main() {
    printf("Testing print_xif_tag_text:\n");

    // Test cases that should print something
    printf("0x829a: "); print_xif_tag_text(0x829a); printf("\n"); // ExposureTime
    printf("0x8828: "); print_xif_tag_text(0x8828); printf("\n"); // OECF
    printf("0x9000: "); print_xif_tag_text(0x9000); printf("\n"); // ExifVersion
    printf("0x9102: "); print_xif_tag_text(0x9102); printf("\n"); // CompressedBitsPerPixel
    printf("0x9201: "); print_xif_tag_text(0x9201); printf("\n"); // ShutterSpeedValue
    printf("0x9214: "); print_xif_tag_text(0x9214); printf("\n"); // SubjectArea
    printf("0x927c: "); print_xif_tag_text(0x927c); printf("\n"); // MakeNote
    printf("0x9292: "); print_xif_tag_text(0x9292); printf("\n"); // SubSecTimeDigitized
    printf("0xa000: "); print_xif_tag_text(0xa000); printf("\n"); // FlashpixVersion
    printf("0xa004: "); print_xif_tag_text(0xa004); printf("\n"); // RelatedSoundFile
    printf("0xa20b: "); print_xif_tag_text(0xa20b); printf("\n"); // FlashEnergy
    printf("0xa217: "); print_xif_tag_text(0xa217); printf("\n"); // SensingMethod
    printf("0xa300: "); print_xif_tag_text(0xa300); printf("\n"); // FileSource
    printf("0xa301: "); print_xif_tag_text(0xa301); printf("\n"); // SceneType
    printf("0xa302: "); print_xif_tag_text(0xa302); printf("\n"); // CFAPattern
    printf("0xa401: "); print_xif_tag_text(0xa401); printf("\n"); // CustomRendered
    printf("0xa420: "); print_xif_tag_text(0xa420); printf("\n"); // ImageUniqueID

    // Test cases that should print nothing (fall-through behavior of original code)
    printf("0x1000: "); print_xif_tag_text(0x1000); printf("\n"); // Arbitrary value below any defined tag
    printf("0x829b: "); print_xif_tag_text(0x829b); printf("\n"); // Between 0x829a and 0x829d
    printf("0x9001: "); print_xif_tag_text(0x9001); printf("\n"); // Between 0x9000 and 0x9003
    printf("0x9216: "); print_xif_tag_text(0x9216); printf("\n"); // Between 0x9214 and 0x927c
    printf("0xa216: "); print_xif_tag_text(0xa216); printf("\n"); // Between 0xa215 and 0xa217
    printf("0xa303: "); print_xif_tag_text(0xa303); printf("\n"); // Above 0xa302, below 0xa401
    printf("0xffff: "); print_xif_tag_text(0xffff); printf("\n"); // Arbitrary value above all defined tags
    printf("0xa421: "); print_xif_tag_text(0xa421); printf("\n"); // Boundary case, `param_1 < 0xa421` is false

    return 0;
}