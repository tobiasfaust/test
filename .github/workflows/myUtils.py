import os
import json

def renameDirs(root: str) -> None:
    """
    Iteriert über alle Verzeichnisse im angegebenen Zielverzeichnis
         
    <b>Parameter:</b>
        root (string): das Root verzeichnis über welches iteriert werden soll
         
    <b>Rückgabewert:</b>
        keiner
    """
    for dirpath, dirnames, filenames in os.walk(root):
        for dirname in dirnames:
            # Überprüfen, ob das Verzeichnis mit '.zip' endet
            if dirname.endswith('.zip'):
                # Neuer Name ohne '.zip' Endung
                new_dirname = dirname[:-4]
                # Erstelle den vollständigen Pfad zum alten und neuen Verzeichnis
                old_dirpath = os.path.join(dirpath, dirname)
                new_dirpath = os.path.join(dirpath, new_dirname)
                # Benenne das Verzeichnis um
                os.rename(old_dirpath, new_dirpath)
                print(f"Verzeichnis umbenannt: {old_dirpath} -> {new_dirpath}")


def process_manifests(root: str) -> None:
    """
    Funktion zum Suchen der 'manifest.json', Extrahieren der relevanten Daten und Erstellen der neuen JSON-Datei "manifest_all.json"
         
    <b>Parameter:</b>
        root (string): das Root verzeichnis über welches iteriert werden soll
         
    <b>Rückgabewert:</b>
        keiner
    """
    
    headerIsWritten = False
    new_manifest_data = {}

    # Durchlaufe alle Unterverzeichnisse und Dateien im Verzeichnis
    for dirpath, dirnames, filenames in os.walk(root):
        # Prüfe, ob 'manifest.json' im aktuellen Verzeichnis existiert
        if 'manifest.json' in filenames:
            manifest_path = os.path.join(dirpath, 'manifest.json')

            try:
                # Öffne und lade die JSON-Daten aus der 'manifest.json' Datei
                with open(manifest_path, 'r', encoding='utf-8') as file:
                    manifest_data = json.load(file)
                    if headerIsWritten is False:
                        # Extrahiere die relevanten Informationen: 'name', 'chipFamily', 'version', 'stage' und 'parts'
                        name = manifest_data.get('name')
                        version = manifest_data.get('version')
                        stage = manifest_data.get('stage')
                        build = manifest_data.get('build')
                        
                        # Wenn die erforderlichen Felder vorhanden sind, erstelle die neue 'manifest_all.json' Datei
                        if name and version and stage:
                            headerIsWritten = True
                            # Das neue Dictionary für 'manifest_all.json'
                            new_manifest_data = {
                                "name": name,
                                "version": version,
                                "stage": stage,
                                "build": build, 
                                "builds": []  # Wir werden die "builds" später mit chipFamily und parts füllen
                            }

                    # Füge 'chipFamily' und 'parts' als Array zu 'builds' hinzu
                    chipFamily = manifest_data.get('chipFamily')
                    parts = manifest_data.get('parts', [])

                    if chipFamily:
                        new_manifest_data["builds"].append({
                            "chipFamily": chipFamily,
                            "parts": parts
                        })
                        
            except json.JSONDecodeError:
                print(f"Warnung: Fehler beim Parsen der JSON-Datei {manifest_path}.")
            except Exception as e:
                print(f"Fehler beim Verarbeiten der Datei {manifest_path}: {e}")
    
    if manifest_path :
        # Der Pfad für die neue 'manifest_all.json' Datei
        parent_dir = os.path.dirname(os.path.dirname(manifest_path))
        new_manifest_path = os.path.join(parent_dir, 'manifest_all.json')
                            
        # Schreibe die neue JSON-Datei
        with open(new_manifest_path, 'w', encoding='utf-8') as new_file:
            json.dump(new_manifest_data, new_file, indent=4, ensure_ascii=False)
                            
        print(f"Manifest-Daten erfolgreich in {new_manifest_path} gespeichert.")
            

def search_manifests_and_extract_version(root: str) -> list :
    """
    Funktion zum Suchen und Extrahieren der relevanten Informationen aus manifest.json-Dateien
         
    <b>Parameter:</b>
        root (string): das Root verzeichnis über welches iteriert werden soll
         
    <b>Rückgabewert:</b>
        Liste von JsonObjekten
    """
    results = []  # Liste, um die Ergebnisse zu speichern
    
    # Durchlaufe alle Unterverzeichnisse im angegebenen Verzeichnis
    for dirpath, dirnames, filenames in os.walk(root):
        # Prüfe, ob eine 'manifest_all.json' Datei im aktuellen Verzeichnis existiert
        if 'manifest_all.json' in filenames:
            manifest_path = os.path.join(dirpath, 'manifest_all.json')
            
            try:
                # Öffne und lade die JSON-Daten aus der Datei
                with open(manifest_path, 'r', encoding='utf-8') as file:
                    manifest_data = json.load(file)
                    
                    # Extrahiere 'version' und 'stage' falls vorhanden
                    version = manifest_data.get('version', None)
                    stage = manifest_data.get('stage', None)
                    
                    # Falls sowohl 'version' und 'stage' vorhanden sind, füge sie zum Ergebnis hinzu
                    # entferne den root-folder "web-installer" aus dem Pfad
                    if version is not None and stage is not None:
                        results.append({
                            'path': os.sep.join(manifest_path.strip(os.sep).split(os.sep)[1:]) ,
                            'version': version,
                            'stage': stage
                        })
            
            except json.JSONDecodeError:
                print(f"Warnung: Kann die JSON-Datei nicht lesen: {manifest_path}")
            except Exception as e:
                print(f"Fehler beim Verarbeiten von {manifest_path}: {e}")
    
    return results


# Funktion zum Speichern der extrahierten Daten in einer neuen JSON-Datei
def save_results_to_json(results, output_file):
    try:
        with open(output_file, 'w', encoding='utf-8') as outfile:
            json.dump(results, outfile, indent=4, ensure_ascii=False)
        print(f"Ergebnisse wurden in {output_file} gespeichert.")
    except Exception as e:
        print(f"Fehler beim Speichern der Ergebnisse: {e}")


# funktion zum kopieren von dateien, je nach OS
def copyFile(src, dst):
    '''Copy file from src to dst'''
    if os.name == 'nt':
        os.system(f"copy {src} {dst} > nul 2>&1")
    else:
        os.system(f"cp {src} {dst} > nul 2>&1")