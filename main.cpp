#include "windows.h"
#include "zip_packer.h"




int main()
{
    ZipPacker packer;
    auto result = packer.init_package("result.zip", "qazwsx123");
    result = packer.pack("C:/test", "C:/");
    result = packer.pack("�s�W��Ƨ�/yyy", "�s�W��Ƨ�/");
    packer.finish();
};