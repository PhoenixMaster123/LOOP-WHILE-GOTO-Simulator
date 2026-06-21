#include <qtpreprocessorsupport.h>

#include <QTextBlock>
#include <QtGlobal>
#include <QtLogging>
#include <algorithm>
#include <chrono>
#include <functional>
#include <ranges>

#include "editormodel.hpp"
#include "executionstate.hpp"
#include "virtualmachineservice.hpp"

EditorModel::EditorModel(QObject* parent) : QAbstractListModel(parent) {
  connect(this, &QAbstractListModel::rowsInserted, this,
          &EditorModel::updateMainTabIndex);
  connect(this, &QAbstractListModel::rowsRemoved, this,
          &EditorModel::updateMainTabIndex);
  connect(this, &EditorModel::mainTabIndexChanged, this,
          &EditorModel::updateAllTabNames);
  _executionStateTimer.setSingleShot(true);
  _executionStateTimer.setInterval(std::chrono::seconds(5));
  connect(&_executionStateTimer, &QTimer::timeout, this,
          &EditorModel::stopExecution);
  connect(this, &EditorModel::executionStateChanged, &_executionStateTimer,
          &QTimer::stop);
}
EditorModel::~EditorModel() {}

QHash<int, QByteArray> EditorModel::roleNames() const {
  return QHash<int, QByteArray>{
      {Qt::DisplayRole, "display"},
      {StoredTabTextRole, "storedTabText"},
      {TabNameRole, "tabName"},
      {DisplayTabNameRole, "displayTabName"},
      {IsModifiedRole, "isModified"},
      {IsTemporaryRole, "isTemporary"},
      {TextDocumentRole, "textDocument"},
      {OpenRole, "open"},
      {IsReadOnlyRole, "isReadOnly"},
      {BackgroundCompilationTimerRole, "backgroundCompilationTimer"},
      {CursorPositionRole, "cursorPosition"},
      {CursorPositionEditRole, "cursorPositionEdit"},
      {CursorLineNumberRole, "cursorLineNumber"}};
}

int EditorModel::rowCount(const QModelIndex& index) const {
  Q_UNUSED(index)
  return _tabs.count();
}

QVariant EditorModel::data(const QModelIndex& index, int role) const {
  switch (role) {
    case StoredTabTextRole:
      return storedTabTextAt(index.row());
    case Qt::DisplayRole:
    case DisplayTabNameRole:
      return displayTabNameAt(index.row());
    case TabNameRole:
      return tabNameAt(index.row());
    case IsModifiedRole:
      return isTabModifiedAt(index.row());
    case IsTemporaryRole:
      return isTabTemporaryAt(index.row());
    case IsReadOnlyRole:
      return isTabReadOnlyAt(index.row());
    case CursorPositionRole:
      return cursorPositionAt(index.row());
    case CursorLineNumberRole:
      return cursorLineNumberAt(index.row());
  }
  return QVariant();
}

bool EditorModel::isTabReadOnlyAt(qsizetype index) const {
  Q_UNUSED(index)
  return _executionState != ExecutionState::Idle;
}

QString EditorModel::storedTabTextAt(qsizetype index) const {
  const TabModelOptional tab = tabAt(index);
  if (tab.has_value() && !tab.value().isNull()) {
    return tab.value()->storedText();
  }
  return QString();
}

QString EditorModel::tabNameAt(qsizetype index) const {
  const TabModelOptional tab = tabAt(index);
  if (tab.has_value() && !tab.value().isNull()) {
    return tab.value()->name();
  }
  return QString();
}

QString EditorModel::displayTabNameAt(qsizetype index) const {
  const TabModelOptional tab = tabAt(index);
  if (tab.has_value() && !tab.value().isNull()) {
    return tab.value()->displayName();
  }
  return QString();
}

bool EditorModel::isTabModifiedAt(qsizetype index) const {
  const TabModelOptional tab = tabAt(index);
  return tab.has_value() && !tab.value().isNull() && tab.value()->isModified();
}

