#!/usr/bin/env python3

import os
import sys
import struct
import subprocess
import tempfile

SECTOR_SIZE = 512
MAX_NAME_LEN = 31

TYPE_NONE = 0x00
TYPE_DIR = 0x01
TYPE_FILE = 0x02
TYPE_EXEC = 0x03

ELF_MAGIC = b'\x7fELF'

class FSBuilder:
    def __init__(self, image_offset=0):
        self.sectors = [b'\x00' * 512]  # Reserve sector 0 for FS header
        self.path_to_sector = {}
        self.image_offset = image_offset
        self.root_sector_index = None

    def pad_name(self, name):
        name_bytes = name.encode('utf-8')[:MAX_NAME_LEN]
        return name_bytes + b'\x00' * (MAX_NAME_LEN - len(name_bytes))

    def write_sector(self, data):
        assert len(data) == SECTOR_SIZE
        self.sectors.append(data)
        return len(self.sectors) - 1

    def align_file(self, data):
        pad = (SECTOR_SIZE - (len(data) % SECTOR_SIZE)) % SECTOR_SIZE
        return data + (b'\x00' * pad)


    def make_file_descriptor(self, name, start_sector, num_sectors, is_exec, entry_point, self_sector):
        descriptor = bytearray(SECTOR_SIZE)
        descriptor[0] = TYPE_EXEC if is_exec else TYPE_FILE
        descriptor[1:0x20] = self.pad_name(name)

        offset = self.image_offset

        struct.pack_into("<I", descriptor, 0x20, start_sector + offset)     # file start
        struct.pack_into("<I", descriptor, 0x24, num_sectors)               # file size
        struct.pack_into("<I", descriptor, 0x28, entry_point)               # entry point (if exec)
        struct.pack_into("<I", descriptor, 0x2C, self_sector + offset)      # pointer to self

        return descriptor


    def make_directory_descriptor_chain(self, name, entry_sectors):
        descriptors = []
        max_entries_per_sector = (SECTOR_SIZE - 0x20 - 4) // 4  # 0x20 to 0x1FB, each entry 4 bytes

        chunks = [
            entry_sectors[i:i + max_entries_per_sector]
            for i in range(0, len(entry_sectors), max_entries_per_sector)
        ]

        for i, chunk in enumerate(chunks):
            descriptor = bytearray(SECTOR_SIZE)
            descriptor[0] = TYPE_DIR
            descriptor[1:0x20] = self.pad_name(name)
            for j, entry in enumerate(chunk):
                struct.pack_into("<I", descriptor, 0x20 + j * 4, entry + self.image_offset)
            descriptors.append(descriptor)

        # Link chaining
        for i in range(len(descriptors) - 1):
            next_sector = len(self.sectors) + i + 1  # sector we're going to write next
            struct.pack_into("<I", descriptors[i], 0x1FC, next_sector)

        # Write all descriptor sectors
        indices = []
        for desc in descriptors:
            index = self.write_sector(desc)
            indices.append(index)

        return indices[0]  # return root sector of the directory


    def extract_elf_info(self, path):
        with open(path, "rb") as f:
            elf_header = f.read(64)
            if not elf_header.startswith(ELF_MAGIC):
                return None, None

            is_64bit = elf_header[4] == 2
            entry_offset = 0x18 if not is_64bit else 0x18
            entry_size = 4 if not is_64bit else 8
            entry_fmt = "<I" if not is_64bit else "<Q"

            entry_point = struct.unpack_from(entry_fmt, elf_header, entry_offset)[0]
        return True, entry_point

    def objcopy_to_bin(self, input_path):
        with tempfile.NamedTemporaryFile(delete=False) as tmp:
            output_path = tmp.name
        subprocess.run(["objcopy", "-O", "binary", input_path, output_path], check=True)
        with open(output_path, "rb") as f:
            data = f.read()
        os.remove(output_path)
        return data


    def add_file(self, path, name):
        is_elf, entry_point = self.extract_elf_info(path)
        if is_elf:
            data = self.objcopy_to_bin(path)
            is_exec = True
            name = os.path.splitext(name)[0] + ".bin"
        else:
            with open(path, "rb") as f:
                data = f.read()
            entry_point = 0
            is_exec = False

        data = self.align_file(data)
        start_sector = len(self.sectors)
        for i in range(0, len(data), SECTOR_SIZE):
            self.write_sector(data[i:i+SECTOR_SIZE])
        num_sectors = len(data) // SECTOR_SIZE

        # Compute where descriptor will be stored
        self_sector = len(self.sectors)
        descriptor = self.make_file_descriptor(name, start_sector, num_sectors, is_exec, entry_point, self_sector)
        self.write_sector(descriptor)

        self.path_to_sector[path] = self_sector
        return self_sector


    def add_directory(self, dir_path, is_root=False):
        entries = []
        for entry in sorted(os.listdir(dir_path)):
            full_path = os.path.join(dir_path, entry)
            if os.path.isdir(full_path):
                child_sector = self.add_directory(full_path)
                entries.append(child_sector)
            elif os.path.isfile(full_path):
                child_sector = self.add_file(full_path, entry)
                entries.append(child_sector)

        name = os.path.basename(dir_path) or "root"
        sector_index = self.make_directory_descriptor_chain(name, entries)

        self.path_to_sector[dir_path] = sector_index
        if is_root:
            self.root_sector_index = sector_index
        return sector_index


    def build_image(self, root_path, output_file):
        self.root_sector_index = self.add_directory(root_path, is_root=True)

        total_sectors = len(self.sectors)

        # Write FS header at sector 0
        fs_header = bytearray(512)
        struct.pack_into("<I", fs_header, 0x00, self.root_sector_index + self.image_offset)
        struct.pack_into("<I", fs_header, 0x04, total_sectors + self.image_offset)
        self.sectors[0] = fs_header

        with open(output_file, "wb") as f:
            for sector in self.sectors:
                f.write(sector)

        print(f"Filesystem image written to: {output_file}")
        print(f"Root descriptor at: sector {self.root_sector_index + self.image_offset}")
        print(f"Total sectors: {total_sectors}")



if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("input_directory", help="Root directory to include in FS")
    parser.add_argument("output_image", help="Output binary image file")
    parser.add_argument("--offset", type=int, default=0, help="Sector offset to apply to all pointers")
    args = parser.parse_args()

    builder = FSBuilder(image_offset=args.offset)
    builder.build_image(args.input_directory, args.output_image)

