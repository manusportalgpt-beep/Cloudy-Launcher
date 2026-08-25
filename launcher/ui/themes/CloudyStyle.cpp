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
        // One continuous application canvas.
        "QWidget#cloudyWorkspace { background: palette(window); color: palette(windowtext); }"
        "QFrame#cloudyHeader { background: palette(base); border-bottom: 1px solid palette(mid); }"
        "QFrame#cloudyHeader QLabel { background: transparent; }"
        "QLabel#cloudyHeaderName { color: palette(windowtext); font-size: 17px; font-weight: 700; }"
        "QLabel#cloudyHeaderTagline, QLabel#cloudyWorkspaceSubtitle { color: palette(mid); font-size: 11px; }"
        "QLabel#cloudyWorkspaceTitle { color: palette(windowtext); font-size: 15px; font-weight: 600; }"
        "QToolButton#cloudyHeaderPrimary, QToolButton#cloudyHeaderButton, QToolButton#cloudyAccountButton { min-height: 34px; padding: 7px 11px; border: 1px solid palette(mid); border-radius: 10px; background: palette(base); color: palette(button-text); }"
        "QToolButton#cloudyHeaderPrimary { background: palette(highlight); color: palette(highlighted-text); border-color: palette(highlight); font-weight: 600; }"
        "QToolButton#cloudyHeaderPrimary:hover, QToolButton#cloudyHeaderButton:hover, QToolButton#cloudyAccountButton:hover { background: palette(alternate-base); }"
        "QToolButton#cloudyHeaderPrimary:hover { background: palette(highlight); }"
        "QWidget#cloudyWorkspaceBody { background: palette(window); }"
        "QFrame#cloudyWorkspaceNav { background: transparent; border: 0; border-right: 1px solid palette(mid); border-radius: 0; }"
        "QFrame#cloudyDetailCanvas { background: transparent; border: 0; border-left: 1px solid palette(mid); border-radius: 0; }"
        "QToolButton#cloudyNavLibrary, QToolButton#cloudyNavAdd, QToolButton#cloudyNavAccounts, QToolButton#cloudyNavSettings, QToolButton#cloudyNavSkins { min-height: 42px; padding: 9px 12px; border: 1px solid transparent; border-radius: 11px; background: transparent; color: palette(windowtext); text-align: left; }"
        "QToolButton#cloudyNavLibrary:hover, QToolButton#cloudyNavAdd:hover, QToolButton#cloudyNavAccounts:hover, QToolButton#cloudyNavSettings:hover, QToolButton#cloudyNavSkins:hover { background: palette(alternate-base); border-color: palette(mid); }"
        "QToolButton#cloudyNavLibrary:checked, QToolButton#cloudyNavAdd:checked, QToolButton#cloudyNavAccounts:checked, QToolButton#cloudyNavSettings:checked, QToolButton#cloudyNavSkins:checked { background: palette(highlight); border-color: palette(highlight); color: palette(highlighted-text); font-weight: 600; }"
        "QLabel#cloudyNavCaption { color: palette(mid); font-size: 10px; padding: 8px; }"
        "QStackedWidget#cloudyContentStack { background: transparent; border: 0; }"
        "QFrame#cloudyFooter { background: palette(base); border-top: 1px solid palette(mid); }"
        "QFrame#cloudyFooter QLabel { color: palette(mid); font-size: 10px; }"
        // Library content shares the same surfaces as every other Cloudy page.
        "QWidget#librarySurface, QWidget#cloudyPageWorkspace, QWidget#cloudySettingsShell, QWidget#cloudySettingsContent, QWidget#cloudyAccountSurface { background: palette(window); }"
        "QWidget#cloudyPageWorkspace QWidget#cloudySettingsHeader { background: transparent; }"
        "QWidget#cloudyPageWorkspace QDialogButtonBox { background: palette(base); border-top: 1px solid palette(mid); padding: 8px; }"
        "QLabel#libraryTitle { color: palette(windowtext); font-size: 23px; font-weight: 700; }"
        "QLabel#librarySubtitle { color: palette(mid); font-size: 12px; }"
        "QLineEdit#librarySearch, QLineEdit#cloudySearch { min-width: 260px; min-height: 34px; padding: 8px 12px; border: 1px solid palette(mid); border-radius: 11px; background: palette(base); color: palette(text); }"
        "QLineEdit#librarySearch:focus, QLineEdit#cloudySearch:focus { border: 2px solid palette(highlight); padding: 7px 11px; }"
        "QToolButton#libraryCreateButton { min-height: 34px; padding: 8px 15px; border: 1px solid palette(highlight); border-radius: 11px; background: palette(highlight); color: palette(highlighted-text); font-weight: 600; }"
        "QToolButton#libraryCreateButton:hover { background: palette(highlight); }"
        "QAbstractItemView#cloudyInstanceView { background: transparent; border: 0; padding: 8px 2px; }"
        // Detail canvas and shared action language.
        "QLabel#cloudyDetailCaption { color: palette(mid); font-size: 10px; font-weight: 600; text-transform: uppercase; letter-spacing: 1px; }"
        "QLabel#cloudyInstanceIcon { background: palette(alternate-base); border: 1px solid palette(mid); border-radius: 18px; padding: 8px; }"
        "QLabel#cloudyInstanceTitle { color: palette(windowtext); font-size: 17px; font-weight: 700; }"
        "QLabel#cloudyInstanceMeta { color: palette(mid); font-size: 11px; }"
        "QFrame#cloudyDetailSeparator { color: palette(mid); background: palette(mid); min-height: 1px; max-height: 1px; }"
        "QToolButton#cloudyDetailPrimary, QToolButton#cloudyDetailButton, QToolButton#cloudyDetailDanger { min-height: 32px; padding: 7px 10px; border: 0; border-radius: 8px; background: transparent; color: palette(button-text); text-align: left; }"
        "QToolButton#cloudyDetailPrimary { background: palette(highlight); border-color: palette(highlight); color: palette(highlighted-text); font-weight: 700; }"
        "QToolButton#cloudyDetailPrimary:hover, QToolButton#cloudyDetailButton:hover, QToolButton#cloudyDetailDanger:hover { background: palette(alternate-base); }"
        "QToolButton#cloudyDetailDanger { color: palette(windowtext); }"
        "QToolButton#cloudyDetailPrimary:hover { background: palette(highlight); }"
        // Shared settings/account/editor language.
        "QMainWindow, QDialog { background: palette(window); color: palette(windowtext); }"
        "QPushButton, QToolButton { min-height: 30px; padding: 6px 11px; border: 1px solid palette(mid); border-radius: 9px; background: palette(button); color: palette(button-text); }"
        "QPushButton:hover, QToolButton:hover { background: palette(alternate-base); }"
        "QPushButton:pressed, QToolButton:pressed { background: palette(mid); }"
        "QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox, QDateEdit, QTimeEdit, QTextEdit, QPlainTextEdit { min-height: 30px; padding: 6px 9px; border: 1px solid palette(mid); border-radius: 9px; background: palette(base); color: palette(text); selection-background-color: palette(highlight); selection-color: palette(highlighted-text); }"
        "QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus, QDateEdit:focus, QTimeEdit:focus, QTextEdit:focus, QPlainTextEdit:focus { border: 2px solid palette(highlight); padding: 5px 8px; }"
        "QGroupBox { margin-top: 12px; padding: 14px 10px 10px 10px; border: 1px solid palette(mid); border-radius: 12px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px; color: palette(windowtext); font-weight: 600; }"
        "QTreeView, QListView, QTableView { background: palette(base); alternate-background-color: palette(alternate-base); border: 1px solid palette(mid); border-radius: 11px; }"
        "QHeaderView::section { padding: 8px 10px; border: 0; border-bottom: 1px solid palette(mid); background: palette(alternate-base); color: palette(windowtext); font-weight: 600; }"
        "QLabel#cloudySettingsTitle { color: palette(windowtext); font-size: 19px; font-weight: 700; }"
        "QLabel#cloudyPageTitle, QLabel#cloudySettingsPageTitle { color: palette(windowtext); font-size: 18px; font-weight: 700; }"
        "QLabel#cloudySettingsSubtitle { color: palette(mid); }"
        "QListView#cloudyPageNav, QListView#cloudySettingsNav { background: palette(base); border: 1px solid palette(mid); border-radius: 12px; padding: 8px; }"
        "QListView#cloudyPageNav::item, QListView#cloudySettingsNav::item { min-height: 34px; padding: 8px 10px; border-radius: 9px; }"
        "QListView#cloudyPageNav::item:hover, QListView#cloudySettingsNav::item:hover { background: palette(alternate-base); }"
        "QListView#cloudyPageNav::item:selected, QListView#cloudySettingsNav::item:selected { background: palette(highlight); color: palette(highlighted-text); }"
        "QFrame#cloudySection, QGroupBox#cloudySection { background: palette(base); border: 1px solid palette(mid); border-radius: 12px; }"
        "QPushButton#cloudyPrimaryButton, QToolButton#cloudyPrimaryButton { background: palette(highlight); color: palette(highlighted-text); border-color: palette(highlight); border-radius: 10px; padding: 8px 14px; font-weight: 600; }"
        "QScrollArea#cloudyScrollArea { border: 0; background: transparent; }"
        "QTabWidget#cloudyTabs::pane { border: 1px solid palette(mid); border-radius: 10px; top: -1px; }"
        "QTabBar#cloudyTabs::tab { padding: 9px 13px; border: 0; }"
        "QTabBar#cloudyTabs::tab:selected { border-bottom: 2px solid palette(highlight); }"
        "QProgressBar#cloudyProgress { min-height: 6px; max-height: 6px; border: 0; border-radius: 3px; background: palette(alternate-base); text-align: center; }"
        "QProgressBar#cloudyProgress::chunk { border-radius: 3px; background: palette(highlight); }"
        "QFrame#cloudyAccountCard { background: palette(base); border: 1px solid palette(mid); border-radius: 12px; }"
        "QListView#cloudyAccountList { background: transparent; border: 0; padding: 6px; }"
        "QListView#cloudyAccountList::item { padding: 9px 10px; border-radius: 9px; }"
        "QListView#cloudyAccountList::item:hover { background: palette(alternate-base); }"
        "QListView#cloudyAccountList::item:selected { background: palette(highlight); color: palette(highlighted-text); }"
        "QDialog#cloudyDialog { background: palette(window); }"
        "QDialog#cloudyDialog QGroupBox { border: 1px solid palette(mid); border-radius: 12px; margin-top: 10px; padding: 12px 10px 10px 10px; }"
        "QWidget#cloudyEmbeddedPage { background: palette(window); border: 0; }"
        "QFrame#cloudyLoadingPage { background: palette(window); }"
        "QLabel#cloudyLoadingTitle { color: palette(windowtext); font-size: 18px; font-weight: 600; }"
        "QLabel#cloudyLoadingSubtitle { color: palette(mid); font-size: 12px; }"
        "QDialog#cloudyDialog QListView { background: palette(base); border: 1px solid palette(mid); border-radius: 11px; padding: 6px; }");
}

}  // namespace CloudyStyle
