#include <qtpreprocessorsupport.h>

#include <QtGlobal>
#include <algorithm>
#include <iterator>

#include "variablesstatemodel.hpp"
#include "virtualmachineservice.hpp"

VariablesStateModel::VariablesStateModel(QObject* parent)
    : QAbstractListModel(parent) {}

VariablesStateModel::~VariablesStateModel() {}

int VariablesStateModel::rowCount(const QModelIndex& parent) const {
  Q_UNUSED(parent)
  if (_virtualMachineService.isNull()) {
    return 0;
  }
  return _virtualMachineService->variablesState().count();
}

QVariant VariablesStateModel::data(const QModelIndex& index, int role) const {
  switch (role) {
    case VariableNameRole:
      return variableNameAt(index.row());
    case VariableValueRole:
      return variableValueAt(index.row());
    case VariableNameColumnWidthRole:
      return variableNameColumnWidth();
    case VariableValueColumnWidthRole:
      return variableValueColumnWidth();
    case Qt::DisplayRole:
      return displayRoleAt(index.row());
  }
  return QVariant();
}

QHash<int, QByteArray> VariablesStateModel::roleNames() const {
  return QHash<int, QByteArray>{
      {Qt::DisplayRole, "display"},
      {VariableNameRole, "variableName"},
      {VariableValueRole, "variableValue"},
      {VariableNameColumnWidthRole, "variableNameColumnWidth"},
      {VariableValueColumnWidthRole, "variableValueColumnWidth"},
      {VariableNameColumnImplicitWidthRole, "variableNameColumnImplicitWidth"},
      {VariableValueColumnImplicitWidthRole,
       "variableValueColumnImplicitWidth"},
  };
}

bool VariablesStateModel::setData(const QModelIndex& index,
                                  const QVariant& value, int role) {
  switch (role) {
    case VariableNameColumnImplicitWidthRole:
      return setVariableNameColumnImplicitWidthVariant(index.row(), value);
    case VariableValueColumnImplicitWidthRole:
      return setVariableValueColumnImplicitWidthVariant(index.row(), value);
  }
  return false;
}

VirtualMachineService* VariablesStateModel::virtualMachineService() const {
  return _virtualMachineService.data();
}

void VariablesStateModel::setVirtualMachineService(
    VirtualMachineService* virtualMachineService) {
  if (_virtualMachineService != virtualMachineService) {
    disconnectVirtualMachineService();
    _virtualMachineService = QPointer(virtualMachineService);
    emit virtualMachineServiceChanged();
    connectVirtualMachineService();
  }
}

void VariablesStateModel::setVariableNameColumnWidth(int value) {
  if (_variableNameColumnWidth != value) {
    _variableNameColumnWidth = value;
    emit variableNameColumnWidthChanged();
    if (_variables.empty()) {
      return;
    }
    const QModelIndex firstIndex = createIndex(0, 0);
    const QModelIndex lastIndex = createIndex(_variables.length() - 1, 0);
    emit dataChanged(firstIndex, lastIndex, {VariableNameColumnWidthRole});
  }
}

void VariablesStateModel::setVariableValueColumnWidth(int value) {
  if (_variableValueColumnWidth != value) {
    _variableValueColumnWidth = value;
    emit variableValueColumnWidthChanged();
    if (_variables.empty()) {
      return;
    }
    const QModelIndex firstIndex = createIndex(0, 0);
    const QModelIndex lastIndex = createIndex(_variables.length() - 1, 0);
    emit dataChanged(firstIndex, lastIndex, {VariableValueColumnWidthRole});
  }
}

void VariablesStateModel::updateVariableNameColumnWidth() {
  if (_variables.empty()) {
    setVariableNameColumnWidth(_variableNameHeaderImplicitWidth);
    return;
  }
  const auto maxWidthIterator = std::max_element(
      _variables.begin(), _variables.end(),
      [](const VariableState& v1, const VariableState& v2) -> bool {
        return v1.nameColumnImplicitWidth < v2.nameColumnImplicitWidth;
      });
  const VariableState& variableStateMaxWidth = *maxWidthIterator;
  const int maxWidth = qMax(variableStateMaxWidth.nameColumnImplicitWidth,
                            _variableNameHeaderImplicitWidth);
  setVariableNameColumnWidth(maxWidth);
}

void VariablesStateModel::updateVariableValueColumnWidth() {
  if (_variables.empty()) {
    setVariableValueColumnWidth(_variableValueHeaderImplicitWidth);
    return;
  }
  const auto maxWidthIterator = std::max_element(
      _variables.begin(), _variables.end(),
      [](const VariableState& v1, const VariableState& v2) -> bool {
        return v1.valueColumnImplicitWidth < v2.valueColumnImplicitWidth;
      });
  const VariableState& variableStateMaxWidth = *maxWidthIterator;
  const int maxWidth = qMax(variableStateMaxWidth.valueColumnImplicitWidth,
                            _variableValueHeaderImplicitWidth);
  setVariableValueColumnWidth(maxWidth);
}

