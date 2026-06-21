#include <QFontDatabase>
#include <QFontMetrics>

#include "systemfontconfiguration.hpp"

SystemFontConfiguraton::SystemFontConfiguraton(QObject* parent)
    : QObject(parent) {}
SystemFontConfiguraton::~SystemFontConfiguraton() {}

QFont SystemFontConfiguraton::defaultFont() const {
  return QFontDatabase::systemFont(QFontDatabase::GeneralFont);
}

int SystemFontConfiguraton::defaultFontSize() const {
  return defaultFont().pointSize();
}

QFont SystemFontConfiguraton::defaultMonospaceFont() const {
  QFont monospaceFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  monospaceFont.setStyleHint(QFont::TypeWriter);
  return monospaceFont;
}

int SystemFontConfiguraton::defaultMonospaceFontSize() const {
  return defaultMonospaceFont().pointSize();
}

qreal SystemFontConfiguraton::calculateSpaceWidthOfFont(
    const QFont& font) const {
  const QFontMetrics fontMetrics(font);
  return fontMetrics.horizontalAdvance(' ');
}
