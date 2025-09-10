#!/usr/bin/env python3

import json
import sys
import time



#po prosty test odbiornika - odczytuje jedną wiadomość i kończy
def main():
    pipe_name = r"\\.\pipe\simulink_data"

    print("=== ONE MESSAGE RECEIVER ===")
    print(f"PIPE: {pipe_name}")
    print("Waiting for connection...")

    # na Windows używamy win32file jeśli dostępne, inaczej próbujemy standardową metodę
    try:
        import win32file
        import win32pipe
        use_win32 = True
        print("USING: win32 API")
    except ImportError:
        use_win32 = False
        print("USING: standard API")

    max_retries = 10

    for attempt in range(max_retries):
        try:
            print(f"ATTEMPT: {attempt + 1}/{max_retries}...")

            if use_win32:
                # używaj Windows API
                try:
                    handle = win32file.CreateFile(pipe_name, win32file.GENERIC_READ, 0, None, win32file.OPEN_EXISTING, 0, None)
                    print("CONNECTED! Waiting for data...")

                    buffer = ""

                    while True:
                        try:
                            result, data = win32file.ReadFile(handle, 1024)
                            if not data:
                                print("Pipe CLOSED without receiving data")
                                break

                            chunk_str = data.decode('utf-8')
                            buffer += chunk_str

                            # szukaj pierwszej kompletnej wiadomości
                            if '\n' in buffer:
                                line = buffer.split('\n', 1)[0]

                                if line.strip():
                                    data = json.loads(line)

                                    print("\n" + "="*50)
                                    print("RECEIVED MESSAGE:")
                                    print("="*50)
                                    print(f"Simulation Time: {data['simTime']:.3f} s")
                                    print(f"Time Step: {data['dt']:.4f} s")
                                    print(f"Amount of data: {len(data['data'])}")

                                    if data['data']:
                                        print(f"Data: {data['data']}")
                                        print(f"Min: {min(data['data']):.6f}")
                                        print(f"Max: {max(data['data']):.6f}")
                                        print(f"Average value: {sum(data['data'])/len(data['data']):.6f}")

                                    print("="*50)
                                    print("DONE!")
                                    win32file.CloseHandle(handle)
                                    return

                        except Exception as e:
                            print(f"ERROR: while reading - {e}")
                            break

                    win32file.CloseHandle(handle)

                except Exception as e:
                    print(f"ERROR: win32 - {e}")
                    if attempt >= max_retries - 1:
                        return
            else:
                # fallback - próbuj otworzyć jako tekst
                try:
                    with open(pipe_name, 'r', encoding='utf-8') as pipe:
                        print("CONNECTED! waiting for data...")

                        # czytaj linię po linii
                        line = pipe.readline()
                        if line.strip():
                            data = json.loads(line)

                            print("\n" + "="*50)
                            print("RECEIVED MESSAGE:")
                            print("="*50)
                            print(f"Simulation Time: {data['simTime']:.3f} s")
                            print(f"Time Step: {data['dt']:.4f} s")
                            print(f"Amount of data: {len(data['data'])}")

                            if data['data']:
                                print(f"Data: {data['data']}")
                                print(f"Min: {min(data['data']):.6f}")
                                print(f"Max: {max(data['data']):.6f}")
                                print(f"Average value: {sum(data['data'])/len(data['data']):.6f}")

                            print("="*50)
                            print("DONE!")
                            return
                        else:
                            print("Received empty line, pipe closed?")

                except OSError as e:
                    if "Invalid argument" in str(e):
                        print("ERROR: OSError - probably issue with Named Pipe on Windows")
                        print("INSTALL pywin32: pip install pywin32")
                        return
                    raise e

        except FileNotFoundError:
            if attempt < max_retries - 1:
                print("Pipe doesn't exist, waiting...")
                time.sleep(1)
            else:
                print("UNABLE to find pipe. Make sure C++ simulation is running.")
                return
        except json.JSONDecodeError as e:
            print(f"ERROR: JSON - {e}")
            return
        except Exception as e:
            print(f"ERROR: {e}")
            if attempt >= max_retries - 1:
                return

if __name__ == "__main__":
    main()