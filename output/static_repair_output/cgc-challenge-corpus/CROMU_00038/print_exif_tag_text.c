#include <stdio.h>  // Required for printf
#include <stdint.h> // Required for uint16_t (explicit 16-bit unsigned integer)

// Function: print_xif_tag_text
// Replaced 'ushort' with standard C 'uint16_t' and refactored the deeply nested
// if-else if structure into a single switch statement for clarity and maintainability.
// This also implicitly handles the original top-level 'if (param_1 < 0xa421)'
// by having a default case that does nothing for unmatched values,
// preserving the original behavior of not printing anything for unknown tags.
void print_xif_tag_text(uint16_t param_1) {
    switch (param_1) {
        case 0x829a: printf("ExposureTime"); break;
        case 0x829d: printf("FNumber"); break;
        case 0x8822: printf("ExposureProgram"); break;
        case 0x8824: printf("SpectralSensitivity"); break;
        case 0x8827: printf("ISOSpeedRatings"); break;
        case 0x8828: printf("OECF"); break;
        case 0x9000: printf("ExifVersion"); break;
        case 0x9003: printf("DateTimeOriginal"); break;
        case 0x9004: printf("DateTimeDigitized"); break;
        case 0x9101: printf("ComponentsConfiguration"); break;
        case 0x9102: printf("CompressedBitsPerPixel"); break;
        case 0x9201: printf("ShutterSpeedValue"); break;
        case 0x9202: printf("ApertureValue"); break;
        case 0x9203: printf("BrightnessValue"); break;
        case 0x9204: printf("ExposureBiasValue"); break;
        case 0x9205: printf("MaxApertureValue"); break;
        case 0x9206: printf("SubjectDistance"); break;
        case 0x9207: printf("MeteringMode"); break;
        case 0x9208: printf("LightSource"); break;
        case 0x9209: printf("Flash"); break;
        case 0x920a: printf("FocalLength"); break;
        case 0x9214: printf("SubjectArea"); break;
        case 0x927c: printf("MakeNote"); break;
        case 0x9286: printf("UserComment"); break;
        case 0x9290: printf("SubSecTime"); break;
        case 0x9291: printf("SubSecTimeOriginal"); break;
        case 0x9292: printf("SubSecTimeDigitized"); break;
        case 0xa000: printf("FlashpixVersion"); break;
        case 0xa001: printf("ColorSpace"); break;
        case 0xa002: printf("PixelXDimension"); break;
        case 0xa003: printf("PixelYDimension"); break;
        case 0xa004: printf("RelatedSoundFile"); break;
        case 0xa20b: printf("FlashEnergy"); break;
        case 0xa20c: printf("SpatialFrequencyResponse"); break;
        case 0xa20e: printf("FocalPlaneXResolution"); break;
        case 0xa20f: printf("FocalPlaneYResolution"); break;
        case 0xa210: printf("FocalPlaneResolutionUnit"); break;
        case 0xa214: printf("SubjectLocation"); break;
        case 0xa215: printf("ExposureIndex"); break;
        case 0xa217: printf("SensingMethod"); break;
        case 0xa300: printf("FileSource"); break;
        case 0xa301: printf("SceneType"); break;
        case 0xa302: printf("CFAPattern"); break;
        case 0xa401: printf("CustomRendered"); break;
        case 0xa402: printf("ExposureMode"); break;
        case 0xa403: printf("WhiteBalance"); break;
        case 0xa404: printf("DigitalZoomRatio"); break;
        case 0xa405: printf("FocalLengthin35mmFilm"); break;
        case 0xa406: printf("SceneCaptureType"); break;
        case 0xa407: printf("GainControl"); break;
        case 0xa408: printf("Contrast"); break;
        case 0xa409: printf("Saturation"); break;
        case 0xa40a: printf("Sharpness"); break;
        case 0xa40b: printf("DeviceSettingDescription"); break;
        case 0xa40c: printf("SubjectDistanceRange"); break;
        case 0xa420: printf("ImageUniqueID"); break;
        default:
            // No action for unknown tags, preserving original behavior
            break;
    }
}

// Main function to demonstrate the usage of print_xif_tag_text.
int main() {
    printf("Testing print_xif_tag_text:\n");

    printf("0x829a: ");
    print_xif_tag_text(0x829a); // ExposureTime
    printf("\n");

    printf("0x9207: ");
    print_xif_tag_text(0x9207); // MeteringMode
    printf("\n");

    printf("0xa405: ");
    print_xif_tag_text(0xa405); // FocalLengthin35mmFilm
    printf("\n");

    printf("0x9214: ");
    print_xif_tag_text(0x9214); // SubjectArea
    printf("\n");

    printf("0xa214: ");
    print_xif_tag_text(0xa214); // SubjectLocation
    printf("\n");

    printf("0xDEAD: ");
    print_xif_tag_text(0xDEAD); // Unknown tag (should print nothing)
    printf("\n");

    printf("0xBEEF: ");
    print_xif_tag_text(0xBEEF); // Unknown tag (outside original top-level range, should print nothing)
    printf("\n");

    return 0;
}