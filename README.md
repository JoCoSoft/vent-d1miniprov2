# vent

PlatformIO based project powering the D1 Mini Pro V2 used in to power IntelliVent.

## Project structure

`/src` - c++ source code

`/lib` - third party dependencies

`/data` - files to upload via plaftormio to the SPIFFS.

Docs: http://docs.platformio.org/en/latest/platforms/espressif8266.html#uploading-files-to-file-system-spiffs

Note: I was having trouble uploading the SPIFFS image from the data directory until setting a lower upload speed as documented here: https://arduino-esp8266.readthedocs.io/en/latest/faq/a01-espcomm_sync-failed.html
