#include <iostream>
#include "Converter.h"

using namespace std;

int main()
{

    // Read an image from disk and write it back:
    cxp::BMP bmp("/home/simon/Документы/GitHub/ConvertXpert/ConvertXpert/test/Shapes.bmp");
    bmp.write("/home/simon/Документы/GitHub/ConvertXpert/ConvertXpert/test/Shapes_copy.bmp");

    // Create a BMP image in memory, modify it, save it on disk
    cxp::BMP bmp2(800, 600, 1);
    bmp2.fill_region(50, 20, 100, 200, 0, 0, 255, 255);
    bmp2.write("/home/simon/Документы/GitHub/ConvertXpert/ConvertXpert/test/img_test.bmp");

    // Create a 24 bits/pixel BMP image in memory, modify it, save it on disk
    cxp::BMP bmp3(209, 203, false);
    bmp3.fill_region(50, 20, 100, 100, 255, 0, 255, 255);
    bmp3.write("/home/simon/Документы/GitHub/ConvertXpert/ConvertXpert/test/img_test_24bits.bmp");

    return 0;
}
