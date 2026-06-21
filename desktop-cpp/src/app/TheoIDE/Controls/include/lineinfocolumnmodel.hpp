#ifndef _THEOIDE_CONTROLS_LINEINFOCOLUMNMODEL_
#define _THEOIDE_CONTROLS_LINEINFOCOLUMNMODEL_

#include <QAbstractListModel>
#include <QByteArray>
#include <QHash>
#include <QList>
#include <QModelIndex>
#include <QQuickTextDocument>
#include <QString>
#include <QTextBlockUserData>
#include <QTextDocument>
#include <QtQmlIntegration>

class LineInfoUserData : public QTextBlockUserData {
 public:
  LineInfoUserData() : QTextBlockUserData(), _breakPointActive(false) {}
  LineInfoUserData(const LineInfoUserData& userData)
      : QTextBlockUserData(userData),
        _breakPointActive(userData._breakPointActive) {}
  ~LineInfoUserData() {}

  bool breakPointActive() const { return _breakPointActive; }
  void setBreakPointActive(const bool value) { _breakPointActive = value; }

 private:
  bool _breakPointActive;
};

class LineInfoColumnModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(int lineCount READ lineCount NOTIFY lineCountChanged)
  Q_PROPERTY(QQuickTextDocument* textDocument READ quickTextDocument WRITE
                 setQuickTextDocument NOTIFY quickTextDocumentChanged)
  QML_ELEMENT
 public:
  LineInfoColumnModel(QObject* parent = nullptr);
  ~LineInfoColumnModel() = default;
  enum LineInfoColumnModelRoles {
    LineNumberRole = Qt::UserRole + 1,
    BreakPointActiveRole,
    LineHeightRole,
  };
  int lineCount() const { return _lineCount; };
  Q_INVOKABLE
  QList<int> getLineNumbersWithActiveBreakpoints() const;
  virtual QVariant data(const QModelIndex& index,
                        int role = Qt::DisplayRole) const override;
  virtual int rowCount(
      const QModelIndex& parent = QModelIndex()) const override;
  virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
  virtual bool setData(const QModelIndex& index, const QVariant& value,
                       int role = Qt::EditRole) override;
  virtual QHash<int, QByteArray> roleNames() const override;
  QQuickTextDocument* quickTextDocument() const;

 public slots:
  void setQuickTextDocument(QQuickTextDocument* textDocument);

 protected slots:
  void setTextDocument(QTextDocument* textDocument);
  void updateLines(const int newLineCount);
  void setLineCount(const int newLineCount);

 signals:
  void lineCountChanged(int newLineCount);
  void quickTextDocumentChanged();

 private:
  QQuickTextDocument* _quickTextDocument = nullptr;
  QTextDocument* _textDocument = nullptr;
  int _lineCount = 0;

  bool getBreakPointActiveAt(const int blockNumber) const;
  bool isTextDocumentUnexpectedlyNull() const;
  bool setBreakPointActiveAt(const QModelIndex& index, const QVariant& value);
  qreal getLineHeightAt(const int blockNumber) const;
  void connectTextDocumentSignals();
  void disconnectTextDocumentSignals();
  void insertLines(const int position, const int numberOfLines);
  void removeLines(const int position, const int numberOfLines);
};

#endif
