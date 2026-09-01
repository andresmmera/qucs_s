/// @file substratelib.h
/// @brief Parse substrate.lib contents to be used in Qucs-S tools
/// @author Andrés Martínez Mera - andresmmera@protonmail.com
/// @date Sep 1, 2026
/// @copyright Copyright (C) 2019-2025 Andrés Martínez Mera
/// @license GPL-3.0-or-later

#ifndef SUBSTRATELIB_H
#define SUBSTRATELIB_H

#include <QString>
#include <QVector>

/// One substrate ("SUBST" component) parsed out of a Qucs-S library file.
///
/// Field order mirrors the fixed property order Qucs uses for the SUBST
/// component: Er, H, T, Tand, Rho, D.
struct SubstrateDef {
  QString name;                 ///< Component name
  QString description;          ///< Humn-readable description

  double  er    = 1.0;          ///< Relative permittivity
  double  h     = 0.0;          ///< Substrate/dielectric height
  QString hUnit = "mm";
  double  t     = 0.0;          ///< Metallization thickness
  QString tUnit = "um";
  double  tand  = 0.0;          ///< Dielectric loss tangent
  double  rho   = 0.0;          ///< Metal resistivity, Ohm*m
  double  rough = 0.0;          ///< Surface roughness, m

  bool isValid() const { return !name.isEmpty(); }
};

/// @brief Parses a Qucs component-library file and returns every substrate
/// ("SUBST") component defined in it, in the order they appear in the file.
/// @param fileName    path of the *.lib file (e.g. Substrates.lib)
/// @param errorString if non-null, receives a human readable message
///                     when the file could not be opened or contained
///                     no substrate definitions.
QVector<SubstrateDef> loadSubstrateLibrary(const QString &fileName,
                                            QString *errorString = nullptr);

/// Tries to locate the "Substrates.lib" that ships with Qucs-S by
/// checking, in order:
///   1. $QUCS_S_LIBRARY_DIR/Substrates.lib (explicit override)
///   2. paths relative to the running executable (covers Windows
///      installs, AppImages, and "make install"-less local builds)
///   3. the well-known *nix install prefixes
///      (/usr/local/share/qucs-s/library, /usr/share/qucs-s/library)
///
/// @return the first existing candidate path, or an empty string if
///         none of them exist.
QString findSubstrateLibraryFile();

#endif // SUBSTRATELIB_H
