#include <iostream>
#include "Converter.h"

using namespace std;

int main()
{
    cxp::BMP bmp("/home/simon/Документы/GitHub/ConvertXpert/ConvertXpert/test/burger.bmp");
    bmp.print_headers();
    bmp.write("burger_copy.bmp");

    return 0;
}
