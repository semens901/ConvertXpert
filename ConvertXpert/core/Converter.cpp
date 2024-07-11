#include "Converter.h"

template <typename T>
cxp::Converter<T>::Converter(std::string file_name)
{
    this->file_name = file_name;
}

cxp::BMP::BMP(const char *fname) 
{
    read(fname);
}

cxp::BMP::BMP(int32_t width, int32_t height, bool has_alpha = true)
{
    if (width <= 0 || height <= 0) 
    {
        throw std::runtime_error("The image width and height must be positive numbers.");
    }

    info_header.width = width;
    info_header.height = height;
    if (has_alpha) 
    {
        info_header.size = sizeof(BMPInfoHeader) + sizeof(BMPColorHeader);
        file_header.offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + sizeof(BMPColorHeader);

        info_header.bit_count = 32;
        info_header.compression = 3;
        row_stride = width * 4;
        data.resize(row_stride * height);
        file_header.file_size = file_header.offset_data + data.size();
    }
    else 
    {
        info_header.size = sizeof(BMPInfoHeader);
        file_header.offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);

        info_header.bit_count = 24;
        info_header.compression = 0;
        row_stride = width * 3;
        data.resize(row_stride * height);

        uint32_t new_stride = make_stride_aligned(4);
        file_header.file_size = file_header.offset_data + data.size() + info_header.height * (new_stride - row_stride);
    }
}


void cxp::BMP::read(const char *fname) 
{
    std::ifstream reader{ fname, std::ios_base::binary };
    if (reader) 
    {
        reader.read((char*)&file_header, sizeof(file_header));
        if(file_header.file_type != BMP_TYPE) 
        {
            throw std::runtime_error("Error! Unrecognized file format.");
        }
        reader.read((char*)&info_header, sizeof(info_header));

        // The BMPColorHeader is used only for transparent images
        if(info_header.bit_count == 32) 
        {
            // Check if the file has bit mask color information
            if(info_header.size >= (sizeof(BMPInfoHeader) + sizeof(BMPColorHeader))) 
            {
                reader.read((char*)&color_header, sizeof(color_header));
                // Check if the pixel data is stored as BGRA and if the color space type is sRGB
                check_color_header(color_header);
            } 
            else 
            {
                std::cerr << "Warning! The file \"" << fname << "\" does not seem to contain bit mask information\n";
                throw std::runtime_error("Error! Unrecognized file format.");
            }
        }

        // Jump to the pixel data location
        reader.seekg(file_header.offset_data, reader.beg);

        // Adjust the header fields for output.
        // Some editors will put extra info in the image file, we only save the headers and the data.
        if(info_header.bit_count == 32) 
        {
            info_header.size = sizeof(BMPInfoHeader) + sizeof(BMPColorHeader);
            file_header.offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + sizeof(BMPColorHeader);
        } 
        else 
        {
            info_header.size = sizeof(BMPInfoHeader);
            file_header.offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
        }
        file_header.file_size = file_header.offset_data;

        if (info_header.height < 0) 
        {
            throw std::runtime_error("The program can treat only BMP images with the origin in the bottom left corner!");
        }

        data.resize(info_header.width * info_header.height * info_header.bit_count / 8);

        // Here we check if we need to take into account row padding
        if (info_header.width % 4 == 0) 
        {
            reader.read((char*)data.data(), data.size());
            file_header.file_size += data.size();
        }
        else 
        {
            row_stride = info_header.width * info_header.bit_count / 8;
            uint32_t new_stride = make_stride_aligned(4);
            std::vector<uint8_t> padding_row(new_stride - row_stride);

            for (int y = 0; y < info_header.height; ++y) 
            {
                reader.read((char*)(data.data() + row_stride * y), row_stride);
                reader.read((char*)padding_row.data(), padding_row.size());
            }
            file_header.file_size += data.size() + info_header.height * padding_row.size();
        }
    }
    else {
        throw std::runtime_error("Unable to open the input image file.");
    }
}


void cxp::BMP::check_color_header(BMPColorHeader& color_header) 
{
    BMPColorHeader expected_color_header;
    if(expected_color_header.red_mask != color_header.red_mask ||
        expected_color_header.blue_mask != color_header.blue_mask ||
        expected_color_header.green_mask != color_header.green_mask ||
        expected_color_header.alpha_mask != color_header.alpha_mask) 
    {
        throw std::runtime_error("Unexpected color mask format! The program expects the pixel data to be in the BGRA format");
    }
    if(expected_color_header.color_space_type != color_header.color_space_type) 
    {
        throw std::runtime_error("Unexpected color space type! The program expects sRGB values");
    }
}

void cxp::BMP::write(const char *fname)
{
    std::ofstream of{ fname, std::ios_base::binary };
    if (of) 
    {
        if (info_header.bit_count == 32) 
        {
            write_headers_and_data(of);
        }
        else if (info_header.bit_count == 24) 
        {
            if (info_header.width % 4 == 0) 
            {
                write_headers_and_data(of);
            }
            else 
            {
                uint32_t new_stride = make_stride_aligned(4);
                std::vector<uint8_t> padding_row(new_stride - row_stride);

                write_headers(of);

                for (int y = 0; y < info_header.height; ++y) {
                    of.write((const char*)(data.data() + row_stride * y), row_stride);
                    of.write((const char*)padding_row.data(), padding_row.size());
                }
            }
        }
        else 
        {
            throw std::runtime_error("The program can treat only 24 or 32 bits per pixel BMP files");
        }
    }
    else 
    {
        throw std::runtime_error("Unable to open the output image file.");
    }
}

void cxp::BMP::write_headers(std::ofstream &of)
{
    of.write((const char*)&file_header, sizeof(file_header));
    of.write((const char*)&info_header, sizeof(info_header));
    if(info_header.bit_count == 32) 
    {
        of.write((const char*)&color_header, sizeof(color_header));
    }
}

void cxp::BMP::write_headers_and_data(std::ofstream &of) 
{
    write_headers(of);
    of.write((const char*)data.data(), data.size());
}

uint32_t cxp::BMP::make_stride_aligned(uint32_t align_stride)
{
    uint32_t new_stride = row_stride;
    while (new_stride % align_stride != 0)
    {
        new_stride++;
    }
    return new_stride;
}

void cxp::BMP::fill_region(uint32_t x0, uint32_t y0, uint32_t w, uint32_t h, uint8_t B, uint8_t G, uint8_t R, uint8_t A) 
{
    if (x0 + w > (uint32_t)info_header.width || y0 + h > (uint32_t)info_header.height) {
        throw std::runtime_error("The region does not fit in the image!");
    }

    uint32_t channels = info_header.bit_count / 8;
    for (uint32_t y = y0; y < y0 + h; ++y) {
        for (uint32_t x = x0; x < x0 + w; ++x) {
            data[channels * (y * info_header.width + x) + 0] = B;
            data[channels * (y * info_header.width + x) + 1] = G;
            data[channels * (y * info_header.width + x) + 2] = R;
            if (channels == 4) {
                data[channels * (y * info_header.width + x) + 3] = A;
            }
        }
    }
}