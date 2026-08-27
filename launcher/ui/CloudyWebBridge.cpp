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
#include "icons/IconList.h"
#include "settings/SettingsObject.h"
#include "translations/TranslationsModel.h"

#include <QApplication>
#include <QBuffer>
#include <QColor>
#include <QDirIterator>
#include <QFileInfo>
#include <QMetaObject>
#include <QPalette>
#include <QRegularExpression>
#include <QStorageInfo>
#include <QVariantMap>
#include <QWindow>

CloudyWebBridge::CloudyWebBridge(MainWindow* window, QObject* parent) : QObject(parent), m_window(window) {}

namespace {
quint64 directorySize(const QString& path)
{
    if (path.isEmpty() || !QFileInfo::exists(path))
        return 0;

    quint64 total = 0;
    QDirIterator iterator(path, QDir::Files | QDir::Hidden | QDir::System, QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        iterator.next();
        total += static_cast<quint64>(iterator.fileInfo().size());
    }
    return total;
}

QString iconDataUri(const QIcon& icon)
{
    const auto pixmap = icon.pixmap(96, 96);
    if (pixmap.isNull())
        return {};

    QByteArray bytes;
    QBuffer buffer(&bytes);
    if (!buffer.open(QIODevice::WriteOnly) || !pixmap.save(&buffer, "PNG"))
        return {};
    return QStringLiteral("data:image/png;base64,") + QString::fromLatin1(bytes.toBase64());
}
}

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
        if (APPLICATION->icons())
            item.insert(QStringLiteral("iconData"), iconDataUri(APPLICATION->icons()->getIcon(instance->iconKey())));
        result.push_back(item);
    }
    return result;
}

