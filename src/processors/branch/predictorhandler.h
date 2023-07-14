#pragma once

#include <QObject>

#include "ripesbranchpredictor.h"

#include "always_not_taken_predictor.h"
#include "always_taken_predictor.h"
#include "counter_predictor.h"
#include "global_predictor.h"
#include "local_predictor.h"

namespace Ripes {

/**
 * @brief The ProcessorHandler class
 * Manages construction and destruction of a VSRTL processor design, when
 * selecting between processors. Manages all interaction and control of the
 * current processor.
 */
class PredictorHandler : public QObject {
  Q_OBJECT

public:
  /// Returns a pointer to the PredictorHandler singleton.
  static PredictorHandler *get() {
    static auto *handler = new PredictorHandler;
    return handler;
  }
};

} // namespace Ripes
