#ifndef _THEOIDE_MODELS_MESSAGEDIALOGMODEL_
#define _THEOIDE_MODELS_MESSAGEDIALOGMODEL_

#include <QList>
#include <QMap>
#include <QString>
#include <functional>
#include <optional>

#include "dialogbutton.hpp"

using DialogModelCallback = std::function<void(void)>;
using DialogModelText = std::optional<QString>;

class MessageDialogDTO {
 public:
  MessageDialogDTO(const QString& title, const QString& text,
                   DialogModelText detailedText,
                   DialogModelText informativeText);

  QString title() const;
  QString text() const;
  DialogModelText detailedText() const;
  DialogModelText informativeText() const;
  void setButtonWithCallback(DialogButton button, DialogModelCallback callback);
  void setButtonWithMainCallback(DialogButton button,
                                 DialogModelCallback callback);
  void setButton(DialogButton button);
  bool hasCallbackFor(DialogButton button) const;
  void runCallbackFor(DialogButton button) const;
  void runMainCallback() const;
  QList<DialogButton> activeDialogButtons() const;

 private:
  QString _text;
  QString _title;
  DialogModelText _detailedText = std::nullopt;
  DialogModelText _informativeText = std::nullopt;
  std::optional<DialogButton> _mainActionButton;
  QMap<DialogButton, DialogModelCallback> _buttonCallbacks;
};

#endif
