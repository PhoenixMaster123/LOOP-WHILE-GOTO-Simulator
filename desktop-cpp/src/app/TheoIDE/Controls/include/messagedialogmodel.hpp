#ifndef _THEOIDE_CONTROLS_MESSAGEDIALOGMODEL_
#define _THEOIDE_CONTROLS_MESSAGEDIALOGMODEL_

#include <QObject>
#include <QPointer>
#include <QSharedPointer>
#include <QString>
#include <QVariantList>
#include <QtQmlIntegration>

#include "dialogbutton.hpp"
#include "dialogservice.hpp"
#include "messagedialogdto.hpp"

class MessageDialogModel : public QObject {
  Q_OBJECT
  Q_PROPERTY(DialogService* dialogService READ dialogService WRITE
                 setDialogService NOTIFY dialogServiceChanged)
  Q_PROPERTY(QString title READ title NOTIFY titleChanged)
  Q_PROPERTY(QString text READ text NOTIFY textChanged)
  Q_PROPERTY(QString detailedText READ detailedText NOTIFY detailedTextChanged)
  Q_PROPERTY(QString informativeText READ informativeText NOTIFY
                 informativeTextChanged)
  Q_PROPERTY(bool open READ open WRITE setOpen NOTIFY openChanged)
  Q_PROPERTY(
      QVariantList dialogButtons READ dialogButtons NOTIFY dialogButtonsChanged)
  QML_ELEMENT public : MessageDialogModel(QObject* parent = nullptr);
  ~MessageDialogModel();
  DialogService* dialogService() const;
  QString title() const;
  QString text() const;
  QString detailedText() const;
  QString informativeText() const;
  bool open() const;
  QVariantList dialogButtons() const;
  Q_INVOKABLE
  void runButtonAction(DialogButton button);

 public slots:
  void setDialogService(DialogService* dialogService);
  void setMessageDialogDTO(QSharedPointer<MessageDialogDTO> dto);
  void setOpen(bool value);
  void showMessage();

 signals:
  void titleChanged();
  void dialogServiceChanged();
  void textChanged();
  void detailedTextChanged();
  void informativeTextChanged();
  void openChanged(bool open);
  void dialogButtonsChanged();
  void hasMessageToShow();

 private:
  QPointer<DialogService> _dialogService;
  QSharedPointer<MessageDialogDTO> _messageDialogDTO;
  bool _open = false;
  void disconnectDialogService();
  void connectDialogService();
};

#endif
