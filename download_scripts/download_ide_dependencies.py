import os
import urllib.request



SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
BASE_DIR = os.path.join(SCRIPT_DIR, "..", "include", "ide_dependencies")

# pliki do pobrania z ich surowymi URL-ami
FILES = [
    (
        "https://raw.githubusercontent.com/enkisoftware/imgui_markdown/main/imgui_markdown.h",
        os.path.join(BASE_DIR, "imgui_markdown.h")
    ),

    # (
    #     "https://raw.githubusercontent.com/BalazsJako/ImGuiColorTextEdit/master/TextEditor.cpp",
    #     os.path.join(BASE_DIR, "TextEditor.cpp")
    # ),
    # (
    #     "https://raw.githubusercontent.com/BalazsJako/ImGuiColorTextEdit/master/TextEditor.h",
    #     os.path.join(BASE_DIR, "TextEditor.h")
    # ),
]

# tworzymy katalogi jeśli nie istnieją
for _, path in FILES:
    os.makedirs(os.path.dirname(path), exist_ok=True)

# pobieramy pliki
for url, path in FILES:
    print(f"Pobieranie {url} do {path}...")
    urllib.request.urlretrieve(url, path)
    print(f"Pobrano {os.path.basename(path)}")

print("Wszystkie pliki zostały pobrane pomyślnie!")