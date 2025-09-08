#!/usr/bin/env python3
"""
Prosty test odbiornika danych z symulacji C++
Wypisuje wszystkie otrzymane wartości
"""

import json
import os
import sys
import signal

def signal_handler(sig, frame):
    print('\n\nZatrzymano odbieranie (Ctrl+C)')
    sys.exit(0)

def main():
    # # Nazwa pipe'a - musi być taka sama jak w C++
    # pipe_name = "/tmp/simulink_data"

    # Dla Windows użyj:
    pipe_name = r"\\.\pipe\simulink_data"

    print(f"=== Prosty odbiornik danych z C++ ===")
    print(f"Pipe: {pipe_name}")
    print(f"Czekam na połączenie...\n")

    # Obsługa Ctrl+C
    signal.signal(signal.SIGINT, signal_handler)

    # Utwórz pipe jeśli nie istnieje (tylko Linux/Mac)
    if not sys.platform.startswith('win'):
        if not os.path.exists(pipe_name):
            try:
                os.mkfifo(pipe_name)
                print(f"Utworzono pipe: {pipe_name}")
            except Exception as e:
                print(f"Błąd tworzenia pipe: {e}")
                return

    message_count = 0
    total_samples = 0

    try:
        # Otwórz pipe do odczytu
        with open(pipe_name, 'r') as pipe:
            print("✓ Połączono z nadajnikiem C++!")
            print("Odbieranie danych...\n")
            print("-" * 60)

            buffer = ""

            while True:
                # Czytaj dane
                chunk = pipe.read(1024)
                if not chunk:
                    print("\n⚠ Pipe zamknięty przez nadajnik")
                    break

                buffer += chunk

                # Przetwarzaj kompletne wiadomości JSON (zakończone \n)
                while '\n' in buffer:
                    line, buffer = buffer.split('\n', 1)

                    if line:
                        try:
                            # Parsuj JSON
                            data = json.loads(line)
                            message_count += 1

                            # Sprawdź czy to reset
                            if data['simTime'] == -1:
                                print("\n🔄 RESET SYMULACJI")
                                print("-" * 60)
                                continue

                            # Wypisz informacje o wiadomości
                            print(f"📨 Wiadomość #{message_count}")
                            print(f"   Czas symulacji: {data['simTime']:.3f} s")
                            print(f"   dt: {data['dt']:.4f} s")
                            print(f"   Liczba próbek: {len(data['data'])}")

                            # Wypisz wartości
                            if data['data']:
                                print(f"   Wartości:")

                                # Jeśli mało danych, wypisz wszystkie
                                if len(data['data']) <= 10:
                                    for i, val in enumerate(data['data']):
                                        print(f"      [{i}]: {val:.6f}")
                                else:
                                    # Jeśli dużo danych, wypisz pierwsze 5 i ostatnie 5
                                    print(f"      Pierwsze 5:")
                                    for i in range(5):
                                        print(f"        [{i}]: {data['data'][i]:.6f}")

                                    print(f"      ...")
                                    print(f"      Ostatnie 5:")
                                    for i in range(-5, 0):
                                        idx = len(data['data']) + i
                                        print(f"        [{idx}]: {data['data'][i]:.6f}")

                                # Statystyki
                                values = data['data']
                                print(f"   📊 Statystyki:")
                                print(f"      Min: {min(values):.6f}")
                                print(f"      Max: {max(values):.6f}")
                                print(f"      Średnia: {sum(values)/len(values):.6f}")

                                total_samples += len(data['data'])

                            print("-" * 60)

                        except json.JSONDecodeError as e:
                            print(f"❌ Błąd parsowania JSON: {e}")
                            print(f"   Nieprawidłowe dane: {line[:100]}...")
                        except KeyError as e:
                            print(f"❌ Brak klucza w JSON: {e}")
                            print(f"   Otrzymane klucze: {data.keys() if 'data' in locals() else 'N/A'}")
                        except Exception as e:
                            print(f"❌ Nieoczekiwany błąd: {e}")

    except FileNotFoundError:
        print(f"❌ Nie znaleziono pipe: {pipe_name}")
        print("Upewnij się, że symulacja C++ jest uruchomiona")
    except KeyboardInterrupt:
        pass  # Obsłużone przez signal_handler
    except Exception as e:
        print(f"❌ Błąd: {e}")

    # Podsumowanie
    print("\n" + "=" * 60)
    print("📊 PODSUMOWANIE:")
    print(f"   Otrzymanych wiadomości: {message_count}")
    print(f"   Łączna liczba próbek: {total_samples}")
    print("=" * 60)

if __name__ == "__main__":
    main()