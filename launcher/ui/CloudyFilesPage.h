// SPDX-License-Identifier: GPL-3.0-only
/*
 * Cloudy Launcher - embedded instance file workspace.
 * Copyright (C) 2026 Cloudy Launcher contributors.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 */

#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QTreeView;
class QFileSystemModel;
class MinecraftInstance;

class CloudyFilesPage final : public QWidget {
    Q_OBJECT

   public:
    explicit CloudyFilesPage(MinecraftInstance* instance, QWidget* parent = nullptr);

   signals:
    void closeRequested();

   private slots:
    void openSelectedFile(const QModelIndex& index);
    void saveFile();
    void createFolder();
    void refreshFiles();

   private:
    QString selectedPath(const QModelIndex& index) const;
    bool loadTextFile(const QString& path);
    void setStatus(const QString& message, bool error = false);

    MinecraftInstance* m_instance = nullptr;
    QString m_rootPath;
    QString m_currentFilePath;
    QFileSystemModel* m_fileModel = nullptr;
    QTreeView* m_fileTree = nullptr;
    QPlainTextEdit* m_editor = nullptr;
    QLabel* m_locationLabel = nullptr;
    QLabel* m_statusLabel = nullptr;
    QLineEdit* m_folderName = nullptr;
    QPushButton* m_saveButton = nullptr;
};
