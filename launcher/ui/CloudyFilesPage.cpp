// SPDX-License-Identifier: GPL-3.0-only
/*
 * Cloudy Launcher - embedded instance file workspace.
 * Copyright (C) 2026 Cloudy Launcher contributors.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 */

#include "CloudyFilesPage.h"

#include <QAbstractItemView>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSaveFile>
#include <QSplitter>
#include <QTreeView>
#include <QVBoxLayout>

#include "minecraft/MinecraftInstance.h"

namespace {
constexpr qint64 kMaxEditableFileSize = 2 * 1024 * 1024;

bool isSafeChildPath(const QString& rootPath, const QString& candidatePath)
{
    const auto root = QDir::cleanPath(QFileInfo(rootPath).absoluteFilePath());
    const auto candidate = QDir::cleanPath(QFileInfo(candidatePath).absoluteFilePath());
    return candidate == root || candidate.startsWith(root + QDir::separator());
}
}  // namespace

CloudyFilesPage::CloudyFilesPage(MinecraftInstance* instance, QWidget* parent) : QWidget(parent), m_instance(instance)
{
    setObjectName(QStringLiteral("cloudyFilesPage"));
    m_rootPath = m_instance ? m_instance->instanceRoot() : QString();

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(18, 14, 18, 18);
    rootLayout->setSpacing(10);

    auto* headerLayout = new QHBoxLayout;
    headerLayout->setSpacing(8);
    auto* heading = new QLabel(tr("Files"), this);
    heading->setObjectName(QStringLiteral("cloudyFilesHeading"));
    heading->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 700;"));
    headerLayout->addWidget(heading);
    m_locationLabel = new QLabel(this);
    m_locationLabel->setObjectName(QStringLiteral("cloudyFilesLocation"));
    m_locationLabel->setText(tr("Instance workspace"));
    headerLayout->addWidget(m_locationLabel, 1);

    auto* refreshButton = new QPushButton(tr("Refresh"), this);
    refreshButton->setObjectName(QStringLiteral("cloudyFilesRefresh"));
    connect(refreshButton, &QPushButton::clicked, this, &CloudyFilesPage::refreshFiles);
    headerLayout->addWidget(refreshButton);

    auto* closeButton = new QPushButton(tr("Back"), this);
    closeButton->setObjectName(QStringLiteral("cloudyFilesBack"));
    connect(closeButton, &QPushButton::clicked, this, &CloudyFilesPage::closeRequested);
    headerLayout->addWidget(closeButton);
    rootLayout->addLayout(headerLayout);

    auto* actionLayout = new QHBoxLayout;
    actionLayout->setSpacing(8);
    m_folderName = new QLineEdit(this);
    m_folderName->setPlaceholderText(tr("New folder name"));
    m_folderName->setClearButtonEnabled(true);
    actionLayout->addWidget(m_folderName, 1);
    auto* createFolderButton = new QPushButton(tr("Create folder"), this);
    createFolderButton->setObjectName(QStringLiteral("cloudyFilesCreateFolder"));
    connect(createFolderButton, &QPushButton::clicked, this, &CloudyFilesPage::createFolder);
    actionLayout->addWidget(createFolderButton);
    m_saveButton = new QPushButton(tr("Save file"), this);
    m_saveButton->setObjectName(QStringLiteral("cloudyFilesSave"));
    m_saveButton->setEnabled(false);
    connect(m_saveButton, &QPushButton::clicked, this, &CloudyFilesPage::saveFile);
    actionLayout->addWidget(m_saveButton);
    rootLayout->addLayout(actionLayout);

    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setObjectName(QStringLiteral("cloudyFilesSplitter"));

    m_fileModel = new QFileSystemModel(this);
    m_fileModel->setReadOnly(false);
    m_fileModel->setRootPath(m_rootPath);

    m_fileTree = new QTreeView(splitter);
    m_fileTree->setObjectName(QStringLiteral("cloudyFilesTree"));
    m_fileTree->setModel(m_fileModel);
    m_fileTree->setRootIndex(m_fileModel->index(m_rootPath));
    m_fileTree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_fileTree->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_fileTree->setDragEnabled(true);
    m_fileTree->setAcceptDrops(true);
    m_fileTree->setDropIndicatorShown(true);
    m_fileTree->setDragDropMode(QAbstractItemView::DragDrop);
    m_fileTree->setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);
    m_fileTree->setAlternatingRowColors(true);
    m_fileTree->setSortingEnabled(true);
    m_fileTree->sortByColumn(0, Qt::AscendingOrder);
    for (int column = 1; column < m_fileModel->columnCount(); ++column)
        m_fileTree->hideColumn(column);
    connect(m_fileTree, &QTreeView::doubleClicked, this, &CloudyFilesPage::openSelectedFile);
    splitter->addWidget(m_fileTree);

    m_editor = new QPlainTextEdit(splitter);
    m_editor->setObjectName(QStringLiteral("cloudyFilesEditor"));
    m_editor->setReadOnly(true);
    m_editor->setLineWrapMode(QPlainTextEdit::NoWrap);
    m_editor->setPlaceholderText(tr("Double-click a small text file to edit it here."));
    splitter->addWidget(m_editor);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    rootLayout->addWidget(splitter, 1);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setObjectName(QStringLiteral("cloudyFilesStatus"));
    m_statusLabel->setWordWrap(true);
    rootLayout->addWidget(m_statusLabel);

    if (m_rootPath.isEmpty() || !QDir(m_rootPath).exists())
        setStatus(tr("The instance folder is unavailable."), true);
    else
        setStatus(tr("Drag files or folders into the tree. Double-click a small text file to edit it."));
}

