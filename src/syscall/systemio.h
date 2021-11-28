#pragma once

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QInputDialog>
#include <QMutex>
#include <QObject>
#include <QTemporaryFile>
#include <QTextStream>
#include <QWaitCondition>

#include <sys/stat.h>
#include <stdexcept>

#include "STLExtras.h"
#include "statusmanager.h"

namespace Ripes {

/*
Copyright (c) 2003-2013,  Pete Sanderson and Kenneth Vollmar

Developed by Pete Sanderson (psanderson@otterbein.edu)
and Kenneth Vollmar (kenvollmar@missouristate.edu)

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject
to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

(MIT license, http://www.opensource.org/licenses/mit-license.html)
*/

/**
 * @brief The SystemIO class
 * Provides standard i/o services needed to simulate the RISCV syscall
 * routines.
 * This class is largely based on the SystemIO.java class of RARS.
 */

class SystemIO : public QObject {
    Q_OBJECT
public:
    static SystemIO& get() {
        static SystemIO sio;
        return sio;
    }

private:
    // String used for description of file error
    static QString s_fileErrorString;  // = ("File operation OK");

    // Flag used for aborting waiting for I/O
    static bool s_abortSyscall;

    // Standard I/O Channels
    enum STDIO { STDIN = 0, STDOUT = 1, STDERR = 2, STDIO_END };

    // Buffer size for syscalls for file I/O
    static constexpr int SYSCALL_BUFSIZE = 128;
    // Maximum number of files that can be open
    static constexpr int SYSCALL_MAXFILES = 32;

    static constexpr int O_RDONLY = 0x00000000;
    static constexpr int O_WRONLY = 0x00000001;
    static constexpr int O_RDWR = 0x00000002;
    static constexpr int O_APPEND = 0x00000008;
    static constexpr int O_CREAT = 0x00000200;  // 512
    static constexpr int O_TRUNC = 0x00000400;  // 1024
    static constexpr int O_EXCL = 0x00000800;   // 2048

    // //////////////////////////////////////////////////////////////////////////////
    // Maintain information on files in use. The index to the arrays is the "file descriptor."

    struct FileIOData {
        // The filenames in use. Null if file descriptor i is not in use.
        static std::map<int, QString> fileNames;
        // The flags of this file, 0=READ, 1=WRITE. Invalid if this file descriptor is not in use.
        static std::map<int, unsigned> fileFlags;
        // The streams in use, associated with the filenames
        static std::map<int, QTextStream> streams;
        // The file pointers in use
        static std::map<int, QFile> files;
        // QByteArray to use as a stdin buffer
        static QByteArray s_stdinBuffer;

        /**
         * @brief s_stdioMutex
         * Used for implementing the waitCondition between the producer/consumer scenario where ecall handling is
         * blocking while waiting for the stdinBuffer to be non-empty.
         */
        static QMutex s_stdioMutex;
        static QWaitCondition s_stdinBufferEmpty;

        // Reset all file information. Closes any open files and resets the arrays
        static void resetFiles() {
            for (int i = 0; i < SYSCALL_MAXFILES; ++i) {
                close(i);
            }
            setupStdio();
        }

        static void setupStdio() {
            fileNames[STDIN] = "STDIN";
            fileNames[STDOUT] = "STDOUT";
            fileNames[STDERR] = "STDERR";
            fileFlags[STDIN] = SystemIO::O_RDONLY;
            fileFlags[STDOUT] = SystemIO::O_WRONLY;
            fileFlags[STDERR] = SystemIO::O_WRONLY;

            if (streams.count(STDIN) == 0) {
                // stdin stream has not yet been created
                streams.emplace(STDIN, &s_stdinBuffer);
            } else {
                // Clear stdin stream and reset stream
                s_stdinBuffer.clear();
                auto success = streams[STDIN].seek(0);
                Q_ASSERT(success);
            }

            // stdout/stderr will be handled via. signal/slots internally in the application
            // streams.emplace(STDOUT, stdout);
            // streams.emplace(STDERR, stderr);
        }

        // Open a file stream assigned to the given file descriptor
        static void openFilestream(int fd, const QString& filename) {
            files.emplace(fd, filename);

            const auto flags = fileFlags[fd];
            const auto qtOpenFlags =  // Translate from stdlib file flags to Qt flags
                (flags & O_RDONLY ? QIODevice::ReadOnly : QIODevice::NotOpen) |
                (flags & O_WRONLY ? QIODevice::WriteOnly : QIODevice::NotOpen) |
                (flags & O_RDWR ? QIODevice::ReadWrite : QIODevice::NotOpen) |
                (flags & O_TRUNC ? QIODevice::Truncate : QIODevice::Append) |
                (flags & O_EXCL ? QIODevice::NewOnly : QIODevice::NotOpen);

            // Try to open file with the given flags
            files[fd].open(qtOpenFlags);

            if (!files[fd].exists() && flags & O_CREAT) {
                files.erase(fd);
                throw std::runtime_error("Could not create file");
            }

            if (!files[fd].exists()) {
                files.erase(fd);
                throw std::runtime_error("File not found");
            }

            if (!files[fd].isOpen()) {
                files.erase(fd);
                throw std::runtime_error("File could not be opened");
            }

            streams.emplace(fd, &files[fd]);
        }

