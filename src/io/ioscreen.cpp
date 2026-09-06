#include "ioscreen.h"

#include <QPainter>

#include "ioregistry.h"

namespace Ripes {

IOScreen::IOScreen(QWidget *parent) : IOBase(IOType::SCREEN, parent) {
  constexpr unsigned defaultWidth = 64;
  constexpr unsigned defaultHeight = 48;
  constexpr unsigned maxSideWidth = 1024;

  m_parameters[WIDTH] =
      IOParam(WIDTH, "Width", defaultWidth, true, 1, maxSideWidth);
  m_parameters[HEIGHT] =
      IOParam(HEIGHT, "Height", defaultHeight, true, 1, maxSideWidth);

  updateScreen();
}

unsigned IOScreen::pixelCount() const {
  const unsigned width = m_parameters.at(WIDTH).value.toUInt();
  const unsigned height = m_parameters.at(HEIGHT).value.toUInt();
  return width * height;
}

AInt IOScreen::presentOffset() const { return pixelCount() * 4; }

unsigned IOScreen::byteSize() const {
  // Framebuffer (one word per pixel) + the PRESENT doorbell register.
  return (pixelCount() + 1) * 4;
}

QString IOScreen::description() const {
  QStringList desc;
  desc << "A double-buffered raster screen. Each pixel is a 24-bit RGB value "
          "(0xRRGGBB, blue in the least significant byte).";
  desc << "";
  desc << "The framebuffer (the back buffer) starts at the peripheral base "
          "address. The byte offset of pixel (x, y) is:";
  desc << "    offset = (y*WIDTH + x) * 4";
  desc << "";
  desc << "Writing pixels does not update the display. Write any value to the "
          "PRESENT register to scan the back buffer onto the screen and flip "
          "to the other buffer for the next frame.";
  return desc.join('\n');
}

VInt IOScreen::ioRead(AInt offset, unsigned size) {
  if (offset >= presentOffset())
    return m_backIdx;
  const auto &buffer = m_buffers[m_backIdx];
  const size_t idx = offset / 4;
  if (idx >= buffer.size())
    return 0;
  return (buffer[idx] >> ((offset % 4) * 8)) & vsrtl::generateBitmask(size * 8);
}

void IOScreen::ioWrite(AInt offset, VInt value, unsigned) {
  if (offset >= presentOffset()) {
    // Doorbell: present the current back buffer.
    present();
    return;
  }
  auto &buffer = m_buffers[m_backIdx];
  const size_t idx = offset / 4;
  if (idx < buffer.size())
    buffer[idx] = static_cast<uint32_t>(value);
  // Note: intentionally no repaint here; the display is only updated on
  // PRESENT.
}

void IOScreen::present() {
  const unsigned width = m_parameters.at(WIDTH).value.toUInt();
  const unsigned height = m_parameters.at(HEIGHT).value.toUInt();
  const auto &buffer = m_buffers[m_backIdx];

  QImage image(width, height, QImage::Format_RGB32);
  for (unsigned y = 0; y < height; y++) {
    auto *scanline = reinterpret_cast<QRgb *>(image.scanLine(y));
    const unsigned rowBase = y * width;
    for (unsigned x = 0; x < width; x++) {
      // Force the alpha byte so the RGB32 image is fully opaque.
      scanline[x] = 0xFF000000u | (buffer[rowBase + x] & 0x00FFFFFFu);
    }
  }

  {
    QMutexLocker locker(&m_imageMutex);
    m_image = image;
  }

  // Flip buffers: the buffer we just presented becomes the front (displayed)
  // buffer, and software now draws into the other one.
  m_backIdx ^= 1;

  emit scheduleUpdate();
}

void IOScreen::reset() {
  for (auto &buffer : m_buffers)
    std::fill(buffer.begin(), buffer.end(), 0);
  m_backIdx = 0;
  {
    QMutexLocker locker(&m_imageMutex);
    m_image.fill(Qt::black);
  }
  emit scheduleUpdate();
}

void IOScreen::updateScreen() {
  const unsigned width = m_parameters.at(WIDTH).value.toUInt();
  const unsigned height = m_parameters.at(HEIGHT).value.toUInt();
  const unsigned nPixels = width * height;

  for (auto &buffer : m_buffers)
    buffer.assign(nPixels, 0);
  m_backIdx = 0;

  {
    QMutexLocker locker(&m_imageMutex);
    m_image = QImage(width, height, QImage::Format_RGB32);
    m_image.fill(Qt::black);
  }

  m_extraSymbols.clear();
  m_extraSymbols.push_back(IOSymbol{"WIDTH", width});
  m_extraSymbols.push_back(IOSymbol{"HEIGHT", height});

  // Rather than enumerating a register per pixel (which would be huge for a
  // real screen), expose the framebuffer implicitly via the base address and
  // export a single PRESENT doorbell register.
  m_regDescs.clear();
  m_regDescs.push_back(RegDesc{"PRESENT", RegDesc::RW::W, 32,
                               static_cast<AInt>(nPixels) * 4,
                               /*exported=*/true});

  updateGeometry();
  emit regMapChanged();
}

QSize IOScreen::minimumSizeHint() const {
  const int width = m_parameters.at(WIDTH).value.toInt();
  const int height = m_parameters.at(HEIGHT).value.toInt();

  // Scale small screens up so they are comfortably visible, while keeping large
  // screens within a reasonable initial bound. The image is scaled to fill the
  // widget in paintEvent, so this only affects the initial/minimum size.
  const int maxSide = std::max(width, height);
  const int scale = std::max(1, 256 / std::max(1, maxSide));
  return QSize(width * scale, height * scale);
}

void IOScreen::paintEvent(QPaintEvent *) {
  QImage image;
  {
    QMutexLocker locker(&m_imageMutex);
    image = m_image; // Cheap: QImage is implicitly shared (copy-on-write).
  }
  if (image.isNull())
    return;

  QPainter painter(this);
  // Nearest-neighbor scaling keeps pixels crisp and is fast.
  const QImage scaled =
      image.scaled(size(), Qt::KeepAspectRatio, Qt::FastTransformation);
  // Center the scaled image within the widget.
  const int x = (width() - scaled.width()) / 2;
  const int y = (height() - scaled.height()) / 2;
  painter.drawImage(x, y, scaled);
}

} // namespace Ripes
