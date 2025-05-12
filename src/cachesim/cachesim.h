#pragma once

#include <map>
#include <math.h>
#include <vector>

#include <QDataStream>
#include <QObject>

#include "VSRTL/core/vsrtl_register.h"
#include "processors/RISC-V/rv_memory.h"
#include "processors/interface/ripesprocessor.h"

namespace Ripes {
class CacheSim;

enum WriteAllocPolicy { WriteAllocate, NoWriteAllocate };
enum WritePolicy { WriteThrough, WriteBack };
enum ReplPolicy { Random, LRU };

struct CachePreset {
  QString name;
  int blocks;
  int lines;
  int ways;

  WritePolicy wrPolicy;
  WriteAllocPolicy wrAllocPolicy;
  ReplPolicy replPolicy;

  friend QDataStream &operator<<(QDataStream &arch, const CachePreset &object) {
    arch << object.name;
    arch << object.blocks;
    arch << object.lines;
    arch << object.ways;
    arch << object.wrPolicy;
    arch << object.wrAllocPolicy;
    arch << object.replPolicy;
    return arch;
  }

  friend QDataStream &operator>>(QDataStream &arch, CachePreset &object) {
    arch >> object.name;
    arch >> object.blocks;
    arch >> object.lines;
    arch >> object.ways;
    arch >> object.wrPolicy;
    arch >> object.wrAllocPolicy;
    arch >> object.replPolicy;
    return arch;
  }

  bool operator==(const CachePreset &other) const {
    return this->name == other.name;
  }
};

class CacheInterface : public QObject {
  Q_OBJECT
public:
  CacheInterface(QObject *parent) : QObject(parent) {}
  virtual ~CacheInterface() {}

  /**
   * @brief access
   * A function called by the logical "child" of this cache, indicating that it
   * desires to access this cache
   */
  virtual void access(AInt address, MemoryAccess::Type type) = 0;
  void setNextLevelCache(const std::shared_ptr<CacheSim> &cache) {
    m_nextLevelCache = cache;
  }

  /**
   * @brief reset
   * Called by the logical child of this cache to  propagating cache resetting
   * or reversing up the cache hierarchy.
   */
  virtual void reset();
  virtual void reverse();

protected:
  /**
   * @brief m_nextLevelCache
   * Pointer to the next level (logical parent) cache.
   */
  std::shared_ptr<CacheSim> m_nextLevelCache;
};

class CacheSim : public CacheInterface {
  Q_OBJECT
public:
  static constexpr unsigned s_invalidIndex = static_cast<unsigned>(-1);

  struct CacheSize {
    unsigned bits = 0;
    std::vector<QString> components;
  };

  struct CacheWay {
    VInt tag = -1;
    std::set<unsigned> dirtyBlocks;
    bool dirty = false;
    bool valid = false;

    // LRU algorithm relies on invalid cache ways to have an initial high value.
    // -1 ensures maximum value for all way sizes.
    unsigned lru = -1;
  };

  struct CacheIndex {
    unsigned line = s_invalidIndex;
    unsigned way = s_invalidIndex;
    unsigned block = s_invalidIndex;
    void assertValid() const {
      Q_ASSERT(line != s_invalidIndex && "Cache line index is invalid");
      Q_ASSERT(way != s_invalidIndex && "Cache way index is invalid");
      Q_ASSERT(block != s_invalidIndex && "Cache word index is invalid");
    }

    bool operator==(const CacheIndex &other) const {
      return this->line == other.line && this->way == other.way &&
             this->block == other.block;
    }
  };

  struct CacheTransaction {
    AInt address;
    CacheIndex index;

    bool isHit = false;
    bool isWriteback = false; // True if the transaction resulted in an eviction
                              // of a dirty cacheline
    MemoryAccess::Type type = MemoryAccess::None;
    bool transToValid =
        false; // True if the cacheline just transitioned from invalid to valid
    bool tagChanged =
        false; // True if transToValid or the previous entry was evicted
  };

  struct CacheAccessTrace {
    int hits = 0;
    int misses = 0;
    int reads = 0;
    int writes = 0;
    int writebacks = 0;
    CacheTransaction lastTransaction;
    CacheAccessTrace() {}
    CacheAccessTrace(const CacheTransaction &transaction)
        : CacheAccessTrace(CacheAccessTrace(), transaction) {}
    CacheAccessTrace(const CacheAccessTrace &pre,
                     const CacheTransaction &transaction) {
      lastTransaction = transaction;
      reads = pre.reads + (transaction.type == MemoryAccess::Read ? 1 : 0);
      writes = pre.writes + (transaction.type == MemoryAccess::Write ? 1 : 0);
      writebacks = pre.writebacks + (transaction.isWriteback ? 1 : 0);
      hits = pre.hits + (transaction.isHit ? 1 : 0);
      misses = pre.misses + (transaction.isHit ? 0 : 1);
    }
  };

  using CacheLine = std::map<unsigned, CacheWay>;

  CacheSim(QObject *parent);
  void setWritePolicy(WritePolicy policy);
  void setWriteAllocatePolicy(WriteAllocPolicy policy);
  void setReplacementPolicy(ReplPolicy policy);

  void access(AInt address, MemoryAccess::Type type) override;
  void undo();
  void reset() override;

  WriteAllocPolicy getWriteAllocPolicy() const { return m_wrAllocPolicy; }
  ReplPolicy getReplacementPolicy() const { return m_replPolicy; }
  WritePolicy getWritePolicy() const { return m_wrPolicy; }

  const std::map<unsigned, CacheAccessTrace> &getAccessTrace() const {
    return m_accessTrace;
  }

  double getHitRate() const;
  unsigned getHits() const;
  unsigned getMisses() const;
  unsigned getWritebacks() const;
  CacheSize getCacheSize() const;

