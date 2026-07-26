import sys
import os
import shutil
import json

with open("apps.json", mode="r") as f:
    APPS = json.load(f)

# returns word (4-byte) aligned address
def align_word(x):
    return (x + 3) & ~3

# pack an integer to word
def pack_word(x):
    return x.to_bytes(4, byteorder="little")

# generate memory image from command line arguments
# NOTE: we have to modify the first line if there are quotes
def gen_args(arg_str, sp_base = 0x1e6400):
    argv = arg_str.split(" ")
    argc = len(argv)

    mem = bytearray()
    mem += pack_word(argc)                                 # argc
    mem += b'\x00' * (4 * (argc + 1))                      # allocate argv (pointer)

    for i, arg in enumerate(argv):
        str_addr = sp_base + len(mem)
        ptr_addr = 4 * (i + 1)
        mem[ptr_addr:ptr_addr+4] = pack_word(str_addr)     # argv (pointer)
        data = arg.encode() + b'\x00'
        data += b'\x00' * (align_word(len(data)) - len(data))
        mem += data                                        # argv (string)
    mem += b'\x00' * 4                                     # 1 additional word for safety
    return mem

# dump the generated memory image to file
def write_args(mem, filename):
    with open(filename, mode="w") as f:
        for i in range(0, len(mem), 4):
            word = int.from_bytes(mem[i:i+4], byteorder="little")
            print(f"{word:08x}", file=f)

# copy input files found in the arguments
def copy_input_files(dirname, args):
    src_dir = os.path.join("input", dirname)
    dst_dir = os.path.join("binary", dirname)
    for name in args.split(" "):
        src_path = os.path.join(src_dir, name)
        dst_path = os.path.join(dst_dir, name)
        if os.path.isfile(src_path):
            shutil.copy(src_path, dst_path)
            print(f"## {src_path} was copied")
            
# delete input files found in the arguments
def delete_copied_files(dirname, args):
    dst_dir = os.path.join("binary", dirname)
    for name in args.split(" "):
        dst_path = os.path.join(dst_dir, name)
        if os.path.isfile(dst_path):
            os.unlink(dst_path)
            print(f"## {dst_path} was deleted")

# generate all files
def make_all():
    for appname, attr in APPS.items():
        print(f"#### {appname}")
        arg = "./" + attr["binname"]
        if attr["args"] != "":
            arg += " " + attr["args"]
        mem = gen_args(arg)

        sp_path = os.path.join("binary", attr["dirname"], appname + "_sp.txt")
        write_args(mem, sp_path)
        print(f"## {sp_path} was generated")

        copy_input_files(attr["dirname"], attr["args"]) 
        print("")

# clean up all generated files
def make_clean():
    for appname, attr in APPS.items():
        print(f"#### {appname}")
        sp_path = os.path.join("binary", attr["dirname"], appname + "_sp.txt")
        if os.path.isfile(sp_path):
            os.unlink(sp_path)
            print(f"## {sp_path} was deleted")

        delete_copied_files(attr["dirname"], attr["args"]) 
        print("")

def main():
    if len(sys.argv) == 1 or (len(sys.argv) == 2 and sys.argv[1] == "all"):
        make_all()
    elif len(sys.argv) == 2 and sys.argv[1] == "clean":
        make_clean()
    else:
        print("Usage: python3 make_args.py [all|clean]")

if __name__ == "__main__":
    main()