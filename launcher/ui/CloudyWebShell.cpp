/*
 * Cloudy Launcher - unified web workspace host.
 * Copyright (C) 2026 Cloudy Launcher contributors.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3.
 */

#include "CloudyWebShell.h"

#include "CloudyWebBridge.h"
#include "MainWindow.h"
#include "Application.h"
#include "settings/SettingsObject.h"

#include <QApplication>
#include <QBuffer>
#include <QColor>
#include <QFile>
#include <QMimeDatabase>
#include <QPalette>
#include <QResizeEvent>
#include <QTimer>
#include <QUrl>
#include <QWebChannel>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineUrlRequestJob>
#include <QWebEngineUrlScheme>
#include <QWebEngineUrlSchemeHandler>
#include <QWebEngineView>

#include <mutex>
#include <utility>

namespace {

QByteArray readCloudyResource(const QString& path)
{
    QFile resource(path);
    if (!resource.open(QIODevice::ReadOnly)) {
        qWarning() << "Cloudy embedded resource not found:" << path;
        return {};
    }
    return resource.readAll();
}

QString buildCloudyDocument()
{
    QByteArray html = readCloudyResource(QStringLiteral(":/cloudy-web/index.html"));
    const QByteArray css = readCloudyResource(QStringLiteral(":/cloudy-web/app.css"));
    const QByteArray channel = readCloudyResource(QStringLiteral(":/cloudy-web/qwebchannel.js"));
    QByteArray app = readCloudyResource(QStringLiteral(":/cloudy-web/app.js"));
    const QByteArray cloudArt = readCloudyResource(QStringLiteral(":/cloudy-web/cloud-art.png")).toBase64();
    const QByteArray cloudyIcon = readCloudyResource(QStringLiteral(":/cloudy-web/cloudy-icon.png")).toBase64();
    const auto providerIcon = [](const QString& name) {
        const auto extension = name == QStringLiteral("modrinth") || name == QStringLiteral("curseforge") || name == QStringLiteral("ftb")
                                   || name == QStringLiteral("technic")
                               ? QStringLiteral(".png")
                               : QStringLiteral(".svg");
        return readCloudyResource(QStringLiteral(":/cloudy-web/providers/") + name + extension).toBase64();
    };

    html.replace("CLOUDY_CLOUD_ART", cloudArt);
    html.replace("CLOUDY_ICON_DATA", QByteArrayLiteral("data:image/png;base64,") + cloudyIcon);
    app.replace("CLOUDY_PROVIDER_CUSTOM", providerIcon(QStringLiteral("custom")));
    app.replace("CLOUDY_PROVIDER_MODRINTH", providerIcon(QStringLiteral("modrinth")));
    app.replace("CLOUDY_PROVIDER_CURSEFORGE", providerIcon(QStringLiteral("curseforge")));
    app.replace("CLOUDY_PROVIDER_FTB", providerIcon(QStringLiteral("ftb")));
    app.replace("CLOUDY_PROVIDER_TECHNIC", providerIcon(QStringLiteral("technic")));
    app.replace("CLOUDY_PROVIDER_IMPORT", providerIcon(QStringLiteral("import")));
    html.replace("<!-- CLOUDY_CSS -->", "<style>" + css + "</style>");
    html.replace("<!-- CLOUDY_WEBCHANNEL_JS -->", "<script>" + channel + "</script>");
    html.replace("<!-- CLOUDY_APP_JS -->", "<script>" + app + "</script>");
    return QString::fromUtf8(html);
}

class CloudyResourceHandler final : public QWebEngineUrlSchemeHandler {
   public:
    explicit CloudyResourceHandler(QByteArray document, QObject* parent = nullptr)
        : QWebEngineUrlSchemeHandler(parent), m_document(std::move(document))
    {
    }

