#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace std::filesystem;

namespace EstrOSFS {
enum BitMap {
    FREE = 0,
    USED = 1,
};

enum InodeType {
    FILE = 1,
    DIRECTORY = 2
};

struct Inode {
    uint32_t type = 0;
    uint32_t size = 0;
    uint32_t blocks[13] {};
    uint32_t reserved;
} __attribute__((packed));

struct DirectoryEntry {
    uint32_t inode_number;
    uint32_t entry_length;
    uint32_t name_length;
    string name;
} __attribute__((packed));

struct SuperBlock {
    uint64_t size_bytes;
    uint32_t n_blocks;
    uint32_t max_inodes;
    uint32_t n_inodes;
    uint32_t root_inode;
} __attribute__((packed));

constexpr uint32_t BLOCK_SIZE = 1024;
constexpr uint32_t INODE_SIZE = 64;
constexpr uint32_t MAX_INODES = 8192;
constexpr uint32_t INODE_SPACE = MAX_INODES * INODE_SIZE;
constexpr uint32_t SUPER_BLOCK_OFFSET = 0x10000;
constexpr uint32_t BLOCK_BP_SIZE = 0xffffffff / BLOCK_SIZE / 8;
constexpr uint32_t INODE_BP_SIZE = MAX_INODES * INODE_SIZE / BLOCK_SIZE / 8;
constexpr uint32_t FIRST_DATA_BLOCK = (EstrOSFS::BLOCK_SIZE + EstrOSFS::BLOCK_BP_SIZE + EstrOSFS::INODE_BP_SIZE + EstrOSFS::INODE_SIZE) / BLOCK_SIZE + 1;
}

void fill_to_block(ofstream& output_file)
{
    size_t size = output_file.tellp();
    if (size % EstrOSFS::BLOCK_SIZE == 0) {
        return;
    }
    for (size_t i = size;
        i < (size / EstrOSFS::BLOCK_SIZE + 1) * EstrOSFS::BLOCK_SIZE;
        i++) {
        output_file << (uint8_t)0;
    }
}

uint32_t write_pointer_block(ofstream& output_file, const vector<uint32_t>& pointers)
{
    uint32_t block_num = output_file.tellp() / EstrOSFS::BLOCK_SIZE;
    output_file.write(reinterpret_cast<const char*>(pointers.data()), pointers.size() * sizeof(uint32_t));

    for (size_t i = pointers.size() * sizeof(uint32_t); i < EstrOSFS::BLOCK_SIZE; ++i) {
        output_file.put(0);
    }

    return block_num;
}

uint32_t pack_file(path file, vector<EstrOSFS::Inode>& inodes, ofstream& output_file)
{
    ifstream input_file(file, ios::binary);
    if (!input_file.is_open()) {
        cerr << "Unable to open file: " << file << endl;
        return 0;
    }

    input_file.seekg(0, ios::end);
    if (input_file.tellg() > 0xffffffff) {
        cerr << "File too large: " << file << endl;
        cerr << "File has to be smaller then " << 0xffffffff << " bytes\n"
             << endl;
        input_file.close();
        return 0;
    }

    uint32_t size = input_file.tellg();
    input_file.seekg(0, ios::beg);

    uint32_t blocks[13] = { 0 };
    if (size == 0) {
        blocks[0] = 0;
    } else {
        blocks[0] = output_file.tellp() / EstrOSFS::BLOCK_SIZE;
    }

    if (size != 0) {
        output_file << input_file.rdbuf();
        input_file.close();
    }
    fill_to_block(output_file);

    for (int i = 1;
        i < size / EstrOSFS::BLOCK_SIZE && i <= 10;
        i++) {
        blocks[i] = blocks[0] + i;
    }
    if (size / EstrOSFS::BLOCK_SIZE > 10) {
        uint32_t total_blocks = (size + EstrOSFS::BLOCK_SIZE - 1) / EstrOSFS::BLOCK_SIZE;

        if (total_blocks > 11) {
            // --- Single Indirect (blocks[10])
            vector<uint32_t> single;
            for (uint32_t i = 11; i < min(total_blocks, (uint32_t)(11 + 256)); ++i) {
                single.push_back(blocks[0] + i);
            }
            if (!single.empty()) {
                blocks[10] = write_pointer_block(output_file, single);
            }
        }

        if (total_blocks > 11 + 256) {
            // --- Double Indirect (blocks[11])
            vector<uint32_t> double_blocks;
            uint32_t remaining = total_blocks - 11 - 256;
            for (uint32_t i = 0; i < (remaining + 255) / 256 && i < 256; ++i) {
                vector<uint32_t> sub;
                for (uint32_t j = 0; j < min(remaining, 256u); ++j) {
                    sub.push_back(blocks[0] + 11 + 256 + i * 256 + j);
                    --remaining;
                    if (remaining == 0)
                        break;
                }
                double_blocks.push_back(write_pointer_block(output_file, sub));
            }
            blocks[11] = write_pointer_block(output_file, double_blocks);
        }

        if (total_blocks > 11 + 256 + 256 * 256) {
            // --- Triple Indirect (blocks[12])
            uint32_t remaining = total_blocks - 11 - 256 - 256 * 256;
            vector<uint32_t> triple_blocks;
            for (uint32_t i = 0; i < (remaining + 65535) / 65536 && i < 256; ++i) {
                vector<uint32_t> double_blocks;
                for (uint32_t j = 0; j < 256 && remaining > 0; ++j) {
                    vector<uint32_t> sub;
                    for (uint32_t k = 0; k < min(remaining, 256u); ++k) {
                        sub.push_back(blocks[0] + 11 + 256 + 256 * 256 + i * 256 * 256 + j * 256 + k);
                        --remaining;
                        if (remaining == 0)
                            break;
                    }
                    double_blocks.push_back(write_pointer_block(output_file, sub));
                    if (remaining == 0)
                        break;
                }
                triple_blocks.push_back(write_pointer_block(output_file, double_blocks));
                if (remaining == 0)
                    break;
            }
            blocks[12] = write_pointer_block(output_file, triple_blocks);
        }
    }

    inodes.push_back({
        .type = EstrOSFS::FILE,
        .size = size,
    });
    copy(blocks, &blocks[13], inodes.back().blocks);

    cout << "Packed file " << file << " : " << size << " bytes\n";
    cout << "block[0]: " << blocks[0] << '\n';
    cout << "inode number: " << inodes.size() - 1 << '\n';
    cout << "fs size: " << (uint64_t)output_file.tellp() << '\n';

    return inodes.size() - 1;
}

