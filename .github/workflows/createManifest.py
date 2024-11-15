import argparse, os
from pathlib import Path
from myUtils import *

parser = argparse.ArgumentParser()

# erwartete Parameter
parser.add_argument('--FwDir', type=str, help='Root Verzeichnis für alle Firmwares', default='web-installer/firmware')
parser.add_argument('--VersDir', type=str, help='Verzeichnis wo eine bestimmte Firware liegt', default='web-installer/firmware/v2.5.0-208-DEV')

# Parsen der Argumente
args = parser.parse_args()

if Path(args.VersDir).is_dir() and Path(args.FwDir).is_dir():
    # Benennen die Verzeichnisse um, die noch ein zip (-> Artifacts) als Endung haben
    renameDirs(args.VersDir)

    # erstelle das Manifest.json aller ESP Architekturen im Hauptverzeichnis der Version
    process_manifests(args.VersDir)

    # Suche nach manifest.json-Dateien und extrahiere die relevanten Informationen
    extracted_data = search_manifests_and_extract_version(args.FwDir)

    # Lösche die ältesten Versionen
    deleteVersions(args.FwDir, 6, extracted_data)

    # lade die Versionen erneut
    extracted_data = search_manifests_and_extract_version(args.FwDir)

    # Wenn Daten extrahiert wurden, speichere sie in einer neuen JSON-Datei
    if extracted_data:
        save_results_to_json(extracted_data, os.path.join(args.FwDir, 'versions.json'))
    else:
        print("Keine relevanten 'manifest_all.json' Dateien gefunden.")
    
else:
    print(f"Der Pfad {args.VersDir} oder {args.FwDir} ist nicht verfügbar")