<!--
The README an app starts with. Step 6 of "Starting an app" in the template's
README puts this file in place of that README.

The order is fixed: what a person arriving from the catalog needs comes first,
what a developer needs comes below the rule. Fill in every section. A section
marked optional is deleted when the app has nothing to say there. Delete every
comment, this one included, when the README is written.
-->

# My app

One plain sentence that says what the app is for, to someone who has never heard of it. Then two or three more on what makes it worth installing.

<!--
The catalog's own screenshots (rule D9), in one row, so the README and the
catalog never drift apart. One <img> for each file in appdata/screenshots/,
with an alt text that says what the screen shows.
-->
<p align="center">
  <img src="appdata/screenshots/1.png" alt="What this screen shows" width="24%">
</p>

## What you need

- A **Kode Dot** with kodeOS.
- Everything else the app needs to be useful: a module, a microSD card, Wi-Fi, a second Dot, a program on a computer or a phone.

Say what the app does without each of them.

## Using it

1. Install My app from the Kode app catalog, like any other Kode Dot app.
2. What the first start asks, and why.
3. The screens, and how to go from one to another.
4. What each button does. Hold the bottom button (BOT) to go back to kodeOS.

## What it does

- **A feature.** One line for each, in the words of the person who uses it.

<!-- Optional: delete this section if nothing is missing. -->
## Not there yet

- **A limit.** What does not work yet, or works less well than a person would expect, said plainly.

---

## Building

- ESP-IDF **v6.1**.
- The kode SDK, pinned in `main/idf_component.yml` and fetched by the component manager on the first build.
- Anything else the build fetches or needs, with its version.

```bash
. ~/.espressif/v6.1/esp-idf/export.sh
idf.py build
```

The image is `build/my_app.bin`.

## Installing a build

With kodeOS on screen, send the image over the USB cable with kodeOS's `kcli.py`; kodeOS installs it and starts it:

```bash
python kcli.py -p PORT install build/my_app.bin
```

Say what an install keeps of the app's storage, and how to get back to kodeOS from an app that does not start: hold BOT and LEFT for about a second, which is a hardware reset.

<!--
Optional sections, in this order, only when the app has them:

## The console        The rows the app adds to kodeOS's console.
## Tested             What was tested, against what, and what was not.
## Numbers            The measurements behind a claim made above.
## Documentation      Links into docs/.

A section only this app has (the rules of a radio, how to connect from another
program, the controls of a game) goes where its reader looks for it: above the
rule when it is for the person using the app, below it when it is for the
developer.
-->

## Credits and licence

The work of others the app is built on, with a link to each, and their trademarks if the app names them.

GPL-3.0-or-later: see `LICENSE`.
