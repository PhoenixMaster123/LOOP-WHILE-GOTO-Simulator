#ifndef _THEOIDE_MODELS_FILESYSTEMSERVICE_
#define _THEOIDE_MODELS_FILESYSTEMSERVICE_

#include <QDir>
#include <QFile>
#include <QFuture>
#include <QMutex>
#include <QObject>
#include <QPointer>
#include <QSharedPointer>
#include <QtQmlIntegration>

class FileError : public std::exception {
 public:
  FileError(const QString &fileName) : _fileName(fileName) {}
  virtual const QString &fileName() const noexcept { return _fileName; }
  virtual const char *what() const noexcept override {
    return _message.toLocal8Bit().data();
  }

 protected:
  virtual void setMessage(const QString &message) { _message = message; }

 private:
  QString _fileName;
  QString _message;
};

class MaxReadFileSizeExceededError : public FileError {
 public:
  MaxReadFileSizeExceededError(const QString &fileName,
                               const int maxFileSizeBytes)
      : FileError(fileName), _maxFileSizeBytes(maxFileSizeBytes) {
    setMessage(QString("File can not be read because it es larger than the "
                       "maximum file size of %1 Bytes")
                   .arg(maxFileSizeBytes));
  }
  int maxFileSizeBytes() const noexcept { return _maxFileSizeBytes; }

 private:
  int _maxFileSizeBytes;
};

class FileOpenReadableError : public FileError {
 public:
  FileOpenReadableError(const QString &fileName) : FileError(fileName) {
    setMessage(QString("Unable to open file %1 for reading").arg(fileName));
  }
};

class FileDoesNotExistError : public FileError {
 public:
  FileDoesNotExistError(const QString &fileName) : FileError(fileName) {
    setMessage(QString("File does not exist %1").arg(fileName));
  }
};

class FileReadError : public FileError {
 public:
  FileReadError(const QString &fileName, const int position)
      : FileError(fileName), _position(position) {
    setMessage(QString("An error occured while reading file %1 at position %2")
                   .arg(fileName)
                   .arg(position));
  }
  int position() const noexcept { return _position; }

 private:
  int _position;
};

class FileAutoCloser {
 public:
  FileAutoCloser(QFile *file) : _file(file) {}
  ~FileAutoCloser() {
    if (_file && _file->isOpen()) {
      _file->close();
    }
  }

 private:
  QPointer<QFile> _file;
};

class FileSystemService : public QObject {
  Q_OBJECT
  Q_PROPERTY(int maxFileSizeBytes READ maxFileSizeBytes WRITE
                 setMaxFileSizeBytes NOTIFY maxFileSizeBytesChanged)
  Q_PROPERTY(int blockSizeBytes READ blockSizeBytes WRITE setBlockSizeBytes
                 NOTIFY blockSizeBytesChanged)
  Q_PROPERTY(QList<QSharedPointer<QFile>> filesBeingRead READ filesBeingRead
                 NOTIFY filesBeingReadChanged)
  QML_ELEMENT
 public:
  FileSystemService(QObject *parent = nullptr);
  ~FileSystemService();
  int maxFileSizeBytes() const;
  int blockSizeBytes() const;
  QList<QSharedPointer<QFile>> filesBeingRead() const;
  void readFile(QSharedPointer<QFile> file);

 public slots:
  void setMaxFileSizeBytes(int maxFileSizeBytes);
  void setBlockSizeBytes(int blockSizeBytes);

 signals:
  void maxFileSizeBytesChanged(int maxFileSizeBytes);
  void blockSizeBytesChanged(int blockSizeBytes);
  void fileRead(QSharedPointer<QFile> file, QString content);
  void fileReadFailed(QSharedPointer<QFile> file, const FileError &error);
  void fileReadFailedMaxReadSizeExceeded(
      QSharedPointer<QFile> file, const MaxReadFileSizeExceededError &error);
  void fileReadFailedFileDoesNotExist(QSharedPointer<QFile> file,
                                      const FileDoesNotExistError &error);
  void fileReadFailedFileReadPermission(QSharedPointer<QFile> file,
                                        const FileReadError &error);
  void fileReadCanceled(QSharedPointer<QFile> file);
  void filesBeingReadChanged(int numberOfFilesBeingRead);

 private:
  mutable QMutex _filesBeingReadMutex;
  int _maxFileSizeBytes = 0;
  int _blockSizeBytes = 0;
  QList<QSharedPointer<QFile>> _filesBeingRead;
  QString readFileSync(QSharedPointer<QFile> file) const;
  QFuture<QString> readFileAsync(QSharedPointer<QFile> file) const;
  void openFileReadable(QSharedPointer<QFile> file) const;
  void ensureFileExists(QSharedPointer<QFile> file) const;
  void addFileToBeingRead(QSharedPointer<QFile> file);
  void removeFileFromBeingRead(QSharedPointer<QFile> file);
};

#endif
