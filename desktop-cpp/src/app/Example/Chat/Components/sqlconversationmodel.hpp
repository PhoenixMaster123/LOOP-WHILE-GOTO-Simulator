#include <QtQml/qqmlregistration.h>

#include <QSqlTableModel>

class SqlConversationModel : public QSqlTableModel {
  Q_OBJECT
  Q_PROPERTY(QString recipient READ recipient WRITE setRecipient NOTIFY
                 recipientChanged);
  QML_ELEMENT

 public:
  SqlConversationModel(QObject *parent = nullptr);
  enum SqlConversationModelRoles {
    AuthorRole = Qt::UserRole,
    RecipientRole,
    TimestampRole,
    MessageRole
  };

  QString recipient() const;
  void setRecipient(const QString &recipient);

  QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  Q_INVOKABLE void sendMessage(const QString &recipient,
                               const QString &message);

 signals:
  void recipientChanged();

 private:
  QString m_recipient;
};
