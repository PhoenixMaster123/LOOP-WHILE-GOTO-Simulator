#ifndef _THEOIDE_CONTROLS_EDITORMODEL_
#define _THEOIDE_CONTROLS_EDITORMODEL_

#include <qtimer.h>
#include <qtmetamacros.h>

#include <QAbstractItemModel>
#include <QDir>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QPointer>
#include <QQuickTextDocument>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QTextDocument>
#include <QUrl>
#include <QVariant>
#include <QtQmlIntegration>
#include <optional>
#include <stdexcept>

#include "VM/include/program.hpp"
#include "compilerservice.hpp"
#include "dialogservice.hpp"
#include "executionstate.hpp"
#include "filesystemservice.hpp"
#include "virtualmachineservice.hpp"

using FileOptional = std::optional<QSharedPointer<QFile>>;

class NoMainTabError : public std::runtime_error {
 public:
  NoMainTabError() : std::runtime_error("Main tab is unset") {}
  ~NoMainTabError() {}
};

class EmptyMainTabError : public std::runtime_error {
 public:
  EmptyMainTabError() : std::runtime_error("Main tab is empty") {}
  ~EmptyMainTabError() {}
};

class MainTabInvalidStateError : public std::runtime_error {
 public:
  MainTabInvalidStateError()
      : std::runtime_error("Main tab is in invalid state") {}
  ~MainTabInvalidStateError() {}
};

class TabModel {
 public:
  TabModel(const QString& name, FileOptional file, const QString& storedText)
      : _textDocument(nullptr),
        _file(file),
        _name(name),
        _storedText(storedText) {}
  TabModel(const QString& name) : TabModel(name, std::nullopt, QString()) {}
  TabModel(const TabModel& tabModel)
      : _textDocument(tabModel._textDocument),
        _file(tabModel._file),
        _name(tabModel._name),
        _storedText(tabModel._storedText),
        _activeBreakPoints(tabModel._activeBreakPoints) {}

  QString name() const { return _name; }
  QString displayName() const {
    if (isModified()) {
      return name() + " *";
    }
    return name();
  }
  FileOptional file() const { return _file; }
  QString storedText() const { return _storedText; }
  QList<int> activeBreakPoints() const { return _activeBreakPoints; }
  QPointer<QTextDocument> textDocument() const { return _textDocument; }
  bool isModified() const {
    return _textDocument && _textDocument->isModified();
  }
  bool isTemporary() const { return !_file.has_value(); }
  int cursorLineNumber() const { return _cursorLineNumber; }
  int cursorPosition() const { return _cursorPosition; }

  void setName(const QString& name) { _name = name; }
  void setStoredText(const QString& text) { _storedText = text; }
  void setActiveBreakPoints(QList<int> activeBreakPoints) {
    _activeBreakPoints = activeBreakPoints;
  }
  void setFile(FileOptional file) { _file = file; }
  void setTextDocument(QTextDocument* textDocument) {
    _textDocument = textDocument;
  }
  void setCursorLineNumber(int cursorLineNumber) {
    _cursorLineNumber = cursorLineNumber;
  }
  void setCursorPosition(int cursorPosition) {
    _cursorPosition = cursorPosition;
  }

 private:
  QPointer<QTextDocument> _textDocument;
  std::optional<QSharedPointer<QFile>> _file;
  QString _name;
  QString _storedText;
  QList<int> _activeBreakPoints;
  int _cursorLineNumber = 1;
  int _cursorPosition = 0;
};

using TabModelOptional = std::optional<QSharedPointer<TabModel>>;

class EditorModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(int mainTabIndex READ mainTabIndex WRITE setMainTabIndex NOTIFY
                 mainTabIndexChanged)
  Q_PROPERTY(int currentTabIndex READ currentTabIndex WRITE setCurrentTabIndex
                 NOTIFY currentTabIndexChanged)
  Q_PROPERTY(FileSystemService* fileSystemService READ fileSystemService WRITE
                 setFileSystemService NOTIFY fileSystemServiceChanged)
  Q_PROPERTY(DialogService* dialogService READ dialogService WRITE
                 setDialogService NOTIFY dialogServiceChanged)
  Q_PROPERTY(CompilerService* compilerService READ compilerService WRITE
                 setCompilerService NOTIFY compilerServiceChanged)
  Q_PROPERTY(
      VirtualMachineService* virtualMachineService READ virtualMachineService
          WRITE setVirtualMachineService NOTIFY virtualMachineServiceChanged)
  Q_PROPERTY(EditorModel::RunningMode runningMode READ runningMode NOTIFY
                 runningModeChanged)
  Q_PROPERTY(int compilationTimeoutMs READ compilationTimeoutMs WRITE
                 setCompilationTimeoutMs NOTIFY compilationTimeoutMsChanged)
  Q_PROPERTY(int executionTimeoutMs READ executionTimeoutMs WRITE
                 setExecutionTimeoutMs NOTIFY executionTimeoutMsChanged)
  Q_PROPERTY(ExecutionState executionState READ executionState NOTIFY
                 executionStateChanged)
  QML_ELEMENT
 public:
  EditorModel(QObject* parent = nullptr);
  ~EditorModel();
  enum EditorModelRole {
    StoredTabTextRole = Qt::UserRole + 1,
    TabNameRole,
    DisplayTabNameRole,
    IsModifiedRole,
    IsTemporaryRole,
    TextDocumentRole,
    OpenRole,
    IsReadOnlyRole,
    BackgroundCompilationTimerRole,
    CursorPositionRole,
    CursorPositionEditRole,
    CursorLineNumberRole,
  };
  enum RunningMode {
    Default,
    Debug,
  };
  Q_ENUM(RunningMode)
  CompilerService* compilerService() const;
  DialogService* dialogService() const;
  FileSystemService* fileSystemService() const;
  VirtualMachineService* virtualMachineService() const;
  QHash<int, QByteArray> roleNames() const override;
  QVariant data(const QModelIndex& index, int role) const override;
  bool setData(const QModelIndex& index, const QVariant& data,
               int role) override;
  int mainTabIndex() const;
  int currentTabIndex() const;
  int executionTimeoutMs() const;
  int compilationTimeoutMs() const;
  ExecutionState executionState() const;
  int rowCount(const QModelIndex& index) const override;
  RunningMode runningMode() const;
  void saveTabAt(qsizetype index);
  void closeTabAt(qsizetype index);

  Q_INVOKABLE
  void openFile(const QUrl& url);
  Q_INVOKABLE
  void saveAllTabs();
  Q_INVOKABLE
  void createNewTab();
  Q_INVOKABLE
  void runScript();
  Q_INVOKABLE
  void runScriptInDebugMode();

 public slots:
  void createTabFromFile(QSharedPointer<QFile> file, const QString& storedText);
  void displayFileReadFailure(QSharedPointer<QFile> file,
                              const FileError& error);
  void setCompilerService(CompilerService* compilerservice);
  void setDialogService(DialogService* dialogService);
  void setFileSystemService(FileSystemService* fileSystemService);
  void setVirtualMachineService(VirtualMachineService* virtualMachineService);
  void setMainTabIndex(int index);
  void setCurrentTabIndex(int index);
  void displayFileReadMaxReadFileSizeExceededFailure(
      QSharedPointer<QFile> file, const MaxReadFileSizeExceededError& error);
  void displayFileReadFileReadPermissionFailure(QSharedPointer<QFile> file,
                                                const FileReadError& error);
  void displayFileReadFileDoesNotExistFailure(
      QSharedPointer<QFile> file, const FileDoesNotExistError& error);
  void stopExecution();
  void setExecutionTimeoutMs(int value);
  void setCompilationTimeoutMs(int value);

 signals:
  void currentTabIndexChanged(int index);
  void compilerServiceChanged();
  void dialogServiceChanged();
  void fileSystemServiceChanged();
  void virtualMachineServiceChanged();
  void mainTabIndexChanged(int index);
  void runningModeChanged(RunningMode runningMode);
  void executionTimeoutMsChanged();
  void compilationTimeoutMsChanged();
  void executionStateChanged();

 protected slots:
  void updateMainTabIndex();
  void updateAllTabNames();
  void handleCompilationRevisionAvailable(int revision);
  void handleExecutionCompleted();
  void displayExecutionFailedForInternalReason();
  void handleVirtualMachineVariableStateChanged();

 private:
  int _mainTabIndex = -1;
  int _currentTabIndex = -1;
  QPointer<FileSystemService> _fileSystemService;
  QPointer<DialogService> _dialogService;
  QPointer<CompilerService> _compilerService;
  QPointer<VirtualMachineService> _virtualMachineService;
  QList<QSharedPointer<TabModel>> _tabs;
  mutable QMutex _tabsMutex;
  mutable QMutex _temporaryTabIndexesMutex;
  QMap<QSharedPointer<TabModel>, int> _temporaryTabIndexes;
  long long _currentRunRevision = 0;
  RunningMode _runningMode = Default;
  ExecutionState _executionState = ExecutionState::Idle;
  QTimer _executionStateTimer;
  int _compilationTimeoutMs = 5000;
  int _executionTimeoutMs = 5000;

  QString tabNameAt(qsizetype index) const;
  QString storedTabTextAt(qsizetype index) const;
  bool isTabModifiedAt(qsizetype index) const;
  bool isTabTemporaryAt(qsizetype index) const;
  bool isTabReadOnlyAt(qsizetype index) const;
  bool tabIndexOutOfRange(qsizetype index) const;
  TabModelOptional mainTab() const;
  TabModelOptional tabAt(qsizetype index) const;
  QString createTabNameRelativeToMainTab(QSharedPointer<QFile> file) const;
  void saveTab(QSharedPointer<TabModel> tab);
  QString displayTabNameAt(qsizetype index) const;
  int appendTemporaryTabIndex(QSharedPointer<TabModel> tab);
  void removeTemporaryTabIndex(QSharedPointer<TabModel> tab);
  void disconnectFileSystemService();
  void connectFileSystemService();
  void connectCompilerService();
  void disconnectCompilerService();
  bool setTextDocumentVariantAt(qsizetype index, const QVariant& data);
  bool setOpenAt(qsizetype index, const QVariant& data);
  CompilationTask createCompilationTaskFromTabContent() const;
  void updateTabNameAt(qsizetype index);
  bool tryPushCompilationTask();
  bool compilationPreconditionsFulfilled() const;
  bool isMainTabIndex(int index) const;
  void connectVirtualMachineService();
  void disconnectVirtualMachineService();
  void startVirtualMachine(const Theo::Program& program);
  void setRunningMode(RunningMode runningMode);
  QSharedPointer<CompilationResult> latestCompilationResult() const;
  int cursorPositionAt(int index) const;
  int cursorLineNumberAt(int index) const;
  bool setCursorPositionAt(int index, int position);
  bool setCursorPositionVariantAt(int index, const QVariant& value, int role);
  void setCursorPositionWithEditRoleAt(int index, int position);
  void setCursorPositionWithDisplayRoleAt(int index, int position);
  void setCursorPositionWithRoleAt(int index, int position,
                                   EditorModelRole role);
  bool setLineNumberAt(int index, int lineNumber);
  void updateCursorPositionFromLineNumberAt(int index);
  void updateLineNumberFromCursorPositionAt(int index);
  void startExecutionStateTimer();
  void setExecutionState(ExecutionState executionState);
  void stopVirtualMachine();
  void stopCompiler();
};

#endif