  AInt buildAddress(unsigned tag, unsigned lineIdx, unsigned blockIdx) const;

  int getBlockBits() const { return m_blocks; }
  int getLineBits() const { return m_lines; }
  int getTagBits() const {
    return 32 - 2 /*byte offset*/ - getBlockBits() - getLineBits();
  }

  int getBlocks() const { return static_cast<int>(std::pow(2, m_blocks)); }
  int getWays() const { return m_ways; }
  int getLines() const { return static_cast<int>(std::pow(2, m_lines)); }
  unsigned getBlockMask() const { return m_blockMask; }
  unsigned getTagMask() const { return m_tagMask; }
  unsigned getLineMask() const { return m_lineMask; }

  unsigned getLineIdx(const AInt address) const;
  unsigned getBlockIdx(const AInt address) const;
  unsigned getTag(const AInt address) const;

  const CacheLine *getLine(unsigned idx) const;

public slots:
  void setBlocks(unsigned blocks);
  void setLines(unsigned lines);
  void setWays(unsigned ways);
  void setPreset(const Ripes::CachePreset &preset);

  /**
   * @brief reverse
   * Slot functions for Reversed signals emitted by the currently attached
   * processor.
   */
  void reverse() override;

signals:
  void configurationChanged();
  void dataChanged(CacheSim::CacheTransaction transaction);
  void hitrateChanged();

  // Signals that the entire cache line @p
  /**
   * @brief wayInvalidated
   * Signals that all ways in the cacheline @param lineIdx which contains way
   * @param wayIdx should be invalidated in the graphical view.
   */
  void wayInvalidated(unsigned lineIdx, unsigned wayIdx);

  /**
   * @brief cacheInvalidated
   * Signals that all cachelines in the cache should be invalidated in the
   * graphical view
   */
  void cacheInvalidated();

private:
  struct CacheTrace {
    CacheTransaction transaction;
    CacheWay oldWay;
  };

  std::pair<unsigned, CacheSim::CacheWay *>
  locateEvictionWay(const CacheTransaction &transaction);
  CacheWay evictAndUpdate(CacheTransaction &transaction);
  void analyzeCacheAccess(CacheTransaction &transaction) const;
  void pushAccessTrace(const CacheTransaction &transaction);
  void popAccessTrace();

  /**
   * @brief updateConfiguration
   * Called whenever one of the cache parameters changes. Emits signal
   * configurationChanged after updating.
   */
  void updateConfiguration();
  void recalculateMasks();

  /**
   * @brief reassociateMemory
   * Binds to a memory component exposed by the processor handler, based on the
   * current cache type.
   */
  void reassociateMemory();

  ReplPolicy m_replPolicy = ReplPolicy::LRU;
  WritePolicy m_wrPolicy = WritePolicy::WriteBack;
  WriteAllocPolicy m_wrAllocPolicy = WriteAllocPolicy::WriteAllocate;

  unsigned m_blockMask = -1;
  unsigned m_lineMask = -1;
  unsigned m_tagMask = -1;

  int m_blocks = 2; // Some power of 2
  int m_lines = 5;  // Some power of 2
  int m_ways = 1;
  unsigned m_byteOffset = -1; // # of bits to represent the # of bytes in a word
  unsigned m_wordBits = -1;

  /**
   * @brief m_cacheLines
   * The datastructure for storing our cache hierachy, as per the current cache
   * configuration.
   */
  std::map<unsigned, CacheLine> m_cacheLines;

  void updateCacheLineReplFields(CacheLine &line, unsigned wayIdx);
  /**
   * @brief revertCacheLineReplFields
   * Called whenever undoing a transaction to the cache. Reverts a cacheline's
   * replacement fields according to the configured replacement policy.
   */
  void revertCacheLineReplFields(CacheLine &line, const CacheWay &oldWay,
                                 unsigned wayIdx);

  /**
   * @brief m_accessTrace
   * The access trace stack contains cache access statistics for each simulation
   * cycle. Contrary to the TraceStack (m_traceStack).
   */
  std::map<unsigned, CacheAccessTrace> m_accessTrace;

  /**
   * @brief m_traceStack
   * The following information is used to track all most-recent modifications
   * made to the stack. The stack is of a fixed sized which is equal to the undo
   * stack of VSRTL memory elements. Storing all modifications allows us to
   * rollback any changes performed to the cache, when clock cycles are undone.
   */
  std::deque<CacheTrace> m_traceStack;

  /**
   * @brief m_isResetting
   * The cacheSim can be reset by either internally modyfing cache configuration
   * parameters or externally through a processor reset. Given that modifying
   * the cache parameters itself will prompt a reset of the processor, we need a
   * way to distinquish whether a processor reset request originated from an
   * internal cache configuration change. If so, we do not emit a processor
   * request signal, avoiding a signalling loop.
   */
  bool m_isResetting = false;

  CacheTrace popTrace();
  void pushTrace(const CacheTrace &trace);
};

const static std::map<ReplPolicy, QString> s_cacheReplPolicyStrings{
    {ReplPolicy::Random, "Random"}, {ReplPolicy::LRU, "LRU"}};
const static std::map<WriteAllocPolicy, QString> s_cacheWriteAllocateStrings{
    {WriteAllocPolicy::WriteAllocate, "Write allocate"},
    {WriteAllocPolicy::NoWriteAllocate, "No write allocate"}};

const static std::map<WritePolicy, QString> s_cacheWritePolicyStrings{
    {WritePolicy::WriteThrough, "Write-through"},
    {WritePolicy::WriteBack, "Write-back"}};

} // namespace Ripes

Q_DECLARE_METATYPE(Ripes::CacheSim::CacheTransaction);
