import serial
import sys
from datetime import datetime

PORT = "COM3"
BAUD = 115200

def main():
    print(f"Connecting to {PORT} at {BAUD} baud...")
    print("Press Ctrl+C to exit\n")

    try:
        with serial.Serial(PORT, BAUD, timeout=1) as ser:
            print(f"Connected! Waiting for data...\n{'-' * 40}")
            while True:
                line = ser.readline()
                if line:
                    text = line.decode("utf-8", errors="replace").strip()
                    timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
                    print(f"[{timestamp}] {text}")

    except serial.SerialException as e:
        print(f"\nError: {e}")
        print("Make sure your Pico is plugged in and COM3 is correct.")
        print("Check Device Manager if unsure which COM port to use.")
        sys.exit(1)

    except KeyboardInterrupt:
        print("\n\nDisconnected.")

if __name__ == "__main__":
    main()
