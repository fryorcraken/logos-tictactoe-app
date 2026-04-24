#ifndef TICTACTOE_PLUGIN_H
#define TICTACTOE_PLUGIN_H

#include <QString>
#include <QVariantList>
#include "tictactoe_interface.h"
#include "LogosViewPluginBase.h"
#include "rep_tictactoe_source.h"

class LogosAPI;

class TictactoePlugin : public TictactoeSimpleSource,
                        public TictactoeInterface,
                        public TictactoeViewPluginBase
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID TictactoeInterface_iid FILE "metadata.json")
    Q_INTERFACES(TictactoeInterface)

public:
    explicit TictactoePlugin(QObject* parent = nullptr);
    ~TictactoePlugin() override;

    QString name()    const override { return "tictactoe"; }
    QString version() const override { return "1.0.0"; }

    Q_INVOKABLE void initLogos(LogosAPI* api);

    // Slots from tictactoe.rep
    void newGame() override;
    int  play(int row, int col) override;
    int  cell(int row, int col) override;
    int  turn() override;
    int  outcome() override;

signals:
    void eventResponse(const QString& eventName, const QVariantList& args);

private:
    void resetBoard();
    int  checkOutcome() const;
    void refreshStatus();

    LogosAPI* m_logosAPI = nullptr;

    // 0 = empty, 1 = X, 2 = O
    int m_board[3][3] = {{0,0,0},{0,0,0},{0,0,0}};
    int m_turn    = 1;  // 1 = X, 2 = O
    int m_outcome = 0;  // 0 = ongoing, 1 = X wins, 2 = O wins, 3 = draw
};

#endif // TICTACTOE_PLUGIN_H
