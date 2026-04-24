#ifndef TICTACTOE_PLUGIN_H
#define TICTACTOE_PLUGIN_H

#include <QObject>
#include <QWidget>
#include <QVariantList>
#include <IComponent.h>
#include "tictactoe_interface.h"

class TictactoePlugin : public QObject, public TictactoeInterface, public IComponent
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IComponent_iid FILE "metadata.json")
    Q_INTERFACES(TictactoeInterface PluginInterface IComponent)

public:
    explicit TictactoePlugin(QObject* parent = nullptr);
    ~TictactoePlugin() override;

    QString name()    const override { return "tictactoe"; }
    QString version() const override { return "1.0.0"; }

    Q_INVOKABLE void initLogos(LogosAPI* api);

    Q_INVOKABLE QWidget* createWidget(LogosAPI* logosAPI = nullptr);
    Q_INVOKABLE void     destroyWidget(QWidget* widget);

signals:
    void eventResponse(const QString& eventName, const QVariantList& args);

private:
    LogosAPI* m_logosAPI = nullptr;
};

#endif // TICTACTOE_PLUGIN_H
