import os
import urllib.request

# wersja ImGui zgodna z conanfile.py
IMGUI_VERSION = "v1.90.5"

# URL bazowy do pobrania backendów
BASE_URL = f"https://raw.githubusercontent.com/ocornut/imgui/{IMGUI_VERSION}/backends/"

# lista plików do pobrania - te pod opengl3 i glfw
FILES = [
    "imgui_impl_glfw.h",
    "imgui_impl_glfw.cpp",
    "imgui_impl_opengl3.h",
    "imgui_impl_opengl3.cpp",
    "imgui_impl_opengl3_loader.h"
]

# ścieżka do folderu backendów
BACKEND_DIR = os.path.join("src", "imgui_backends")

# tworzymy katalog, jeśli nie istnieje
os.makedirs(BACKEND_DIR, exist_ok=True)

# pobieramy pliki
for filename in FILES:
    url = BASE_URL + filename
    path = os.path.join(BACKEND_DIR, filename)

    print(f"Pobieranie {url} do {path}...")
    urllib.request.urlretrieve(url, path)
    print(f"Pobrano {filename}")

print("Wszystkie pliki zostały pobrane pomyślnie!")