bool EditorModel::isTabTemporaryAt(qsizetype index) const {
  const TabModelOptional tab = tabAt(index);
  return tab.has_value() && !tab.value().isNull() && tab.value()->isTemporary();
}

bool EditorModel::setData(const QModelIndex& index, const QVariant& data,
                          int role) {
  switch (role) {
    case TextDocumentRole:
      return setTextDocumentVariantAt(index.row(), data);
    case OpenRole:
      return setOpenAt(index.row(), data);
    case CursorPositionEditRole:
      return setCursorPositionVariantAt(index.row(), data, role);
    case CursorPositionRole:
      return setCursorPositionVariantAt(index.row(), data, role);
  }
  return false;
}

bool EditorModel::setOpenAt(qsizetype index, const QVariant& data) {
  if (!data.canConvert<bool>()) {
    return false;
  }
  if (!data.toBool()) {
    closeTabAt(index);
  }
  return true;
}

bool EditorModel::setTextDocumentVariantAt(qsizetype index,
                                           const QVariant& data) {
  auto tabOptional = tabAt(index);
  if (!tabOptional.has_value() || tabOptional.value().isNull()) {
    qCritical() << "Trying to set QTextDocument for tab at index" << index
                << " but the tab does not exist";
    return false;
  }
  if (!data.canConvert<QQuickTextDocument*>()) {
    qCritical() << "QVariant can not be converted to QQuickTextDocument*";
    return false;
  }
  QPointer<QQuickTextDocument> quickTextDocument =
      QPointer(data.value<QQuickTextDocument*>());
  if (quickTextDocument.isNull()) {
    qCritical() << "Cast from QVariant to QQuickTextDocument* failed";
    return false;
  }
  const QPointer<QTextDocument> textDocument(quickTextDocument->textDocument());
  auto tab = tabOptional.value();
  tab->setTextDocument(textDocument);
  return true;
}

FileSystemService* EditorModel::fileSystemService() const {
  return _fileSystemService.data();
}

DialogService* EditorModel::dialogService() const {
  return _dialogService.data();
}

CompilerService* EditorModel::compilerService() const {
  return _compilerService.data();
}

void EditorModel::setCompilerService(CompilerService* compilerService) {
  if (_compilerService != compilerService) {
    disconnectCompilerService();
    _compilerService = QPointer(compilerService);
    emit compilerServiceChanged();
    connectCompilerService();
  }
}

void EditorModel::setFileSystemService(FileSystemService* fileSystemService) {
  if (_fileSystemService != fileSystemService) {
    disconnectFileSystemService();
    _fileSystemService = QPointer<FileSystemService>(fileSystemService);
    emit fileSystemServiceChanged();
    connectFileSystemService();
  }
}

void EditorModel::disconnectFileSystemService() {
  if (_fileSystemService) {
    disconnect(_fileSystemService, nullptr, this, nullptr);
  }
}

void EditorModel::connectFileSystemService() {
  if (_fileSystemService) {
    connect(_fileSystemService, &FileSystemService::fileRead, this,
            &EditorModel::createTabFromFile);
    connect(_fileSystemService, &FileSystemService::fileReadFailed, this,
            &EditorModel::displayFileReadFailure);
    connect(_fileSystemService,
            &FileSystemService::fileReadFailedFileReadPermission, this,
            &EditorModel::displayFileReadFileReadPermissionFailure);
    connect(_fileSystemService,
            &FileSystemService::fileReadFailedFileDoesNotExist, this,
            &EditorModel::displayFileReadFileDoesNotExistFailure);
    connect(_fileSystemService,
            &FileSystemService::fileReadFailedMaxReadSizeExceeded, this,
            &EditorModel::displayFileReadMaxReadFileSizeExceededFailure);
  }
}

void EditorModel::openFile(const QUrl& url) {
  QSharedPointer<QFile> file(new QFile(url.toLocalFile()));
  if (_fileSystemService) {
    _fileSystemService->readFile(file);
  }
}

