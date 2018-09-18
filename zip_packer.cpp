#include "zip_packer.h"
#include <fstream>

ZipPacker::ZipPacker()
    : m_data_size (524288) // 512KB
{
    this->m_package_name = "result.report_bundle";
    this->m_a = NULL;
    this->m_ae = NULL;
    this->m_source_a = NULL;
    this->m_source_ae = NULL;
    this->m_data.reserve(this->m_data_size);
}

ZipPacker::~ZipPacker()
{

}

int ZipPacker::init_package(std::string package_name, std::string password)
{
    int result = 0;
    // init package
    this->m_a = archive_write_new();
    result = archive_write_set_format_zip(this->m_a);
    result = archive_write_add_filter_none(this->m_a);
    if (password != "")
    {
        result = archive_write_set_options(this->m_a, "zip:encryption=aes256");
        result = archive_write_set_passphrase(this->m_a, password.c_str());
    }
    if (package_name != "")
    {
        this->m_package_name = package_name;
    }
    result = archive_write_open_filename(this->m_a, this->m_package_name.c_str()); // set package name
    this->m_ae = archive_entry_new();
    this->m_archive_opend = result;
    return result;
}

/*
source_path : root node to be packed.
eliminated_path : ex: c:/test -> test
*/
int ZipPacker::pack(std::string source_path, std::string eliminated_path)
{
    if (this->m_archive_opend != ARCHIVE_OK) //package not open.
    {
        return this->m_archive_opend;
    }
    this->m_elimated_path = eliminated_path;
    auto result = this->init_source(source_path);
    if (result == ARCHIVE_OK)
    {
        if (this->m_mode == AE_IFDIR)
        {
            this->pack_dir();
        }
        else
        {
            this->pack_file(source_path);
        }
    }
    this->restore();
    return result;
}

void ZipPacker::finish()
{
    setlocale(LC_ALL, "C"); // set back to default
    this->m_package_name = "result.report_bundle";
    if (this->m_source_ae)
    {
        archive_entry_free(this->m_source_ae);
        this->m_source_ae = NULL;
    }
    if (this->m_source_a)
    {
        archive_read_close(this->m_source_a);
        archive_read_free(this->m_source_a);
        this->m_source_a = NULL;
    }
    if (this->m_ae)
    {
        archive_entry_free(this->m_ae);
        this->m_ae = NULL;
    }
    if (this->m_a)
    {
        archive_write_close(this->m_a);
        archive_write_free(this->m_a);
        this->m_a = NULL;
    }
}

int ZipPacker::init_source(std::string source_path)
{
    int result = 0;
    // init source
    this->m_source_ae = archive_entry_new();
    struct stat st;
    stat(source_path.c_str(), &st);
    this->m_mode = AE_IFMT & st.st_mode; //retrieve file type
    if (this->m_mode == AE_IFDIR)
    {
        this->m_source_a = archive_read_disk_new();
        result = archive_read_disk_open(this->m_source_a, source_path.c_str());
    }
    setlocale(LC_ALL, ""); //set local locale

    return result;
}

void ZipPacker::restore()
{
    setlocale(LC_ALL, "C"); // set back to default
    this->m_elimated_path = "";
    if (this->m_source_ae)
    {
        archive_entry_free(this->m_source_ae);
        this->m_source_ae = NULL;
    }
    if (this->m_source_a)
    {
        archive_read_close(this->m_source_a);
        archive_read_free(this->m_source_a);
        this->m_source_a = NULL;
    }
}

void ZipPacker::pack_dir()
{
    std::vector<std::wstring> entries;
    while(archive_read_next_header2(this->m_source_a, this->m_source_ae) == ARCHIVE_OK)
    {
        // leave while, when get ARCHIVE_EOF
        auto file_name = std::wstring(archive_entry_pathname_w(this->m_source_ae));

        char temp[300];
        size_t len = wcstombs(temp, file_name.c_str(), 300);
        auto name_s = std::string(temp, len);

        this->pack_file(name_s);
        entries.push_back(std::wstring(archive_entry_pathname_w(this->m_source_ae))); //for debuging
        if (archive_read_disk_can_descend(this->m_source_a))
        {
            archive_read_disk_descend(this->m_source_a);
        }
        archive_entry_clear(this->m_source_ae);
    }
}

void ZipPacker::pack_file(std::string file_name)
{
    int result;
    auto store_name = file_name;
    int index = file_name.find(this->m_elimated_path);
    if (index != std::string::npos)
    {
        store_name = std::string((file_name.data() + this->m_elimated_path.size()), file_name.size() - this->m_elimated_path.size());
    }
    archive_entry_copy_pathname(this->m_ae, store_name.c_str());
    struct stat st;
    stat(file_name.c_str(), &st);
    archive_entry_copy_stat(this->m_ae, &st);
    result = archive_write_header(this->m_a, this->m_ae);
    if ((st.st_mode & AE_IFMT) != AE_IFDIR)
    {
        auto fs = std::ifstream(file_name, std::ifstream::in | std::fstream::binary);
        size_t len = 0;
        char *data_ptr = reinterpret_cast<char*>(this->m_data.data());
        fs.read(data_ptr, this->m_data_size);
        len = fs.gcount();
        while (len > 0)
        {
            archive_write_data(this->m_a, this->m_data.data(), len);
            fs.read(data_ptr, this->m_data_size);
            len = fs.gcount();
        }
        fs.close();
    }
    archive_entry_clear(this->m_ae);
}