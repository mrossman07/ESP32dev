Can you create a simple web server for ESP32 that supports live webpage update using websockets.

8:22 PM

Architected comprehensive ESP32 WebSocket server with embedded web interface
Architected comprehensive ESP32 WebSocket server with embedded web interface
Here's your ESP32 WebSocket Live Dashboard! Here's a quick rundown of what's included:

Arduino Sketch Features

Uses ESPAsyncWebServer + AsyncTCP for non-blocking async handling
WebSocket server at /ws — pushes JSON state to all connected clients every second
Handles incoming toggleLED command from the browser in real-time
Simulated sensor values (temperature, humidity, light) — just swap in real analogRead() / DHT calls
No ArduinoJson dependency — JSON is built manually to keep it lean
Webpage Features (embedded in PROGMEM)

Auto-reconnecting WebSocket client
Live-updating cards for temperature, humidity, light level, and uptime with animated progress bars
LED toggle button that sends a command back to the ESP32
Timestamped WebSocket log panel
Setup Steps

Install libraries via Arduino Library Manager:
ESPAsyncWebServer (me-no-dev)
AsyncTCP (me-no-dev)
Set your WIFI_SSID and WIFI_PASSWORD at the top
Flash to your ESP32, then open the Serial Monitor to get the IP address
Visit http://<ESP32-IP> in any browser on the same network
Esp32 websocket server
INO 
/Users/markr/Documents/Arduino/libraries/ESPAsyncWebServer/src/WebAuthentication.cpp: In function 'bool getMD5(uint8_t*, uint16_t, char*)':
/Users/markr/Documents/Arduino/libraries/ESPAsyncWebServer/src/WebAuthentication.cpp:74:3: error: 'mbedtls_md5_starts_ret' was not declared in this scope; did you mean 'mbedtls_md5_starts'?
   74 |   mbedtls_md5_starts_ret(&_ctx);
      |   ^~~~~~~~~~~~~~~~~~~~
      |   mbedtls_md5_starts
/Users/markr/Documents/Arduino/libraries/ESPAsyncWebServer/src/WebAuthentication.cpp:75:3: error: 'mbedtls_md5_update_ret' was not declared in this scope; did you mean 'mbedtls_md5_update'?
   75 |   mbedtls_md5_update_ret(&_ctx, data, len);
      |   ^~~~~~~~~~~~~~~~~~
      |   mbedtls_md5_update
/Users/markr/Documents/Arduino/libraries/ESPAsyncWebServer/src/WebAuthentication.cpp:76:3: error: 'mbedtls_md5_finish_ret' was not declared in this scope; did you mean 'mbedtls_md5_finish'?
   76 |   mbedtls_md5_finish_ret(&_ctx, _buf);
      |   ^~~~~~~~~~~~~~~~~~~~
      |   mbedtls_md5_finish
exit status 1

C

Show more
8:30 PM

Identified deprecated mbedtls functions requiring library update
Identified deprecated mbedtls functions requiring library update
This is a known compatibility issue between ESPAsyncWebServer and newer ESP32 Arduino core versions (3.x+), which updated mbedtls and dropped the _ret suffix from those functions. The fix is a quick 3-line edit in WebAuthentication.cpp:

Open this file:

/Users/markr/Documents/Arduino/libraries/ESPAsyncWebServer/src/WebAuthentication.cpp
Find these three lines (around line 74):

cpp
mbedtls_md5_starts_ret(&_ctx);
mbedtls_md5_update_ret(&_ctx, data, len);
mbedtls_md5_finish_ret(&_ctx, _buf);
Replace them with:

cpp
mbedtls_md5_starts(&_ctx);
mbedtls_md5_update(&_ctx, data, len);
mbedtls_md5_finish(&_ctx, _buf);
Just drop the _ret suffix from each function name. That's it — save the file and recompile.

Why this happens: The _ret variants were added in mbedtls 2.7 as deprecation wrappers that returned error codes. In mbedtls 3.x (bundled with ESP32 core 3.0+) they were removed entirely, leaving only the original names. The ESPAsyncWebServer library hasn't been updated to account for this yet.

Alternative: If you'd rather not patch the library manually, you can pin your ESP32 board core to 2.0.17 in the Arduino Boards Manager, which still ships the older mbedtls. But patching the file is the cleaner long-term fix.

