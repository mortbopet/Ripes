#pragma once

#include <QDir>
#include <QFile>
#include <QObject>
#include <QProcess>
#include <QString>

QT_FORWARD_DECLARE_CLASS(QTextDocument)

#include <memory>

namespace Ripes {

/**
 * @brief The CCManager class
 * Manages the detection, verification and execution of a valid C/C++ compiler
 * suitable for the ISAs targetted by the various processor models of Ripes.
 */
class CCManager : public QObject {
  Q_OBJECT
public:
  struct CompileCommand {
    QFileInfo bin;
    QStringList args;

    QString toString() const {
      return (QStringList(bin.absoluteFilePath()) + args).join(" ");
    }
  };
  struct CompileError {
    QString errMsg;
    QString _stdout;
    QString _stderr;
    QString toString(const CompileCommand &cc) const;
  };
  struct CCRes {
    QStringList inFiles;
    QString outFile;
    CompileError errorOutput;
    CompileCommand cc;
    bool success = false;
    bool aborted = false;

    void clean() { QFile::remove(outFile); }
  };

  static CCManager &get() {
    static CCManager manager;
    return manager;
  }

  static bool hasValidCC();
  static const QString &currentCC() { return get().m_currentCC; }

  static QString getError();

  /**
   * @brief compile
   * Runs the current compiler, with the current set of arguments, on file @p
   * filename. @returns information about the compiled file, such as source
   * file, output file and compilation status. If no @p outname has been
   * provided, the output file will be placed in a temporary directory.
   */
  CCRes compile(const QStringList &files, QString outname = QString(),
                bool showProgressdiag = true);
  CCRes compile(const QTextDocument *source, QString outname = QString(),
                bool showProgressdiag = true);
  CCRes compileRaw(const QString &rawsource, QString outname = QString(),
                   bool showProgressdiag = true);

  CompileCommand createCompileCommand(const QStringList &files,
                                      const QString &outname) const;

signals:
  /**
   * @brief ccChanged
   * Emitted whenever the current CC path changed. @param res contains
   * information regarding the compiler validation.
   */
  void ccChanged(CCManager::CCRes res);

public slots:
  /**
   * @brief trySetCC
   * Attempts to set the path @param CC to be the current compiler.
   * @return true if the CC was successfully validated and set
   */
  bool trySetCC(const QString &CC);

private:
  /**
   * @brief tryAutodetectCC
   * Will attempt to scan the current PATH to locate a valid compiler
   */
  QString tryAutodetectCC();

  /**
   * @brief verifyCC
   * Attempts to compile a simple test program using the provided compiler path
   * @param CC.
   * @returns true if successful.
   */
  CCRes verifyCC(const QString &CC);

  CCManager();
  QString m_currentCC;
#ifdef RIPES_WITH_QPROCESS
  QProcess m_process;
#endif
  bool m_errored = false;
  bool m_aborted = false;
  std::unique_ptr<QFile> m_tmpSrcFile;
};

} // namespace Ripes
