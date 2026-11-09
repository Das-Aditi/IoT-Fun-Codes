"""
bridge.py - Single ESP32 USB-to-PHP Bridge
Run:   python bridge.py
Needs: pip install pyserial
Make sure the Arduino Serial Monitor is closed before running.
"""
import sys
import time
import urllib.request

import serial
import serial.tools.list_ports

# Target endpoint (points to app/api.php or root api.php)
API = "PATH_TO_API_IN_THE_ROOT_FOLDER"

def find_port():
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        d = (p.description or "").upper() + (p.manufacturer or "").upper()
        if any(k in d for k in ("CP210", "CH340", "CH910", "USB SERIAL", "SILICON LABS", "WCH", "ESP32")):
            return p.device
    return ports[0].device if ports else None

port = sys.argv[1] if len(sys.argv) > 1 else find_port()
if not port:
    sys.exit("No serial port found. Plug the ESP32 in with a data-capable USB cable.")

print(f"Reading {port} -> posting to {API}")

while True:
    try:
        with serial.Serial(port, 115200, timeout=1) as s:
            print("[OK] Connected to ESP32. Waiting for DATA lines...")
            while True:
                line = s.readline().decode("utf-8", "ignore").strip()
                if not line:
                    continue
                if not line.startswith("DATA,"):
                    print("ESP32:", line)  # Setup messages, debug alerts
                    continue

                body = line[5:].encode("utf-8")
                try:
                    req = urllib.request.Request(
                        API, 
                        data=body,
                        headers={"Content-Type": "application/x-www-form-urlencoded"}
                    )
                    urllib.request.urlopen(req, timeout=1.5).read()
                except Exception as e:
                    print("[POST FAILED]", e, "- Is Apache running?")
    except serial.SerialException as e:
        print("[Port problem]", e, "- Retrying in 2s (Serial Monitor open?)")
        time.sleep(2)
    except KeyboardInterrupt:
        print("\nStopping Bridge.")
        break