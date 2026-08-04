/***************************************************************************
                              imagepainting.h
                             ---------------
    copyright            : (C) 2025 by Andrés Martínez Mera
    email                : andresmartinezmera@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef IMAGEPAINTING_H
#define IMAGEPAINTING_H

#include "rectangle.h"
#include <QColor>
#include <QPen>
#include <QPixmap>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QDebug>
#include <QPainter>
#include <QObject>
#include <QComboBox>
#include <QCheckBox>
#include <QApplication>
#include <QSvgRenderer>

class ImagePainting : public QObject, public qucs::Rectangle {
  Q_OBJECT
public:
  ImagePainting();
  Painting* newOne() override;
  void paint(QPainter* painter) override;
  bool load(const QString& s) override;
  QString save() override;
  QString saveCpp() override;
  QString saveJSON() override;
  bool Dialog(QWidget* parent = nullptr) override;
  static Element* info(QString& Name, char* &BitmapFile, bool getNewOne = false);

  void setImageFromPixmap(const QPixmap& pixmap);
  void setImageFromPath(const QString& path);
  void setImageFromClipboard();

  // Override selection and interaction methods
  bool getSelected(const QPoint& click, int tolerance) override;
  bool resizeTouched(const QPoint& click, int tolerance) override;
  void MouseMoving(const QPoint& onGrid, Schematic* sch, const QPoint& cursor) override;
  bool MousePressing(Schematic* sch = nullptr) override;
  void MouseResizeMoving(int x, int y, Schematic* p) override;
  void ResetDragTracking();
  bool rotate() noexcept override;
  bool rotate(int xc, int yc) noexcept override;

  int getImageWidth() const;
  int getImageHeight() const;

private:
  QString imagePath;
  QPixmap image;
  QPixmap originalImage;

  /// SVG @{
  // Raw bytes of the embedded asset exactly as read from disk/base64.
  // The image can't be stored as a QPixmap because for SVG the XML is needed
  QByteArray m_rawData;

  // True when m_rawData holds SVG/XML content and should be rendered
  // vectorially via m_svgRenderer instead of rasterized via QPixmap.
  bool m_isSvg = false;
  std::unique_ptr<QSvgRenderer> m_svgRenderer;

  /// @}

  void loadImage();

  /// @brief Load image data
  /// @param data Image data, either XML SVG data or QPixmap
  /// @return false if something went wrong, true if it loaded ok
  /// @details Shared decode path used by both load() (base64 from a .sch file) and
  /// loadImage() (reading an external file referenced by imagePath).
  /// This avoids duplicating the SVG-vs-raster branch logic.
  bool loadFromRawData(const QByteArray& data);

  /// @brief Detects if data is XML (SVG) or raw image data
  bool detectSvg(const QByteArray& data) const;

  enum DraggedCorner { TopLeft, TopRight, BottomLeft, BottomRight, NotSet };
  DraggedCorner m_draggedCorner = NotSet;
  int m_lastDragX = -1;
  int m_lastDragY = -1;

  // Local pen properties
  QColor penColor;
  int penWidth;
  Qt::PenStyle penStyle;
  bool m_filled;

  // Aspect ratio control
  bool m_keepAspectRatio;
  double m_aspectRatio; // cached aspect ratio

  // Dialog widget members
  QLineEdit* m_pathEdit;
  QLineEdit* m_widthEdit;
  QLineEdit* m_heightEdit;
  QCheckBox* m_aspectRatioCheck;
  QPushButton* m_resetButton;
  QLabel* m_statusLabel;

  // Dialog handler methods
  void onBrowseClicked();
  void onResetClicked();
  void onAspectRatioToggled(bool checked);
  void onPathChanged(const QString& newPath);
  void updateHeight();

  // Helper methods
  void updateAspectRatio();
  void applyAspectRatioToResize(int& newWidth, int& newHeight);
};

#endif // IMAGEPAINTING_H