QVariantList CloudyWebBridge::accounts() const
{
    QVariantList result;
    if (!APPLICATION || !APPLICATION->accounts())
        return result;

    const auto* list = APPLICATION->accounts();
    result.reserve(list->count());
    for (int i = 0; i < list->count(); ++i) {
        const auto account = list->at(i);
        if (!account)
            continue;

        QVariantMap item;
        item.insert(QStringLiteral("id"), account->internalId());
        item.insert(QStringLiteral("name"), account->displayName());
        item.insert(QStringLiteral("profileName"), account->profileName());
        item.insert(QStringLiteral("uuid"), account->profileId());
        item.insert(QStringLiteral("type"), account->typeString());
        item.insert(QStringLiteral("active"), account->isActive());
        item.insert(QStringLiteral("ownsMinecraft"), account->ownsMinecraft());
        item.insert(QStringLiteral("hasProfile"), account->hasProfile());

        if (account->hasProfile()) {
            QByteArray faceBytes;
            QBuffer buffer(&faceBytes);
            buffer.open(QIODevice::WriteOnly);
            account->getFace(64, 64).save(&buffer, "PNG");
            item.insert(QStringLiteral("face"), QStringLiteral("data:image/png;base64,") + QString::fromLatin1(faceBytes.toBase64()));
        }
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

bool CloudyWebBridge::firstRun() const
{
    return APPLICATION && APPLICATION->isFirstRun();
}

QVariantMap CloudyWebBridge::storageData() const
{
    QVariantMap result;
    if (!APPLICATION)
        return result;

    const QString applicationRoot = APPLICATION->root();
    const QString dataRoot = APPLICATION->dataRoot();
    result.insert(QStringLiteral("applicationBytes"), QVariant::fromValue<qulonglong>(directorySize(applicationRoot)));
    result.insert(QStringLiteral("dataBytes"), QVariant::fromValue<qulonglong>(directorySize(dataRoot)));

    const QStorageInfo storage(dataRoot.isEmpty() ? applicationRoot : dataRoot);
    if (storage.isValid()) {
        result.insert(QStringLiteral("diskAvailableBytes"), QVariant::fromValue<qulonglong>(storage.bytesAvailable()));
        result.insert(QStringLiteral("diskTotalBytes"), QVariant::fromValue<qulonglong>(storage.bytesTotal()));
    }
    return result;
}

QString CloudyWebBridge::language() const
{
    return APPLICATION && APPLICATION->settings() ? APPLICATION->settings()->get(QStringLiteral("Language")).toString() : QStringLiteral("en_US");
}

QString CloudyWebBridge::weatherTheme() const
{
    return APPLICATION && APPLICATION->settings() ? APPLICATION->settings()->get(QStringLiteral("CloudyWeatherTheme")).toString() : QStringLiteral("cloudy");
}

QString CloudyWebBridge::snowVariant() const
{
    return APPLICATION && APPLICATION->settings() ? APPLICATION->settings()->get(QStringLiteral("CloudySnowVariant")).toString() : QStringLiteral("light");
}

bool CloudyWebBridge::soundsEnabled() const
{
    return !APPLICATION || !APPLICATION->settings() || APPLICATION->settings()->get(QStringLiteral("CloudySoundEnabled")).toBool();
}

int CloudyWebBridge::globalMaxMemory() const
{
    return APPLICATION && APPLICATION->settings() ? APPLICATION->settings()->get(QStringLiteral("MaxMemAlloc")).toInt() : 4096;
}

QString CloudyWebBridge::selectedInstanceId() const
{
    return APPLICATION && APPLICATION->settings() ? APPLICATION->settings()->get(QStringLiteral("SelectedInstance")).toString() : QString();
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

void CloudyWebBridge::openModInstaller(const QString& instanceId, const QString& resourceType)
{
    if (m_window)
        m_window->webOpenResourceInstaller(instanceId, resourceType);
}

void CloudyWebBridge::openFiles(const QString& instanceId)
{
    if (m_window)
        m_window->webOpenFiles(instanceId);
}

void CloudyWebBridge::openInstancePage(const QString& instanceId, const QString& page)
{
    if (m_window)
        m_window->webOpenInstancePage(instanceId, page);
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

void CloudyWebBridge::minimizeWindow()
{
    if (m_window)
        m_window->showMinimized();
}

void CloudyWebBridge::toggleMaximizeWindow()
{
    if (!m_window)
        return;
    if (m_window->isMaximized())
        m_window->showNormal();
    else
        m_window->showMaximized();
}

void CloudyWebBridge::closeWindow()
{
    if (m_window)
        m_window->close();
}

void CloudyWebBridge::beginWindowDrag()
{
    if (m_window && m_window->windowHandle())
        m_window->windowHandle()->startSystemMove();
}

void CloudyWebBridge::setLanguage(const QString& language)
{
    if (!APPLICATION || !APPLICATION->settings())
        return;
    const QString normalized = language.startsWith(QStringLiteral("ru"), Qt::CaseInsensitive) ? QStringLiteral("ru_RU") : QStringLiteral("en_US");
    APPLICATION->settings()->set(QStringLiteral("Language"), normalized);
    if (APPLICATION->translations())
        APPLICATION->translations()->selectLanguage(normalized);
    emit stateChanged();
}

void CloudyWebBridge::setWeatherTheme(const QString& theme)
{
    static const QStringList validThemes{ QStringLiteral("cloudy"), QStringLiteral("rain"), QStringLiteral("storm"), QStringLiteral("sunny"), QStringLiteral("night"), QStringLiteral("snow") };
    if (!APPLICATION || !APPLICATION->settings() || !validThemes.contains(theme))
        return;
    APPLICATION->settings()->set(QStringLiteral("CloudyWeatherTheme"), theme);
    emit stateChanged();
}

void CloudyWebBridge::setSnowVariant(const QString& variant)
{
    if (!APPLICATION || !APPLICATION->settings() || (variant != QStringLiteral("light") && variant != QStringLiteral("dark")))
        return;
    APPLICATION->settings()->set(QStringLiteral("CloudySnowVariant"), variant);
    emit stateChanged();
}

void CloudyWebBridge::setSoundsEnabled(bool enabled)
{
    if (!APPLICATION || !APPLICATION->settings())
        return;
    APPLICATION->settings()->set(QStringLiteral("CloudySoundEnabled"), enabled);
    emit stateChanged();
}

void CloudyWebBridge::setGlobalMemory(int megabytes)
{
    if (!APPLICATION || !APPLICATION->settings())
        return;
    const int safeMax = qBound(512, megabytes, 65536);
    const int safeMin = qMin(512, safeMax);
    APPLICATION->settings()->set(QStringLiteral("MinMemAlloc"), safeMin);
    APPLICATION->settings()->set(QStringLiteral("MaxMemAlloc"), safeMax);
    emit stateChanged();
}

bool CloudyWebBridge::addOfflineAccount(const QString& username)
{
    if (!APPLICATION || !APPLICATION->accounts())
        return false;

    static const QRegularExpression usernamePattern(QStringLiteral("^[A-Za-z0-9_]{3,16}$"));
    const QString cleanName = username.trimmed();
    if (!usernamePattern.match(cleanName).hasMatch())
        return false;

    const auto account = MinecraftAccount::createOffline(cleanName);
    if (!account)
        return false;

    const bool shouldBecomeDefault = !APPLICATION->accounts()->defaultAccount();
    account->login()->start();
    APPLICATION->accounts()->addAccount(account);
    if (shouldBecomeDefault)
        APPLICATION->accounts()->setDefaultAccount(account);
    emit stateChanged();
    return true;
}

void CloudyWebBridge::completeFirstRun()
{
    if (!APPLICATION)
        return;
    APPLICATION->completeFirstRun();
    emit stateChanged();
}