void EditorModel::createTabFromFile(QSharedPointer<QFile> file,
                                    const QString& content) {
  QMutexLocker locker(&_tabsMutex);
  const int index = _tabs.size();
  beginInsertRows(QModelIndex(), index, index);
  const QString tabName = createTabNameRelativeToMainTab(file);
  _tabs.append(QSharedPointer<TabModel>(new TabModel(tabName, file, content)));
  endInsertRows();
}

void EditorModel::setDialogService(DialogService* dialogService) {
  if (_dialogService != dialogService) {
    _dialogService = QPointer(dialogService);
    emit dialogServiceChanged();
  }
}

void EditorModel::saveAllTabs() {
  for (QSharedPointer<TabModel> tab : _tabs) {
    saveTab(tab);
  }
}

void EditorModel::saveTabAt(qsizetype index) {
  TabModelOptional tab = tabAt(index);
  if (tab.has_value()) {
    saveTab(tab.value());
  }
}

void EditorModel::saveTab(QSharedPointer<TabModel> tab) {
  if (tab.isNull()) {
    return;
  }
  qDebug() << "save tab not implemented:" << tab->name();
}

void EditorModel::createNewTab() {
  QSharedPointer<TabModel> tab(new TabModel(qtTrId("Temporary")));
  const int temporaryIndex = appendTemporaryTabIndex(tab);
  tab->setName(QString("%1-%2").arg(tab->name()).arg(temporaryIndex));
  QMutexLocker locker(&_tabsMutex);
  const int tabIndex = _tabs.size();
  emit beginInsertRows(QModelIndex(), tabIndex, tabIndex);
  _tabs.append(tab);
  emit endInsertRows();
}

void EditorModel::runScript() {
  setRunningMode(Default);
  const bool compilationTaskPushed = tryPushCompilationTask();
  if (!compilationTaskPushed) {
    setExecutionState(ExecutionState::Idle);
  }
}

void EditorModel::runScriptInDebugMode() {
  setRunningMode(Debug);
  const bool compilationTaskPushed = tryPushCompilationTask();
  if (!compilationTaskPushed) {
    setExecutionState(ExecutionState::Idle);
  }
}

void EditorModel::stopExecution() {
  if (_executionState == ExecutionState::Idle) {
    return;
  }
  switch (_executionState) {
    case ExecutionState::Compiling:
      stopCompiler();
      break;
    case ExecutionState::Halt:
    case ExecutionState::Executing:
      stopVirtualMachine();
      break;
    default:
      break;
  }
}

void EditorModel::stopVirtualMachine() {
  if (_virtualMachineService.isNull()) {
    return;
  }
  _virtualMachineService->stopExecution();
}

void EditorModel::stopCompiler() {
  if (_compilerService.isNull()) {
    return;
  }
  // TODO stop compilation
  _compilerService->reset();
}

bool EditorModel::compilationPreconditionsFulfilled() const {
  if (_compilerService.isNull()) {
    qCritical() << "Unable to compile script because compiler service is null";
    return false;
  }
  if (_tabs.isEmpty()) {
    if (_dialogService) {
      _dialogService->addNoScriptToCompile();
      return false;
    }
    qWarning() << "Tried to inform the user that there is no open tab for "
                  "compilation, but the message service is null";
    return false;
  }
  return true;
}

bool EditorModel::tryPushCompilationTask() {
  if (!compilationPreconditionsFulfilled()) {
    return false;
  }
  try {
    const CompilationTask task = createCompilationTaskFromTabContent();
    _currentRunRevision = task.revision();
    setExecutionState(ExecutionState::Compiling);
    startExecutionStateTimer();
    _compilerService->compile(task);
  } catch (const NoMainTabError& error) {
    if (_dialogService) {
      _dialogService->addNoMainScriptSelected();
      return false;
    }
    qWarning() << "Tried to inform the user that no main script was selected, "
                  "but the message service is null";
    return false;
  } catch (const EmptyMainTabError& error) {
    if (_dialogService) {
      _dialogService->addMainScriptIsEmpty();
      return false;
    }
    qWarning() << "Tried to inform the user that the main script is empty, but "
                  "the message service was null";
    return false;
  } catch (const MainTabInvalidStateError& error) {
    qFatal() << "Main tab is in invalid state.";
    return false;
  }
  return true;
}

