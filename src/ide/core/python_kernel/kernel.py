import sys
import json
import os
import matplotlib.pyplot as plt


namespace = {}

buffer = []

# Odbieramy ścieżkę do katalogu roboczego
if len(sys.argv) > 1:
    working_directory = sys.argv[1]
else:
    working_directory = "/home/patryk/Desktop"  # Domyślny katalog roboczy, jeśli nie przekazano argumentu

# Zmiana katalogu roboczego
try:
    os.chdir(working_directory)  # Zmiana katalogu roboczego
    print(f"Zmieniono katalog roboczy na: {working_directory}", flush=True)
except Exception as e:
    print(f"[[ERROR]] Błąd przy zmianie katalogu roboczego: {e}", flush=True)

while True:
    try:
        line = sys.stdin.readline()
        if not line:
            break

        if line.strip() == "[[END]]":
            code = "\n".join(buffer)
            buffer.clear()

            if code.strip() == "":
                continue

            if code == "__vars__":
                # Zwrócenie zmiennych z namespace
                filtered = {k: v for k, v in namespace.items() if not k.startswith("__")}
                safe_filtered = {k: repr(v) for k, v in filtered.items()}
                print(json.dumps(safe_filtered), flush=True)
                print("[[OK]]", flush=True)
                continue

            if code == "'__pltmap__'":
                if "__getplot__" in namespace:
                    print(namespace['__getplot__'], flush=True)
                    del namespace['__getplot__']
                    plt.close()
                    print("[[OK]]", flush=True)
                    continue



            try:
                # Wykonaj kod
                exec(code, namespace)
                print("[[OK]]", flush=True)
            except Exception as e:
                # Obsługa błędów
                print(f"[[ERROR]] {e}", flush=True)
        else:
            # Zbieranie linii kodu
            buffer.append(line.rstrip('\n'))

    except Exception as e:
        print(f"[[FATAL]] {e}", flush=True)
        break
