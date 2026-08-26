# Cloudy Web shell visual audit

## 2026-08-26

After promoting QWebEngineView to the only visible central widget, fresh routes were captured on the rebased build. Library, New instance, Settings and Accounts render from a self-contained local HTML/CSS/JS bundle with the same 38px header and 54px rail. The supplied transparent cloud artwork is used as a low-contrast animated ambient layer. Web Settings and Accounts pages expose a coherent first-class route and an explicit button for the full native preference/account manager; OAuth wording remains secure and licensed/offline profiles remain distinct. New instance exposes Custom, Modrinth, CurseForge, FTB, Technic and Import cards, while the complete version/loader/task workflow remains native and opens inside the body rectangle, leaving the Cloudy header/rail visible. This is a functional hybrid boundary, not a screenshot-only mockup.


The Skin Studio web route shows a lightweight cloud preview and explicit handoff to the existing 3D/import workflow. The Settings handoff smoke opened the real PageDialog inside the body rectangle while the WebEngine-rendered Cloudy header, rail and account pill stayed visible; no detached second window was observed. Native settings controls remain dense by design because they are real preferences, not placeholders.


## External implementation references

Qt documents `QWebEngineView` as the QWidget-based class for rendering web content and specifies CMake linkage through `Qt6::WebEngineWidgets`: https://doc.qt.io/qt-6/qtwebenginewidgets-module.html. Qt WebChannel publishes QObject APIs to HTML/JavaScript clients and is linked through `Qt6::WebChannel`: https://doc.qt.io/qt-6/qtwebchannel-index.html. The Qt WebChannel JavaScript API documents the official `qwebchannel.js` client and its `QWebChannel` initialization contract: https://doc.qt.io/qt-6/qtwebchannel-javascript.html. The Qt `setup-qt` action documents `qtwebchannel qtwebengine` as addon module names for desktop Qt installs: https://github.com/marketplace/actions/setup-qt. Qt 6.10 also documents WebView2 support for the separate Qt WebView module on Windows: https://www.qt.io/blog/qt-6.10-released. Cloudy currently uses QWidget `QWebEngineView` because it is the direct fit for this existing desktop Widgets codebase; the WebChannel API is local-only and does not expose credentials or paths.


A regression test with the preview fixture exposed a saturated legacy red palette leaking into the web canvas. The implementation was corrected so the web shell imports only text contrast roles while keeping its deliberate neutral Cloudy surface tokens; native Qt widgets continue to use the user's complete ThemeManager palette. The final 1440x900 capture is again a restrained dark workspace with the supplied cloud artwork at low opacity, not a red or gradient canvas.


The final all-route smoke captured the Library and New Instance web pages from the same binary. Library has one connected canvas, no duplicated native shell, restrained dark palette, compact add tile and supplied cloud artwork. New Instance now shows Custom, Modrinth, CurseForge, FTB, Technic and Import source cards, the white/gray outlined action style and a small outlined cloud anchor below the source choices. The explicit Open builder action still opens the complete native version/loader/provider flow in the same body rectangle.
