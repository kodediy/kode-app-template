# kode app template

The starting point for an app on the Kode Dot. An app is a whole firmware built on the kode SDK: kodeOS installs it into its apps region, runs it, and takes the board back on any reset. The template passes every rule of the [kode-apps catalog](https://github.com/kodediy/kode-apps/blob/main/RULES.md) that the catalog's checks reject on, so an app made from it starts green.

The template is an empty app with the two shapes of code an app has:

| Where | What goes there |
|---|---|
| `main/app_ao.c`, the states | Work that **reacts** to events. An Active Object never blocks. |
| `main/app_ao.c`, `AppAo_job()` | Work that **waits** on something, as a job on the Active Object's `kode_worker`. It may block. Its result comes back to the Active Object in one done event. |
| `main/app_main.c` | Brings kodeOS up with `kode_init()`, then starts the Active Object. |

## Requirements

- ESP-IDF **v6.1** (`. ~/.espressif/v6.1/esp-idf/export.sh`).
- SSH access to `kodediy/kode-sdk` and `kodediy/kode_slave_c5`. The component manager fetches the SDK over SSH, with its submodules.

## Starting an app

1. Create a repository `kodediy/kode-app-<name>` from this template ("Use this template" on GitHub).
2. Rename the project: `project(my_app)` in `CMakeLists.txt` becomes `project(<name>)`. The name is 1 to 31 characters of `a-z`, `0-9` and `_`, the same as in the repository name. It is the app's identity on the Dot and the name of the image, `build/<name>.bin`.
3. Set what the launcher shows: `KODE_APP_MANIFEST("My app", "tools")` in `main/app_main.c`, the name on the pill and up to four categories from the catalog's `categories.txt`.
4. Set the version: `PROJECT_VER` in `CMakeLists.txt`, raised for every version the catalog publishes.
5. Replace `appdata/icon.png`, set `appdata/app.json` (its colour and a description of up to 52 characters) and replace the placeholder in `appdata/screenshots/` with screenshots of the app, taken with `kcli.py screenshot`.

## Writing the app

- **An Active Object never blocks.** Not `kode_delay_ms()`, not any `kode_*` call whose comment in `kode.h` says it blocks. Ask the owner with its request call and handle the answer when it arrives, or hand the waiting to the Active Object's worker.
- **Subscribe, then ask.** Every Active Object kodeOS starts already runs when `kode_init()` returns, so what it announced at startup is past. Subscribe first, then request the state you need.
- **The app's signals are private.** Number them from `MAX_SIG` and post them, never publish them: the bus is sized for kodeOS's signals.
- **The worker runs one job and decides nothing.** The Active Object submits a function with `kode_worker_submit()` and goes on; when the function returns, the worker posts one `KqpWorkerDoneEvt` with the signal the Active Object chose, the job's sequence number, the status and one value. The Active Object keeps every decision: it keeps the sequence number of the job it waits for, ignores a done event with another, and decides what runs next. Only the Active Object that owns the worker submits to it.
- **A job's argument outlives the step that submits it.** It lives in the Active Object's state or in a static, never on the stack of the step, because the job may still read it until the done event.
- **A job never reaches the flash.** Its stack is in PSRAM, which must not run while the flash cache is off: no ESP-IDF flash, partition or NVS call and no `kode_nvs_set()`, `kode_nvs_get()` or `kode_nvs_del()` (see `kode_worker.h`). The job hands the value back in the done event and the Active Object stores it. The worker's stack is 8 KB, the least the catalog accepts (rule C8); raise it for a deep job.
- **The queue and the stack** of the Active Object are the two arrays in `app_ao.c`. A published event reaches every subscriber, so a queue too short for a burst is a QP assert, not a dropped event.
- **Coming home is not the app's job.** Any reset returns to kodeOS, because the app never confirms its own slot (`CONFIG_KODEOS_BOOT_ROLE_APP=y`); holding BOT does it from any app. `kode_exit_to_os()` leaves on purpose.

The code follows the style of the SDK's `components/khal`: a licence line and a Doxygen header on every file, documented calls, braces on their own lines, the constant on the left of a comparison, no ternaries, everything in English. The catalog's `tools/cstyle.py` fixes and lists most of it.

## Building

```bash
idf.py build
```

On a clean checkout the first build fetches the SDK from GitHub, with your SSH key, before it compiles. The board settings travel with the SDK.

## Installing on the Dot

With kodeOS on the board, write the image to the app mailbox and let kodeOS adopt it:

```bash
python -m esptool --chip esp32p4 -p /dev/cu.usbmodem101 write-flash 0x1C00000 build/my_app.bin
```

esptool resets the board at the end; kodeOS finds the image in the mailbox, installs it into the apps region and runs it. From then on the app survives the resets development produces (opening the serial port, `esp_restart()`, a crash), so a crashing app loops until you hold **BOT + LEFT** for about a second, which is a hardware reset and returns the board to kodeOS. The app stays installed and appears in the launcher.

`idf.py -p /dev/cu.usbmodem101 monitor` shows the app's log. A running template prints `kaos.boot running as an app`, then `app.ao up` and `app.ao job done: status 0, value 0`.

## Publishing in the catalog

Open a pull request on `kodediy/kode-apps` that adds the app as `apps/<name>`; the reviewer builds the app, runs the rules and tells you what to fix. [RULES.md](https://github.com/kodediy/kode-apps/blob/main/RULES.md) has every rule and the commands to check them before the pull request. `ram_budget.txt` is the app's internal-RAM budget (rule B8): add a line for every large static the app adds, with its reason.

## Updating the SDK

The SDK version is the `version:` line in `main/idf_component.yml`. Change it and build; the catalog takes the newest release or the one before it.

## The radio

The radio is off (`CONFIG_KODEOS_RADIO=n`, `CONFIG_ESP_HOSTED_ENABLED=n` and `CONFIG_BT_ENABLED=n` in `sdkconfig.defaults`). To use Wi-Fi, BLE or the network, delete the three lines, delete `sdkconfig` and build.

## CI

`.github/workflows/build.yml` builds the app on every push and pull request where GitHub Actions is on. Kode's own repositories keep Actions off, so the build to trust is `idf.py build` on your machine.

## Licence

GPL-3.0-or-later, like the SDK it links: see `LICENSE`.
