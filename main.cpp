#include "windows.h"
#include "zip_packer.h"




int main()
{
    ZipPacker packer;
    auto result = packer.init_package("result.zip", "qazwsx123");
    result = packer.pack("C:/test", "C:/");
    result = packer.pack("新增資料夾/yyy", "新增資料夾/");
    packer.finish();
};