#ifndef _THEOIDE_MODELS_VIRTUALMACHINESERVICE_
#define _THEOIDE_MODELS_VIRTUALMACHINESERVICE_

#include <QFuture>
#include <QMap>
#include <QObject>
#include <QPromise>
#include <QSet>
#include <QSharedPointer>
#include <QString>
#include <QtQmlIntegration>
#include <exception>
#include <stdexcept>

#include "VM/include/program.hpp"
#include "VM/include/vm.hpp"

using VMData = Theo::VM::Activation::Data;

class VirtualMachineIsNullError : public std::runtime_error {
 public:
  VirtualMachineIsNullError()
      : std::runtime_error("The virtual machine is null") {}
  ~VirtualMachineIsNullError() {}
};

class ActivationsAreEmptyError : public std::runtime_error {
 public:
  ActivationsAreEmptyError()
      : std::runtime_error("The activations vector of the vm is empty") {}
  ~ActivationsAreEmptyError() {};
};

class VirtualMachineService : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningChanged)
  Q_PROPERTY(bool isDebugging READ isDebugging NOTIFY isDebuggingChanged)
  Q_PROPERTY(QMap<QString, int> variablesState READ variablesState NOTIFY
                 variablesStateChanged)
  QML_ELEMENT
 public:
  VirtualMachineService(QObject* parent = nullptr);
  ~VirtualMachineService();
  bool isRunning() const;
  bool isDebugging() const;
  QList<int> getBreakpointsOfTab(const QString& tabName) const;
  QMap<QString, int> variablesState() const;

 public slots:
  void execute(const Theo::Program& program);
  void debug(const Theo::Program& program);
  void setBreakPointAt(const QString& tabName, const int lineNumber);
  void deleteBreakPointAt(const QString& tabName, const int lineNumber);
  void restart();
  void restartDebug();
  void stopExecution();
  void stepNextBreakPoint();
  void stepNextPossibleBreakPoint();

 signals:
  void executionCompleted();
  void isRunningChanged();
  void isDebuggingChanged();
  void stopCalled();
  void variablesStateChanged();
  void executionFailedForInternalReason();

 protected slots:
  void deinitVirtualMachine();

 private:
  QSharedPointer<Theo::VM> _virtualMachine;
  QMap<QString, QSet<int>> _breakPoints;
  bool _isDebugging = false;
  QMap<QString, int> _variablesState;

  void setIsDebugging(const bool isDebugging);
  void initVirtualMachine(const Theo::Program& program);
  void applyBreakPoints();
  void resetVirtualMachine();
  void startVirtualMachine();
  bool programEndReached() const;
  void cleanUpIfEndReached();
  QFuture<VMData> executeAsync();
  static void internalExecute(QPromise<VMData>& promise,
                              QSharedPointer<Theo::VM> vm);
  void setVariablesState(QMap<QString, int> variablesState);
  void setVariablesState(VMData data);
  void handleActivationsAreEmptyError(const ActivationsAreEmptyError& error);
  void handleVirtualMachineIsNullError(const VirtualMachineIsNullError& error);
  void handleExecutionError(const std::exception& error);
  void enableSteppingMode();
  void disableSteppingMode();
};

#endif
