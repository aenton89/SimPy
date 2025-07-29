# src/python_kernel/kernel.py

import sys
import json

namespace = {}

buffer = []

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
                filtered = {k: v for k, v in namespace.items() if not k.startswith("__")}
                safe_filtered = {k: repr(v) for k, v in filtered.items()}
                print(json.dumps(safe_filtered), flush=True)
                print("[[OK]]", flush=True)
                continue

            try:
                exec(code, namespace)
                print("[[OK]]", flush=True)
            except Exception as e:
                print(f"[[ERROR]] {e}", flush=True)
        else:
            buffer.append(line.rstrip('\n'))

    except Exception as e:
        print(f"[[FATAL]] {e}", flush=True)
        break

