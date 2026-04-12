### ESP-IDF 3.0 upgrade issues
ESP-IDF 3.0 introduces a handy method *ledcAttach* instead of *ledcSetup* and *ledcAttachPin*.

However, trying to build the project after an update will give you an error, since arduino-CAN library, one of project's dependencies, uses legacy API and hasn't been upgraded in a while. In order to fix that, you must manually change the *ESP32SJA1000.cpp* file located at *lib/arduino-CAN/src*. You must **remove** the line 
`#include "esp_intr.h"` **replacing** it with two lines:
```
#include "esp_intr_alloc.h"
#include "rom/gpio.h"
```
After that, try building the project again. This will **ONLY** work if you have the newest ESP-IDF 3.0 installed.