    void requestStarted(QWebEngineUrlRequestJob* job) override
    {
        const auto url = job->requestUrl();
        const auto path = url.path();
        if (url.host() != QStringLiteral("shell") || path.contains(QStringLiteral(".."))) {
            job->fail(QWebEngineUrlRequestJob::UrlInvalid);
            return;
        }

        const bool isIndex = path.isEmpty() || path == QStringLiteral("/") || path == QStringLiteral("/index.html");
        if (isIndex) {
            auto* buffer = new QBuffer(job);
            buffer->setData(m_document);
            buffer->open(QIODevice::ReadOnly);
            job->reply(QByteArrayLiteral("text/html"), buffer);
            return;
        }

        QString resourcePath;
        if (path == QStringLiteral("/qwebchannel.js")) {
            resourcePath = QStringLiteral(":/qtwebchannel/qwebchannel.js");
        } else {
            resourcePath = QStringLiteral(":/cloudy-web") + path;
        }

        QFile resource(resourcePath);
        if (!resource.open(QIODevice::ReadOnly)) {
            qWarning() << "Cloudy web resource not found:" << resourcePath;
            job->fail(QWebEngineUrlRequestJob::UrlNotFound);
            return;
        }

        auto* buffer = new QBuffer(job);
        buffer->setData(resource.readAll());
        buffer->open(QIODevice::ReadOnly);

        QMimeDatabase mimeDatabase;
        auto mimeType = mimeDatabase.mimeTypeForFile(resourcePath, QMimeDatabase::MatchExtension).name().toUtf8();
        if (resourcePath.endsWith(QStringLiteral(".js")))
            mimeType = QByteArrayLiteral("application/javascript");
        else if (resourcePath.endsWith(QStringLiteral(".svg")))
            mimeType = QByteArrayLiteral("image/svg+xml");
        else if (resourcePath.endsWith(QStringLiteral(".css")))
            mimeType = QByteArrayLiteral("text/css");
        else if (resourcePath.endsWith(QStringLiteral(".html")))
            mimeType = QByteArrayLiteral("text/html");

        job->reply(mimeType, buffer);
    }

