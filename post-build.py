import shutil
import os

meta_dir = os.path.dirname(os.path.realpath(__file__))

resource_dir = os.path.join(meta_dir, 'resources')
windows_build_dir = os.path.join(meta_dir, 'KhiinWin/x64/Debug/resources')

if __name__ == "__main__":
    if os.path.exists(windows_build_dir):
        shutil.rmtree(windows_build_dir)
    shutil.copytree(resource_dir, windows_build_dir)
