#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import os

# Footer signature and structure
FOOTER_SIGNATURE = b'#GBT#ROM'
FOOTER_SIZE = 16  # 8 (signature) + 2 (checksum) + 6 (reserved)

def calculate_checksum(data):
    return sum(data) & 0xFFFF

def has_existing_footer(data):
    if len(data) < FOOTER_SIZE:
        return False
    return data[-FOOTER_SIZE:-FOOTER_SIZE+8] == FOOTER_SIGNATURE

def process_file(input_file):
    try:
        with open(input_file, 'rb') as f:
            file_data = f.read()
        
        # Check if file already has our footer
        if has_existing_footer(file_data):
            return False
        
        checksum = calculate_checksum(file_data)
        
        footer = FOOTER_SIGNATURE
        footer += checksum.to_bytes(2, byteorder='big')
        footer += b'\x00' * 6
        
        output_file = os.path.splitext(input_file)[0] + ".rbu"
        with open(output_file, 'wb') as f:
            f.write(file_data)
            f.write(footer)
        return True
    
    except Exception as e:
        return False

def main():    
    if len(sys.argv) != 2:
        return
    
    input_file = sys.argv[1]
    
    if not os.path.exists(input_file):
        return
    
    if not process_file(input_file):
        sys.exit(1)

if __name__ == "__main__":
    main()