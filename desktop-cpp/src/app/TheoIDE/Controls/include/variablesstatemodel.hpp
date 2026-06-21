#ifndef _THEOIDE_CONTROLS_VARIABLESSTATEMODEL_
#define _THEOIDE_CONTROLS_VARIABLESSTATEMODEL_

#include <qnamespace.h>
#include <qtmetamacros.h>
#include <qvariant.h>

#include <QAbstractItemModel>
#include <QAbstractListModel>
#include <QByteArray>
#include <QHash>
#include <QObject>
#include <QPointer>
#include <QVariant>
#include <QtQmlIntegration>

#include "virtualmachineservice.hpp"

struct VariableState {
  VariableState(const QString& name, int value) : name(name), value(value) {}
  VariableState(const VariableState& instance)
      : name(instance.name),
        value(instance.value),
        nameColumnImplicitWidth(instance.nameColumnImplicitWidth),
        valueColumnImplicitWidth(instance.valueColumnImplicitWidth) {}
  QString name;
  int value;
  int nameColumnImplicitWidth = 0;
  int valueColumnImplicitWidth = 0;
};

class VariablesStateModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(
      VirtualMachineService* virtualMachineService READ virtualMachineService
          WRITE setVirtualMachineService NOTIFY virtualMachineServiceChanged)
  Q_PROPERTY(int variableNameColumnWidth READ variableNameColumnWidth NOTIFY
                 variableNameColumnWidthChanged)
  Q_PROPERTY(int variableValueColumnWidth READ variableValueColumnWidth NOTIFY
                 variableValueColumnWidthChanged)
  QML_ELEMENT
 public:
  VariablesStateModel(QObject* parent = nullptr);
  ~VariablesStateModel();
  enum VariablesStateModelRole {
    VariableNameRole = Qt::UserRole + 1,
    VariableValueRole,
    VariableNameColumnWidthRole,
    VariableValueColumnWidthRole,
    VariableNameColumnImplicitWidthRole,
    VariableValueColumnImplicitWidthRole,
  };
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;
  bool setData(const QModelIndex& index, const QVariant& value,
               int role = Qt::EditRole) override;
  VirtualMachineService* virtualMachineService() const;
  int variableNameColumnWidth() const;
  int variableValueColumnWidth() const;

 public slots:
  void setVirtualMachineService(VirtualMachineService* virtualMachineService);
  void setVariableNameHeaderImplicitWidth(int width);
  void setVariableValueHeaderImplicitWidth(int width);

 signals:
  void virtualMachineServiceChanged();
  void variableNameColumnWidthChanged();
  void variableValueColumnWidthChanged();

 protected slots:
  void setVariableNameColumnWidth(int value);
  void setVariableValueColumnWidth(int value);
  void updateVariableNameColumnWidth();
  void updateVariableValueColumnWidth();
  void updateVariablesState();

 private:
  QPointer<VirtualMachineService> _virtualMachineService;
  int _variableNameColumnWidth = 0;
  int _variableValueColumnWidth = 0;
  int _variableNameHeaderImplicitWidth = 0;
  int _variableValueHeaderImplicitWidth = 0;
  QList<VariableState> _variables;

  QString variableNameAt(int index) const;
  int variableValueAt(int index) const;
  QString displayRoleAt(int index) const;
  bool setVariableNameColumnImplicitWidthVariant(int index,
                                                 const QVariant& value);
  bool setVariableValueColumnImplicitWidthVariant(int index,
                                                  const QVariant& value);
  bool indexOutOfRange(int index) const;
  void setValueAt(int index, int value);
  void setValueAt(QList<VariableState>::iterator iterator, int value);
  QList<VariableState>::iterator insertVariableStateAt(
      QList<VariableState>::iterator iterator, const VariableState& value);
  void appendVariableState(const VariableState& state);
  void removeVariableStateAt(int index);
  void connectVirtualMachineService();
  void disconnectVirtualMachineService();
};

#endif