   private:
    QByteArray m_document;
};

void registerCloudyScheme()
{
    static std::once_flag registered;
    std::call_once(registered, [] {
        QWebEngineUrlScheme scheme(QByteArrayLiteral("cloudy"));
        scheme.setSyntax(QWebEngineUrlScheme::Syntax::Host);
        scheme.setFlags(QWebEngineUrlScheme::SecureScheme | QWebEngineUrlScheme::LocalScheme | QWebEngineUrlScheme::CorsEnabled);
        QWebEngineUrlScheme::registerScheme(scheme);
    });
}

QString cloudyNativeStyleSheet()
{
    const auto applicationTheme = APPLICATION && APPLICATION->settings() ? APPLICATION->settings()->get("ApplicationTheme").toString() : QString();
    if (applicationTheme != QStringLiteral("dark")) {
        // Named user themes remain fully controlled by Cloudy ThemeManager.
        return {};
    }

    const auto window = QStringLiteral("#0a1220");
    const auto surface = QStringLiteral("#0f1a2b");
    const auto text = QStringLiteral("#f3f7ff");
    const auto muted = QStringLiteral("#b0c1d8");
    const auto line = QStringLiteral("#3b5575");
    const auto highlight = QStringLiteral("#1b304d");
    return QStringLiteral(
               "QWidget#cloudyNativePage { background: %1; color: %2; }"
               "QWidget#cloudyNativePage QScrollArea, QWidget#cloudyNativePage QScrollArea > QWidget > QWidget, QWidget#cloudyNativePage QStackedWidget { background: %1; border: 0; }"
               "QWidget#cloudyNativePage QGroupBox { color: %2; background: %1; border: 1px solid %3; border-radius: 8px; margin-top: 10px; padding-top: 10px; }"
               "QWidget#cloudyNativePage QLabel { color: %2; }"
               "QWidget#cloudyNativePage #cloudySettingsHeader { background: %1; border-bottom: 1px solid %3; }"
               "QWidget#cloudyNativePage #cloudyPageTitle { font-size: 16px; font-weight: 650; padding: 2px 0; }"
               "QWidget#cloudyNativePage #cloudyPageWorkspace { background: %1; }"
               "QWidget#cloudyNativePage #cloudyPageNav { background: %1; border: 0; padding: 4px 0; }"
               "QWidget#cloudyNativePage #cloudyPageNav::item { color: %4; border: 1px solid transparent; border-radius: 8px; padding: 8px 10px; margin: 1px 0; }"
               "QWidget#cloudyNativePage #cloudyPageNav::item:hover { color: %2; background: %5; }"
               "QWidget#cloudyNativePage #cloudyPageNav::item:selected { color: %2; background: %5; border: 1px solid %3; }"
               "QWidget#cloudyNativePage QLineEdit, QWidget#cloudyNativePage QPlainTextEdit, QWidget#cloudyNativePage QTextEdit, QWidget#cloudyNativePage QComboBox, QWidget#cloudyNativePage QSpinBox, QWidget#cloudyNativePage QDoubleSpinBox { color: %2; background: %5; border: 1px solid %3; border-radius: 7px; padding: 6px 8px; selection-background-color: %6; }"
               "QWidget#cloudyNativePage QListView, QWidget#cloudyNativePage QTreeView, QWidget#cloudyNativePage QTableView { color: %2; background: %5; alternate-background-color: %1; border: 1px solid %3; border-radius: 8px; selection-background-color: %6; selection-color: %7; }"
               "QWidget#cloudyNativePage QHeaderView::section { color: %4; background: %1; border: 0; border-bottom: 1px solid %3; padding: 6px 8px; }"
               "QWidget#cloudyNativePage QTabBar::tab { color: %4; background: transparent; border: 1px solid transparent; border-radius: 8px; padding: 8px 10px; margin-right: 3px; }"
               "QWidget#cloudyNativePage QTabBar::tab:hover, QWidget#cloudyNativePage QTabBar::tab:selected { color: %2; background: %5; border-color: %3; }"
               "QWidget#cloudyNativePage QPushButton { color: %2; background: transparent; border: 1px solid %3; border-radius: 8px; padding: 7px 13px; }"
               "QWidget#cloudyNativePage QPushButton:hover { background: %5; border-color: %2; }"
               "QWidget#cloudyNativePage QDialogButtonBox QPushButton { min-width: 72px; }"
               "QWidget#cloudyNativePage QDialogButtonBox QPushButton[text=\\\"OK\\\"] { color: %8; background: %6; border-color: %6; font-weight: 650; }"
               "QWidget#cloudyNativePage QCheckBox::indicator, QWidget#cloudyNativePage QRadioButton::indicator { width: 15px; height: 15px; border: 1px solid %3; border-radius: 4px; background: %5; }"
               "QWidget#cloudyNativePage QCheckBox::indicator:checked, QWidget#cloudyNativePage QRadioButton::indicator:checked { background: %6; border-color: %6; }"
               "QWidget#cloudyNativePage QScrollBar:vertical { background: transparent; width: 8px; margin: 2px; }"
               "QWidget#cloudyNativePage QScrollBar::handle:vertical { background: %3; border-radius: 4px; min-height: 24px; }"
           )
        .arg(window)
        .arg(text)
        .arg(line)
        .arg(muted)
        .arg(surface)
        .arg(highlight)
        .arg(text)
        .arg(text);
}

}  // namespace

CloudyWebShell::CloudyWebShell(MainWindow* window, QWidget* parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("cloudyWebShell"));
    registerCloudyScheme();

    m_webView = new QWebEngineView(this);
    m_webView->setObjectName(QStringLiteral("cloudyWebView"));
    m_webView->setContextMenuPolicy(Qt::NoContextMenu);
    m_webView->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, false);
    m_webView->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, false);
    m_webView->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);
    const auto document = buildCloudyDocument();
    const auto initialDocument = document.isEmpty()
                                     ? QByteArrayLiteral(
                                           "<!doctype html><meta charset=\"utf-8\"><title>Cloudy Launcher</title>"
                                           "<style>body{margin:0;background:#0a1220;color:#f3f7ff;font:16px system-ui;padding:32px}h1{font-weight:600}</style>"
                                           "<h1>Cloudy Launcher</h1><p>The embedded workspace could not be loaded. Check the installation files and restart Cloudy.</p>")
                                     : document.toUtf8();
    if (document.isEmpty())
        qCritical() << "Cloudy embedded start document is empty.";
    m_webView->page()->profile()->installUrlSchemeHandler(QByteArrayLiteral("cloudy"),
                                                          new CloudyResourceHandler(initialDocument, m_webView->page()->profile()));
    m_webView->setGeometry(rect());

    m_channel = new QWebChannel(m_webView);
    m_bridge = new CloudyWebBridge(window, m_channel);
    m_channel->registerObject(QStringLiteral("cloudy"), m_bridge);
    m_webView->page()->setWebChannel(m_channel);
    connect(m_webView, &QWebEngineView::loadFinished, this, [](bool ok) {
        if (!ok)
            qCritical() << "Cloudy WebEngine failed to render its embedded start page.";
    });

    // Let the top-level window enter the event loop before starting WebEngine's
    // first navigation. This avoids a Windows startup race where the process
    // appears briefly in Task Manager but the initial document never paints.
    QTimer::singleShot(0, this, [this] {
        if (!m_webView)
            return;
        m_webView->setUrl(QUrl(QStringLiteral("cloudy://shell/index.html")));
    });
}

