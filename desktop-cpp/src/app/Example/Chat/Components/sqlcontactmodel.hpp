#include <QtQml/qqmlregistration.h>

#include <QSqlQueryModel>

class SqlContactModel : public QSqlQueryModel {
  Q_OBJECT
  QML_ELEMENT
 public:
  SqlContactModel(QObject *parent = nullptr);
};