void VariablesStateModel::updateVariablesState() {
  if (_virtualMachineService.isNull()) {
    return;
  }
  const auto variablesState(_virtualMachineService->variablesState());
  QList<VariableState> variablesToAppend;
  QList<VariableState>::iterator variablesIterator = _variables.begin();
  const int numberOfItemsToUpdate =
      qMin(variablesState.size(), _variables.size());
  for (const QString& variableName : variablesState.keys()) {
    if (variablesIterator == _variables.end()) {
      variablesToAppend.append(
          VariableState(variableName, variablesState[variableName]));
      continue;
    }
    VariableState& currentVariableState = *variablesIterator;
    currentVariableState.name = variableName;
    currentVariableState.value = variablesState[variableName];
    ++variablesIterator;
  }
  if (numberOfItemsToUpdate > 0) {
    const QModelIndex first(createIndex(0, 0));
    const QModelIndex last(createIndex(numberOfItemsToUpdate - 1, 0));
    emit dataChanged(first, last,
                     {VariableNameRole, VariableValueRole, Qt::DisplayRole});
  }
  if (!variablesToAppend.isEmpty()) {
    const int first = _variables.size();
    const int last = first + variablesToAppend.size() - 1;
    beginInsertRows(QModelIndex(), first, last);
    _variables.append(variablesToAppend);
    endInsertRows();
  }
  if (variablesState.size() < _variables.size()) {
    const int first = variablesState.size();
    const int last = _variables.size() - 1;
    beginRemoveRows(QModelIndex(), first, last);
    _variables.remove(first, last - first + 1);
    endRemoveRows();
  }
}

QString VariablesStateModel::variableNameAt(int index) const {
  if (indexOutOfRange(index)) {
    return QString();
  }
  return _variables.at(index).name;
}

int VariablesStateModel::variableValueAt(int index) const {
  if (indexOutOfRange(index)) {
    return 0;
  }
  return _variables.at(index).value;
}

QString VariablesStateModel::displayRoleAt(int index) const {
  if (indexOutOfRange(index)) {
    return QString();
  }
  const VariableState state = _variables.at(index);
  return QString("%1: %2").arg(state.name).arg(state.value);
}

int VariablesStateModel::variableNameColumnWidth() const {
  return _variableNameColumnWidth;
}

int VariablesStateModel::variableValueColumnWidth() const {
  return _variableValueColumnWidth;
}

bool VariablesStateModel::setVariableNameColumnImplicitWidthVariant(
    int index, const QVariant& value) {
  if (indexOutOfRange(index)) {
    return false;
  }
  if (value.isValid() && value.canConvert<int>()) {
    const int width = value.toInt();
    _variables[index].nameColumnImplicitWidth = width;
    updateVariableNameColumnWidth();
    return true;
  }
  return false;
}

bool VariablesStateModel::setVariableValueColumnImplicitWidthVariant(
    int index, const QVariant& value) {
  if (indexOutOfRange(index)) {
    return false;
  }
  if (value.isValid() && value.canConvert<int>()) {
    const int width = value.toInt();
    _variables[index].valueColumnImplicitWidth = width;
    updateVariableValueColumnWidth();
    return true;
  }
  return false;
}

bool VariablesStateModel::indexOutOfRange(int index) const {
  return index < 0 || index >= _variables.size();
}

void VariablesStateModel::setValueAt(int index, int value) {
  if (indexOutOfRange(index)) {
    return;
  }
  _variables[index].value = value;
  const QModelIndex modelIndex = createIndex(index, 0);
  emit dataChanged(modelIndex, modelIndex,
                   {VariableValueRole, Qt::DisplayRole});
}

void VariablesStateModel::setValueAt(QList<VariableState>::iterator iterator,
                                     int value) {
  const auto index(std::distance(_variables.begin(), iterator));
  setValueAt(index, value);
}

QList<VariableState>::iterator VariablesStateModel::insertVariableStateAt(
    QList<VariableState>::iterator iterator, const VariableState& value) {
  const auto index(std::distance(_variables.begin(), iterator));
  beginInsertRows(QModelIndex(), index, index);
  const auto returnValue = _variables.insert(iterator, value);
  return returnValue;
  endInsertRows();
}

void VariablesStateModel::appendVariableState(const VariableState& state) {
  const int index = _variables.size();
  beginInsertRows(QModelIndex(), index, index);
  _variables.append(state);
  endInsertRows();
}

void VariablesStateModel::removeVariableStateAt(int index) {
  if (indexOutOfRange(index)) {
    return;
  }
  beginRemoveRows(QModelIndex(), index, index);
  _variables.remove(index);
  endRemoveRows();
}

void VariablesStateModel::connectVirtualMachineService() {
  if (_virtualMachineService.isNull()) {
    return;
  }
  connect(_virtualMachineService, &VirtualMachineService::variablesStateChanged,
          this, &VariablesStateModel::updateVariablesState);
}

void VariablesStateModel::disconnectVirtualMachineService() {
  if (_virtualMachineService.isNull()) {
    return;
  }
  disconnect(_virtualMachineService, nullptr, this, nullptr);
  disconnect(this, nullptr, _virtualMachineService, nullptr);
}

void VariablesStateModel::setVariableNameHeaderImplicitWidth(int width) {
  if (_variableNameHeaderImplicitWidth != width) {
    _variableNameHeaderImplicitWidth = width;
    updateVariableNameColumnWidth();
  }
}

void VariablesStateModel::setVariableValueHeaderImplicitWidth(int width) {
  if (_variableValueHeaderImplicitWidth != width) {
    _variableValueHeaderImplicitWidth = width;
    updateVariableValueColumnWidth();
  }
}
