#include "filesystem.h"
#include "ata.h"
#include "memutils.h"
#include "print.h"
#include <stdint.h>

uint32_t root_sector = 0;
File_t* fs_buffer = 0;

void fs_set_buffer(uint32_t* buffer)
{
    fs_buffer = buffer;
}

void fs_set_root(uint32_t sector)
{
    root_sector = sector;
}

FileHeader_t fetch_header(uint32_t sector)
{
    ata_read_sector(sector, fs_buffer);
    FileHeader_t file_header;
    memcpy(&file_header, fs_buffer, 32);
    return file_header;
}

void fetch_sector(File_t* file, uint32_t sector)
{
    ata_read_sector(sector, fs_buffer);
    memcpy(file, fs_buffer, 512);
}

void make_error(char* message, int error, File_t* file)
{
    file->error.file_type = ERROR;
    file->error.type = error;
    memcpy((char*)file->error.message, message, strlen(message) + 1);
}

File_t read_file_descriptor_rec(uint32_t file_sector, char* path)
{
    File_t file;
    fetch_sector(&file, file_sector);

    if (file.file_descriptor.type != DIRECTORY) {
        return file;
    }

    if (strlen(path) == 0) {
        return file;
    }

    if (strlen(path) == 1 && path[0] == '/') {
        return file;
    }

    char name[32];
    name[31] = '\0';

    for (int i = 0; i < DIRECTORY_LENGTH; i++) {
        uint32_t sector = file.directory.entries[i];
        FileHeader_t header = fetch_header(sector);
        memcpy(name, header.name, 31);
        if (strstr(path, name) == (path + 1)) {
            char* new_path = path + strlen(name);
            return read_file_descriptor_rec(sector, new_path);
        }
    }
    make_error("File not found", FILE_NOT_FOUND, &file);
    return file;
}

File_t read_file_descriptor(char* path)
{
    File_t root;
    fetch_sector(&root, root_sector);

    if (path[0] != '/') {
        make_error("Not root path", FILE_NOT_FOUND, &root);
        return root;
    }

    if (strlen(path) == 1) {
        return root;
    }

    char name[32];
    name[31] = '\0';

    for (int i = 0; i < DIRECTORY_LENGTH; i++) {

        uint32_t sector = root.directory.entries[i];
        FileHeader_t header = fetch_header(sector);

        memcpy(name, header.name, 31);
        if (strstr(path, name) == (path + 1)) {
            char* new_path = path + strlen(name) + 1;
            return read_file_descriptor_rec(sector, new_path);
        }
    }

    make_error("File not found", FILE_NOT_FOUND, &root);
    return root;
}

File_t read_file_descriptor_sector(uint32_t sector)
{
    File_t file;
    fetch_sector(&file, sector);
    return file;
}

void get_directory_list(DirectoryEntry_t* list, Directory_t dir)
{
    for (int i = 0; i < DIRECTORY_LENGTH; i++) {
        list[i].sector = dir.entries[i];
        list[i].header = fetch_header(dir.entries[i]);
    }
}

void read_file(uint8_t* buffer, FileDescriptor_t file)
{
    uint32_t sector = file.start;
    uint32_t length = file.length;
    for (int i = 0; i < length; i++) {
        ata_read_sector(sector + i, buffer + (512 * i));
    }
}
