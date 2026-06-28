This project aimed to analyze and capture Bluetooth Low Energy (BLE) traffic between an ESP32-C3 acting as a custom keyboard and a host device. The objective was to confirm successful key transmission (HID reports) and study encryption behavior.
Using a Nordic nRF BLE sniffer and Wireshark, we successfully captured the BLE link and confirmed encryption behavior, pairing sequences, and control exchanges.
For a standard (hardware) keyboard, HID keypress data was easily viewable and decodable. However, for the ESP32-C3, link encryption was correctly negotiated and enforced—resulting in unreadable (encrypted) HID payloads that Wireshark could not decrypt.
The inability to decrypt was caused by secure link establishment (LE Secure Connections or legacy pairing with enforced encryption) and the absence of correctly formatted Long Term Keys (LTKs) in Wireshark’s decryption engine.
Despite that, the project successfully demonstrated the working BLE keyboard device, verified HID transmissions at the packet level, and confirmed correct encryption behavior of the ESP32-C3 stack.
Key Concepts Overview
•	LTK (Long Term Key): 128-bit key used to derive session encryption keys for BLE links.
•	EDIV / RAND: Legacy identifiers linking stored LTKs to a bonded device.
•	Secure Connections (SC): Modern BLE pairing based on ECDH key exchange, creating ephemeral keys that enhance link security.
•	LL_ENC_REQ / LL_START_ENC_REQ: Control opcodes marking the start of link-layer AES-CCM encryption.
•	Bad MIC: Indicates Wireshark attempted decryption but failed integrity validation—caused by wrong key, endianness, or missing initialization vectors.
•	Bad CRC: Physical-layer capture error (incomplete or corrupted RF frame), unrelated to encryption but can break session reconstruction.

