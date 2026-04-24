#include "tictactoe_plugin.h"
#include "logos_api.h"
#include <QDebug>

// play() return codes
static constexpr int PLAY_OK          = 0;
static constexpr int PLAY_ERR_BOUNDS  = 1;
static constexpr int PLAY_ERR_OCCUPIED = 2;
static constexpr int PLAY_ERR_OVER    = 3;

TictactoePlugin::TictactoePlugin(QObject* parent)
    : TictactoeSimpleSource(parent)
{
    resetBoard();
    refreshStatus();
}

TictactoePlugin::~TictactoePlugin() = default;

void TictactoePlugin::initLogos(LogosAPI* api)
{
    m_logosAPI = api;
    setBackend(this);
    qDebug() << "TictactoePlugin: initialized";
}

void TictactoePlugin::newGame()
{
    resetBoard();
    refreshStatus();
}

int TictactoePlugin::play(int row, int col)
{
    if (row < 0 || row > 2 || col < 0 || col > 2) {
        return PLAY_ERR_BOUNDS;
    }
    if (m_outcome != 0) {
        return PLAY_ERR_OVER;
    }
    if (m_board[row][col] != 0) {
        return PLAY_ERR_OCCUPIED;
    }

    m_board[row][col] = m_turn;
    m_outcome = checkOutcome();
    if (m_outcome == 0) {
        m_turn = (m_turn == 1) ? 2 : 1;
    }
    refreshStatus();
    return PLAY_OK;
}

int TictactoePlugin::cell(int row, int col)
{
    if (row < 0 || row > 2 || col < 0 || col > 2) {
        return -1;
    }
    return m_board[row][col];
}

int TictactoePlugin::turn()
{
    return m_turn;
}

int TictactoePlugin::outcome()
{
    return m_outcome;
}

void TictactoePlugin::resetBoard()
{
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            m_board[r][c] = 0;
    m_turn = 1;
    m_outcome = 0;
}

int TictactoePlugin::checkOutcome() const
{
    // Rows
    for (int r = 0; r < 3; ++r) {
        int a = m_board[r][0];
        if (a != 0 && a == m_board[r][1] && a == m_board[r][2]) return a;
    }
    // Cols
    for (int c = 0; c < 3; ++c) {
        int a = m_board[0][c];
        if (a != 0 && a == m_board[1][c] && a == m_board[2][c]) return a;
    }
    // Diagonals
    int d = m_board[0][0];
    if (d != 0 && d == m_board[1][1] && d == m_board[2][2]) return d;
    int e = m_board[0][2];
    if (e != 0 && e == m_board[1][1] && e == m_board[2][0]) return e;

    // Draw: board full and no winner
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            if (m_board[r][c] == 0) return 0;
    return 3;
}

void TictactoePlugin::refreshStatus()
{
    switch (m_outcome) {
        case 1: setStatus(QStringLiteral("X wins")); break;
        case 2: setStatus(QStringLiteral("O wins")); break;
        case 3: setStatus(QStringLiteral("Draw")); break;
        default:
            setStatus(m_turn == 1 ? QStringLiteral("X's turn")
                                  : QStringLiteral("O's turn"));
            break;
    }
}
