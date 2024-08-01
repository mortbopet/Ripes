#include "programutilities.h"
#include "elfio/elfio.hpp"
#include "libelfin/dwarf/dwarf++.hh"
#include "statusmanager.h"

#include <QRegularExpression>

namespace Ripes {

QString loadFlatBinaryFile(Program &program, const QString &filepath,
                           unsigned long entryPoint, unsigned long loadAt) {
  QFile file(filepath);
  if (!file.open(QIODevice::ReadOnly)) {
    return "Error: Could not open file " + file.fileName();
  }
  ProgramSection section;
  section.name = TEXT_SECTION_NAME;
  section.address = loadAt;
  section.data = file.readAll();

  program.sections[TEXT_SECTION_NAME] = section;
  program.entryPoint = entryPoint;
  return QString();
}

using namespace ELFIO;
/**
 * @brief The ELFIODwarfLoader class provides
 * a loader implementation for Dwarf sections
 * using the ELFIO library.
 */
class ELFIODwarfLoader : public ::dwarf::loader {
public:
  ELFIODwarfLoader(elfio &reader) : reader(reader) {}

  const void *load(::dwarf::section_type section, size_t *size_out) override {
    auto sec = reader.sections[::dwarf::elf::section_type_to_name(section)];
    if (sec == nullptr)
      return nullptr;
    *size_out = sec->get_size();
    return sec->get_data();
  }

private:
  elfio &reader;
};

/**
 * @brief createDwarfLoader
 * Creates a Dwarf loader for the provided elfio reader.
 * @param reader The elfio reader to create the Dwarf loader for.
 * @return A shared pointer to the newly created Dwarf loader.
 */
static std::shared_ptr<ELFIODwarfLoader> createDwarfLoader(elfio &reader) {
  return std::make_shared<ELFIODwarfLoader>(reader);
}

/**
 * @brief isInternalSourceFile
 * Determines if the given filename is likely originated from within the Ripes
 * editor. These files are typically temporary files like /.../Ripes.abc123.c.
 *
 * @param filename A string containing the filename to check.
 * @return True if the filename is likely originated from within the Ripes
 * editor, otherwise false.
 */
static bool isInternalSourceFile(const QString &filename) {
  static QRegularExpression re("Ripes.[a-zA-Z0-9]+.c");
  return re.match(filename).hasMatch();
}

bool loadElfFile(Program &program, QFile &file) {
  ELFIO::elfio reader;

  // No file validity checking is performed - it is expected that Loaddialog has
  // done all validity checking.
  if (!reader.load(file.fileName().toStdString())) {
    assert(false);
  }

  for (const auto &elfSection : reader.sections) {
    // Do not load .debug sections
    if (!QString::fromStdString(elfSection->get_name()).startsWith(".debug")) {
      ProgramSection section;
      section.name = QString::fromStdString(elfSection->get_name());
      section.address = elfSection->get_address();
      // QByteArray performs a deep copy of the data when the data array is
      // initialized at construction
      section.data = QByteArray(elfSection->get_data(),
                                static_cast<int>(elfSection->get_size()));
      program.sections[section.name] = section;
    }

    if (elfSection->get_type() == SHT_SYMTAB) {
      // Collect function symbols
      const ELFIO::symbol_section_accessor symbols(reader, elfSection);
      for (unsigned int j = 0; j < symbols.get_symbols_num(); ++j) {
        std::string name;
        ELFIO::Elf64_Addr value = 0;
        ELFIO::Elf_Xword size;
        unsigned char bind;
        unsigned char type = STT_NOTYPE;
        ELFIO::Elf_Half section_index;
        unsigned char other;
        symbols.get_symbol(j, name, value, size, bind, type, section_index,
                           other);

        if (type != STT_FUNC)
          continue;
        program.symbols[value] = QString::fromStdString(name);
      }
    }
  }

  // Load DWARF information into the source mapping of the program.
  // We'll only load information from compilation units which originated from a
  // source file that plausibly arrived from within the Ripes editor.
  QString editorSrcFile;
  try {
    ::dwarf::dwarf dw(createDwarfLoader(reader));
    for (auto &cu : dw.compilation_units()) {
      for (auto &line : cu.get_line_table()) {
        if (!line.file)
          continue;
        QString filePath = QString::fromStdString(line.file->path);
        if (editorSrcFile.isEmpty()) {
          // Try to see if this compilation unit is from the Ripes editor:
          if (isInternalSourceFile(filePath))
            editorSrcFile = filePath;
        }
        if (editorSrcFile != filePath)
          continue;
        program.sourceMapping[line.address].insert(line.line - 1);
      }
    }
    if (!editorSrcFile.isEmpty()) {
      // Finally, we need to generate a hash of the source file that we've
      // loaded source mappings from, so the editor knows what editor contents
      // applies to this program.
      QFile srcFile(editorSrcFile);
      if (srcFile.open(QFile::ReadOnly))
        program.sourceHash = Program::calculateHash(srcFile.readAll());
      else
        throw ::dwarf::format_error("Could not find source file " +
                                    editorSrcFile.toStdString());
    }
  } catch (::dwarf::format_error &e) {
    std::string msg = "Could not load debug information: ";
    msg += e.what();
    GeneralStatusManager::setStatusTimed(QString::fromStdString(msg), 2500);
  } catch (...) {
    // Something else went wrong.
  }

  program.entryPoint = reader.get_entry();

  return true;
}

} // namespace Ripes
