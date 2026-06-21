#ifndef _THEOIDE_MODELS_DIALOGBUTTON_
#define _THEOIDE_MODELS_DIALOGBUTTON_

#include <QObject>
#include <QtQmlIntegration>

class DialogButtonWrapper : public QObject {
  Q_OBJECT
  QML_NAMED_ELEMENT(DialogButton)
  QML_UNCREATABLE("Wrapper of Enum")
 public:
  enum Value {
    Ok,
    Open,
    Save,
    Cancel,
    Close,
    Discard,
    Apply,
    Reset,
    RestoreDefaults,
    Help,
    SaveAll,
    Yes,
    YesToAll,
    No,
    NoToAll,
    Abort,
    Retry,
    Ignore,
    NoButton,
  };
  Q_ENUM(Value)
 private:
  explicit DialogButtonWrapper(QObject* parent = nullptr);
};

using DialogButton = DialogButtonWrapper::Value;

#endif