bool EditorModel::isMainTabIndex(int index) const {
  return index == _mainTabIndex;
}

CompilationTask EditorModel::createCompilationTaskFromTabContent() const {
  QString mainTabName;
  int revision = 0;
  QMap<QString, QString> content;
  for (const int tabIndex : std::views::iota(0, _tabs.size())) {
    auto tab = _tabs.at(tabIndex);
    if (tab.isNull()) {
      if (isMainTabIndex(tabIndex)) {
        throw MainTabInvalidStateError();
      }
      continue;
    }
    if (isMainTabIndex(tabIndex)) {
      mainTabName = tab->name();
    }
    auto tabTextDocument = tab->textDocument();
    if (tabTextDocument.isNull()) {
      if (isMainTabIndex(tabIndex)) {
        throw MainTabInvalidStateError();
      }
      continue;
    }
    const QString tabContent = tabTextDocument->toPlainText();
    if (isMainTabIndex(tabIndex) && tabContent.isEmpty()) {
      throw EmptyMainTabError();
    }
    revision += tabTextDocument->revision();
    content.insert(tab->name(), tabContent);
  }
  if (mainTabName.isEmpty()) {
    throw NoMainTabError();
  }
  return CompilationTask(revision, content, mainTabName);
}

void EditorModel::closeTabAt(qsizetype index) {
  QMutexLocker locker(&_tabsMutex);
  TabModelOptional tab = tabAt(index);
  if (!tab.has_value() || tab.value().isNull()) {
    qCritical() << "Trying to close tab at index" << index
                << "but the tab does not exist";
    return;
  }
  const QSharedPointer<TabModel> tabModel = tab.value();
  const std::function<void(void)> removeTab = [this, index,
                                               tabModel]() -> void {
    beginRemoveRows(QModelIndex(), index, index);
    _tabs.removeAt(index);
    removeTemporaryTabIndex(tabModel);
    endRemoveRows();
  };
  if (!tabModel->isModified()) {
    removeTab();
    return;
  }
  if (_dialogService.isNull()) {
    qCritical()
        << "Tried to ask the user if the unsaved changes should be saved "
           "before closing the tab, but the dialog service is null.";
    removeTab();
    return;
  }
  const std::function<void(void)> saveTab = [this, tabModel,
                                             removeTab]() -> void {
    this->saveTab(tabModel);
    removeTab();
  };
  _dialogService->addUnsavedChangesInFile(tabModel->name(), saveTab, removeTab);
}

TabModelOptional EditorModel::tabAt(qsizetype index) const {
  if (tabIndexOutOfRange(index) || _tabs.at(index).isNull()) {
    return std::nullopt;
  }
  return _tabs.at(index);
}

bool EditorModel::tabIndexOutOfRange(qsizetype index) const {
  return index >= _tabs.size() || index < 0;
}

int EditorModel::appendTemporaryTabIndex(QSharedPointer<TabModel> tab) {
  QMutexLocker locker(&_temporaryTabIndexesMutex);
  if (_temporaryTabIndexes.isEmpty()) {
    const int nextIndex = 1;
    _temporaryTabIndexes.insert(tab, nextIndex);
    return nextIndex;
  }
  const auto indexes = _temporaryTabIndexes.values();
  const int highestIndex = *std::max_element(indexes.begin(), indexes.end());
  const int nextIndex = highestIndex + 1;
  _temporaryTabIndexes.insert(tab, nextIndex);
  return nextIndex;
}

void EditorModel::removeTemporaryTabIndex(QSharedPointer<TabModel> tab) {
  QMutexLocker locker(&_temporaryTabIndexesMutex);
  _temporaryTabIndexes.remove(tab);
}

