#include "filesystem.h"
#include <harddrive/hdd.h>
#include <heap.h>
#include <memutils.h>
#include <print.h>
#include <stdint.h>

uint32_t fsheader_sector = 0;
FS_FileSystemHeader fsheader = { 0 };

void fs_init_filesystem(uint32_t _fsheader_sector)
{
    fsheader_sector = _fsheader_sector;
    FS_FileSystemHeader* buf = (FS_FileSystemHeader*)malloc(SECTOR_SIZE);
    hdd_read(fsheader_sector, 1, buf);
    fsheader = *buf;
    free(buf);
}

char* retrive_string(char* str, uint32_t offset, uint32_t string_sector)
{
    uint8_t* buffer = (uint8_t*)malloc(SECTOR_SIZE);
    hdd_read(string_sector, 1, buffer);
    uint32_t len = strlen((char*)buffer + offset);
    if (str == NULL) {
        str = (char*)malloc(len + 1);
        strncpy(str, (char*)buffer + offset, len + 1);
    } else {
        str = (char*)realloc(str, len);
        strncpy(str, (char*)buffer + offset, len + 1);
    }
    free(buffer);
    return str;
}

char* get_first_name(char* path)
{
    while (path[0] == '/') {
        path++;
    }

    if (path == NULL || strlen(path) == 0) {
        return NULL;
    }

    uint32_t name_length;
    char* cptr = strchr(path, '/');
    if (cptr == NULL) {
        name_length = strlen(path);
    } else {
        name_length = cptr - path;
    }

    char* name = (char*)malloc(name_length + 1);
    if (name == NULL) {
        return 0;
    }
    strncpy(name, path, name_length);
    name[name_length] = '\0';
    return name;
}

uint32_t path_to_sector(char* path, uint32_t directory_sector)
{
    while (path[0] == '/') {
        path++;
    }
    char* name = get_first_name(path);
    if (name == NULL) {
        return directory_sector;
    }

    FS_HDDFileDescriptor* file_desc = (void*)malloc(SECTOR_SIZE);
    if (file_desc == NULL) {
        free(name);
        return 0;
    }

    hdd_read(directory_sector, 1, file_desc);
    if (file_desc->type != FS_DIRECTORY) {
        free(name);
        free(file_desc);
        return 0;
    }

    uint32_t* entries = NULL;
    if (file_desc->file_size_bytes < file_desc->framgment_size_sectors * SECTOR_SIZE) {
        entries = (uint32_t*)malloc(file_desc->framgment_size_sectors * SECTOR_SIZE);
        hdd_read(file_desc->file_sector, file_desc->framgment_size_sectors, entries);
    } else {
        free(name);
        free(file_desc);
        return 0; // TODO: implement fragmented files
    }

    uint32_t sector = 0;

    char* file_name = NULL;
    for (uint32_t i = 0; i < file_desc->file_size_bytes / 4; i++) {
        FS_HDDFileDescriptor* sub_file_desc = (FS_HDDFileDescriptor*)malloc(SECTOR_SIZE);
        hdd_read(entries[i], 1, sub_file_desc);
        file_name = retrive_string(file_name, sub_file_desc->name_offset, sub_file_desc->string_sector);
        printf("%s\n", file_name);
        if (strcmp(name, file_name) == 0) {
            sector = path_to_sector(path + strlen(name), entries[i]);
            free(sub_file_desc);
            break;
        }

        free(sub_file_desc);
    }

    free(file_name);
    free(entries);
    free(name);
    free(file_desc);
    return sector;
}

FS_RAMFileDescriptor* fs_open_file(char* path)
{
    uint32_t sector = path_to_sector(path, fsheader.root);
    FS_HDDFileDescriptor *hddfile = (FS_HDDFileDescriptor*)malloc(SECTOR_SIZE);
    hdd_read(sector, 1, hddfile);

    FS_RAMFileDescriptor* file = (FS_RAMFileDescriptor*)malloc(sizeof(FS_RAMFileDescriptor));

    memcpy(&file->hdd_file_descriptor, hddfile, sizeof(FS_HDDFileDescriptor));

    free(hddfile);

    file->hdd_sector = sector;
    file->name = retrive_string(NULL, file->hdd_file_descriptor.name_offset, file->hdd_file_descriptor.string_sector);
    file->path = retrive_string(NULL, file->hdd_file_descriptor.path_offset, file->hdd_file_descriptor.string_sector);
    file->last_accessed_path = retrive_string(NULL, file->hdd_file_descriptor.last_accessed_path_offset, file->hdd_file_descriptor.string_sector);
    file->last_modified_path = retrive_string(NULL, file->hdd_file_descriptor.last_modified_path_offset, file->hdd_file_descriptor.string_sector);
    return file;
}

void fs_close_file(FS_RAMFileDescriptor* file) {
    free(file->name);
    free(file->path);
    free(file->last_accessed_path);
    free(file->last_modified_path);
    free(file);
}
