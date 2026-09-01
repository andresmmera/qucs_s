/// @file substratelib.cpp
/// @brief Parse substrate.lib contents to be used in Qucs-S tools
/// @author Andrés Martínez Mera - andresmmera@protonmail.com
/// @date Sep 1, 2026
/// @copyright Copyright (C) 2019-2025 Andrés Martínez Mera
/// @license GPL-3.0-or-later

#include "substratelib.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QRegularExpression>
#include <QTextStream>

// Splits a Qucs value string such as "0.8 mm", "35 um" or "3.00" into
// its numeric part and an (optional) unit suffix. When the string has
// no unit suffix, *unit is left untouched so the caller's default is
// kept.
static void splitValueUnit(const QString &raw, double *value, QString *unit)
{
  static const QRegularExpression re(
      "^([+-]?[0-9]*\\.?[0-9]+(?:[eE][+-]?[0-9]+)?)\\s*([A-Za-z]*)$");

  const QString s = raw.trimmed();
  const QRegularExpressionMatch m = re.match(s);
  if (m.hasMatch()) {
    *value = m.captured(1).toDouble();
    const QString u = m.captured(2);
    if (!u.isEmpty())
      *unit = u;
  } else {
    *value = s.toDouble();
  }
}

QVector<SubstrateDef> loadSubstrateLibrary(const QString &fileName,
                                            QString *errorString)
{
  QVector<SubstrateDef> result;

  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    if (errorString)
      *errorString = QObject::tr("Cannot open substrate library file:\n%1")
                         .arg(fileName);
    return result;
  }

  QTextStream in(&file);
  const QString content = in.readAll();
  file.close();

  // Split the library into <Component ...> ... </Component> blocks.
  static const QRegularExpression componentRe(
      "<Component\\s+([^>]+)>(.*?)</Component>",
      QRegularExpression::DotMatchesEverythingOption);

  QRegularExpressionMatchIterator it = componentRe.globalMatch(content);
  while (it.hasNext()) {
    const QRegularExpressionMatch cm = it.next();
    const QString name = cm.captured(1).trimmed();
    const QString body = cm.captured(2);

    // Only interested in components whose model is a SUBST line; the
    // library file may in principle contain other component types.
    static const QRegularExpression substRe(
        "<SUBST\\b([^>]*)>", QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpressionMatch sm = substRe.match(body);
    if (!sm.hasMatch())
      continue;

    const QString substLine = sm.captured(1);

    // Pull every "value" state pair out of the SUBST definition, in
    // order. For a Qucs SUBST component this order is fixed:
    //   Er, H, T, Tand, Rho, D(=Rough)
    static const QRegularExpression valueRe("\"([^\"]*)\"\\s*\\d+");
    QRegularExpressionMatchIterator vit = valueRe.globalMatch(substLine);

    QStringList values;
    while (vit.hasNext())
      values << vit.next().captured(1);

    if (values.size() < 6)
      continue; // malformed / unexpected entry, skip it

    SubstrateDef def;
    def.name = name;

    static const QRegularExpression descRe(
        "<Description>(.*?)</Description>",
        QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpressionMatch dm = descRe.match(body);
    if (dm.hasMatch())
      def.description = dm.captured(1).trimmed();

    // Er, Tand, Rho and Rough are plain numbers (no unit suffix in the
    // library); H and T carry a length unit that overrides the default.
    double v;
    QString noUnit;
    splitValueUnit(values.at(0), &v, &noUnit);    def.er    = v;
    splitValueUnit(values.at(1), &v, &def.hUnit); def.h     = v;
    splitValueUnit(values.at(2), &v, &def.tUnit); def.t     = v;
    splitValueUnit(values.at(3), &v, &noUnit);    def.tand  = v;
    splitValueUnit(values.at(4), &v, &noUnit);    def.rho   = v;
    splitValueUnit(values.at(5), &v, &noUnit);    def.rough = v;

    result << def;
  }

  if (result.isEmpty() && errorString)
    *errorString =
        QObject::tr("No substrate definitions found in:\n%1").arg(fileName);

  return result;
}

QString findSubstrateLibraryFile()
{
  QStringList candidates;

  // 1) explicit path
  candidates << "/usr/local/share/qucs-s/library/Substrates.lib";

  // 2) relative to the running executable, e.g. Windows install
  const QDir appDir(QCoreApplication::applicationDirPath());
  candidates << appDir.filePath("library/Substrates.lib");
  candidates << appDir.filePath("../share/qucs-s/library/Substrates.lib");
  candidates << appDir.filePath("../lib/qucs-s/library/Substrates.lib");

  for (const QString &c : std::as_const(candidates)) {
    if (QFileInfo::exists(c))
      return QDir::cleanPath(c);
  }
  return QString();
}