TabModelOptional EditorModel::mainTab() const { return tabAt(_mainTabIndex); }

QString EditorModel::createTabNameRelativeToMainTab(
    QSharedPointer<QFile> file) const {
  const QFileInfo fileInfo(*file);
  const TabModelOptional mainTab = this->mainTab();
  if (!mainTab.has_value() || mainTab.value().isNull()) {
    return fileInfo.fileName();
  }
  const FileOptional mainTabFile = mainTab.value()->file();
  if (!mainTabFile.has_value() || mainTabFile.value().isNull()) {
    return fileInfo.fileName();
  }
  const QFileInfo mainTabFileInfo(*mainTabFile.value());
  const QDir mainTabFileDir = mainTabFileInfo.absoluteDir();
  return mainTabFileDir.relativeFilePath(file->fileName());
}

int EditorModel::mainTabIndex() const { return _mainTabIndex; }

void EditorModel::setMainTabIndex(int index) {
  if (_mainTabIndex != index) {
    _mainTabIndex = index;
    emit mainTabIndexChanged(index);
    updateAllTabNames();
  }
}

void EditorModel::updateAllTabNames() {
  for (qsizetype i = 0; i < _tabs.size(); ++i) {
    updateTabNameAt(i);
  }
}

void EditorModel::updateTabNameAt(qsizetype index) {
  auto tabOptional = tabAt(index);
  if (!tabOptional.has_value() || tabOptional.value().isNull()) {
    return;
  }
  auto tab = tabOptional.value();
  const FileOptional fileOptional = tab->file();
  if (!fileOptional.has_value()) {
    return;
  }
  const auto file = fileOptional.value();
  const auto newTabName = createTabNameRelativeToMainTab(file);
  if (newTabName == tab->name()) {
    return;
  }
  tab->setName(newTabName);
  const QModelIndex modelIndex = createIndex(index, 0);
  emit dataChanged(modelIndex, modelIndex);
}

const QString ERROR_WHILE_DIALOG_SERVICE_NULL_MESSAGE =
    "An error occured, but the dialog service is null.";

void EditorModel::displayFileReadFileReadPermissionFailure(
    QSharedPointer<QFile> file, const FileReadError& error) {
  Q_UNUSED(file);
  if (_dialogService.isNull()) {
    qCritical() << ERROR_WHILE_DIALOG_SERVICE_NULL_MESSAGE << error.what();
    return;
  }
  _dialogService->addReadPermissionDenied(error.fileName());
}

void EditorModel::displayFileReadFileDoesNotExistFailure(
    QSharedPointer<QFile> file, const FileDoesNotExistError& error) {
  Q_UNUSED(file)
  if (_dialogService.isNull()) {
    qCritical() << ERROR_WHILE_DIALOG_SERVICE_NULL_MESSAGE << error.what();
    return;
  }
  _dialogService->addFileDoesNotExist(error.fileName());
}

void EditorModel::displayFileReadMaxReadFileSizeExceededFailure(
    QSharedPointer<QFile> file, const MaxReadFileSizeExceededError& error) {
  Q_UNUSED(file)
  if (_dialogService.isNull()) {
    qCritical() << ERROR_WHILE_DIALOG_SERVICE_NULL_MESSAGE << error.what();
    return;
  }
  _dialogService->addMaxReadFileSizeExceeded(error.fileName(),
                                             error.maxFileSizeBytes());
}

void EditorModel::displayFileReadFailure(QSharedPointer<QFile> file,
                                         const FileError& error) {
  Q_UNUSED(file)
  if (_dialogService.isNull()) {
    qCritical() << "An error occured but the dialog service was null:"
                << error.what();
    return;
  }

  {
    const auto fileReadError =
        QScopedPointer(dynamic_cast<const FileReadError*>(&error));

    if (fileReadError) {
      _dialogService->addReadPermissionDenied(fileReadError->fileName());
      return;
    }
  }

  {
    const auto maxFileSizeExceededError = QScopedPointer(
        dynamic_cast<const MaxReadFileSizeExceededError*>(&error));
    if (maxFileSizeExceededError) {
      _dialogService->addMaxReadFileSizeExceeded(
          maxFileSizeExceededError->fileName(),
          maxFileSizeExceededError->maxFileSizeBytes());
      return;
    }
  }
  const auto fileDoesNotExistError =
      QScopedPointer(dynamic_cast<const FileDoesNotExistError*>(&error));
  if (fileDoesNotExistError) {
    _dialogService->addFileDoesNotExist(fileDoesNotExistError->fileName());
    return;
  }
}