// returns its inode number
uint32_t pack_directory(path directory, vector<EstrOSFS::Inode>& inodes, ofstream& output_file)
{
    uint32_t inode_number = inodes.size();
    inodes.push_back({ 0 }); // save space for itself

    vector<EstrOSFS::DirectoryEntry> entries;
    for (auto& entry : directory_iterator(directory)) {
        uint32_t child_inode_number;
        if (entry.is_directory()) {
            child_inode_number = pack_directory(entry.path(), inodes, output_file);
        } else if (entry.is_regular_file()) {
            child_inode_number = pack_file(entry.path(), inodes, output_file);
        }
        if (child_inode_number == 0) {
            continue;
        }
        string name = entry.path().filename();
        entries.push_back({ .inode_number = child_inode_number,
            .entry_length = (uint32_t)(sizeof(EstrOSFS::DirectoryEntry) + name.length() - sizeof(string) - 4),
            .name_length = (uint32_t)name.length(),
            .name = name });
    }

    uint32_t blocks[13] = { 0 };
    blocks[0] = output_file.tellp() / EstrOSFS::BLOCK_SIZE;

    for (int i = 0; i < entries.size(); i++) {
        output_file.write((char*)&entries[i], sizeof(EstrOSFS::DirectoryEntry) - sizeof(string) - 4);
        output_file.write(entries[i].name.c_str(), entries[i].name_length);
    }
    fill_to_block(output_file);

    for (int i = 1;
        i < entries.size() * sizeof(EstrOSFS::DirectoryEntry) / EstrOSFS::BLOCK_SIZE && i <= 10;
        i++) {
        blocks[i] = blocks[0] + i;
    }
    if (entries.size() / EstrOSFS::BLOCK_SIZE > 10) {
        // TODO: add indirect blocks
    }

    uint32_t size = 0;
    for (int i = 0; i < entries.size(); i++) {
        size += entries[i].entry_length;
    }
    inodes[inode_number] = {
        .type = EstrOSFS::DIRECTORY,
        .size = size,
    };
    copy(blocks, &blocks[13], inodes[inode_number].blocks);

    cout << "Packed directory: " << directory << " : " << entries.size() << " entries\n";
    cout << "block[0]: " << blocks[0] << '\n';
    cout << "inode number: " << inode_number << '\n';
    cout << "fs size: " << (uint64_t)output_file.tellp() << '\n';

    return inode_number;
}

