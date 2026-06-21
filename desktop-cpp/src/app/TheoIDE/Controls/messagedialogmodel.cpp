#include <algorithm>

#include "messagedialogmodel.hpp"

MessageDialogModel::MessageDialogModel(QObject* parent) : QObject(parent) {}
MessageDialogModel::~MessageDialogModel() {}

DialogService* MessageDialogModel::dialogService() const {
  return _dialogService.data();
}

QString MessageDialogModel::text() const {
  if (_messageDialogDTO) {
    return _messageDialogDTO->text();
  }
  return QString();
}

QString MessageDialogModel::detailedText() const {
  if (_messageDialogDTO && _messageDialogDTO->detailedText().has_value()) {
    return _messageDialogDTO->detailedText().value();
  }
  return QString();
}

QString MessageDialogModel::informativeText() const {
  if (_messageDialogDTO && _messageDialogDTO->informativeText().has_value()) {
    return _messageDialogDTO->informativeText().value();
  }
  return QString();
}

QString MessageDialogModel::title() const {
  if (_messageDialogDTO) {
    return _messageDialogDTO->title();
  }
  return QString();
}

bool MessageDialogModel::open() const { return _open; }

QVariantList MessageDialogModel::dialogButtons() const {
  if (_messageDialogDTO.isNull()) {
    return {QVariant::fromValue(DialogButton::NoButton)};
  }
  const QList<DialogButton> activeDialogButtons =
      _messageDialogDTO->activeDialogButtons();
  QVariantList returnValue(activeDialogButtons.size());
  std::transform(activeDialogButtons.begin(), activeDialogButtons.end(),
                 returnValue.begin(), [](DialogButton button) -> QVariant {
                   return QVariant::fromValue(button);
                 });
  return returnValue;
}

void MessageDialogModel::runButtonAction(DialogButton button) {
  if (_messageDialogDTO.isNull()) {
    qFatal() << "Trying to run callback for button" << button
             << "but the message dialog dto was null";
    return;
  }
  if (!_messageDialogDTO->hasCallbackFor(button)) {
    qWarning() << "Trying to run callback for button" << button
               << "but no callback was registered";
    return;
  }
  _messageDialogDTO->runCallbackFor(button);
  setOpen(false);
  showMessage();
}

void MessageDialogModel::setDialogService(DialogService* dialogService) {
  if (_dialogService != dialogService) {
    disconnectDialogService();
    _dialogService = QPointer<DialogService>(dialogService);
    emit dialogServiceChanged();
    connectDialogService();
  }
}

void MessageDialogModel::setMessageDialogDTO(
    QSharedPointer<MessageDialogDTO> dto) {
  if (_messageDialogDTO != dto) {
    _messageDialogDTO = dto;
    emit titleChanged();
    emit textChanged();
    emit detailedTextChanged();
    emit informativeTextChanged();
    emit dialogButtonsChanged();
  }
}

void MessageDialogModel::setOpen(bool open) {
  if (_open != open) {
    _open = open;
    emit openChanged(open);
  }
}

void MessageDialogModel::showMessage() {
  if (_open) {
    return;
  }
  if (_dialogService.isNull()) {
    qFatal() << "Trying to fetch next message from dialog service, but the "
                "dialog service is null";
    return;
  }
  if (_dialogService->isEmpty()) {
    return;
  }
  auto messageOptional = _dialogService->remove();
  if (!messageOptional.has_value()) {
    qFatal() << "Received a nullopt message from dialog service";
    return;
  }
  setMessageDialogDTO(messageOptional.value());
  emit hasMessageToShow();
}

void MessageDialogModel::disconnectDialogService() {
  if (_dialogService) {
    disconnect(_dialogService, nullptr, this, nullptr);
  }
}

void MessageDialogModel::connectDialogService() {
  if (_dialogService) {
    connect(_dialogService, &DialogService::elementAdded, this,
            &MessageDialogModel::showMessage);
  }
}
