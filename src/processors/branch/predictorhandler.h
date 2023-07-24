#pragma once

#include <QObject>

#include "ripesbranchpredictor.h"

#include "predictors/always_not_taken_predictor.h"
#include "predictors/always_taken_predictor.h"
#include "predictors/counter_predictor.h"
#include "predictors/global_predictor.h"
#include "predictors/local_predictor.h"

#include "../../ripessettings.h"

namespace Ripes {

/**
 * @brief The ProcessorHandler class
 * Manages the branch predictor and provides an ISA and processor agnostic
 * interface.
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

private:
  RipesBranchPredictor<XLEN_T, ARRAY_T> *_getPredictor() {
    return this->current_predictor.get();
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

  void _clock() { getPredictor()->saveState(); }

  void _reverse() { getPredictor()->restoreState(); }

  std::unique_ptr<RipesBranchPredictor<XLEN_T, ARRAY_T>> current_predictor;
};

} // namespace Ripes
