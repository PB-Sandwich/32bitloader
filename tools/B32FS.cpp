#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string.h>
#include <string>
#include <vector>

using namespace std;
using namespace std::filesystem;

namespace B32FS {

struct FSHeader {
    uint32_t root_descriptor;
    uint32_t filesystem_size;
} __attribute__((packed));

enum FileType {
    NONE,
    DIRECTORY,
    FILE,
    BLOCK_DEVICE,
};

enum Attributes {
    EXECUTABLE = 1,
    PROTECT_LOCATION = 0b10,
};

struct DirectoryEntry {
    uint32_t descriptor;
} __attribute__((packed));

struct FileDescriptor {
    uint8_t type;
    uint32_t string_sector;
    uint32_t name_offset;
    uint32_t path_offset;
    uint32_t last_accessed_path_offset;
    uint32_t last_modified_path_offset;
    uint32_t attributes;
    uint32_t file_size_bytes;
    uint32_t framgment_size_sectors;
    uint32_t file_sector;
} __attribute__((packed));

}

uint32_t pack_filesystem_rec(path directory_path, path root_path, ofstream& file, B32FS::FSHeader& fsheader)
{
    vector<uint32_t> file_descriptors;

    uint8_t* sector = new uint8_t[512];
    memset(sector, 0, 512);

    for (const auto& entry :
        directory_iterator(directory_path)) {
        if (entry.is_regular_file()) {
            ifstream file_in(entry.path());

            // get the files size
            file_in.seekg(0, ios::end);
            uint32_t file_size = (uint32_t)file_in.tellg();
            uint32_t file_size_sectors = (file_size + 511) / 512;
            file_in.seekg(0, ios::beg);

            // prepare metadata
            uint32_t path_offset = (uint32_t)entry.path().filename().string().length();
            path rel_path = entry.path().lexically_relative(root_path);
            string actual_path = '/' + rel_path.parent_path().string();
            uint32_t last_accessed_path_offset = path_offset + actual_path.length();

            // prepare descriptor
            B32FS::FileDescriptor* file_descriptor = (B32FS::FileDescriptor*)sector;
            file_descriptor->type = B32FS::FILE;
            file_descriptor->string_sector = fsheader.filesystem_size + 1;
            file_descriptor->name_offset = 0;
            file_descriptor->path_offset = path_offset + 1;
            file_descriptor->last_accessed_path_offset = last_accessed_path_offset + 2;
            file_descriptor->last_modified_path_offset = last_accessed_path_offset + 2;
            file_descriptor->attributes = 0;
            file_descriptor->file_size_bytes = file_size;
            file_descriptor->framgment_size_sectors = file_size_sectors;
            file_descriptor->file_sector = fsheader.filesystem_size + 2;

            file.write((const char*)sector, 512);

            string string_sector = entry.path().filename().string() + '\0' + actual_path + '\0' + "MKB32FS" + '\0';
            copy(string_sector.begin(), string_sector.end(), sector);
            file.write((const char*)sector, 512);

            // copy file and close
            file << file_in.rdbuf();
            for (int i = file_size; i < file_size_sectors * 512; i++) {
                file << (uint8_t)0;
            }
            file_in.close();

            file_descriptors.push_back(fsheader.filesystem_size);
            fsheader.filesystem_size += file_size_sectors + 2;
        }
        if (entry.is_directory()) {
            file_descriptors.push_back(pack_filesystem_rec(entry.path(), directory_path, file, fsheader));
        }
    }

    // prepare metadata
    uint32_t directory_size_sectors = (file_descriptors.size() * 4 + 511) / 512;

    uint32_t path_offset = (uint32_t)directory_path.filename().string().length() + 1;
    path rel_path = directory_path.lexically_relative(root_path);
    string actual_path = '/' + rel_path.parent_path().string();
    uint32_t last_accessed_path_offset = path_offset + actual_path.length() + 1;

    uint32_t descriptor_sector = fsheader.filesystem_size;

    B32FS::FileDescriptor* dir = (B32FS::FileDescriptor*)sector;
    dir->type = B32FS::DIRECTORY,
    dir->string_sector = fsheader.filesystem_size + 1,
    dir->name_offset = 0,
    dir->path_offset = path_offset,
    dir->last_accessed_path_offset = last_accessed_path_offset,
    dir->last_modified_path_offset = last_accessed_path_offset,
    dir->attributes = 0,
    dir->file_size_bytes = file_descriptors.size() * 4;
    dir->framgment_size_sectors = directory_size_sectors;
    dir->file_sector = fsheader.filesystem_size + 2,

    file.write((const char*)sector, 512);

    string name = directory_path.filename();
    string string_sector = name + '\0' + actual_path + '\0' + "MKB32FS" + '\0';

    copy(string_sector.begin(), string_sector.end(), sector);
    file.write((const char*)sector, 512);

    fsheader.filesystem_size += 2;

    for (uint32_t descriptor : file_descriptors) {
        file.write((const char*)&descriptor, 4);
    }
    for (int i = file_descriptors.size() * 4; i < directory_size_sectors * 512; i++) {
        file << (uint8_t)0;
    }
    fsheader.filesystem_size += directory_size_sectors;

    delete[] sector;
    return descriptor_sector;
}

