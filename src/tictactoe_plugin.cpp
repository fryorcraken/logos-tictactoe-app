#include "tictactoe_plugin.h"
#include "tictactoe_backend.h"
#include "logos_api.h"
#include <QDebug>
#include <QDir>
#include <QQuickWidget>
#include <QQmlContext>
#include <QUrl>

TictactoePlugin::TictactoePlugin(QObject* parent) : QObject(parent) {}
TictactoePlugin::~TictactoePlugin() = default;

void TictactoePlugin::initLogos(LogosAPI* api)
{
    m_logosAPI = api;
}

QWidget* TictactoePlugin::createWidget(LogosAPI* logosAPI)
{
    auto* backend = new TictactoeBackend(logosAPI);

    auto* quickWidget = new QQuickWidget();
    quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    quickWidget->rootContext()->setContextProperty("backend", backend);
    backend->setParent(quickWidget);

    // Dev mode: set QML_PATH=$PWD/src/qml to load from disk without rebuilding.
    QString devSource = qgetenv("QML_PATH");
    QUrl qmlUrl = devSource.isEmpty()
        ? QUrl("qrc:/src/qml/Main.qml")
        : QUrl::fromLocalFile(QDir(devSource).filePath("Main.qml"));

    quickWidget->setSource(qmlUrl);

    if (quickWidget->status() == QQuickWidget::Error) {
        qWarning() << "TictactoePlugin: failed to load QML";
        for (const auto& e : quickWidget->errors())
            qWarning() << e.toString();
    }

    return quickWidget;
}

void TictactoePlugin::destroyWidget(QWidget* widget)
{
    delete widget;
}
