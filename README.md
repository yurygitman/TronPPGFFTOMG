# BreathRate — public tester package

A self-contained web bundle that lets any tester get the BreathRate dashboard running and **see live data** in the browser.

- **XIAO ESP32-S3** → one-click browser flash (ESP Web Tools), nothing to install.
- **Any Arduino** (Uno/Nano/Mega/Leonardo…) → the page shows the sketch with a **Copy** button + a **download `.ino`**; upload once via the Arduino IDE. The sketch is dual-target (auto-selects `GPIO1` on ESP32, `A0` on Arduino) and emits the identical 6-field stream, so it connects to the dashboard exactly like the ESP32.

```
web/
├── index.html            ← landing / onboarding (flash + open dashboard)
├── dashboard.html        ← the live, simulation-free dashboard
└── firmware/
    ├── manifest.json     ← ESP Web Tools manifest (ESP32-S3 offsets)
    ├── bootloader.bin    (0x0)
    ├── partitions.bin    (0x8000)
    ├── boot_app0.bin     (0xe000)
    └── firmware.bin       (0x10000)
```

## Hard requirement: HTTPS + a Chromium browser
Browser flashing (ESP Web Tools) and `Connect` (Web Serial) **only work over `https://` or `localhost`**, in **Chrome / Edge / Brave on desktop**. Safari, Firefox, and phones can't do Web Serial. Any static host with HTTPS works — no server code, no special headers.

## Deploy (pick one)

**A) Netlify Drop — fastest, instant HTTPS, no account/git**
1. Go to <https://app.netlify.com/drop>
2. Drag the whole **`web/`** folder onto the page.
3. You get a public `https://<name>.netlify.app` link — send it to testers. Done.

**B) GitHub Pages**
```bash
cd web
git init && git add . && git commit -m "BreathRate tester bundle"
gh repo create breathrate-tester --public --source=. --push
gh api -X POST repos/:owner/breathrate-tester/pages -f source.branch=main -f source.path=/
# → https://<you>.github.io/breathrate-tester/
```

**C) Vercel** — `npx vercel deploy --prod` from inside `web/`.

**D) Local test (works fully — localhost is a secure context)**
```bash
cd web && python3 -m http.server 8000
# open http://localhost:8000  → you can flash AND connect from here
```

## Tester flow
1. Open the link in Chrome/Edge/Brave on a computer.
2. **⚡ Flash my XIAO** → pick the port (one-time, ~30 s; it offers to erase — say yes).
3. **Open Live Dashboard → Connect XIAO** → pick the same port, finger on the sensor.

## Updating the firmware
Re-compile and refresh the four bins, then redeploy:
```bash
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32S3 --output-dir /tmp/fw ../PulseSensor_WebSerial_Plus
cp /tmp/fw/*.ino.bin firmware/firmware.bin
cp /tmp/fw/*.bootloader.bin firmware/bootloader.bin
cp /tmp/fw/*.partitions.bin firmware/partitions.bin
# boot_app0.bin only changes with the core version
```
Bump `"version"` in `firmware/manifest.json` so ESP Web Tools shows the new build.

> Built for the **Seeed XIAO ESP32-S3** (8 MB). A different ESP32-S3 board may need different flash settings or sensor pin.
