/*
 * Cloudy Launcher - unified web workspace host.
 * Copyright (C) 2026 Cloudy Launcher contributors.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 */

#pragma once

#include <QWidget>

class CloudyWebBridge;
class QWebChannel;
class QWebEngineView;
class MainWindow;

class CloudyWebShell : public QWidget {
    Q_OBJECT

   public:
    explicit CloudyWebShell(MainWindow* window, QWidget* parent = nullptr);

    QWebEngineView* webView() const { return m_webView; }
    CloudyWebBridge* bridge() const { return m_bridge; }

    void showNativePage(QWidget* page);
    void restoreWebPage();

   protected:
    void resizeEvent(QResizeEvent* event) override;

   private:
    QRect nativeContentRect() const;
    void updateNativePageGeometry();

    QWebEngineView* m_webView = nullptr;
    QWebChannel* m_channel = nullptr;
    CloudyWebBridge* m_bridge = nullptr;
    QWidget* m_nativePage = nullptr;
};
