#!/usr/bin/env python3
"""
Cloudy Launcher — web shell preview server.

Cloudy Launcher is a Qt6 desktop application whose UI is an embedded web shell
(launcher/resources/cloudy_web/). The native C++ host (CloudyWebShell.cpp)
assembles a single HTML document at runtime by substituting placeholders for
inlined CSS, JS, and base64-encoded images, then loads it into a QWebEngineView.

This script replicates that assembly (buildCloudyDocument) so the web shell
can be served as a static page in a browser preview. Without the native QWebChannel
bridge the UI renders with its built-in default/empty state (onboarding flow,
navigation, weather scene) — exactly what the app shows before any instance or
account data exists.

Serves on 0.0.0.0:3000.
"""

import base64
import http.server
import socketserver
import os
import sys

WEB_DIR = os.path.join(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
    "launcher",
    "resources",
    "cloudy_web",
)

# Provider icon file extension, matching CloudyWebShell.cpp providerIcon().
# modrinth/curseforge/ftb/technic use .png; custom/import use .svg.
PROVIDER_EXTENSIONS = {
    "custom": ".svg",
    "modrinth": ".png",
    "curseforge": ".png",
    "ftb": ".png",
    "technic": ".png",
    "import": ".svg",
}


def read_resource(name):
    path = os.path.join(WEB_DIR, name)
    with open(path, "rb") as f:
        return f.read()


def b64(data):
    return base64.b64encode(data).decode("ascii")


def build_document():
    html = read_resource("index.html")
    css = read_resource("app.css")
    channel = read_resource("qwebchannel.js")
    app = read_resource("app.js")

    cloud_art_b64 = b64(read_resource("cloud-art.png"))
    cloudy_icon_b64 = b64(read_resource("cloudy-icon.png"))

    # Inline images as base64 data URIs, matching buildCloudyDocument().
    html = html.replace(b"CLOUDY_CLOUD_ART", cloud_art_b64.encode())
    html = html.replace(
        b"CLOUDY_ICON_DATA",
        (b"data:image/png;base64," + cloudy_icon_b64.encode()),
    )

    # Provider icons: raw base64 (the data-URI prefix already lives in app.js).
    for name, ext in PROVIDER_EXTENSIONS.items():
        icon_b64 = b64(read_resource(os.path.join("providers", name + ext)))
        app = app.replace(
            ("CLOUDY_PROVIDER_" + name.upper()).encode(),
            icon_b64.encode(),
        )

    # Inline CSS and JS into the document.
    html = html.replace(b"<!-- CLOUDY_CSS -->", b"<style>" + css + b"</style>")
    html = html.replace(
        b"<!-- CLOUDY_WEBCHANNEL_JS -->", b"<script>" + channel + b"</script>"
    )
    html = html.replace(b"<!-- CLOUDY_APP_JS -->", b"<script>" + app + b"</script>")

    return html


DOCUMENT = build_document()


class Handler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path in ("", "/", "/index.html"):
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(DOCUMENT)))
            self.end_headers()
            self.wfile.write(DOCUMENT)
        else:
            self.send_response(404)
            self.end_headers()

    def log_message(self, fmt, *args):
        sys.stderr.write("%s - %s\n" % (self.address_string(), fmt % args))


class Server(socketserver.ThreadingTCPServer):
    allow_reuse_address = True


if __name__ == "__main__":
    port = int(os.environ.get("PORT", "3000"))
    with Server(("0.0.0.0", port), Handler) as httpd:
        sys.stderr.write("Cloudy web shell preview serving on 0.0.0.0:%d\n" % port)
        httpd.serve_forever()