void EditorModel::updateMainTabIndex() {
  if (_mainTabIndex < 0 && _tabs.size() > 0) {
    setMainTabIndex(0);
    return;
  }
  if (_tabs.size() == 0) {
    setMainTabIndex(-1);
    return;
  }
  if (_mainTabIndex >= _tabs.size()) {
    setMainTabIndex(0);
    return;
  }
}

void EditorModel::setCurrentTabIndex(int currentTabIndex) {
  if (_currentTabIndex != currentTabIndex) {
    _currentTabIndex = currentTabIndex;
    emit currentTabIndexChanged(currentTabIndex);
  }
}

int EditorModel::currentTabIndex() const { return _currentTabIndex; }

void EditorModel::connectCompilerService() {
  if (_compilerService.isNull()) {
    return;
  }
  connect(_compilerService, &CompilerService::atLeastRevisionAvailable, this,
          &EditorModel::handleCompilationRevisionAvailable);
  connect(this, &EditorModel::rowsInserted, _compilerService,
          &CompilerService::reset);
  connect(this, &EditorModel::rowsRemoved, _compilerService,
          &CompilerService::reset);
  connect(this, &EditorModel::mainTabIndexChanged, _compilerService,
          &CompilerService::reset);
}

void EditorModel::disconnectCompilerService() {
  if (_compilerService.isNull()) {
    return;
  }
  disconnect(_compilerService, nullptr, this, nullptr);
  disconnect(this, nullptr, _compilerService, nullptr);
}

QSharedPointer<CompilationResult> EditorModel::latestCompilationResult() const {
  if (_compilerService.isNull()) {
    qCritical() << "Tried to get the latest compilation result, but the "
                   "compiler service is null";
    return nullptr;
  }
  const auto result = _compilerService->result();
  if (result.isNull()) {
    qCritical() << "Compiler service has provided a null reference to result.";
  }
  return result;
}

void EditorModel::handleCompilationRevisionAvailable(int revision) {
  if (revision < _currentRunRevision) {
    return;
  }
  const auto compilationResult = latestCompilationResult();
  if (compilationResult.isNull()) {
    setExecutionState(ExecutionState::Idle);
    return;
  }
  const Theo::CodegenResult result = compilationResult->result();
  if (!result.generated_correctly) {
    setExecutionState(ExecutionState::Idle);
    if (_dialogService.isNull()) {
      qCritical() << "Tried to inform the user that the compilation failed, "
                     "but the dialog service is null";
      return;
    }
    _dialogService->addCompilationFailed(result);
    return;
  }
  startVirtualMachine(result.code);
}

void EditorModel::startVirtualMachine(const Theo::Program& program) {
  if (_virtualMachineService.isNull()) {
    setExecutionState(ExecutionState::Idle);
    qCritical() << "Tried to start vm, but virtual machine service is null";
    return;
  }
  setExecutionState(ExecutionState::Executing);
  startExecutionStateTimer();
  switch (_runningMode) {
    case Debug:
      _virtualMachineService->debug(program);
      return;
    case Default:
      _virtualMachineService->execute(program);
      return;
    default:
      return;
  }
}

VirtualMachineService* EditorModel::virtualMachineService() const {
  return _virtualMachineService.data();
}

void EditorModel::setVirtualMachineService(
    VirtualMachineService* virtualMachineService) {
  if (_virtualMachineService != virtualMachineService) {
    disconnectVirtualMachineService();
    _virtualMachineService = QPointer(virtualMachineService);
    emit virtualMachineServiceChanged();
    connectVirtualMachineService();
  }
}

