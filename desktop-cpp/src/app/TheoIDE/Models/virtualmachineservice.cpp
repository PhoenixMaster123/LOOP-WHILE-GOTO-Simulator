#include <qtpreprocessorsupport.h>

#include <QFutureWatcher>
#include <QtConcurrentRun>
#include <QtLogging>
#include <exception>

#include "virtualmachineservice.hpp"

VirtualMachineService::VirtualMachineService(QObject* parent)
    : QObject(parent) {}

VirtualMachineService::~VirtualMachineService() {}

void VirtualMachineService::execute(const Theo::Program& program) {
  initVirtualMachine(program);
  startVirtualMachine();
}

void VirtualMachineService::debug(const Theo::Program& program) {
  setIsDebugging(true);
  initVirtualMachine(program);
  applyBreakPoints();
  startVirtualMachine();
}

void VirtualMachineService::restart() {
  resetVirtualMachine();
  startVirtualMachine();
}

void VirtualMachineService::restartDebug() {
  resetVirtualMachine();
  applyBreakPoints();
  startVirtualMachine();
}

QFuture<VMData> VirtualMachineService::executeAsync() {
  return QtConcurrent::run(&VirtualMachineService::internalExecute,
                           _virtualMachine);
}

void VirtualMachineService::internalExecute(QPromise<VMData>& promise,
                                            QSharedPointer<Theo::VM> vm) {
  if (vm.isNull()) {
    promise.setException(std::make_exception_ptr(VirtualMachineIsNullError()));
    return;
  }
  while (!promise.isCanceled()) {
    promise.suspendIfRequested();
    const bool shouldStop = vm->executeSingle();
    if (shouldStop) {
      auto activations = vm->getActivations();
      if (activations.empty()) {
        promise.setException(
            std::make_exception_ptr(ActivationsAreEmptyError()));
        return;
      }
      promise.addResult(activations.back().getActivationVariables());
      return;
    }
  }
}

void VirtualMachineService::deinitVirtualMachine() {
  _virtualMachine.clear();
  emit isRunningChanged();
  setIsDebugging(false);
  emit executionCompleted();
}

bool VirtualMachineService::programEndReached() const {
  if (_virtualMachine.isNull()) {
    return true;
  }
  return _virtualMachine->isDone();
}

void VirtualMachineService::startVirtualMachine() {
  if (_virtualMachine.isNull()) {
    deinitVirtualMachine();
    return;
  }
  QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
  const std::function<void(void)> deleteWatcher = [watcher]() -> void {
    watcher->deleteLater();
  };
  connect(watcher, &QFutureWatcher<void>::finished, deleteWatcher);
  connect(watcher, &QFutureWatcher<void>::canceled, deleteWatcher);
  connect(watcher, &QFutureWatcher<void>::canceled, this,
          &VirtualMachineService::deinitVirtualMachine);
  connect(this, &VirtualMachineService::stopCalled, watcher,
          &QFutureWatcher<void>::cancel);
  const QFuture future =
      executeAsync()
          .then([this](VMData data) -> void {
            setVariablesState(data);
            disableSteppingMode();
            cleanUpIfEndReached();
          })
          .onFailed([this](const VirtualMachineIsNullError& error) -> void {
            handleVirtualMachineIsNullError(error);
          })
          .onFailed([this](const ActivationsAreEmptyError& error) -> void {
            handleActivationsAreEmptyError(error);
          })
          .onFailed([this](const std::exception& error) -> void {
            handleExecutionError(error);
          });
  watcher->setFuture(future);
}

void VirtualMachineService::handleExecutionError(const std::exception& error) {
  deinitVirtualMachine();
  qCritical() << "Execution failed because of an exception, not taken into "
                 "account during development"
              << error.what();
  emit executionFailedForInternalReason();
}

