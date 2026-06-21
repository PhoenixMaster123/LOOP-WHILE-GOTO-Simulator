#ifndef _THEOIDE_MODELS_COMPILERSERVICE_
#define _THEOIDE_MODELS_COMPILERSERVICE_

#include <QFuture>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QtQmlIntegration>

#include "gen.hpp"

class CompilationTask {
 public:
  CompilationTask(const int revision, const QMap<QString, QString> content,
                  const QString& mainTabName)
      : _mainTabName(mainTabName), _revision(revision), _content(content) {}
  int revision() const { return _revision; }
  QMap<QString, QString> content() const { return _content; }
  QString mainTabName() const { return _mainTabName; }

 private:
  QString _mainTabName;
  int _revision;
  QMap<QString, QString> _content;
};

class CompilationResult {
 public:
  CompilationResult(const int revision, const Theo::CodegenResult& result)
      : _revision(revision), _result(result) {}
  int revision() const { return _revision; }
  Theo::CodegenResult result() const { return _result; }

 private:
  int _revision;
  Theo::CodegenResult _result;
};

using AsyncResult = QFuture<QSharedPointer<Theo::CodegenResult>>;

class CompilerService : public QObject {
  Q_OBJECT
  Q_PROPERTY(
      QSharedPointer<CompilationResult> result READ result NOTIFY resultChanged)
  Q_PROPERTY(QSharedPointer<CompilationResult> lastSuccessfulResult READ
                 lastSuccessfulResult NOTIFY lastSuccessfulResultChanged)
  QML_ELEMENT
 public:
  CompilerService(QObject* parent = nullptr);
  ~CompilerService();
  QSharedPointer<CompilationResult> result() const;
  QSharedPointer<CompilationResult> lastSuccessfulResult() const;
  void compile(CompilationTask task);

 public slots:
  void reset();

 signals:
  void atLeastRevisionAvailable(int revision);
  void resultChanged();
  void resetTriggered();
  void lastSuccessfulResultChanged();

 protected slots:
  void setResult(QSharedPointer<CompilationResult> result);
  void setLastSuccessfulResult(QSharedPointer<CompilationResult> result);

 private:
  mutable QMutex _resultMutex;
  QSharedPointer<CompilationResult> _result;
  QSharedPointer<CompilationResult> _lastSuccessfulResult;
  QFuture<Theo::CodegenResult> compileAsync(const CompilationTask& task);
  std::map<std::string, std::string> convertContentToStdMap(
      const QMap<QString, QString>& map);
  bool checkRevisionAlreadyCompiled(int revision);
};

#endif