        // Retrieve a stream for use
        static QTextStream& getStreamInUse(int fd) { return streams[fd]; }

        // Determine whether a given filename is already in use.
        static bool filenameInUse(const QString& requestedFilename) {
            return llvm::any_of(fileNames,
                                [&](auto fn) { return !fn.second.isEmpty() && fn.second == requestedFilename; });
        }

        // Determine whether a given fd is already in use with the given flag.
        static bool fdInUse(int fd, int flag) {
            if (fd < 0 || fd >= SYSCALL_MAXFILES) {
                return false;
            } else if (fileNames[fd].isEmpty()) {
                return false;
            } else if ((fileFlags[fd] & flag) == static_cast<unsigned>(flag) /* also compares ie. O_RDONLY (0x0) */) {
                return true;
            }
            return false;
        }

        // Close the file with file descriptor fd. No errors are recoverable -- if the user's
        // made an error in the call, it will come back to him.
        static void close(int fd) {
            // Can't close STDIN, STDOUT, STDERR, or invalid fd
            if (fd < STDIO_END || fd >= SYSCALL_MAXFILES)
                return;

            fileFlags[fd] = -1;
            files[fd].close();
            streams.erase(fd);
            files.erase(fd);
            fileNames.erase(fd);
        }

        // Attempt to open a new file with the given flag, using the lowest available file descriptor.
        // Check that filename is not in use, flag is reasonable, and there is an available file descriptor.
        // Return: file descriptor in 0...(SYSCALL_MAXFILES-1), or -1 if error
        static int nowOpening(const QString& filename, int flag) {
            int i = 0;
            if (filenameInUse(filename)) {
                s_fileErrorString = "File name " + filename + " is already open.";
                return -1;
            }

            while (!fileNames[i].isEmpty() && i < SYSCALL_MAXFILES) {
                i++;
            }  // Attempt to find available file descriptor

            if (i >= SYSCALL_MAXFILES)  // no available file descriptors
            {
                s_fileErrorString = "File name " + filename + " exceeds maximum open file limit of " + SYSCALL_MAXFILES;
                return -1;
            }

            // Must be OK -- put filename in table
            fileNames[i] = filename;  // our table has its own copy of filename
            fileFlags[i] = flag;
            s_fileErrorString = "File operation OK";
            return i;
        }
    };

public:
    /**
     * Open a file for either reading or writing.
     *
     * @param filename string containing filename
     * @param  flags    0 for read, 1 for write
     * @return file descriptor in the range 0 to SYSCALL_MAXFILES-1, or -1 if error
     */
    static int openFile(QString filename, int flags) {
        SystemIO::get();  // Ensure that SystemIO is constructed
        // Internally, a "file descriptor" is an index into a table
        // of the filename, flag, and the File???putStream associated with
        // that file descriptor.

        int retValue = -1;
        int fdToUse;

        // Check internal plausibility of opening this file
        fdToUse = FileIOData::nowOpening(filename, flags);
        retValue = fdToUse;  // return value is the fd
        if (fdToUse < 0) {
            return -1;
        }  // fileErrorString would have been set

        try {
            FileIOData::openFilestream(fdToUse, filename);
        } catch (int) {
            s_fileErrorString = "File " + filename + " could not be opened.";
            retValue = -1;
        }

        return retValue;  // return the "file descriptor"
    }

    /**
     * Read bytes from file.
     *
     * @param fd     file descriptor
     * @param offset where in the file to seek to
     * @param base   the point to reference 0 for start of file, 1 for current position, 2 for end of the file
     * @return -1 on error
     */
    static int seek(int fd, int offset, int base) {
        SystemIO::get();                  // Ensure that SystemIO is constructed
        if (!FileIOData::fdInUse(fd, 0))  // Check the existence of the "read" fd
        {
            s_fileErrorString = "File descriptor " + QString::number(fd) + " is not open for reading";
            return -1;
        }
        if (fd < 0 || fd >= SYSCALL_MAXFILES)
            return -1;
        auto& stream = FileIOData::getStreamInUse(fd);

        if (base == SEEK_SET) {
            offset += 0;
        } else if (base == SEEK_CUR) {
            offset += stream.pos();
        } else if (base == SEEK_END) {
            offset += FileIOData::files[fd].size();
        } else {
            return -1;
        }
        if (offset < 0) {
            return -1;
        }
        stream.seek(offset);
        return offset;
    }

