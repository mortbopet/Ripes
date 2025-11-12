#include "pipelinediagrammodel.h"

#include "processorhandler.h"
#include "ripessettings.h"

#include <vector>

namespace Ripes {

static AInt indexToAddress(unsigned index) {
  if (auto spt = ProcessorHandler::getProgram()) {
    return (index * ProcessorHandler::currentISA()->instrBytes()) +
           spt->getSection(TEXT_SECTION_NAME)->address;
  }
  return 0;
}

PipelineDiagramModel::PipelineDiagramModel(QObject *parent)
    : QAbstractTableModel(parent) {
  connect(ProcessorHandler::get(), &ProcessorHandler::processorClocked, this,
          &PipelineDiagramModel::processorWasClocked, Qt::DirectConnection);
  connect(ProcessorHandler::get(), &ProcessorHandler::processorReset, this,
          &PipelineDiagramModel::reset);
}

QVariant PipelineDiagramModel::headerData(int section,
                                          Qt::Orientation orientation,
                                          int role) const {
  if (role != Qt::DisplayRole)
    return QVariant();
  if (orientation == Qt::Horizontal) {
    // Cycle number
    return QString::number(section);
  } else {
    const auto addr = indexToAddress(section);
    return ProcessorHandler::disassembleInstr(addr);
  }
}

int PipelineDiagramModel::rowCount(const QModelIndex &) const {
  return ProcessorHandler::getCurrentProgramSize() /
         ProcessorHandler::currentISA()->instrBytes();
}

int PipelineDiagramModel::columnCount(const QModelIndex &) const {
  return m_cycleStageInfos.size();
}

void PipelineDiagramModel::processorWasClocked() {
  const auto maxCycles =
      RipesSettings::value(RIPES_SETTING_PIPEDIAGRAM_MAXCYCLES).toInt();
  const auto cycleCount = ProcessorHandler::getProcessor()->getCycleCount();

  if (m_atMaxCycles) {
    return;
  }

  gatherStageInfo();

  if (cycleCount >= maxCycles) {
    m_atMaxCycles = true;
  }

  // Implement sliding window to prevent continuously growing memory usage
  // during long simulations. Only cleanup when significantly over the limit to
  // reduce performance impact.
  const size_t cleanupThreshold =
      static_cast<size_t>(maxCycles * 1.2); // 20% over limit
  if (m_cycleStageInfos.size() > cleanupThreshold) {
    // Notify model views before data changes
    beginResetModel();

    // Keep only the most recent maxCycles entries
    const size_t targetSize = static_cast<size_t>(maxCycles);
    auto it = m_cycleStageInfos.begin();
    std::advance(it, m_cycleStageInfos.size() - targetSize);
    m_cycleStageInfos.erase(m_cycleStageInfos.begin(), it);

    // Notify model views after data changes
    endResetModel();
  }
}

void PipelineDiagramModel::reset() {
  m_atMaxCycles = false;
  m_cycleStageInfos.clear();
  gatherStageInfo();
}

void PipelineDiagramModel::prepareForView() {
  beginResetModel();
  endResetModel();
}

void PipelineDiagramModel::gatherStageInfo() {
  long long cycleCount = ProcessorHandler::getProcessor()->getCycleCount();
  auto stageInfoForCycle = m_cycleStageInfos.find(cycleCount);
  if (stageInfoForCycle != m_cycleStageInfos.end()) {
    // Already gathered stage info for this cycle.
    return;
  }
  m_cycleStageInfos[cycleCount];
  for (auto idx : ProcessorHandler::getProcessor()->structure().stageIt())
    m_cycleStageInfos[cycleCount][idx] =
        ProcessorHandler::getProcessor()->stageInfo(idx);
}

QVariant PipelineDiagramModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid())
    return QVariant();

  if (role == Qt::TextAlignmentRole) {
    return Qt::AlignCenter;
  }

  if (role != Qt::DisplayRole)
    return QVariant();

  if (!m_cycleStageInfos.count(index.column()))
    return QVariant();

  const AInt addr = indexToAddress(index.row());
  const auto &stageInfo = m_cycleStageInfos.at(index.column());

  QStringList stagesForAddr;
  QString stageStr;
  for (const auto &si : stageInfo) {
    if (si.second.pc == addr && si.second.stage_valid &&
        si.second.state == StageInfo::State::None) {
      if (m_cycleStageInfos.count(index.column() - 1)) {
        const auto &prevCycleStageInfo =
            m_cycleStageInfos.at(index.column() - 1);
        if (prevCycleStageInfo.at(si.first).stage_valid &&
            prevCycleStageInfo.at(si.first).pc == si.second.pc) {
          stageStr = "-";
          if (!si.second.namedState.isEmpty()) {
            stageStr += " (" + si.second.namedState + ")";
          }
          stagesForAddr << stageStr;
          continue;
        }
      }
      stageStr = ProcessorHandler::getProcessor()->stageName(si.first);
      if (!si.second.namedState.isEmpty()) {
        stageStr += " (" + si.second.namedState + ")";
      }
      stagesForAddr << stageStr;
    }
  }

  if (stagesForAddr.size() == 0) {
    return QVariant();
  }

  return stagesForAddr.join('/');
}

QString PipelineDiagramModel::toString() const {
  QString textualRepr;

  // Copy headers
  textualRepr.append('\t');
  for (int j = 0; j < columnCount(); j++) {
    textualRepr.append(headerData(j, Qt::Horizontal).toString());
    textualRepr.append('\t');
  }
  textualRepr.append('\n');
  // Copy data
  for (int i = 0; i < rowCount(); ++i) {
    textualRepr.append(headerData(i, Qt::Vertical).toString());
    textualRepr.append('\t');
    for (int j = 0; j < columnCount(); j++) {
      textualRepr.append(data(index(i, j)).toString());
      textualRepr.append('\t');
    }
    textualRepr.append('\n');
  }
  return textualRepr;
}

} // namespace Ripes