void EditorModel::connectVirtualMachineService() {
  if (_virtualMachineService.isNull()) {
    return;
  }
  connect(_virtualMachineService, &VirtualMachineService::variablesStateChanged,
          this, &EditorModel::handleVirtualMachineVariableStateChanged);
  connect(_virtualMachineService,
          &VirtualMachineService::executionFailedForInternalReason, this,
          &EditorModel::displayExecutionFailedForInternalReason);
  connect(_virtualMachineService,
          &VirtualMachineService::executionFailedForInternalReason, this,
          &EditorModel::handleExecutionCompleted);
  connect(_virtualMachineService, &VirtualMachineService::executionCompleted,
          this, &EditorModel::handleExecutionCompleted);
}

void EditorModel::displayExecutionFailedForInternalReason() {
  if (_dialogService.isNull()) {
    qCritical()
        << "Tried to inform the user that the execution failed because of some "
           "intrnal programming fault, but the dialog service is null";
    return;
  }
  _dialogService->addExecutionFailedForInternalReason();
}

void EditorModel::disconnectVirtualMachineService() {
  if (_virtualMachineService.isNull()) {
    return;
  }
  disconnect(_virtualMachineService, nullptr, this, nullptr);
}

void EditorModel::handleExecutionCompleted() {
  qInfo() << "Execution completed";
  setExecutionState(ExecutionState::Idle);
}

void EditorModel::setRunningMode(RunningMode runningMode) {
  if (_runningMode != runningMode) {
    _runningMode = runningMode;
    emit runningModeChanged(runningMode);
  }
}

EditorModel::RunningMode EditorModel::runningMode() const {
  return _runningMode;
}

int EditorModel::cursorPositionAt(int index) const {
  const auto tabOptional = tabAt(index);
  if (!tabOptional.has_value() || tabOptional.value().isNull()) {
    return 0;
  }
  const auto tab = tabOptional.value();
  return tab->cursorPosition();
}

int EditorModel::cursorLineNumberAt(int index) const {
  const auto tabOptional = tabAt(index);
  if (!tabOptional.has_value() || tabOptional.value().isNull()) {
    return 1;
  }
  const auto tab = tabOptional.value();
  return tab->cursorLineNumber();
}

bool positionOutOfRange(QPointer<QTextDocument> textDocument, int position) {
  if (textDocument.isNull() || position < 0) {
    return true;
  }
  QTextBlock lastBlock = textDocument->lastBlock();
  const int lastPosition =
      qMax(lastBlock.position(), lastBlock.position() + lastBlock.length() - 1);
  return position > lastPosition;
}

bool EditorModel::setCursorPositionAt(int index, int position) {
  const auto tabOptional = tabAt(index);
  if (!tabOptional.has_value() || tabOptional.value().isNull()) {
    return false;
  }
  auto tab = tabOptional.value();
  if (tab->cursorPosition() == position || position < 0) {
    return false;
  }
  if (tab->textDocument().isNull() || tab->textDocument()->isEmpty()) {
    const bool changed = tab->cursorPosition() != 0;
    tab->setCursorPosition(0);
    return changed;
  }
  const auto textDocument = tab->textDocument();
  if (positionOutOfRange(textDocument, position)) {
    return false;
  }
  tab->setCursorPosition(position);
  return true;
}

void EditorModel::updateLineNumberFromCursorPositionAt(int index) {
  const auto tabOptional = tabAt(index);
  if (!tabOptional.has_value() || tabOptional.value().isNull()) {
    return;
  }
  auto tab = tabOptional.value();
  const int cursorPosition = tab->cursorPosition();
  const auto textDocument = tab->textDocument();
  if (textDocument.isNull() ||
      positionOutOfRange(textDocument, cursorPosition)) {
    return;
  }
  if (cursorPosition == 0) {
    setLineNumberAt(index, 1);
    return;
  }
  const QTextBlock block = textDocument->findBlock(cursorPosition);
  setLineNumberAt(index, block.blockNumber() + 1);
}

