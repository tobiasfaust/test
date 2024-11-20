from myUtils import *
"""
This script creates the manifest_all.json as release asset.

Arguments:
    -a, --ReleaseURL (str): The URL of the release. Its an mandatory argument.
    -f, --FwDir (str): Release Verzeichnis für die Firmwares. Default: 'releases'
Usage:
    python createReleaseAssetManifest.py -a <ReleaseURL> -f <FwDir>
"""
import argparse, os

parser = argparse.ArgumentParser()

# erwartete Parameter
parser.add_argument('-a', '--ReleaseURL', type=str, help='The URL of the release. Its an mandatory argument', required=True)
parser.add_argument('-f', '--FwDir', type=str, help='Release Verzeichnis für die Firmwares', default='releases')

# Parsen der Argumente
args = parser.parse_args()

if args.ReleaseURL:
    # ändere in allen manifest.json-Dateien unterhalb args.FwDir den URL-Pfad in allen 'path' variablen 
    # im array 'parts' zur Release-URL
    changeURL(args.FwDir, args.ReleaseURL)

    # erstelle das manifest_all.json
    process_manifests(args.FwDir)

