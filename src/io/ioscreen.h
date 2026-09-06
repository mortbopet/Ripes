#pragma once

#include <QImage>
#include <QMutex>
#include <QVariant>
#include <QWidget>

#include <array>
#include <cstdint>
#include <vector>

#include "iobase.h"

namespace Ripes {

/**
 * @brief The IOScreen class
 * A raster screen peripheral.
 *
 *  - Rendering is done by blitting a single QImage (scaled to the widget),
 *    rather than drawing every pixel as an individual shape.
 *  - It is double buffered. Software always draws into the "back" buffer,
 *    mapped at the peripheral's base address. Pixel writes do NOT trigger a
 *    repaint. Only when software writes the PRESENT (doorbell) register is the
 *    back buffer scanned into an image, displayed, and the back/front buffers
 *    swapped. This avoids per-pixel repaint overhead and tearing.
 *
 * Memory map (offsets from the peripheral base):
 *  - [0, width*height*4)          : framebuffer, one 24-bit RGB word per pixel
 *                                   (0xRRGGBB; blue in the least significant
 *                                   byte). Pixel (x, y) is at
 *                                   (y*width + x) * 4.
 *  - PRESENT (= width*height*4)   : doorbell. Writing any value presents the
 *                                   current back buffer and flips buffers.
 */
class IOScreen : public IOBase {
  Q_OBJECT

  enum Parameters { WIDTH, HEIGHT };

public:
  IOScreen(QWidget *parent);
  ~IOScreen() { unregister(); }

  virtual unsigned byteSize() const override;
  virtual QString description() const override;
  virtual QString baseName() const override { return "Screen"; }

  virtual const std::vector<RegDesc> &registers() const override {
    return m_regDescs;
  }
  virtual const std::vector<IOSymbol> *extraSymbols() const override {
    return &m_extraSymbols;
  }

  /**
   * Hardware read/write functions
   */
  virtual VInt ioRead(AInt offset, unsigned size) override;
  virtual void ioWrite(AInt offset, VInt value, unsigned size) override;

  virtual void reset() override;

protected:
  virtual void parameterChanged(unsigned) override { updateScreen(); }

  /**
   * QWidget drawing
   */
  void paintEvent(QPaintEvent *event) override;
  QSize minimumSizeHint() const override;

private:
  void updateScreen();

  /// Number of framebuffer pixels (width*height).
  unsigned pixelCount() const;
  /// Byte offset of the PRESENT doorbell register.
  AInt presentOffset() const;

  /// Scans the current back buffer into m_image and flips the buffers.
  void present();

  // Double-buffered framebuffers. Software draws into m_buffers[m_backIdx];
  // m_image displays the most recently presented buffer.
  std::array<std::vector<uint32_t>, 2> m_buffers;
  unsigned m_backIdx = 0;

  // Presented image. Guarded by m_imageMutex since it is produced on the
  // processor thread (during a run) and consumed by paintEvent on the GUI
  // thread.
  QImage m_image;
  QMutex m_imageMutex;

  std::vector<RegDesc> m_regDescs;
  std::vector<IOSymbol> m_extraSymbols;
};

} // namespace Ripes