QRect CloudyWebShell::nativeContentRect() const
{
    // Keep the web-rendered Cloudy app bar and rail visible while a real native
    // workflow is open. Native pages occupy only the body content rectangle.
    constexpr int railWidth = 64;
    constexpr int headerHeight = 48;
    return QRect(railWidth, headerHeight, qMax(0, width() - railWidth), qMax(0, height() - headerHeight));
}

void CloudyWebShell::showNativePage(QWidget* page)
{
    if (!page)
        return;

    if (m_nativePage) {
        m_nativePage->hide();
        m_nativePage->deleteLater();
    }

    m_nativePage = page;
    m_nativePage->setParent(this);
    m_nativePage->setObjectName(QStringLiteral("cloudyNativePage"));
    m_nativePage->setWindowFlags(Qt::Widget);
    m_nativePage->setAttribute(Qt::WA_DeleteOnClose, false);
    const auto nativeStyle = cloudyNativeStyleSheet();
    if (!nativeStyle.isEmpty()) {
        QPalette nativePalette = m_nativePage->palette();
        nativePalette.setColor(QPalette::Window, QColor(QStringLiteral("#0a1220")));
        nativePalette.setColor(QPalette::Base, QColor(QStringLiteral("#0f1a2b")));
        nativePalette.setColor(QPalette::AlternateBase, QColor(QStringLiteral("#112039")));
        nativePalette.setColor(QPalette::Button, QColor(QStringLiteral("#14233a")));
        nativePalette.setColor(QPalette::Text, QColor(QStringLiteral("#f3f7ff")));
        nativePalette.setColor(QPalette::WindowText, QColor(QStringLiteral("#f3f7ff")));
        nativePalette.setColor(QPalette::ButtonText, QColor(QStringLiteral("#f3f7ff")));
        nativePalette.setColor(QPalette::Highlight, QColor(QStringLiteral("#1b304d")));
        nativePalette.setColor(QPalette::HighlightedText, QColor(QStringLiteral("#ffffff")));
        m_nativePage->setPalette(nativePalette);
        for (auto* child : m_nativePage->findChildren<QWidget*>())
            child->setPalette(nativePalette);
    }
    m_nativePage->setStyleSheet(nativeStyle);
    m_nativePage->setGeometry(nativeContentRect());
    m_nativePage->raise();
    m_nativePage->show();
}

void CloudyWebShell::restoreWebPage()
{
    if (m_nativePage) {
        m_nativePage->hide();
        m_nativePage->deleteLater();
        m_nativePage = nullptr;
    }
    m_webView->raise();
    m_webView->setFocus();
    if (m_bridge)
        m_bridge->stateChanged();
}

void CloudyWebShell::prepareForShutdown()
{
    if (!m_webView || !m_webView->page())
        return;

    if (m_nativePage)
        m_nativePage->hide();

    m_webView->hide();
    m_webView->stop();
    m_webView->page()->setWebChannel(nullptr);
    m_webView->page()->setVisible(false);
    qInfo() << "Cloudy WebEngine page stopped before shutdown.";
}

void CloudyWebShell::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (m_webView)
        m_webView->setGeometry(rect());
    updateNativePageGeometry();
}

void CloudyWebShell::updateNativePageGeometry()
{
    if (m_nativePage)
        m_nativePage->setGeometry(nativeContentRect());
}
