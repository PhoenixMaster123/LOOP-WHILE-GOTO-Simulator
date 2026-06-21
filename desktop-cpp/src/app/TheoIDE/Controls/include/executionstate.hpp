#ifndef _THEOIDE_CONTROLS_EXECUTIONSTATE_
#define _THEOIDE_CONTROLS_EXECUTIONSTATE_

#include <qtmetamacros.h>

#include <QObject>
#include <QtQmlIntegration>

class ExecutionStateWrapper : public QObject {
  Q_OBJECT
  QML_NAMED_ELEMENT(ExecutionState)
  QML_UNCREATABLE("Wrapper for enum")
 public:
  enum ExecutionState {
    Idle = 0,
    Compiling,
    Executing,
    Halt,
  };
  Q_ENUM(ExecutionState)
  ~ExecutionStateWrapper();

 private:
  ExecutionStateWrapper();
};

using ExecutionState = ExecutionStateWrapper::ExecutionState;

#endif
