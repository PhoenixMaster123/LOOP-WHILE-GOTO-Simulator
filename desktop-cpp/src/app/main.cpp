#include <QGuiApplication>
#include <QIcon>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QSqlDatabase>
#include <QSqlError>
#include <QTranslator>
#include <QtCore>
#include <QtQml>

using namespace Qt::Literals::StringLiterals;

int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);
  app.setApplicationName("TheoIDE");
  app.setApplicationDisplayName("Theo IDE");
  app.setOrganizationName("theoide");
  app.setOrganizationDomain("Theo-IDE.github.io");

  QIcon::setThemeSearchPaths({":/icons"});
  QIcon::setThemeName("theoide-material");
  QIcon::setFallbackThemeName("default");

  QTranslator translator;
  const QStringList uiLanguages = QLocale::system().uiLanguages();
  for (const QString &locale : uiLanguages) {
    const QString baseName = "QtTest_" + QLocale(locale).name();
    if (translator.load(":/i18n/" + baseName)) {
      app.installTranslator(&translator);
      break;
    }
  }

  QQmlApplicationEngine engine;
  const QUrl url(u"qrc:/qt/qml/app/main.qml"_s);
  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreated, &app,
      [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) QCoreApplication::exit(-1);
      },
      Qt::QueuedConnection);
  engine.load(url);

  return app.exec();
}
