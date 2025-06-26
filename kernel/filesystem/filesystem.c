#include "filesystem.h"
#include "memutils.h"
#include "print.h"
#include <stdint.h>
#include <harddrive/ata.h>

uint32_t root_sector = 0;
uint32_t fs_header_sector = 0;
File_t* fs_buffer = 0;
uint32_t fs_size = 0;

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

void write_sector(File_t* file, uint32_t sector)
{
    memcpy(fs_buffer, file, 512);
    ata_write_sector(sector, fs_buffer);
}

void save_fs_header()
{
    FileSystemHeader_t header;
    header.root = root_sector;
    header.fs_size = fs_size;
    ata_read_sector(fs_header_sector, fs_buffer);
    memcpy(fs_buffer, &header, sizeof(FileSystemHeader_t));
    ata_write_sector(fs_header_sector, fs_buffer);
}

void filesystem_init(uint32_t _fs_header_sector, uint32_t* _fs_buffer)
{
    fs_header_sector = _fs_header_sector;
    fs_buffer = _fs_buffer;

    FileSystemHeader_t fs_header;
    ata_read_sector(fs_header_sector, fs_buffer);
    memcpy(&fs_header, fs_buffer, sizeof(FileSystemHeader_t));

    root_sector = fs_header.root;
    fs_size = fs_header.fs_size;

    return;
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

FSError_t create_file_descriptor(char* path, char* name, FileDescriptor_t file)
{
    FSError_t error;
    error.file_type = 0;
    File_t dir_u = read_file_descriptor(path);
    if (dir_u.directory.type == ERROR) {
        return dir_u.error;
    }
    if (dir_u.directory.type != DIRECTORY) {
        make_error("Not a directory", NOT_DIRECTORY, &dir_u);
        return dir_u.error;
    }

    if (strlen(name) > 31) {
        char* msg = "Name too long";
        memcpy(error.message, msg, strlen(msg));
        error.type = NAME_TOO_LONG;
        return error;
    }

    memcpy(file.name, name, strlen(name));

    Directory_t dir = dir_u.directory;
    int index = 0;
    for (; index < DIRECTORY_LENGTH; index++) {
        if (dir.entries[index] == 0) {
            break;
        }
    }
    if (index == DIRECTORY_LENGTH) {

    } else {
        dir.entries[index] = fs_size;
        file.sector = fs_size;
        write_sector(&file, fs_size);
        fs_size++;
        save_fs_header();
    }

    return error;
}

FSError_t write_file(uint8_t* data, uint32_t size, FileDescriptor_t file)
{
    FSError_t error;
    error.type = 0;
    uint32_t required_sectors = (size + 511) / 512;

    if (required_sectors > 0xffff) {
        error.type = TOO_LARGE;
        char* msg = "Data too large for file";
        memcpy(error.message, msg, strlen(msg));
        return error;
    }

    if (file.start == 0) {
        file.start = fs_size;
        file.length = required_sectors;
        fs_size += required_sectors;
        save_fs_header();
        for (int i = 0; i < file.length; i++) {
            ata_write_sector(file.start + i, data + (SECTOR_SIZE * i));
        }
    } else if (required_sectors <= file.length) {
        for (int i = 0; i < file.length; i++) {
            ata_write_sector(file.start + i, data + (SECTOR_SIZE * i));
        }
    }
    return error;
}
