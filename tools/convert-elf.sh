#!/bin/bash

# Check for input directory
if [[ -z "$1" ]]; then
    echo "Usage: $0 <directory>"
    exit 1
fi

INPUT_DIR="$1"

# Find all regular files under the directory
find "$INPUT_DIR" -type f | while read -r file; do
    # Check if it's an ELF file
    if file "$file" | grep -q "ELF"; then
        echo "Processing: $file"

        # Extract entry point address
        entry_point=$(readelf -h "$file" | awk '/Entry point address:/ {gsub(/^0x/, "", $4); print $4}')
        if [[ -z "$entry_point" ]]; then
            echo "  Skipping (no entry point)"
            continue
        fi

        # Convert entry point to little-endian raw bytes
        entry_bin=$(printf "%08x" "0x$entry_point" | sed 's/../& /g' | awk '{for(i=4;i>=1;i--) printf "\\x%s", $i}')

        # Use objcopy to extract raw binary to temp file
        temp_bin=$(mktemp)
        if ! objcopy -O binary "$file" "$temp_bin"; then
            echo "  Skipping (objcopy failed)"
            rm -f "$temp_bin"
            continue
        fi

        # Overwrite the original file: entry point + binary
        {
            printf "$entry_bin"
            cat "$temp_bin"
        } > "$file"

        echo "  Replaced with entry-prefixed binary"

        rm -f "$temp_bin"
    fi
done

