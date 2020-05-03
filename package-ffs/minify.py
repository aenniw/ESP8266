import gzip
import os
import shutil
import requests
import json

Import("env")

UI_DIRECTORY = "../esp-webui"
OAUTH_TOKEN = "GH_TOKEN"

HEADERS = {'User-Agent': 'gh-actions'}
if os.getenv(OAUTH_TOKEN):
    HEADERS['Authorization'] = 'token ' + os.getenv(OAUTH_TOKEN)


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


def valid_ffs_name(ffs_name, ffs_max_len=30):
    if len(ffs_name) >= ffs_max_len:
        raise AttributeError(
            "File name is to long for SPIFFS max 30 characters. ", ffs_name)


def rm_prefix(value, prefix):
    if value.startswith(prefix):
        return value[len(prefix):]
    return value


def process_dir(directory, prefix=False):
    print("process_dir", directory, prefix)
    for root, dirs, files in os.walk(directory):
        dest_root = "./data/" + \
            "/".join((rm_prefix(root, directory)
                      if prefix else root).split("/")[2:])

        for dir_to_process in dirs:
            if not dir_to_process.startswith("."):
                if not os.path.exists(dest_root + "/" + dir_to_process):
                    os.mkdir(dest_root + "/" + dir_to_process)

        for name in files:
            if directory.count("/") == 1:
                ffs_name = directory[len(directory):]
            else:
                ffs_name = directory[directory.find("/", 3):]
            ffs_name += "/" + name

            if name.endswith((".html", ".css", ".js")):
                valid_ffs_name(ffs_name, 27)
                with open(root + "/" + name, 'rb') as f_in, gzip.open(dest_root + "/" + name + ".gz", 'wb') as f_out:
                    shutil.copyfileobj(f_in, f_out)
            elif name.endswith(".json"):
                valid_ffs_name(ffs_name)
                with open(root + "/" + name, 'r') as f_in, open(dest_root + "/" + name, 'w') as f_out:
                    while True:
                        c = f_in.read(1)
                        if not c:
                            break
                        elif c == " " or c == "\t" or c == "\r" or c == "\n":
                            continue
                        f_out.write(c)
            else:
                valid_ffs_name(ffs_name)
                shutil.copyfile(root + "/" + name, dest_root + "/" + name)


def download_resource(url, directory):
    file_name = '%s/%s' % (directory, url.split('/')[-1:][0])
    if not os.path.isfile(file_name):
        with open(file_name, 'wb') as file:
            file.write(requests.get(url, headers=HEADERS).content)
        return True
    return False


def compress_html_resources():
    print("Cleanup of resources for FS")
    cleanup_dir("./data")

    print("Download Web-UI")
    if not os.path.isdir(UI_DIRECTORY):
        os.mkdir(UI_DIRECTORY)
    releases = requests.get(
        "https://api.github.com/repos/aenniw/esp-webui/releases/latest", headers=HEADERS).content.decode('utf-8')
    for asset_url in [x['browser_download_url'] for x in json.loads(releases)['assets']]:
        if download_resource(asset_url, UI_DIRECTORY):
            print("Downloaded %s" % asset_url)

    print("Building resources for FS")
    process_dir("./resources")
    process_dir(UI_DIRECTORY, True)


if "uploadfs" in BUILD_TARGETS or "buildfs" in BUILD_TARGETS:
    compress_html_resources()
