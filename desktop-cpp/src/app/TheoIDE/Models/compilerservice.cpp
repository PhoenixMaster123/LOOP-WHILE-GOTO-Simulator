#include <QtConcurrentRun>
#include <functional>
#include <map>
#include <string>

#include "compiler.hpp"
#include "compilerservice.hpp"
#include "gen.hpp"
#include "scan.hpp"

CompilerService::CompilerService(QObject* parent) : QObject(parent) {}

CompilerService::~CompilerService() {}

bool CompilerService::checkRevisionAlreadyCompiled(int revision) {
  if (_result.isNull()) {
    return false;
  }
  return _result->revision() >= revision;
}

void CompilerService::compile(CompilationTask task) {
  if (checkRevisionAlreadyCompiled(task.revision())) {
    emit atLeastRevisionAvailable(task.revision());
    return;
  }
  qInfo() << "Schedule compilation of revision" << task.revision();
  auto watcher = new QFutureWatcher<void>(this);
  std::function<void(void)> deleteWatcher = [watcher]() -> void {
    watcher->deleteLater();
  };
  connect(watcher, &QFutureWatcher<void>::canceled, deleteWatcher);
  connect(watcher, &QFutureWatcher<void>::finished, deleteWatcher);
  connect(this, &CompilerService::resetTriggered, watcher,
          &QFutureWatcher<void>::cancel);
  const QFuture<void> future = compileAsync(task).then(
      [this, task](Theo::CodegenResult codegenResult) -> void {
        auto result = QSharedPointer<CompilationResult>(
            new CompilationResult(task.revision(), codegenResult));
        setResult(result);
        emit atLeastRevisionAvailable(task.revision());
        qInfo() << "Compilation of revision" << task.revision() << "done";
      });
  watcher->setFuture(future);
}

void CompilerService::setResult(QSharedPointer<CompilationResult> result) {
  if (_result == result) {
    return;
  }
  QMutexLocker locker(&_resultMutex);
  if (result.isNull() || _result.isNull() ||
      result->revision() > _result->revision()) {
    _result = result;
    emit resultChanged();
    if (_result && _result->result().generated_correctly) {
      setLastSuccessfulResult(_result);
    }
  }
}

std::map<std::string, std::string> CompilerService::convertContentToStdMap(
    const QMap<QString, QString>& map) {
  std::map<std::string, std::string> result;
  for (auto pair : map.asKeyValueRange()) {
    const std::string key = pair.first.toStdString();
    const std::string value = pair.second.toStdString();
    const std::pair<std::string, std::string> convertedPair(key, value);
    result.insert(convertedPair);
  }
  return result;
}

QFuture<Theo::CodegenResult> CompilerService::compileAsync(
    const CompilationTask& task) {
  auto content = convertContentToStdMap(task.content());
  auto compile = [](QPromise<Theo::CodegenResult>& promise,
                    std::map<Theo::FileName, Theo::FileContent> files,
                    std::string main) -> void {
    Theo::CodegenResult result = Theo::compile(files, main);
    if (promise.isCanceled()) {
      return;
    }
    promise.addResult(result);
  };
  return QtConcurrent::run(compile, content, task.mainTabName().toStdString());
}

QSharedPointer<CompilationResult> CompilerService::result() const {
  return _result;
}

void CompilerService::reset() {
  emit resetTriggered();
  setResult(nullptr);
  setLastSuccessfulResult(nullptr);
}

QSharedPointer<CompilationResult> CompilerService::lastSuccessfulResult()
    const {
  return _lastSuccessfulResult;
}

void CompilerService::setLastSuccessfulResult(
    QSharedPointer<CompilationResult> result) {
  if (_lastSuccessfulResult == result) {
    return;
  }
  _lastSuccessfulResult = result;
  emit lastSuccessfulResultChanged();
}
