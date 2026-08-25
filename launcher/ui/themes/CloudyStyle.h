// SPDX-License-Identifier: GPL-3.0-only
/*
 * Cloudy Launcher UI additions.
 * This file is part of an independent Prism Launcher fork.
 */
#pragma once

#include <QString>

namespace CloudyStyle {

/**
 * Component-level styling for Cloudy-built-in themes.
 *
 * Selectors intentionally target Cloudy object names and palette roles. The
 * stylesheet is not applied to user-provided custom themes, so their QSS/CSS
 * remains authoritative.
 */
QString applicationStyleSheet();

}  // namespace CloudyStyle