/Users/markr/Library/Arduino15/packages/esp32/tools/esp-rv32/2511/bin/../lib/gcc/riscv32-esp-elf/14.2.0/../../../../riscv32-esp-elf/bin/ld: /Users/markr/Library/Caches/arduino/sketches/22DC8598D1F4B667948DBF5AF7A7156A/libraries/ESPAsyncWebServer/AsyncWebSocket.cpp.o: in function AsyncWebLock::lock() const':
/Users/markr/Documents/Arduino/libraries/ESPAsyncWebServer/src/AsyncWebSocket.cpp:858:(.text._ZN17AsyncWebLockGuardC2ERK12AsyncWebLock[_ZN17AsyncWebLockGuardC5ERK12AsyncWebLock]+0x4): undefined reference to pxCurrentTCB'
/Users/markr/Library/Arduino15/packages/esp32/tools/esp-rv32/2511/bin/../lib/gcc/riscv32-esp-elf/14.2.0/../../../../riscv32-esp-elf/bin/ld: /Users/markr/Library/Caches/arduino/sketches/22DC8598D1F4B667948DBF5AF7A7156A/libraries/ESPAsyncWebServer/AsyncWebSocket.cpp.o: in function AsyncWebLock::lock() const':
/Users/markr/Documents/Arduino/libraries/ESPAsyncWebServer/src/AsyncWebSynchronization.h:30:(.text._ZN17AsyncWebLockGuardC2ERK12AsyncWebLock[_ZN17AsyncWebLockGuardC5ERK12AsyncWebLock]+0xa): undefined reference to pxCurrentTCB'
/Users/markr/Library/Arduino15/packages/esp32/tools/esp-rv32/2511/bin/../lib/gcc/riscv32-esp-elf/14.2.0/../../../../riscv32-esp-elf/bin/ld: /Users/markr/Documents/Arduino/libraries/ESPAsyncWebServer/src/AsyncWebSynchronization.h:32:(.text._ZN17AsyncWebLockGuardC2ERK12AsyncWebLock[_ZN17AsyncWebLockGuardC5ERK12AsyncWebLock]+0x2a): undefined reference to `pxCurrentTCB'
collect2: error: ld returned 1 exit status
exit status 1

Show more
8:32 PM

Diagnosed RISC-V compilation issue and identified abandoned library fork
Diagnosed RISC-V compilation issue and identified abandoned library fork
The riscv32-esp-elf in the path tells me you're compiling for an ESP32-C3 or C6 (RISC-V chip), not the classic Xtensa ESP32. The me-no-dev/ESPAsyncWebServer library is essentially abandoned and has a broken FreeRTOS mutex implementation that fails to link on RISC-V cores.

The fix: switch to the actively maintained fork.

Step 1 — Remove the old library

Delete (or rename) both of these folders:

/Users/markr/Documents/Arduino/libraries/ESPAsyncWebServer
/Users/markr/Documents/Arduino/libraries/AsyncTCP
Step 2 — Install the maintained replacements

In Arduino IDE go to Tools → Manage Libraries and install:

Library	Author	Replaces
ESPAsyncWebServer	mathieucarbou	me-no-dev/ESPAsyncWebServer
AsyncTCP	mathieucarbou	me-no-dev/AsyncTCP
Search for mathieucarbou in the library manager to find both. These are drop-in replacements — no code changes needed.

Step 3 — Recompile

Clean build (Sketch → Clean Build Folder) then upload.

Why this happens: pxCurrentTCB is an internal FreeRTOS symbol used to detect if code is running inside an ISR. The me-no-dev library references it directly, which worked on older Xtensa toolchains but the RISC-V linker (used for C3/C6/H2 chips) refuses to resolve it. The mathieucarbou fork uses the proper xPortInIsrContext() API instead, which works across all ESP32 variants.

No libraries found for mathieucarbou

8:37 PM

Weighed manual installation versus library manager alternatives
Weighed manual installation versus library manager alternatives
No problem — install them manually via ZIP instead.

Step 1 — Download the ZIPs

Go to these two GitHub pages and click Code → Download ZIP on each:

https://github.com/mathieucarbou/AsyncTCP
https://github.com/mathieucarbou/ESPAsyncWebServer
Step 2 — Install in Arduino IDE

For each ZIP: Sketch → Include Library → Add .ZIP Library… and select the downloaded file.

Step 3 — Remove the old ones first

Make sure the old me-no-dev versions are gone from:

/Users/markr/Documents/Arduino/libraries/
Delete or rename both ESPAsyncWebServer and AsyncTCP folders there, otherwise Arduino will be confused by duplicates.

Step 4 — Restart Arduino IDE, then clean build and upload.

If you still hit issues, there's a second well-maintained alternative — ESP Async WebServer by lacamera — which is in the Library Manager and also supports RISC-V cores. Just search lacamera in Tools → Manage Libraries. It's a slightly different API but I can adapt the sketch for it if needed.

Update was suggested but this happened.
Downloading Async TCP@3.4.10
Async TCP@3.4.10
Installing Async TCP@3.4.10
Replacing Async TCP@3.3.2 with Async TCP@3.4.10
Installed Async TCP@3.4.10
Downloading ESP Async WebServer@3.10.0
ESP Async WebServer@3.10.0
Installing ESP Async WebServer@3.10.0
Replacing ESP Async WebServer@3.6.0 with ESP Async WebServer@3.10.0
Installed ESP Async WebServer@3.10.0
stat /Users/markr/Documents/Arduino/libraries/ESP_Async_WebServer/src/AsyncMessagePack.cpp: no such file or directory
Compilation error: stat /Users/markr/Documents/Arduino/libraries/ESP_Async_WebServer/src/AsyncMessagePack.cpp: no such file or directory

Show more
8:41 PM

Diagnosed corrupted installation; prescribed deletion and reinstallation
Diagnosed corrupted installation; prescribed deletion and reinstallation
This is a corrupted install — the ZIP extracted incompletely. The fix is to delete and reinstall manually.

Step 1 — Delete the broken folders

In Finder, navigate to /Users/markr/Documents/Arduino/libraries/ and delete these two folders:

ESP_Async_WebServer
AsyncTCP
Step 2 — Also clear the Arduino cache

/Users/markr/Library/Caches/arduino/
Delete the sketches folder inside there to force a clean rebuild.

Step 3 — Reinstall via ZIP

Download directly from GitHub (these links go straight to the latest ZIP):

https://github.com/mathieucarbou/AsyncTCP/archive/refs/heads/main.zip
https://github.com/mathieucarbou/ESPAsyncWebServer/archive/refs/heads/main.zip
Then install each via Sketch → Include Library → Add .ZIP Library…

Step 4 — Restart Arduino IDE completely, then try compiling again.

The AsyncMessagePack.cpp error means the Library Manager only partially extracted the ZIP — a known Arduino IDE 2.x bug that occasionally happens with larger libraries. Installing from a manually downloaded ZIP bypasses that installer and always works cleanly.