void EditorModel::updateCursorPositionFromLineNumberAt(int index) {
  const auto tabOptional = tabAt(index);
  if (!tabOptional.has_value() || tabOptional.value().isNull()) {
    return;
  }
  auto tab = tabOptional.value();
  const int lineNumber = tab->cursorLineNumber();
  const auto textDocument = tab->textDocument();
  if (textDocument.isNull() || lineNumber > textDocument->blockCount()) {
    return;
  }
  const QTextBlock block = textDocument->findBlockByNumber(lineNumber);
  setCursorPositionWithDisplayRoleAt(index, block.position());
  setCursorPositionWithEditRoleAt(index, block.position());
}

bool EditorModel::setCursorPositionVariantAt(int index, const QVariant& value,
                                             int role) {
  if (value.isValid() && value.canConvert<int>()) {
    const int position = value.toInt();
    switch (role) {
      case CursorPositionRole:
        setCursorPositionWithDisplayRoleAt(index, position);
        updateLineNumberFromCursorPositionAt(index);
        return true;
      case CursorPositionEditRole:
        setCursorPositionWithEditRoleAt(index, position);
        updateLineNumberFromCursorPositionAt(index);
        return true;
      default:
        return false;
    }
  }
  return false;
}

void EditorModel::setCursorPositionWithEditRoleAt(int index, int position) {
  setCursorPositionWithRoleAt(index, position, CursorPositionEditRole);
}

void EditorModel::setCursorPositionWithDisplayRoleAt(int index, int position) {
  setCursorPositionWithRoleAt(index, position, CursorPositionRole);
}

void EditorModel::setCursorPositionWithRoleAt(int index, int position,
                                              EditorModelRole role) {
  const bool positionChanged = setCursorPositionAt(index, position);
  if (positionChanged) {
    const QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, {role});
  }
}

bool EditorModel::setLineNumberAt(int index, int lineNumber) {
  const auto tabOptional = tabAt(index);
  if (!tabOptional.has_value() || tabOptional.value().isNull()) {
    return false;
  }
  auto tab = tabOptional.value();
  if (tab->cursorLineNumber() == lineNumber || lineNumber < 1) {
    return false;
  }
  tab->setCursorLineNumber(lineNumber);
  const QModelIndex modelIndex = createIndex(index, 0);
  emit dataChanged(modelIndex, modelIndex, {CursorLineNumberRole});
  return true;
}

void EditorModel::startExecutionStateTimer() {
  switch (_executionState) {
    case ExecutionState::Compiling:
      _executionStateTimer.setInterval(_compilationTimeoutMs);
      break;
    case ExecutionState::Executing:
      _executionStateTimer.setInterval(_executionTimeoutMs);
      break;
    default:
      return;
  }
  _executionStateTimer.setSingleShot(true);
  _executionStateTimer.start();
}

int EditorModel::executionTimeoutMs() const { return _executionTimeoutMs; }

void EditorModel::setExecutionTimeoutMs(int value) {
  if (_executionTimeoutMs != value) {
    _executionTimeoutMs = value;
    emit executionTimeoutMsChanged();
  }
}

int EditorModel::compilationTimeoutMs() const { return _compilationTimeoutMs; }

void EditorModel::setCompilationTimeoutMs(int value) {
  if (_compilationTimeoutMs != value) {
    _compilationTimeoutMs = value;
    emit compilationTimeoutMsChanged();
  }
}

void EditorModel::setExecutionState(ExecutionState executionState) {
  if (_executionState != executionState) {
    _executionState = executionState;
    emit executionStateChanged();
  }
}

void EditorModel::handleVirtualMachineVariableStateChanged() {
  if (_executionState == ExecutionState::Executing) {
    setExecutionState(ExecutionState::Halt);
  }
}

ExecutionState EditorModel::executionState() const { return _executionState; }
