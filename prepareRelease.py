import argparse
import os
import requests
from zipfile import ZipFile
import shutil

LINUX_APP = "https://github.com/mortbopet/Ripes/releases/download/continuous/Ripes-continuous-linux-x86_64.AppImage"
WIN_APP = "https://github.com/mortbopet/Ripes/releases/download/continuous/Ripes-continuous-win-x86_64.zip"
OSX_APP = "https://github.com/mortbopet/Ripes/releases/download/continuous/Ripes-continuous-mac-x86_64.zip"
APP_URLS = [LINUX_APP, WIN_APP, OSX_APP]


TMP_DIR = "ripes_release_tmp"

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-v', '--version',
                        help='Version number', required=True)
    files = []

    args = parser.parse_args()

    def zip_extract_repl(file):
        with ZipFile(file, 'r') as zip_obj:
            temp_zipdir = file + 'tempzip'
            zip_obj.extractall(temp_zipdir)
            repl_version(temp_zipdir)
            os.remove(file)
            shutil.make_archive(file.replace('.zip', ""), 'zip', temp_zipdir)
            shutil.rmtree(temp_zipdir)

    def repl_version(folder):
        def _repl(f):
            newname = f.replace('continuous', args.version)
            os.rename(f, newname)
            if newname != f and newname.endswith(".zip"):
                zip_extract_repl(newname)

        for (dirpath, dirnames, filenames) in os.walk(folder):
            for d in dirnames:
                _repl(os.path.join(dirpath, d))
            for f in filenames:
                _repl(os.path.join(dirpath, f))

    shutil.rmtree(TMP_DIR)
    os.mkdir(TMP_DIR)
    for url in APP_URLS:
        r = requests.get(url, allow_redirects=True)
        fn = os.path.join(TMP_DIR, url.split("/")[-1])
        open(fn, 'wb').write(r.content)
        files.append(fn)

    repl_version(TMP_DIR)
