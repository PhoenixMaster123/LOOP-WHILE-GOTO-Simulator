#include <qtpreprocessorsupport.h>

#include <QtLogging>

#include "lineinfocolumnmodel.hpp"

LineInfoColumnModel::LineInfoColumnModel(QObject* parent)
    : QAbstractListModel(parent) {}

void LineInfoColumnModel::updateLines(const int newLineCount) {
  const int currentLineCount = _lineCount;
  if (newLineCount == currentLineCount) {
    return;
  }
  const QModelIndex firstLine = createIndex(0, 0);
  const QModelIndex lastLine = createIndex(currentLineCount - 1, 0);
  emit dataChanged(firstLine, lastLine);
  const int lineCountDifference = newLineCount - currentLineCount;
  if (lineCountDifference > 0) {
    insertLines(currentLineCount, lineCountDifference);
  } else {
    removeLines(currentLineCount, -lineCountDifference);
  }
  setLineCount(newLineCount);
}

void LineInfoColumnModel::insertLines(const int position,
                                      const int numberOfLines) {
  beginInsertRows(QModelIndex(), position, position + numberOfLines - 1);
  endInsertRows();
}

void LineInfoColumnModel::removeLines(const int position,
                                      const int numberOfLines) {
  beginRemoveRows(QModelIndex(), position - numberOfLines, position - 1);
  endRemoveRows();
}

bool LineInfoColumnModel::isTextDocumentUnexpectedlyNull() const {
  if (_textDocument == nullptr) {
    qCritical() << "Programming mistake detected. Member _textDocument is "
                   "expected to be non null but it is null!";
    return true;
  }
  return false;
}

QVariant LineInfoColumnModel::data(const QModelIndex& index, int role) const {
  switch (role) {
    case Qt::DisplayRole:
    case LineNumberRole:
      return index.row() + 1;
    case BreakPointActiveRole:
      return getBreakPointActiveAt(index.row());
    case LineHeightRole:
      return getLineHeightAt(index.row());
  }
  return QVariant();
}

qreal LineInfoColumnModel::getLineHeightAt(const int blockNumber) const {
  if (isTextDocumentUnexpectedlyNull()) {
    return 0;
  }
  QTextBlock block = _textDocument->findBlockByNumber(blockNumber);
  if (block.blockNumber() >= 0 && block.layout() != nullptr) {
    return block.layout()->boundingRect().height();
  }
  return 0;
}

bool LineInfoColumnModel::getBreakPointActiveAt(const int blockNumber) const {
  if (isTextDocumentUnexpectedlyNull()) {
    return false;
  }
  QTextBlock block = _textDocument->findBlockByNumber(blockNumber);
  if (block.blockNumber() >= 0 && block.userData() != nullptr) {
    const LineInfoUserData* userData =
        static_cast<LineInfoUserData*>(block.userData());
    return userData->breakPointActive();
  }
  return false;
}

bool LineInfoColumnModel::setData(const QModelIndex& index,
                                  const QVariant& value, int role) {
  switch (role) {
    case BreakPointActiveRole:
      return setBreakPointActiveAt(index, value);
    default:
      return false;
  }
}

int LineInfoColumnModel::rowCount(const QModelIndex& parent) const {
  Q_UNUSED(parent)
  return lineCount();
}

Qt::ItemFlags LineInfoColumnModel::flags(const QModelIndex& index) const {
  Q_UNUSED(index)
  return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsEditable |
         Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemNeverHasChildren;
}

bool LineInfoColumnModel::setBreakPointActiveAt(const QModelIndex& index,
                                                const QVariant& value) {
  if (isTextDocumentUnexpectedlyNull()) {
    return false;
  }
  const int blockNumber = index.row();

  if (blockNumber < 0 || blockNumber >= _lineCount || !value.isValid() ||
      !value.canConvert<bool>()) {
    return false;
  }

  QTextBlock block = _textDocument->findBlockByNumber(blockNumber);
  if (block.blockNumber() < 0) {
    return false;
  }

  const bool nextValue = value.toBool();

  LineInfoUserData* userData;
  if (block.userData() == nullptr) {
    userData = new LineInfoUserData();
    block.setUserData(userData);
  } else {
    userData = static_cast<LineInfoUserData*>(block.userData());
  }

  const bool currentValue = userData->breakPointActive();
  if (nextValue != currentValue) {
    userData->setBreakPointActive(nextValue);
    emit dataChanged(index, index);
  }
  return true;
}

QHash<int, QByteArray> LineInfoColumnModel::roleNames() const {
  return QHash<int, QByteArray>{{Qt::DisplayRole, "display"},
                                {LineNumberRole, "lineNumber"},
                                {BreakPointActiveRole, "breakPointActive"},
                                {LineHeightRole, "lineHeight"}};
}

QQuickTextDocument* LineInfoColumnModel::quickTextDocument() const {
  return _quickTextDocument;
}

void LineInfoColumnModel::setQuickTextDocument(
    QQuickTextDocument* textDocument) {
  if (_quickTextDocument != textDocument) {
    _quickTextDocument = textDocument;
    emit quickTextDocumentChanged();
  }
  if (_quickTextDocument != nullptr) {
    setTextDocument(_quickTextDocument->textDocument());
  } else {
    setTextDocument(nullptr);
  }
}

void LineInfoColumnModel::disconnectTextDocumentSignals() {
  if (_textDocument != nullptr) {
    disconnect(_textDocument, nullptr, this, nullptr);
  }
}

void LineInfoColumnModel::connectTextDocumentSignals() {
  if (_textDocument != nullptr) {
    connect(_textDocument, &QTextDocument::blockCountChanged, this,
            &LineInfoColumnModel::updateLines);
  }
}

void LineInfoColumnModel::setTextDocument(QTextDocument* textDocument) {
  if (textDocument != _textDocument) {
    disconnectTextDocumentSignals();
    _textDocument = textDocument;
    connectTextDocumentSignals();
  }
  if (_textDocument == nullptr) {
    setLineCount(0);
  } else {
    updateLines(_textDocument->blockCount());
  }
}

void LineInfoColumnModel::setLineCount(const int newLineCount) {
  if (_lineCount != newLineCount) {
    _lineCount = newLineCount;
    emit lineCountChanged(newLineCount);
  }
}

QList<int> LineInfoColumnModel::getLineNumbersWithActiveBreakpoints() const {
  QList<int> returnValue;
  if (isTextDocumentUnexpectedlyNull()) {
    return returnValue;
  }
  for (QTextBlock block = _textDocument->begin(); block != _textDocument->end();
       block = block.next()) {
    if (block.userData() != nullptr &&
        static_cast<LineInfoUserData*>(block.userData())->breakPointActive()) {
      returnValue.push_back(block.blockNumber() + 1);
    }
  }
  return returnValue;
}
