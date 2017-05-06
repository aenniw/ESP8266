import gzip
import os
import shutil

Import("env")


def cleanup_dir(directory):
    if not os.path.exists(directory):
        os.makedirs(directory)
    for root, dirs, files in os.walk(directory):
        for dir_to_proccess in dirs:
            if not dir_to_proccess.startswith("."):
                cleanup_dir(root + "/" + dir_to_proccess)
                os.rmdir(root + "/" + dir_to_proccess)
        for name in files:
            os.remove(root + "/" + name)


def process_dir(directory):
    for root, dirs, files in os.walk(directory):
        dest_root = "./data/" + "/".join(root.split("/")[2:])
        for name in files:
            if name.endswith((".html", ".css", ".js")):
                with open(root + "/" + name, 'rb') as f_in, gzip.open(dest_root + "/" + name + ".gz", 'wb') as f_out:
                    shutil.copyfileobj(f_in, f_out)
            elif name.endswith(".json"):
                with open(root + "/" + name, 'r') as f_in, open(dest_root + "/" + name, 'w') as f_out:
                    while True:
                        c = f_in.read(1)
                        if not c:
                            break
                        elif c == " " or c == "\t" or c == "\r" or c == "\n":
                            continue
                        f_out.write(c)
            else:
                shutil.copyfile(root + "/" + name, dest_root + "/" + name)
        for dir_to_process in dirs:
            if not dir_to_process.startswith("."):
                if not os.path.exists(dest_root + "/" + dir_to_process):
                    os.mkdir(dest_root + "/" + dir_to_process)
                process_dir(root + "/" + dir_to_process)


def compress_html_resources():
    print "Cleanup of resources for FS"
    cleanup_dir("./data")
    print "Building resources for FS"
    process_dir("./resources")


if "uploadfs" in BUILD_TARGETS or "buildfs" in BUILD_TARGETS:
    compress_html_resources()