QString CloudyFilesPage::selectedPath(const QModelIndex& index) const
{
    if (!index.isValid() || !m_fileModel)
        return {};
    return m_fileModel->filePath(index);
}

void CloudyFilesPage::openSelectedFile(const QModelIndex& index)
{
    const auto path = selectedPath(index);
    if (path.isEmpty())
        return;

    const QFileInfo info(path);
    if (info.isDir()) {
        m_currentFilePath.clear();
        m_editor->clear();
        m_editor->setReadOnly(true);
        m_saveButton->setEnabled(false);
        m_locationLabel->setText(tr("Folder: %1").arg(info.fileName()));
        setStatus(tr("Folder selected. You can rename it with F2 or create a child folder."));
        return;
    }

    if (!isSafeChildPath(m_rootPath, path)) {
        setStatus(tr("The selected path is outside the instance workspace."), true);
        return;
    }

    if (info.size() > kMaxEditableFileSize) {
        m_currentFilePath.clear();
        m_editor->clear();
        m_editor->setReadOnly(true);
        m_saveButton->setEnabled(false);
        setStatus(tr("This file is larger than 2 MiB and is not opened in the editor."), true);
        return;
    }

    loadTextFile(path);
}

bool CloudyFilesPage::loadTextFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        setStatus(tr("Cannot read %1: %2").arg(QFileInfo(path).fileName(), file.errorString()), true);
        return false;
    }

    const auto bytes = file.readAll();
    if (bytes.contains('\0')) {
        m_currentFilePath.clear();
        m_editor->clear();
        m_editor->setReadOnly(true);
        m_saveButton->setEnabled(false);
        setStatus(tr("Binary files are kept in the tree and are not edited as text."));
        return false;
    }

    m_currentFilePath = path;
    m_editor->setPlainText(QString::fromUtf8(bytes));
    m_editor->setReadOnly(false);
    m_saveButton->setEnabled(true);
    m_locationLabel->setText(tr("Editing: %1").arg(QFileInfo(path).fileName()));
    setStatus(tr("Editing a UTF-8 text file. Save uses an atomic native write."));
    return true;
}

void CloudyFilesPage::saveFile()
{
    if (m_currentFilePath.isEmpty() || !isSafeChildPath(m_rootPath, m_currentFilePath)) {
        setStatus(tr("No safe text file is selected."), true);
        return;
    }

    QSaveFile file(m_currentFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        setStatus(tr("Cannot save %1: %2").arg(QFileInfo(m_currentFilePath).fileName(), file.errorString()), true);
        return;
    }

    const auto bytes = m_editor->toPlainText().toUtf8();
    if (file.write(bytes) != bytes.size() || !file.commit()) {
        setStatus(tr("Saving %1 failed: %2").arg(QFileInfo(m_currentFilePath).fileName(), file.errorString()), true);
        return;
    }
    setStatus(tr("Saved %1 safely.").arg(QFileInfo(m_currentFilePath).fileName()));
}

void CloudyFilesPage::createFolder()
{
    const auto name = m_folderName->text().trimmed();
    if (name.isEmpty() || name == QStringLiteral(".") || name == QStringLiteral("..") || name.contains('/') || name.contains('\\')) {
        setStatus(tr("Enter a folder name without path separators."), true);
        return;
    }

    QString parentPath = m_rootPath;
    const auto current = m_fileTree->currentIndex();
    if (current.isValid()) {
        const QFileInfo selectedInfo(selectedPath(current));
        if (selectedInfo.isDir())
            parentPath = selectedInfo.absoluteFilePath();
        else if (selectedInfo.exists())
            parentPath = selectedInfo.absolutePath();
    }

    if (!isSafeChildPath(m_rootPath, QDir(parentPath).filePath(name))) {
        setStatus(tr("The target folder is outside the instance workspace."), true);
        return;
    }

    QDir parent(parentPath);
    if (!parent.mkdir(name)) {
        setStatus(tr("Could not create folder %1.").arg(name), true);
        return;
    }
    m_folderName->clear();
    refreshFiles();
    setStatus(tr("Created folder %1.").arg(name));
}

void CloudyFilesPage::refreshFiles()
{
    if (!m_fileModel || m_rootPath.isEmpty())
        return;
    m_fileModel->setRootPath(QString());
    m_fileModel->setRootPath(m_rootPath);
    m_fileTree->setRootIndex(m_fileModel->index(m_rootPath));
    setStatus(tr("File tree refreshed."));
}

void CloudyFilesPage::setStatus(const QString& message, bool error)
{
    if (!m_statusLabel)
        return;
    m_statusLabel->setText(message);
    auto palette = m_statusLabel->palette();
    palette.setColor(QPalette::WindowText, error ? QColor(QStringLiteral("#ffb4ab")) : QColor(QStringLiteral("#a9bdd8")));
    m_statusLabel->setPalette(palette);
}
