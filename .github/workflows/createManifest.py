import argparse
from pathlib import Path
from myUtils import *

parser = argparse.ArgumentParser()

# erwartete Parameter
parser.add_argument('--FwDir', type=str, help='Verzeichnis wo eine bestimmte Firware liegt', default='firmware/v2.5.0-208-DEV')

# Parsen der Argumente
args = parser.parse_args()

if Path(args.FwDir).is_dir():
    # Benennen die Verzeichnisse um, die noch ein zip (-> Artifacts) als Endung haben
    renameDirs(args.FwDir)

    # erstelle das Manifest.json aller ESP Architekturen im Hauptverzeichnis der Version
    process_manifests(args.FwDir)


    output_file = 'versions.json'  # Ausgabedatei im Root-Verzeichnis

    # Suche nach manifest.json-Dateien und extrahiere die relevanten Informationen
    extracted_data = search_manifests_and_extract_version("firmware")
        
    # Wenn Daten extrahiert wurden, speichere sie in einer neuen JSON-Datei
    if extracted_data:
        save_results_to_json(extracted_data, output_file)
    else:
        print("Keine relevanten 'manifest_all.json' Dateien gefunden.")
print(f"Der Pfad {args.FwDir} ist nicht verfügbar")