int pack_filesystem(vector<string> args)
{
    path directory_path = args[2];
    path file_path = args[3];
    uint32_t offset = 0;
    if (args.size() > 4) {
        offset = stoi(args[4]);
    }

    if (!exists(directory_path)) {
        cerr << directory_path << " does not exist\n\n";
        return 1;
    }

    if (!is_directory(directory_path)) {
        cerr << directory_path << " is not a directory\n\n";
        return 1;
    }

    ofstream file(file_path, ios::binary);
    if (!file.is_open()) {
        cerr << "Failed to open " << file_path << " for writing\n\n";
        return 1;
    }

    vector<uint32_t> file_descriptors;

    B32FS::FSHeader fsheader = {
        .root_descriptor = 0,
        .filesystem_size = 1 + offset,
    };

    uint8_t* sector = new uint8_t[512];
    memset(sector, 0, 512);

    // reserve space for the header
    file.write((const char*)sector, 512);

    for (const auto& entry :
        directory_iterator(directory_path)) {
        if (entry.is_regular_file()) {
            ifstream file_in(entry.path());

            // get the files size
            file_in.seekg(0, ios::end);
            uint32_t file_size = (uint32_t)file_in.tellg();
            uint32_t file_size_sectors = (file_size + 511) / 512;
            file_in.seekg(0, ios::beg);

            // prepare metadata
            uint32_t path_offset = (uint32_t)entry.path().filename().string().length();
            string actual_path = entry.path().string().substr(directory_path.string().length(), entry.path().string().length() - directory_path.string().length() - path_offset);
            uint32_t last_accessed_path_offset = path_offset + actual_path.length();

            // prepare descriptor
            B32FS::FileDescriptor* file_descriptor = (B32FS::FileDescriptor*)sector;
            file_descriptor->type = B32FS::FILE;
            file_descriptor->string_sector = fsheader.filesystem_size + 1;
            file_descriptor->name_offset = 0;
            file_descriptor->path_offset = path_offset + 1;
            file_descriptor->last_accessed_path_offset = last_accessed_path_offset + 2;
            file_descriptor->last_modified_path_offset = last_accessed_path_offset + 2;
            file_descriptor->attributes = 0;
            file_descriptor->file_size_bytes = file_size;
            file_descriptor->framgment_size_sectors = file_size_sectors;
            file_descriptor->file_sector = fsheader.filesystem_size + 2;

            file.write((const char*)sector, 512);

            string string_sector = entry.path().filename().string() + '\0' + actual_path + '\0' + "MKB32FS" + '\0';
            copy(string_sector.begin(), string_sector.end(), sector);
            file.write((const char*)sector, 512);

            // copy file and close
            file << file_in.rdbuf();
            for (int i = file_size; i < file_size_sectors * 512; i++) {
                file << (uint8_t)0;
            }
            file_in.close();

            file_descriptors.push_back(fsheader.filesystem_size);
            fsheader.filesystem_size += file_size_sectors + 2;
        }
        if (entry.is_directory()) {
            file_descriptors.push_back(pack_filesystem_rec(entry.path(), directory_path, file, fsheader));
        }
    }

    uint32_t directory_size_sectors = (file_descriptors.size() * 4 + 511) / 512;

    B32FS::FileDescriptor* root = (B32FS::FileDescriptor*)sector;
    root->type = B32FS::DIRECTORY,
    root->string_sector = fsheader.filesystem_size + 1,
    root->name_offset = 0,
    root->path_offset = 5,
    root->last_accessed_path_offset = 7,
    root->last_modified_path_offset = 7,
    root->attributes = 0,
    root->file_size_bytes = file_descriptors.size() * 4;
    root->framgment_size_sectors = directory_size_sectors;
    root->file_sector = fsheader.filesystem_size + 2,

    file.write((const char*)sector, 512);

    string name = "root";
    string string_sector = name + '\0' + "/" + '\0' + "MKB32FS" + '\0';

    copy(string_sector.begin(), string_sector.end(), sector);
    file.write((const char*)sector, 512);

    fsheader.root_descriptor = fsheader.filesystem_size;
    fsheader.filesystem_size += 2;

    for (uint32_t descriptor : file_descriptors) {
        file.write((const char*)&descriptor, 4);
    }
    for (int i = file_descriptors.size() * 4; i < directory_size_sectors * 512; i++) {
        file << (uint8_t)0;
    }
    fsheader.filesystem_size += directory_size_sectors;

    file.seekp(0, ios::beg);
    file.write((const char*)&fsheader, sizeof(fsheader));
    file.close();

    delete[] sector;
    return 0;
}

int main(int argc, char* argv[])
{
    vector<string> args;
    for (int i = 0; i < argc; i++) {
        args.push_back(argv[i]);
    }

    if (args.size() < 4) {
        cerr << "Usage:\n\t  pack|unpack  directory  file  [offset]\n\n";
        return 1;
    }

    if (args[1] == "pack") {
        if (pack_filesystem(args) != 0) {
            return 1;
        }
    }
}
