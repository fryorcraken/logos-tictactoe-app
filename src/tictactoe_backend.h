#ifndef TICTACTOE_BACKEND_H
#define TICTACTOE_BACKEND_H

#include <QObject>
#include <QString>
#include <QVariantList>

class LogosAPI;

class TictactoeBackend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList board READ board NOTIFY boardChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(int outcome READ outcome NOTIFY outcomeChanged)
    Q_PROPERTY(int currentPlayer READ currentPlayer NOTIFY currentPlayerChanged)

public:
    explicit TictactoeBackend(LogosAPI* api, QObject* parent = nullptr);

    Q_INVOKABLE void newGame();
    Q_INVOKABLE int  play(int row, int col);

    QVariantList board() const;
    QString      status() const         { return m_status; }
    int          outcome() const        { return m_outcome; }
    int          currentPlayer() const  { return m_turn; }

signals:
    void boardChanged();
    void statusChanged();
    void outcomeChanged();
    void currentPlayerChanged();

private:
    void resetBoard();
    int  computeOutcome() const;
    void refreshStatus();

    LogosAPI* m_logosAPI = nullptr;

    int     m_board[3][3] = {{0,0,0},{0,0,0},{0,0,0}};
    int     m_turn    = 1;   // 1 = X, 2 = O
    int     m_outcome = 0;   // 0 = ongoing, 1 = X wins, 2 = O wins, 3 = draw
    QString m_status;
};

#endif // TICTACTOE_BACKEND_H
