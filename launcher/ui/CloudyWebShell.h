/*
 * Cloudy Launcher - unified web workspace host.
 * Copyright (C) 2026 Cloudy Launcher contributors.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 */

#pragma once

#include <QList>
#include <QMetaObject>
#include <QPointer>
#include <QWidget>
#include "QObjectPtr.h"

class CloudyWebBridge;
class QWebChannel;
class QWebEngineView;
class MainWindow;
class Task;
class QFrame;
class QLabel;
class QProgressBar;
class QToolButton;

class CloudyWebShell : public QWidget {
    Q_OBJECT

   public:
    explicit CloudyWebShell(MainWindow* window, QWidget* parent = nullptr);

    QWebEngineView* webView() const { return m_webView; }
    CloudyWebBridge* bridge() const { return m_bridge; }

    void showNativePage(QWidget* page);
    void restoreWebPage();
    void watchTask(Task* task, const QString& title, const QString& iconName = {});
    void setTaskQueueCount(int count);
    void prepareForShutdown();

   protected:
    void resizeEvent(QResizeEvent* event) override;

   private:
    QRect nativeContentRect() const;
    void updateNativePageGeometry();
    void updateTaskDockGeometry();
    void updateTaskProgress(qint64 current, qint64 total);
    void finishTask(bool successful, const QString& message = {});
    void toggleTaskDock();

    QWebEngineView* m_webView = nullptr;
    QWebChannel* m_channel = nullptr;
    CloudyWebBridge* m_bridge = nullptr;
    QWidget* m_nativePage = nullptr;
    QFrame* m_taskDock = nullptr;
    QToolButton* m_taskToggle = nullptr;
    QToolButton* m_taskAbort = nullptr;
    QLabel* m_taskIcon = nullptr;
    QLabel* m_taskTitle = nullptr;
    QString m_taskBaseTitle;
    int m_taskQueueCount = 0;
    QLabel* m_taskStatus = nullptr;
    QLabel* m_taskPercent = nullptr;
    QProgressBar* m_taskProgress = nullptr;
    QWidget* m_taskDetails = nullptr;
    QPointer<Task> m_watchedTask;
    bool m_taskOutcomeKnown = false;
    bool m_taskSuccessful = false;
    QString m_taskFailure;
    bool m_taskExpanded = false;
    QList<QMetaObject::Connection> m_taskConnections;
};
