# Cloudy Launcher — Base44 Dev Environment

## What this project is

Cloudy Launcher is a Qt6 desktop application (a fork of PrismLauncher) — a Minecraft
launcher built with C++23, CMake, Qt6, Qt WebEngine, and vcpkg. It is **not** a web
application: there is no HTTP server, no web framework, no database. The full native
build requires a display server (X11/Wayland) and cannot run in a browser.

## How the preview works here

The launcher's UI is an embedded **web shell**: `launcher/resources/cloudy_web/`
contains `index.html`, `app.css`, `app.js`, and image assets. The native C++ host
(`launcher/ui/CloudyWebShell.cpp`, `buildCloudyDocument()`) assembles these into a
single HTML document at runtime by substituting placeholders (`CLOUDY_CSS`,
`CLOUDY_APP_JS`, `CLOUDY_ICON_DATA`, `CLOUDY_CLOUD_ART`, `CLOUDY_PROVIDER_*`) with
inlined CSS/JS and base64-encoded images, then loads it into a QWebEngineView.

`scripts/serve_web.py` replicates that assembly and serves the result as a static
page on port 3000. `cloudy-state.js` provides a mock bridge (backed by localStorage)
that seeds instances, accounts, and settings so the launcher is fully functional
in the browser: instance creation, launch with progress overlay, Modrinth mod
search/install, stop, and settings toggles all work without the native host.

The served document is ~2 MB because all CSS, JS, and images are inlined.

## Running

```bash
docker compose -f docker-compose.base44.yml up -d
```

- Service: `web` (python:3.12-slim) on host port 3000.
- No secrets or external credentials required for the preview.
- No database or other infrastructure services.

## Verifying

```bash
curl -sf http://localhost:3000/ | grep -o "Cloudy Launcher"   # should match
curl -sf http://localhost:3000/ | grep -c "CLOUDY_"           # should be 0 (all substituted)
```

## Editing the web shell

Edit files under `launcher/resources/cloudy_web/` (`index.html`, `app.css`,
`app.js`, `cloudy-state.js`). The serve script rebuilds the document on every
process start, so restart the container to pick up changes:
`docker compose -f docker-compose.base44.yml restart web`.

Key files:
- `cloudy-state.js` — mock bridge with localStorage persistence (instances,
  accounts, settings, mod management). Seeds 3 instances and 1 account.
- `app.js` — UI logic, routing, event handling, Modrinth API search.
- `app.css` — all styles including weather themes and component styles.

## Native build (not used for the preview)

The full C++ build uses CMake + vcpkg + Qt 6.10 (see `Containerfile`,
`CMakePresets.json`). It is a desktop GUI build and is not part of the Base44
preview workflow.
