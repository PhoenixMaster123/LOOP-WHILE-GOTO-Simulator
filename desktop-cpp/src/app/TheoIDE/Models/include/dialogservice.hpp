#ifndef _THEOIDE_MODELS_DIALOGSERVICE_
#define _THEOIDE_MODELS_DIALOGSERVICE_

#include <QList>
#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QtQmlIntegration>
#include <functional>

#include "gen.hpp"
#include "messagedialogdto.hpp"

class DialogService : public QObject {
  Q_OBJECT
  QML_ELEMENT
 public:
  DialogService(QObject* parent = nullptr);
  ~DialogService();
  void addReadPermissionDenied(const QString& fileName);
  void addFileDoesNotExist(const QString& fileName);
  void addUnsavedChangesInFile(const QString& fileName,
                               std::function<void(void)> onSave,
                               std::function<void(void)> onDiscard);
  void addMaxReadFileSizeExceeded(const QString& fileName,
                                  const int maxFileSizeBytes);
  void addNoScriptToCompile();
  void addMainScriptIsEmpty();
  void addNoMainScriptSelected();
  void addCompilationFailed(const Theo::CodegenResult& result);
  void addExecutionFailedForInternalReason();

  std::optional<QSharedPointer<MessageDialogDTO>> remove();
  bool isEmpty() const;

 signals:
  void elementAdded();

 protected:
  void add(QSharedPointer<MessageDialogDTO> dialogDTO);

 private:
  QList<QSharedPointer<MessageDialogDTO>> _dialogDTOs;
};

#endif
