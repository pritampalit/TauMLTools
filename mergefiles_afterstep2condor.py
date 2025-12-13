import sys

def merge_files(filelist_path, entries_path, output_path):
    # Read filelist and entries
    with open(filelist_path, 'r') as filelist_file:
        filelist_lines = filelist_file.readlines()

    with open(entries_path, 'r') as entries_file:
        entries_lines = entries_file.readlines()

    # Extract filenames and map entries
    entries_dict = {}
    for line in entries_lines:
        parts = line.strip().split()
        if len(parts) == 2:
            root_file_name = parts[0].split('_')[-1].replace('.root', '')
            entries_dict[root_file_name] = parts[1]

    # Process filelist and write the output
    with open(output_path, 'w') as output_file:
        for line in filelist_lines:
            root_file = line.strip()
            event_tuple = root_file.split('/')[-1].split('_')[-1].replace('.root', '')
            entry = entries_dict.get(event_tuple)  # Ignore if no match
            if entry:  # Only write if a match is found
                xrd_path = f"{root_file} {entry}\n"
                output_file.write(xrd_path)

if __name__ == "__main__":
    # Accept paths as command-line arguments
    if len(sys.argv) != 4:
        print("Usage: python merge_files.py <filelist_path> <entries_path> <output_path>")
        sys.exit(1)
    
    filelist_path = sys.argv[1]
    entries_path = sys.argv[2]
    output_path = sys.argv[3]

    merge_files(filelist_path, entries_path, output_path)
    print(f"Merged file written to {output_path}")

