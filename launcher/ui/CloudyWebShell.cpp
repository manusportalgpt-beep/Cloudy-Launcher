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

#include <QBuffer>
#include <QFile>
#include <QMimeDatabase>
#include <QResizeEvent>
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
    const QByteArray app = readCloudyResource(QStringLiteral(":/cloudy-web/app.js"));
    const QByteArray cloudArt = readCloudyResource(QStringLiteral(":/cloudy-web/cloud-art.png")).toBase64();

    html.replace("CLOUDY_CLOUD_ART", cloudArt);
    html.replace("<!-- CLOUDY_CSS -->", "<style>" + css + "</style>");
    html.replace("<!-- CLOUDY_WEBCHANNEL_JS -->", "<script>" + channel + "</script>");
    html.replace("<!-- CLOUDY_APP_JS -->", "<script>" + app + "</script>");
    return QString::fromUtf8(html);
}

class CloudyResourceHandler final : public QWebEngineUrlSchemeHandler {
   public:
    explicit CloudyResourceHandler(QObject* parent = nullptr) : QWebEngineUrlSchemeHandler(parent) {}

    void requestStarted(QWebEngineUrlRequestJob* job) override
    {
        const auto url = job->requestUrl();
        const auto path = url.path();
        if (url.host() != QStringLiteral("shell") || path.contains(QStringLiteral(".."))) {
            job->fail(QWebEngineUrlRequestJob::UrlInvalid);
            return;
        }

        QString resourcePath;
        if (path == QStringLiteral("/qwebchannel.js")) {
            resourcePath = QStringLiteral(":/qtwebchannel/qwebchannel.js");
        } else {
            const auto normalizedPath = path.isEmpty() || path == QStringLiteral("/") ? QStringLiteral("/index.html") : path;
            resourcePath = QStringLiteral(":/cloudy-web") + normalizedPath;
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
    m_webView->page()->profile()->installUrlSchemeHandler(QByteArrayLiteral("cloudy"), new CloudyResourceHandler(m_webView->page()->profile()));
    m_webView->setGeometry(rect());

    m_channel = new QWebChannel(m_webView);
    m_bridge = new CloudyWebBridge(window, m_channel);
    m_channel->registerObject(QStringLiteral("cloudy"), m_bridge);
    m_webView->page()->setWebChannel(m_channel);
    m_webView->setHtml(buildCloudyDocument(), QUrl(QStringLiteral("cloudy://shell/")));
}

QRect CloudyWebShell::nativeContentRect() const
{
    // Keep the web-rendered Cloudy app bar and rail visible while a real native
    // workflow is open. Native pages occupy only the body content rectangle.
    constexpr int railWidth = 54;
    constexpr int headerHeight = 38;
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
