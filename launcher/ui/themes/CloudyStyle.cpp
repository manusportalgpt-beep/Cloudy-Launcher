// SPDX-License-Identifier: GPL-3.0-only
/*
 * Cloudy Launcher UI additions.
 * This file is part of an independent Prism Launcher fork.
 */
#include "CloudyStyle.h"

namespace CloudyStyle {

QString applicationStyleSheet()
{
    return QStringLiteral(
        // Cloudy follows the reference as one flat dark canvas.
        "QWidget#cloudyWorkspace { background: palette(window); color: palette(windowtext); }"
        "QFrame#cloudyHeader { min-height: 38px; max-height: 38px; background: palette(base); border-bottom: 1px solid palette(mid); }"
        "QFrame#cloudyHeader QLabel { background: transparent; }"
        "QLabel#cloudyHeaderBrand { background: transparent; }"
        "QLabel#cloudyHeaderName { color: palette(windowtext); font-size: 12px; font-weight: 600; }"
        "QToolButton#cloudyHeaderNavigation { min-width: 24px; max-width: 24px; min-height: 24px; max-height: 24px; padding: 2px; border: 0; border-radius: 5px; background: transparent; color: palette(mid); }"
        "QToolButton#cloudyHeaderNavigation:hover { background: palette(alternate-base); color: palette(windowtext); }"
        "QToolButton#cloudyHeaderNavigation:disabled { color: palette(mid); }"
        "QLabel#cloudyBreadcrumbSeparator { color: palette(mid); font-size: 11px; }"
        "QLabel#cloudyWorkspaceTitle { color: palette(windowtext); font-size: 11px; font-weight: 500; }"
        "QLabel#cloudyRunningStatus { min-height: 22px; max-height: 22px; padding: 0 12px; border: 1px solid palette(mid); border-radius: 11px; background: palette(window); color: palette(mid); font-size: 10px; }"
        "QWidget#cloudyWorkspaceBody { background: palette(window); }"
        "QFrame#cloudyWorkspaceNav { background: palette(base); border: 0; border-right: 1px solid palette(mid); border-radius: 0; }"
        "QToolButton#cloudyNavLibrary, QToolButton#cloudyNavAdd, QToolButton#cloudyNavAccounts, QToolButton#cloudyNavSettings, QToolButton#cloudyNavSkins { min-width: 40px; max-width: 40px; min-height: 40px; max-height: 40px; padding: 9px; border: 0; border-radius: 7px; background: transparent; color: palette(mid); }"
        "QToolButton#cloudyNavLibrary:hover, QToolButton#cloudyNavAdd:hover, QToolButton#cloudyNavAccounts:hover, QToolButton#cloudyNavSettings:hover, QToolButton#cloudyNavSkins:hover { background: palette(alternate-base); color: palette(windowtext); }"
        "QToolButton#cloudyNavLibrary:checked, QToolButton#cloudyNavAdd:checked, QToolButton#cloudyNavAccounts:checked, QToolButton#cloudyNavSettings:checked, QToolButton#cloudyNavSkins:checked { background: palette(alternate-base); color: palette(windowtext); }"
        "QStackedWidget#cloudyContentStack { background: transparent; border: 0; }"
        "QFrame#cloudyFooter { min-height: 0; max-height: 0; background: transparent; border: 0; }"
        "QFrame#cloudyFooter QLabel { color: palette(mid); font-size: 10px; }"
        // Library is deliberately sparse like the reference concept.
        "QWidget#librarySurface, QWidget#cloudyPageWorkspace, QWidget#cloudySettingsShell, QWidget#cloudySettingsContent, QWidget#cloudyAccountSurface { background: palette(window); border: 0; }"
        "QLabel#libraryTitle { color: palette(windowtext); font-size: 20px; font-weight: 600; }"
        "QLabel#librarySubtitle { color: palette(mid); font-size: 12px; }"
        "QLineEdit#librarySearch, QLineEdit#cloudySearch { min-width: 110px; min-height: 28px; max-height: 28px; padding: 4px 8px; border: 1px solid palette(mid); border-radius: 6px; background: palette(base); color: palette(text); }"
        "QLineEdit#librarySearch:focus, QLineEdit#cloudySearch:focus { border: 1px solid palette(highlight); }"
        "QToolButton#cloudyReferenceAccount { min-width: 210px; min-height: 42px; max-height: 42px; padding: 6px 10px; border: 1px solid palette(mid); border-radius: 8px; background: palette(base); color: palette(windowtext); text-align: left; }"
        "QToolButton#cloudyReferenceAccount:hover { background: palette(alternate-base); }"
        "QAbstractItemView#cloudyInstanceView { background: transparent; border: 0; padding: 0; }"
        // The contextual rail exists only for a selected instance.
        "QFrame#cloudyDetailCanvas { background: transparent; border: 0; border-left: 1px solid palette(mid); border-radius: 0; }"
        "QLabel#cloudyDetailCaption { color: palette(mid); font-size: 9px; font-weight: 600; text-transform: uppercase; letter-spacing: 1px; }"
        "QLabel#cloudyInstanceIcon { background: palette(alternate-base); border: 1px solid palette(mid); border-radius: 8px; padding: 6px; }"
        "QLabel#cloudyInstanceTitle { color: palette(windowtext); font-size: 15px; font-weight: 600; }"
        "QLabel#cloudyInstanceMeta { color: palette(mid); font-size: 10px; }"
        "QFrame#cloudyDetailSeparator { color: palette(mid); background: palette(mid); min-height: 1px; max-height: 1px; }"
        "QToolButton#cloudyDetailPrimary, QToolButton#cloudyDetailButton, QToolButton#cloudyDetailDanger { min-height: 30px; padding: 6px 8px; border: 0; border-radius: 6px; background: transparent; color: palette(button-text); text-align: left; }"
        "QToolButton#cloudyDetailPrimary { background: palette(highlight); color: palette(highlighted-text); font-weight: 600; }"
        "QToolButton#cloudyDetailPrimary:hover, QToolButton#cloudyDetailButton:hover, QToolButton#cloudyDetailDanger:hover { background: palette(alternate-base); }"
        // Shared content language keeps legacy functional pages in the same shell.
        "QMainWindow, QDialog { background: palette(window); color: palette(windowtext); }"
        "QPushButton, QToolButton { min-height: 28px; padding: 5px 9px; border: 1px solid palette(mid); border-radius: 6px; background: palette(button); color: palette(button-text); }"
        "QPushButton:hover, QToolButton:hover { background: palette(alternate-base); }"
        "QPushButton:pressed, QToolButton:pressed { background: palette(mid); }"
        "QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox, QDateEdit, QTimeEdit, QTextEdit, QPlainTextEdit { min-height: 28px; padding: 5px 8px; border: 1px solid palette(mid); border-radius: 6px; background: palette(base); color: palette(text); selection-background-color: palette(highlight); selection-color: palette(highlighted-text); }"
        "QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus, QDateEdit:focus, QTimeEdit:focus, QTextEdit:focus, QPlainTextEdit:focus { border: 1px solid palette(highlight); }"
        "QGroupBox { margin-top: 10px; padding: 10px 8px 8px 8px; border: 1px solid palette(mid); border-radius: 7px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; color: palette(windowtext); font-weight: 500; }"
        "QTreeView, QListView, QTableView { background: palette(base); alternate-background-color: palette(alternate-base); border: 1px solid palette(mid); border-radius: 7px; }"
        "QHeaderView::section { padding: 6px 8px; border: 0; border-bottom: 1px solid palette(mid); background: palette(alternate-base); color: palette(windowtext); font-weight: 500; }"
        "QLabel#cloudySettingsTitle, QLabel#cloudyPageTitle, QLabel#cloudySettingsPageTitle { color: palette(windowtext); font-size: 18px; font-weight: 600; }"
        "QLabel#cloudySettingsSubtitle { color: palette(mid); }"
        "QListView#cloudyPageNav, QListView#cloudySettingsNav { background: transparent; border: 0; padding: 4px; }"
        "QListView#cloudyPageNav::item, QListView#cloudySettingsNav::item { min-height: 30px; padding: 6px 8px; border-radius: 6px; }"
        "QListView#cloudyPageNav::item:hover, QListView#cloudySettingsNav::item:hover { background: palette(alternate-base); }"
        "QListView#cloudyPageNav::item:selected, QListView#cloudySettingsNav::item:selected { background: palette(alternate-base); color: palette(windowtext); }"
        "QFrame#cloudySection, QGroupBox#cloudySection { background: transparent; border: 1px solid palette(mid); border-radius: 7px; }"
        "QPushButton#cloudyPrimaryButton, QToolButton#cloudyPrimaryButton { background: palette(highlight); color: palette(highlighted-text); border-color: palette(highlight); border-radius: 6px; padding: 6px 10px; font-weight: 600; }"
        "QScrollArea#cloudyScrollArea { border: 0; background: transparent; }"
        "QTabWidget#cloudyTabs::pane { border: 1px solid palette(mid); border-radius: 7px; top: -1px; }"
        "QTabBar#cloudyTabs::tab { padding: 7px 10px; border: 0; }"
        "QTabBar#cloudyTabs::tab:selected { border-bottom: 2px solid palette(highlight); }"
        "QProgressBar#cloudyProgress { min-height: 5px; max-height: 5px; border: 0; border-radius: 2px; background: palette(alternate-base); text-align: center; }"
        "QProgressBar#cloudyProgress::chunk { border-radius: 2px; background: palette(highlight); }"
        "QFrame#cloudyAccountCard { background: transparent; border: 0; }"
        "QListView#cloudyAccountList { background: transparent; border: 0; padding: 4px; }"
        "QListView#cloudyAccountList::item { padding: 7px 8px; border-radius: 6px; }"
        "QListView#cloudyAccountList::item:hover { background: palette(alternate-base); }"
        "QListView#cloudyAccountList::item:selected { background: palette(alternate-base); color: palette(windowtext); }"
        "QDialog#cloudyDialog { background: palette(window); }"
        "QDialog#cloudyDialog QGroupBox { border: 1px solid palette(mid); border-radius: 7px; margin-top: 8px; padding: 10px 8px 8px 8px; }"
        "QWidget#cloudyEmbeddedPage { background: palette(window); border: 0; }"
        "QFrame#cloudyLoadingPage { background: palette(window); }"
        "QLabel#cloudyLoadingTitle { color: palette(windowtext); font-size: 16px; font-weight: 500; }"
        "QLabel#cloudyLoadingSubtitle { color: palette(mid); font-size: 11px; }");
}

}  // namespace CloudyStyle
