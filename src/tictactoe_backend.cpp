#include "tictactoe_backend.h"

TictactoeBackend::TictactoeBackend(LogosAPI* api, QObject* parent)
    : QObject(parent), m_logosAPI(api)
{
    resetBoard();
    refreshStatus();
}

void TictactoeBackend::newGame()
{
    resetBoard();
    refreshStatus();
    emit boardChanged();
    emit outcomeChanged();
    emit currentPlayerChanged();
}

int TictactoeBackend::play(int row, int col)
{
    if (row < 0 || row > 2 || col < 0 || col > 2) return 1;
    if (m_outcome != 0)                           return 3;
    if (m_board[row][col] != 0)                   return 2;

    m_board[row][col] = m_turn;
    const int prevOutcome = m_outcome;
    m_outcome = computeOutcome();
    if (m_outcome == 0) {
        m_turn = (m_turn == 1) ? 2 : 1;
        emit currentPlayerChanged();
    }
    refreshStatus();
    emit boardChanged();
    if (m_outcome != prevOutcome) emit outcomeChanged();
    return 0;
}

QVariantList TictactoeBackend::board() const
{
    QVariantList list;
    list.reserve(9);
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            list.append(m_board[r][c]);
    return list;
}

void TictactoeBackend::resetBoard()
{
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            m_board[r][c] = 0;
    m_turn = 1;
    m_outcome = 0;
}

int TictactoeBackend::computeOutcome() const
{
    for (int r = 0; r < 3; ++r) {
        int a = m_board[r][0];
        if (a != 0 && a == m_board[r][1] && a == m_board[r][2]) return a;
    }
    for (int c = 0; c < 3; ++c) {
        int a = m_board[0][c];
        if (a != 0 && a == m_board[1][c] && a == m_board[2][c]) return a;
    }
    int d = m_board[0][0];
    if (d != 0 && d == m_board[1][1] && d == m_board[2][2]) return d;
    int e = m_board[0][2];
    if (e != 0 && e == m_board[1][1] && e == m_board[2][0]) return e;

    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            if (m_board[r][c] == 0) return 0;
    return 3;
}

void TictactoeBackend::refreshStatus()
{
    QString next;
    switch (m_outcome) {
        case 1: next = QStringLiteral("X wins");  break;
        case 2: next = QStringLiteral("O wins");  break;
        case 3: next = QStringLiteral("Draw");    break;
        default:
            next = m_turn == 1 ? QStringLiteral("X's turn")
                               : QStringLiteral("O's turn");
            break;
    }
    if (next != m_status) {
        m_status = next;
        emit statusChanged();
    }
}