int pack(vector<string> args)
{
    path directory = args[2];
    path file = args[3];

    if (!exists(directory) || !is_directory(directory)) {
        cerr << "Input directory does not exist or is not a directory: " << directory << endl;
        return 1;
    }

    ofstream output_file(file, ios::binary);
    if (!output_file.is_open()) {
        cerr << "Failed to open output file for writing: " << file << endl;
        return 1;
    }

    cout << "EstrOS Filesystem v1\n"
         << "Super Block location: " << EstrOSFS::SUPER_BLOCK_OFFSET << "\nBlock size " << EstrOSFS::BLOCK_SIZE << '\n';
    cout << "Packing directory " << directory << " into " << file << '\n';

    for (int i = 0;
        i < (EstrOSFS::SUPER_BLOCK_OFFSET + EstrOSFS::BLOCK_SIZE + EstrOSFS::BLOCK_BP_SIZE + EstrOSFS::INODE_BP_SIZE + EstrOSFS::INODE_SPACE) / EstrOSFS::BLOCK_SIZE * EstrOSFS::BLOCK_SIZE;
        i++) {
        output_file << (uint8_t)0;
    }
    vector<EstrOSFS::Inode> inodes;

    uint32_t root_inode = pack_directory(directory, inodes, output_file);

    size_t fs_size_bytes = output_file.tellp();
    size_t fs_size_blocks = fs_size_bytes / EstrOSFS::BLOCK_SIZE;
    cout << "Total filesystem size: " << fs_size_bytes << " bytes; " << fs_size_blocks << " blocks\n";
    cout << "Number of index nodes " << inodes.size() << '\n';

    EstrOSFS::SuperBlock super_block = {
        .size_bytes = fs_size_bytes,
        .n_blocks = (uint32_t)(output_file.tellp() / EstrOSFS::BLOCK_SIZE),
        .max_inodes = EstrOSFS::MAX_INODES,
        .n_inodes = (uint32_t)inodes.size(),
        .root_inode = root_inode,
    };
    output_file.seekp(EstrOSFS::SUPER_BLOCK_OFFSET, ios::beg);
    output_file.write((char*)&super_block, sizeof(super_block));

    output_file.seekp(EstrOSFS::SUPER_BLOCK_OFFSET + EstrOSFS::BLOCK_SIZE, ios::beg);
    for (int i = 0; i < fs_size_blocks / 8; i++) {
        output_file << (uint8_t)0xff;
    }
    uint8_t last_bp = 0;
    for (uint8_t i = 0; i < fs_size_blocks - (fs_size_blocks / 8 * 8); i++) {
        last_bp |= 1 << i;
    }
    output_file.write((char*)&last_bp, 1);

    output_file.seekp(EstrOSFS::SUPER_BLOCK_OFFSET + EstrOSFS::BLOCK_SIZE + EstrOSFS::BLOCK_BP_SIZE, ios::beg);
    for (int i = 0; i < inodes.size() / 8; i++) {
        output_file << (uint8_t)0xff;
    }
    for (uint8_t i = 0; i < inodes.size() - (inodes.size() / 8 * 8); i++) {
        last_bp |= 1 << i;
    }
    output_file.write((char*)&last_bp, 1);

    output_file.seekp(((EstrOSFS::SUPER_BLOCK_OFFSET + EstrOSFS::BLOCK_SIZE + EstrOSFS::BLOCK_BP_SIZE + EstrOSFS::INODE_BP_SIZE) / EstrOSFS::BLOCK_SIZE + 1) * EstrOSFS::BLOCK_SIZE, ios::beg);
    output_file.write((char*)inodes.begin().base(), EstrOSFS::INODE_SIZE * inodes.size());
    cout << "Wrote " << inodes.size() << " inodes to the filesystem image\n";

    output_file.close();

    return 0;
}

int add_boot(vector<string> args)
{
    path file = args[2];
    path fs_img = args[3];

    fstream output_image(fs_img, ios::out | ios::in | ios::binary);
    if (!output_image.is_open()) {
        cerr << "Unable to open file: " << fs_img << endl;
        return 1;
    }
    output_image.seekp(0, ios::beg);

    ifstream input_file(file, ios::binary);
    if (!input_file.is_open()) {
        cerr << "Unable to open file: " << file << endl;
        return 1;
    }

    input_file.seekg(0, ios::end);
    if (input_file.tellg() > 0xffffffff) {
        cerr << "File too large: " << file << endl;
        cerr << "File has to be smaller then " << EstrOSFS::SUPER_BLOCK_OFFSET << " bytes\n";
        input_file.close();
        return 1;
    }
    input_file.seekg(0, ios::beg);

    output_image << input_file.rdbuf();

    return 0;
}

int gen_test(vector<string> args)
{
    path file = args[2];
    uint32_t size = stoul(args[3]);

    ofstream output_file(file, ios::binary);
    if (!output_file.is_open()) {
        cerr << "Unable to open file: " << file << '\n';
        return 1;
    }

    for (uint32_t i = 0; i < size; i += 4) {
        output_file.write((char*)&i, 4);
    }
    return 0;
}

int main(int argc, char* argv[])
{
    vector<string> args;
    for (int i = 0; i < argc; i++) {
        args.push_back(argv[i]);
    }

    if (args.size() < 4) {
        cerr << "EstrOS Filesystem v1 utility:\n";
        cerr << "Usage:\n";
        cerr << "\tpack input-directory filesystem-img-name\n";
        cerr << "\t\tturn a directory into a filesystem image\n\n";
        cerr << "\tboot input-file filesystem-img\n";
        cerr << "\t\tadd a boot loader to the start of a image\n\n";
        cerr << "\tgen-test output-file size\n";
        cerr << "\t\tcreates a file with each uint32_t being filled with its offset in bytes\n\n";
        return 1;
    }
    if (args[1] == "pack") {
        if (pack(args) != 0) {
            return 1;
        }
    } else if (args[1] == "boot") {
        if (add_boot(args) != 0) {
            return 1;
        }
    } else if (args[1] == "get-test") {
        if (gen_test(args) != 0) {
            return 1;
        }
    }
}
