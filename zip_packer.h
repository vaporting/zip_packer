#ifndef _ZIP_PACKER_H_
#define _ZIP_PACKER_H_

#include "archive.h"
#include "archive_entry.h"
#include <string>
#include <vector>
#include <sys/stat.h>

class ZipPacker
{
public:
    ZipPacker();
    ~ZipPacker();
    int init_package(std::string package_name = "", std::string password = "");
    int pack(std::string source_path, std::string eliminated_path = "");
    void finish();
protected:
    int init_source(std::string source_path);
    void restore();
    void pack_dir();
    void pack_file(std::string file_name);
protected:
    struct archive *m_a;
    struct archive_entry *m_ae;
    struct archive *m_source_a;
    struct archive_entry *m_source_ae;
    std::string m_package_name;
    std::string m_elimated_path;
    unsigned short m_mode;
    const size_t m_data_size;
    std::vector<uint8_t> m_data;
    int m_archive_opend;
};

#endif
