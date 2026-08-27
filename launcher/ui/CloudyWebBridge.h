/*
 * Cloudy Launcher - web workspace bridge.
 * Copyright (C) 2026 Cloudy Launcher contributors.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <QObject>
#include <QVariantList>
#include <QVariantMap>

class MainWindow;

/**
 * Small, explicit QObject API exposed to the local Cloudy web shell.
 *
 * The browser layer never receives credentials, file-system paths, or raw
 * backend objects. It only receives display-safe instance/account state and
 * asks MainWindow to execute the existing native actions.
 */
class CloudyWebBridge : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString activeAccount READ activeAccount NOTIFY stateChanged)
    Q_PROPERTY(bool hasActiveAccount READ hasActiveAccount NOTIFY stateChanged)
    Q_PROPERTY(bool firstRun READ firstRun NOTIFY stateChanged)
    Q_PROPERTY(QVariantList accountData READ accounts NOTIFY stateChanged)
    Q_PROPERTY(QVariantList instanceData READ instances NOTIFY stateChanged)
    Q_PROPERTY(QVariantMap paletteData READ paletteData NOTIFY stateChanged)
    Q_PROPERTY(QVariantMap storageData READ storageData NOTIFY stateChanged)
    Q_PROPERTY(QString language READ language NOTIFY stateChanged)
    Q_PROPERTY(QString weatherTheme READ weatherTheme NOTIFY stateChanged)
    Q_PROPERTY(QString snowVariant READ snowVariant NOTIFY stateChanged)
    Q_PROPERTY(bool soundsEnabled READ soundsEnabled NOTIFY stateChanged)
    Q_PROPERTY(int globalMaxMemory READ globalMaxMemory NOTIFY stateChanged)
    Q_PROPERTY(QString selectedInstanceId READ selectedInstanceId NOTIFY stateChanged)

   public:
    explicit CloudyWebBridge(MainWindow* window, QObject* parent = nullptr);

    Q_INVOKABLE QVariantList instances() const;
    Q_INVOKABLE QVariantList accounts() const;
    Q_INVOKABLE QString activeAccount() const;
    Q_INVOKABLE bool hasActiveAccount() const;
    Q_INVOKABLE bool firstRun() const;
    Q_INVOKABLE QVariantMap paletteData() const;
    Q_INVOKABLE QVariantMap storageData() const;
    Q_INVOKABLE QString language() const;
    Q_INVOKABLE QString weatherTheme() const;
    Q_INVOKABLE QString snowVariant() const;
    Q_INVOKABLE bool soundsEnabled() const;
    Q_INVOKABLE int globalMaxMemory() const;
    Q_INVOKABLE QString selectedInstanceId() const;

   public slots:
    void openLibrary();
    void openNewInstance();
    void openAccounts();
    void openSettings();
    void openSkinStudio();
    void openMods(const QString& instanceId);
    void openModInstaller(const QString& instanceId, const QString& resourceType = QStringLiteral("mods"));
    void openFiles(const QString& instanceId);
    void openInstancePage(const QString& instanceId, const QString& page = QStringLiteral("overview"));
    void selectInstance(const QString& instanceId);
    void launchInstance(const QString& instanceId);
    void refreshInstances();
    void changeSelectedIcon();
    void openLauncherFolder();
    void minimizeWindow();
    void toggleMaximizeWindow();
    void closeWindow();
    void beginWindowDrag();
    void setLanguage(const QString& language);
    void setWeatherTheme(const QString& theme);
    void setSnowVariant(const QString& variant);
    void setSoundsEnabled(bool enabled);
    void setGlobalMemory(int megabytes);
    bool addOfflineAccount(const QString& username);
    void completeFirstRun();

   signals:
    void stateChanged();

   private:
    void invokeWindowAction(const char* method);

    MainWindow* m_window = nullptr;
};
