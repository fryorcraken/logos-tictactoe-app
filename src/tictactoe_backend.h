#ifndef TICTACTOE_BACKEND_H
#define TICTACTOE_BACKEND_H

#include <QObject>
#include <QString>
#include <QVariantList>

class LogosAPI;
class LogosModules;

class TictactoeBackend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList board READ board NOTIFY boardChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(int outcome READ outcome NOTIFY outcomeChanged)
    Q_PROPERTY(int currentPlayer READ currentPlayer NOTIFY currentPlayerChanged)

    // Multiplayer surface (slice 6).
    Q_PROPERTY(int mpStatus           READ mpStatus           NOTIFY mpStatusChanged)
    Q_PROPERTY(int mpMessagesSent     READ mpMessagesSent     NOTIFY mpStatusChanged)
    Q_PROPERTY(int mpMessagesReceived READ mpMessagesReceived NOTIFY mpStatusChanged)
    Q_PROPERTY(QString mpError        READ mpError            NOTIFY mpStatusChanged)

public:
    explicit TictactoeBackend(LogosAPI* api, QObject* parent = nullptr);
    ~TictactoeBackend() override;

    Q_INVOKABLE void newGame();
    Q_INVOKABLE int  play(int row, int col);

    Q_INVOKABLE void enableMultiplayer();
    Q_INVOKABLE void disableMultiplayer();

    QVariantList board() const;
    QString      status() const         { return m_status; }
    int          outcome() const        { return m_outcome; }
    int          currentPlayer() const  { return m_turn; }

    // mpStatus: 0=off, 1=connecting, 2=connected, 3=error
    int     mpStatus() const;
    int     mpMessagesSent() const     { return m_msgSent; }
    int     mpMessagesReceived() const { return m_msgReceived; }
    QString mpError() const            { return m_mpError; }

signals:
    void boardChanged();
    void statusChanged();
    void outcomeChanged();
    void currentPlayerChanged();
    void mpStatusChanged();

private:
    void resetBoard();
    int  computeOutcome() const;
    void refreshStatus();

    void broadcastMove(int row, int col, int player);
    void broadcastNewGame();
    void setMpError(const QString& err);
    void registerDeliveryHandlers();   // runs once per plugin lifetime

    LogosAPI*     m_logosAPI = nullptr;
    LogosModules* m_logos    = nullptr;

    int     m_board[3][3] = {{0,0,0},{0,0,0},{0,0,0}};
    int     m_turn    = 1;   // 1 = X, 2 = O
    int     m_outcome = 0;   // 0 = ongoing, 1 = X wins, 2 = O wins, 3 = draw
    QString m_status;

    bool    m_mpEnabled          = false;
    bool    m_mpConnected        = false;
    bool    m_mpHandlersRegistered = false;
    int     m_msgSent            = 0;
    int     m_msgReceived        = 0;
    QString m_mpError;

    // Content topic for move broadcast — see LIP-23.
    const QString m_contentTopic = QStringLiteral("/tictactoe/1/moves/proto");
};

#endif // TICTACTOE_BACKEND_H
