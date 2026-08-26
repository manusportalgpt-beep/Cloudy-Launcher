/*
 * Cloudy Launcher - web workspace bridge.
 * Copyright (C) 2026 Cloudy Launcher contributors.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 */

#include "CloudyWebBridge.h"

#include "Application.h"
#include "BaseInstance.h"
#include "InstanceList.h"
#include "minecraft/auth/AccountList.h"
#include "minecraft/auth/MinecraftAccount.h"
#include "MainWindow.h"

#include <QApplication>
#include <QColor>
#include <QMetaObject>
#include <QPalette>
#include <QVariantMap>

CloudyWebBridge::CloudyWebBridge(MainWindow* window, QObject* parent) : QObject(parent), m_window(window) {}

QVariantList CloudyWebBridge::instances() const
{
    QVariantList result;
    if (!APPLICATION || !APPLICATION->instances())
        return result;

    auto* list = APPLICATION->instances();
    result.reserve(list->count());
    for (int i = 0; i < list->count(); ++i) {
        auto* instance = list->at(i);
        if (!instance)
            continue;

        QVariantMap item;
        item.insert(QStringLiteral("id"), instance->id());
        item.insert(QStringLiteral("name"), instance->name());
        item.insert(QStringLiteral("icon"), instance->iconKey());
        item.insert(QStringLiteral("running"), instance->isRunning());
        item.insert(QStringLiteral("managed"), instance->isManagedPack());
        item.insert(QStringLiteral("managedType"), instance->getManagedPackType());
        item.insert(QStringLiteral("managedVersion"), instance->getManagedPackVersionName());
        result.push_back(item);
    }
    return result;
}

QString CloudyWebBridge::activeAccount() const
{
    if (!APPLICATION || !APPLICATION->accounts())
        return {};
    const auto account = APPLICATION->accounts()->defaultAccount();
    return account ? account->displayName() : QString();
}

bool CloudyWebBridge::hasActiveAccount() const
{
    return !activeAccount().isEmpty();
}

QVariantMap CloudyWebBridge::paletteData() const
{
    QVariantMap result;
    const auto palette = qApp->palette();
    const auto color = [&palette](QPalette::ColorRole role) { return palette.color(role); };
    const auto insert = [&result, &color](const QString& name, QPalette::ColorRole role, bool neutralOnly) {
        const auto value = color(role);
        if (!neutralOnly || value.saturationF() <= 0.30)
            result.insert(name, value.name(QColor::HexArgb));
    };
    // A user custom palette remains authoritative for native Qt widgets. The
    // web shell keeps its restrained Cloudy surface tokens stable and imports
    // only readable text roles; this prevents a legacy accent from turning the
    // connected workspace neon red/blue while retaining theme-aware contrast.
    insert(QStringLiteral("text"), QPalette::Text, false);
    insert(QStringLiteral("buttonText"), QPalette::ButtonText, false);
    insert(QStringLiteral("highlightedText"), QPalette::HighlightedText, false);
    return result;
}

void CloudyWebBridge::invokeWindowAction(const char* method)
{
    if (!m_window || !method)
        return;
    QMetaObject::invokeMethod(m_window, method, Qt::QueuedConnection);
}

void CloudyWebBridge::openLibrary()
{
    if (m_window)
        m_window->restoreMainContent();
    emit stateChanged();
}

void CloudyWebBridge::openNewInstance()
{
    invokeWindowAction("on_actionAddInstance_triggered");
}

void CloudyWebBridge::openAccounts()
{
    invokeWindowAction("on_actionManageAccounts_triggered");
}

void CloudyWebBridge::openSettings()
{
    invokeWindowAction("on_actionSettings_triggered");
}

void CloudyWebBridge::openSkinStudio()
{
    invokeWindowAction("on_actionManageSkins_triggered");
}

void CloudyWebBridge::openMods(const QString& instanceId)
{
    if (!m_window)
        return;
    m_window->webOpenInstancePage(instanceId, QStringLiteral("mods"));
}

void CloudyWebBridge::openFiles(const QString& instanceId)
{
    if (!m_window)
        return;
    m_window->webSelectInstance(instanceId);
    QMetaObject::invokeMethod(m_window, "on_actionViewSelectedInstFolder_triggered", Qt::QueuedConnection);
}

void CloudyWebBridge::selectInstance(const QString& instanceId)
{
    if (!m_window)
        return;
    m_window->webSelectInstance(instanceId);
    emit stateChanged();
}

void CloudyWebBridge::launchInstance(const QString& instanceId)
{
    if (!m_window)
        return;
    m_window->webSelectInstance(instanceId);
    QMetaObject::invokeMethod(m_window, "on_actionLaunchInstance_triggered", Qt::QueuedConnection);
}

void CloudyWebBridge::refreshInstances()
{
    invokeWindowAction("refreshInstances");
    emit stateChanged();
}

void CloudyWebBridge::changeSelectedIcon()
{
    invokeWindowAction("on_actionChangeInstIcon_triggered");
}

void CloudyWebBridge::openLauncherFolder()
{
    invokeWindowAction("on_actionViewLauncherRootFolder_triggered");
}
