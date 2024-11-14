import os
import sys
import json
import argparse
from myUtils import *

# Argument parser
parser = argparse.ArgumentParser(description='Generate JSON and handle binaries.')
parser.add_argument('--repositoryname', type=str, help='Repository name')
parser.add_argument('--subversion', type=str, help='Subversion (Unique ID)')
parser.add_argument('--stage', type=str, help='Stage (Branch name)')
parser.add_argument('--binarypath', type=str, help='Path of binary files')
parser.add_argument('--releasepath', type=str, default="release", help='Path of destination, BIN and JSON files')
parser.add_argument('--releasefile', type=str, help='Path of release file, contains version number')
parser.add_argument('--arch', type=str, help='Architecture (ESP8266|ESP32)')
parser.add_argument('--artifactpath', type=str, default="artifacts", help='Path of all artifacts')
parser.add_argument('--debug', type=bool, help='Enable debug messages')

args = parser.parse_args()

# Echo input parameter
if args.debug:
    print(f"\n\nEcho input parameter")
    print(f"REPOSITORYNAME={args.repositoryname}")
    print(f"SUBVERSION={args.subversion}")
    print(f"STAGE={args.stage}")
    print(f"BINARYPATH={args.binarypath}")
    print(f"RELEASEPATH={args.releasepath}")
    print(f"ARCHITECTURE={args.arch}")
    print(f"RELEASEFILE={args.releasefile}")
    print(f"ARTIFACTPATH={args.artifactpath}")
    print(f"DEBUG={args.debug}")

if args.binarypath is not None and not os.path.isdir(args.binarypath):
    print(f"\n\nBinarypath {args.binarypath} not found\n")
    sys.exit()

if args.releasefile is not None and not os.path.isfile(args.releasefile):
    print(f"\n\nReleasefile {args.releasefile} not found\n")
    sys.exit()

os.makedirs(args.releasepath, exist_ok=True)
os.makedirs(args.artifactpath, exist_ok=True)

with open(args.releasefile, 'r') as file:
    #ermittle den String aus der Datei
    VERSION = file.read().split('"')[1]
    VersionNumber = ''.join(filter(str.isdigit, VERSION))

for root, _, files in os.walk(args.binarypath):
    for file in files:
        if file == 'firmware.bin':
            FILENAME = os.path.splitext(file)[0]
            FILEEXT = os.path.splitext(file)[1][1:]
            FIRMWARENAME = os.path.basename(args.binarypath)

            BINARYFILENAME = f"{FILENAME}.{args.arch}.v{VERSION}-{args.subversion}.{args.stage}"
            DOWNLOADURL = f"https://tobiasfaust.github.io/{args.repositoryname}/firmware/{BINARYFILENAME}.{FILEEXT}"

            json_data = {
                "name": f"Release {VERSION}-{args.stage}",
                "version": VERSION,
                "subversion": args.subversion,
                "number": int(f'{VersionNumber}{args.subversion}'),
                "stage": args.stage,
                "arch": args.arch,
                "download-url": DOWNLOADURL
            }

            if args.debug:
                print(f"\n\nEcho json string")
                print(json.dumps(json_data, indent=2))

            with open(os.path.join(args.releasepath, f"{BINARYFILENAME}.json"), 'w') as json_file:
                json.dump(json_data, json_file, indent=2)
            copyFile(os.path.join(root, file), os.path.join(args.releasepath, f'{BINARYFILENAME}.{FILEEXT}'))
            
            ################ Create Manifest ##################
            manifest_data = {
                "name": f"Release {VERSION}-{args.stage}",
                "chipFamily": args.arch,
                "stage": args.stage,
                "build": args.subversion,
                "version": f"v{VERSION}-{args.subversion}",
                "parts": []
            }

            if os.path.isfile(os.path.join(args.binarypath, "bootloader.bin")):
                manifest_data["parts"].append({
                    "path": f"https://tobiasfaust.github.io/test/firmware/v{VERSION}-{args.subversion}-{args.stage}/{FIRMWARENAME}/bootloader.{args.arch}.v{VERSION}-{args.subversion}.{args.stage}.bin",
                    "offset": 4096
                })
            if os.path.isfile(os.path.join(args.binarypath, "partitions.bin")):
                manifest_data["parts"].append({
                    "path": f"https://tobiasfaust.github.io/test/firmware/v{VERSION}-{args.subversion}-{args.stage}/{FIRMWARENAME}/partitions.{args.arch}.v{VERSION}-{args.subversion}.{args.stage}.bin",
                    "offset": 32768
                })
            if os.path.isfile(os.path.join(args.binarypath, "littlefs.bin")):
                manifest_data["parts"].append({
                    "path": f"https://tobiasfaust.github.io/test/firmware/v{VERSION}-{args.subversion}-{args.stage}/{FIRMWARENAME}/littlefs.{args.arch}.v{VERSION}-{args.subversion}.{args.stage}.bin",
                    "offset": 3473408
                })

            OFFSET = 0 if "ESP8266" in args.arch else 65536
            manifest_data["parts"].append({
                "path": f"https://tobiasfaust.github.io/{args.repositoryname}/firmware/v{VERSION}-{args.subversion}-{args.stage}/{FIRMWARENAME}/{BINARYFILENAME}.{FILEEXT}",
                "offset": OFFSET
            })

            if args.debug:
                print(f"\n\nEcho Manifest string")
                print(json.dumps(manifest_data, indent=2))

            with open(os.path.join(args.releasepath, "manifest.json"), 'w') as manifest_file:
                json.dump(manifest_data, manifest_file, indent=2)
            with open(os.path.join(args.artifactpath, "manifest.json"), 'w') as manifest_file:
                json.dump(manifest_data, manifest_file, indent=2)

# process the rest of binaries into ARTIFACTPATH
for root, _, files in os.walk(args.binarypath):
    for file in files:
        if file.endswith(".bin"):
            FILENAME = os.path.splitext(file)[0]
            FILEEXT = os.path.splitext(file)[1][1:]

            BINARYFILENAME = f"{FILENAME}.{args.arch}.v{VERSION}-{args.subversion}.{args.stage}"
            copyFile(os.path.join(root, file), os.path.join(args.artifactpath, f'{BINARYFILENAME}.{FILEEXT}'))
            
################## handle Github_Outputs ####################
print(f"version={VERSION}")
print(f"subversion={args.subversion}")
print(f"stage={args.stage}")