    /**
     * Read bytes from file.
     *
     * @param fd              file descriptor
     * @param myBuffer        QString to contain bytes read
     * @param lengthRequested number of bytes to read
     * @return number of bytes read, 0 on EOF, or -1 on error
     */
    static int readFromFile(int fd, QByteArray& myBuffer, int lengthRequested) {
        SystemIO::get();  // Ensure that SystemIO is constructed
        /////////////// DPS 8-Jan-2013  //////////////////////////////////////////////////
        /// Read from STDIN file descriptor while using IDE - get input from Messages pane.
        if (!FileIOData::fdInUse(fd, O_RDONLY))  // Check the existence of the "read" fd
        {
            s_fileErrorString = "File descriptor " + QString::number(fd) + " is not open for reading";
            return -1;
        }
        // retrieve FileInputStream from storage
        auto& InputStream = FileIOData::getStreamInUse(fd);

        if (fd == STDIN) {
            SystemIOStatusManager::setStatusTimed("Waiting for user input...");
            while (myBuffer.size() == 0) {
                // Lock the stdio objects and try to read from stdio. If no data is present, wait until so.
                FileIOData::s_stdioMutex.lock();
                while (myBuffer.size() == 0) {
                    /** We spin on a wait condition with a timeout. The timeout is required to ensure that we may
                     * observe any abort flags (ie. if execution is stopped while waiting for IO */
                    const bool dataInStdinStrm = FileIOData::s_stdinBufferEmpty.wait(&FileIOData::s_stdioMutex, 100);
                    if (s_abortSyscall) {
                        FileIOData::s_stdioMutex.unlock();
                        s_abortSyscall = false;
                        SystemIOStatusManager::clearStatus();
                        return -1;
                    }
                    if (dataInStdinStrm) {
                        myBuffer = InputStream.read(lengthRequested).toUtf8();
                    }
                }
                FileIOData::s_stdioMutex.unlock();
            }
        } else {
            // Reads up to lengthRequested bytes of data from this Input stream into an array of bytes.
            myBuffer = InputStream.read(lengthRequested).toUtf8();
        }

        if (myBuffer.size() == 0) {
            if (fd == STDIN) {
                Q_ASSERT(false);  // EOF should never be possible for STDIN
            } else {
                // End of file - write EOF file character into buffer
                myBuffer.append(sizeof(int), EOF);
            }
        }

        SystemIOStatusManager::clearStatus();
        return myBuffer.size();

    }  // end readFromFile

    /**
     * Write bytes to file.
     *
     * @param fd              file descriptor
     * @param myBuffer        byte array containing characters to write
     * @param lengthRequested number of bytes to write
     * @return number of bytes written, or -1 on error
     */

    static int writeToFile(int fd, const QString& myBuffer, int lengthRequested) {
        SystemIO::get();  // Ensure that SystemIO is constructed
        if (fd == STDOUT || fd == STDERR) {
            emit get().doPrint(myBuffer);
            return myBuffer.size();
        }

        if (!FileIOData::fdInUse(fd, O_WRONLY | O_RDWR))  // Check the existence of the "write" fd
        {
            s_fileErrorString = "File descriptor " + QString::number(fd) + " is not open for writing";
            return -1;
        }
        // retrieve FileOutputStream from storage
        auto& outputStream = FileIOData::getStreamInUse(fd);

        outputStream << myBuffer;
        outputStream.flush();
        return lengthRequested;

    }  // end writeToFile

    /**
     * Close the file with specified file descriptor
     *
     * @param fd the file descriptor of an open file
     */
    static void closeFile(int fd) { FileIOData::close(fd); }

    static void printString(const QString& string) { emit get().doPrint(string); }
    static void reset() { FileIOData::resetFiles(); }
    static void abortSyscall(bool state) { s_abortSyscall = state; }

signals:
    void doPrint(const QString&);

public slots:
    /**
     * @brief putStdInData
     * Pushes @p data onto the stdin buffer object
     */
    void putStdInData(const QByteArray& data) {
        FileIOData::s_stdioMutex.lock();
        FileIOData::s_stdinBuffer.append(data);
        FileIOData::s_stdioMutex.unlock();
        FileIOData::s_stdinBufferEmpty.wakeAll();
    }

private:
    SystemIO() { reset(); }
};

}  // namespace Ripes
