import subprocess
import os
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

scripts = [
    "download_scripts/download_backends.py",
    "download_scripts/download_ide_dependencies.py"
]

for script in scripts:
    script_path = os.path.join(SCRIPT_DIR, script)

    print(f"\n=== Uruchamianie {script} ===")
    result = subprocess.run([sys.executable, script_path])

    if result.returncode != 0:
        print(f"Błąd podczas wykonywania {script}")
        exit(1)

print("\nWszystkie zależności zostały pobrane!")