#pragma once

#include <QObject>

#include "ripesbranchpredictor.h"

#include "always_not_taken_predictor.h"
#include "always_taken_predictor.h"
#include "counter_predictor.h"
#include "global_predictor.h"
#include "local_predictor.h"

#include "../../ripessettings.h"

namespace Ripes {

/**
 * @brief The ProcessorHandler class
 * Manages construction and destruction of a VSRTL processor design, when
 * selecting between processors. Manages all interaction and control of the
 * current processor.
 */
class PredictorHandler : public QObject {

public:
  typedef uint64_t XLEN_T;
  typedef uint16_t ARRAY_T;

  PredictorHandler() {
    connect(RipesSettings::getObserver(RIPES_SETTING_REWINDSTACKSIZE),
            &SettingObserver::modified, this, [=](const auto &size) {
              PredictorHandler::getPredictor()->setReverseStackSize(
                  size.toUInt());
            });
  }

  // Returns a pointer to the PredictorHandler singleton.
  static PredictorHandler *get() {
    static auto *handler = new PredictorHandler;
    return handler;
  }

  static RipesBranchPredictor<XLEN_T, ARRAY_T> *getPredictor() {
    return get()->_getPredictor();
  }

  static void changePredictor(uint8_t predictor, uint16_t num_address_bits,
                              uint16_t num_history_bits,
                              uint16_t num_state_bits) {

    get()->_changePredictor(predictor, num_address_bits, num_history_bits,
                            num_state_bits);
  }

  static void clock() { return get()->_clock(); }
  static void reverse() { return get()->_reverse(); }

  std::unique_ptr<RipesBranchPredictor<XLEN_T, ARRAY_T>> current_predictor;

private:
  RipesBranchPredictor<XLEN_T, ARRAY_T> *_getPredictor() {
    return current_predictor.get();
  }

  void _changePredictor(uint8_t predictor, uint16_t num_address_bits,
                        uint16_t num_history_bits, uint16_t num_state_bits) {
    switch (predictor) {
    case 0:
      this->current_predictor.reset(new LocalPredictor<XLEN_T, ARRAY_T>(
          num_address_bits, num_history_bits, num_state_bits));
      break;
    case 1:
      this->current_predictor.reset(new GlobalPredictor<XLEN_T, ARRAY_T>(
          num_history_bits, num_state_bits));
      break;
    case 2:
      this->current_predictor.reset(
          new CounterPredictor<XLEN_T, ARRAY_T>(num_state_bits));
      break;
    case 3:
      this->current_predictor.reset(
          new AlwaysTakenPredictor<XLEN_T, ARRAY_T>());
      break;
    case 4:
      this->current_predictor.reset(
          new AlwaysNotTakenPredictor<XLEN_T, ARRAY_T>());
      break;
    }
  }

  void _clock() { PredictorHandler::getPredictor()->saveState(); }

  void _reverse() { PredictorHandler::getPredictor()->restoreState(); }
};

} // namespace Ripes
