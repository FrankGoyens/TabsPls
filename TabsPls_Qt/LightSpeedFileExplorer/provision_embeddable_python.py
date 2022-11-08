import argparse
import urllib.request
import os
import shutil
import zipfile

#Download links:
x64windows_embeddable_python="https://www.python.org/ftp/python/3.10.8/python-3.10.8-embed-amd64.zip"
send2trash_module="https://files.pythonhosted.org/packages/47/26/3435896d757335ea53dce5abf8d658ca80757a7a06258451b358f10232be/Send2Trash-1.8.0-py3-none-any.whl"

parser = argparse.ArgumentParser(
        prog="Provision Embeddable Python",
        description="Downloads an embeddable Python distribution and adds the necessary modules needed for TabsPls")

parser.add_argument("-p", "--platform", choices=["x64-Windows"], required=True)
parser.add_argument("-o", "--outdir", default="build", help="Where to put the completed package.")

args = parser.parse_args()

package_folder = os.path.join(args.outdir, "tabspls_embedded_python")
if not os.path.isdir(package_folder):
    print("{} folder does not exist yet, creating...".format(package_folder))
    os.mkdir(package_folder)

print("Downloading embeddable Python Windows distribution...")
with urllib.request.urlopen(x64windows_embeddable_python) as response:
        with open(os.path.join(args.outdir, "python-3.10.8-embed-amd64.zip"), "wb") as embeddable_python_archive:
                shutil.copyfileobj(response, embeddable_python_archive)
print("Done downloading embeddable Python Windows distribution")

print("Downloading send2trash module...")
with urllib.request.urlopen(send2trash_module) as response:
        with open(os.path.join(args.outdir, "Send2Trash-1.8.0-py3-none-any.whl"), "wb") as embeddable_python_archive:
                shutil.copyfileobj(response, embeddable_python_archive)
print("Done downloading send2trash module")

print("Unzipping embeddable Python Windows distribution...")
with zipfile.ZipFile(os.path.join(args.outdir, "python-3.10.8-embed-amd64.zip")) as archive:
        archive.extractall(path=package_folder)
print("Done unzipping embeddable Python Windows distribution")

print("Unzipping send2trash module...")
with zipfile.ZipFile(os.path.join(args.outdir, "Send2Trash-1.8.0-py3-none-any.whl")) as archive:
    for item in archive.infolist():
        if item.filename.startswith("send2trash/"):
            archive.extract(item, path=package_folder)
print("Done unzipping send2trash module")