void VirtualMachineService::handleVirtualMachineIsNullError(
    const VirtualMachineIsNullError& error) {
  Q_UNUSED(error)
  deinitVirtualMachine();
  qCritical() << "Execution failed because virtual machine is null";
  emit executionFailedForInternalReason();
}

void VirtualMachineService::handleActivationsAreEmptyError(
    const ActivationsAreEmptyError& error) {
  Q_UNUSED(error)
  deinitVirtualMachine();
  qCritical() << "Execution failed because activations are empty";
  emit executionFailedForInternalReason();
}

QMap<QString, int> VirtualMachineService::variablesState() const {
  return _variablesState;
}

void VirtualMachineService::setVariablesState(VMData data) {
  QMap<QString, int> variablesState;
  for (auto line : data) {
    variablesState.insert(QString::fromStdString(line.first), line.second);
  }
  setVariablesState(variablesState);
}

void VirtualMachineService::cleanUpIfEndReached() {
  if (programEndReached()) {
    deinitVirtualMachine();
  }
}

void VirtualMachineService::setVariablesState(
    QMap<QString, int> variablesState) {
  if (_variablesState != variablesState) {
    _variablesState = variablesState;
    emit variablesStateChanged();
  }
}

void VirtualMachineService::stopExecution() { emit stopCalled(); }

void VirtualMachineService::resetVirtualMachine() {
  if (_virtualMachine.isNull()) {
    return;
  }
  _virtualMachine->reset();
}

bool VirtualMachineService::isRunning() const {
  return !_virtualMachine.isNull();
}

void VirtualMachineService::initVirtualMachine(const Theo::Program& program) {
  _virtualMachine = QSharedPointer<Theo::VM>(new Theo::VM(program));
  emit isRunningChanged();
}

void VirtualMachineService::setBreakPointAt(const QString& tabName,
                                            const int lineNumber) {
  if (_breakPoints.contains(tabName)) {
    _breakPoints[tabName].insert(lineNumber);
  } else {
    _breakPoints.insert(tabName, {lineNumber});
  }
  if (_isDebugging && _virtualMachine) {
    _virtualMachine->setBreakPoint(tabName.toStdString(), lineNumber, true);
  }
}

void VirtualMachineService::deleteBreakPointAt(const QString& tabName,
                                               const int lineNumber) {
  if (!_breakPoints.contains(tabName)) {
    return;
  }
  const bool removed = _breakPoints[tabName].remove(lineNumber);
  if (removed && _isDebugging && _virtualMachine) {
    _virtualMachine->setBreakPoint(tabName.toStdString(), lineNumber, false);
  }
}

void VirtualMachineService::applyBreakPoints() {
  if (_virtualMachine.isNull()) {
    return;
  }
  for (const QString& fileName : _breakPoints.keys()) {
    for (const int lineNumber : _breakPoints[fileName]) {
      _virtualMachine->setBreakPoint(fileName.toStdString(), lineNumber, true);
    }
  }
}

bool VirtualMachineService::isDebugging() const { return _isDebugging; }

void VirtualMachineService::setIsDebugging(const bool isDebugging) {
  if (_isDebugging != isDebugging) {
    _isDebugging = isDebugging;
    emit isDebuggingChanged();
  }
}

void VirtualMachineService::stepNextBreakPoint() {
  if (!isRunning()) {
    return;
  }
  startVirtualMachine();
}

void VirtualMachineService::stepNextPossibleBreakPoint() {
  if (!isRunning()) {
    return;
  }
  enableSteppingMode();
  startVirtualMachine();
}

void VirtualMachineService::enableSteppingMode() {
  if (_virtualMachine.isNull()) {
    return;
  }
  _virtualMachine->setSteppingMode(true);
}

void VirtualMachineService::disableSteppingMode() {
  if (_virtualMachine.isNull()) {
    return;
  }
  if (_virtualMachine->isSteppingModeEnabled()) {
    _virtualMachine->setSteppingMode(false);
  }
}
