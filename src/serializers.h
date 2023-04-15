#pragma once

#include "VSRTL/external/cereal/include/cereal/cereal.hpp"

#include <QDataStream>
#include <QIODevice>
#include <QVariant>

template <class Archive>
void save(Archive &archive, const QVariant &v) {
  // Seems cumbersome but only easy way to serialize a QVariant in cereal
  QByteArray data;
  QDataStream ds(&data, QIODevice::WriteOnly);
  ds << v;
  std::vector<std::byte> dataVector(
      reinterpret_cast<std::byte *>(std::begin(data)),
      reinterpret_cast<std::byte *>(std::end(data)));
  archive(dataVector);
}

template <class Archive>
void load(Archive &archive, QVariant &v) {
  // Seems cumbersome but only easy way to serialize a QVariant in cereal
  std::vector<std::byte> dataVector;
  archive(dataVector);
  QByteArray data = QByteArray(
      reinterpret_cast<const char *>(dataVector.data()), dataVector.size());
  QDataStream ds(&data, QIODevice::ReadOnly);
  ds >> v;
}
