#include "tictactoe_backend.h"
#include "tictactoe.pb.h"
#include "logos_api.h"
#include "logos_api_client.h"
#include "logos_sdk.h"

#include <QByteArray>
#include <QDebug>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// ── pickFreePort ──────────────────────────────────────────────────────
// Workaround for logos-co/logos-delivery-module#24: createNode fails
// silently when tcpPort or discv5UdpPort is 0. Pick ephemeral ports
// ourselves and pass concrete numbers. Inherent TOCTOU window between
// close() and delivery_module's bind(), acceptable for local dev.
static int pickFreePort(int type)
{
    int fd = ::socket(AF_INET, type, 0);
    if (fd < 0) return 0;
    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = 0;
    if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(fd);
        return 0;
    }
    socklen_t len = sizeof(addr);
    if (::getsockname(fd, reinterpret_cast<sockaddr*>(&addr), &len) < 0) {
        ::close(fd);
        return 0;
    }
    int port = ntohs(addr.sin_port);
    ::close(fd);
    return port;
}

// 90s timeout — delivery_module's createNode spins up libwaku (~20–25s
// cold) which races the SDK default (20s) and has been observed to abort
// the replica mid-handshake. 90s gives libwaku room without surprising
// the user.
static const Timeout kDeliveryTimeout(90000);

// ── Construction ──────────────────────────────────────────────────────

TictactoeBackend::TictactoeBackend(LogosAPI* api, QObject* parent)
    : QObject(parent), m_logosAPI(api)
{
    if (m_logosAPI) m_logos = new LogosModules(m_logosAPI);
    resetBoard();
    refreshStatus();
}

TictactoeBackend::~TictactoeBackend()
{
    delete m_logos;
}

// ── Game ──────────────────────────────────────────────────────────────

void TictactoeBackend::newGame()
{
    resetBoard();
    refreshStatus();
    emit boardChanged();
    emit outcomeChanged();
    emit currentPlayerChanged();
    broadcastNewGame();
}

int TictactoeBackend::play(int row, int col)
{
    if (row < 0 || row > 2 || col < 0 || col > 2) return 1;
    if (m_outcome != 0)                           return 3;
    if (m_board[row][col] != 0)                   return 2;

    const int playedBy = m_turn;
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
    broadcastMove(row, col, playedBy);
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

// ── Multiplayer ───────────────────────────────────────────────────────

int TictactoeBackend::mpStatus() const
{
    if (!m_mpEnabled)          return 0;
    if (!m_mpError.isEmpty())  return 3;
    if (m_mpConnected)         return 2;
    return 1;
}

// NOTE: the RPC chain below uses the SYNCHRONOUS `invokeRemoteMethod` —
// each call blocks until the reply comes back. That freezes the UI for
// the ~20–25s libwaku cold start. We'd prefer `invokeRemoteMethodAsync`
// but it only exists on logos-module-builder master, and we're pinned to
// tutorial-v1 for basecamp v0.1.1 wire-format compat. Sync is intentional
// here, not an oversight.
void TictactoeBackend::enableMultiplayer()
{
    if (m_mpEnabled) { emit mpStatusChanged(); return; }
    if (!m_logos || !m_logosAPI) {
        setMpError("Logos SDK not initialized");
        return;
    }
    m_mpError.clear();
    emit mpStatusChanged();   // 1=connecting (early)

    registerDeliveryHandlers();

    // The generated DeliveryModule wrapper decodes RPC replies as
    // LogosResult, but delivery_module at this pin returns plain bool.
    // Bypass the typed wrapper for RPCs; still use it for .on() events.
    LogosAPIClient* client = m_logosAPI->getClient("delivery_module");
    if (!client) {
        setMpError("delivery_module client unavailable");
        return;
    }

    auto callBool = [&](const char* what, const QVariant& v) {
        if (!v.isValid())            { setMpError(QStringLiteral("%1: no response").arg(what)); return false; }
        if (!v.toBool())             { setMpError(QStringLiteral("%1 returned false").arg(what)); return false; }
        return true;
    };

    // Pick free ports ourselves because delivery_module rejects port 0
    // (logos-co/logos-delivery-module#24).
    int tcpPort = pickFreePort(SOCK_STREAM);
    int udpPort = pickFreePort(SOCK_DGRAM);
    if (tcpPort == 0 || udpPort == 0) {
        setMpError("failed to pick free ports for delivery_module");
        return;
    }
    QString config = QStringLiteral(
        R"({"logLevel":"INFO","mode":"Core","preset":"logos.dev","tcpPort":%1,"discv5UdpPort":%2})"
    ).arg(tcpPort).arg(udpPort);

    if (!callBool("createNode",
                  client->invokeRemoteMethod("delivery_module", "createNode",
                                             config, kDeliveryTimeout))) return;

    if (!callBool("start",
                  client->invokeRemoteMethod("delivery_module", "start",
                                             QVariantList(), kDeliveryTimeout))) return;

    if (!callBool("subscribe",
                  client->invokeRemoteMethod("delivery_module", "subscribe",
                                             m_contentTopic, kDeliveryTimeout))) {
        client->invokeRemoteMethod("delivery_module", "stop",
                                   QVariantList(), kDeliveryTimeout);
        return;
    }

    m_mpEnabled = true;
    emit mpStatusChanged();   // connecting → (soon) connected
}

void TictactoeBackend::disableMultiplayer()
{
    if (!m_mpEnabled) return;

    if (m_logosAPI) {
        if (LogosAPIClient* client = m_logosAPI->getClient("delivery_module")) {
            client->invokeRemoteMethod("delivery_module", "unsubscribe",
                                       m_contentTopic, kDeliveryTimeout);
            client->invokeRemoteMethod("delivery_module", "stop",
                                       QVariantList(), kDeliveryTimeout);
        }
    }
    m_mpEnabled   = false;
    m_mpConnected = false;
    m_msgSent     = 0;
    m_msgReceived = 0;
    m_mpError.clear();
    emit mpStatusChanged();   // 0=off
}

// Handlers can only be registered once per plugin lifetime —
// LogosAPIConsumer::onEvent appends without dedup and has no .off(), so
// a naive enable → disable → enable would stack duplicate handlers.
// The handlers gate on m_mpEnabled so they're inert when off.
void TictactoeBackend::registerDeliveryHandlers()
{
    if (m_mpHandlersRegistered) return;
    if (!m_logos) return;

    m_logos->delivery_module.on("messageReceived",
        [this](const QString& /*eventName*/, const QVariantList& data) {
            if (!m_mpEnabled) return;
            if (data.size() < 3) return;

            // messageReceived delivers data[2] as base64. We encode our own
            // payload as base64 before send(), so the full path is
            // decode(delivery base64) → decode(our base64) → protobuf.
            QByteArray deliveryPayload = QByteArray::fromBase64(data[2].toString().toUtf8());
            QByteArray protoBytes      = QByteArray::fromBase64(deliveryPayload);

            tictactoe::GameMessage msg;
            if (!msg.ParseFromArray(protoBytes.data(), protoBytes.size())) {
                qWarning() << "TictactoeBackend: failed to parse protobuf message";
                return;
            }

            if (msg.has_move()) {
                int row = static_cast<int>(msg.move().row());
                int col = static_cast<int>(msg.move().col());
                // Apply to local state as the current local player. Own echoes
                // and mirrored plays land on occupied cells and are rejected
                // by play()'s `if (m_board[row][col] != 0)` guard — benign.
                // See README "Multiplayer limitations" for the full story.
                if (row < 0 || row > 2 || col < 0 || col > 2)   return;
                if (m_outcome != 0)                             return;
                if (m_board[row][col] != 0)                     return;

                const int prevOutcome = m_outcome;
                m_board[row][col] = m_turn;
                m_outcome = computeOutcome();
                if (m_outcome == 0) {
                    m_turn = (m_turn == 1) ? 2 : 1;
                    emit currentPlayerChanged();
                }
                refreshStatus();
                emit boardChanged();
                if (m_outcome != prevOutcome) emit outcomeChanged();

                ++m_msgReceived;
                emit mpStatusChanged();
            } else if (msg.has_new_game()) {
                resetBoard();
                refreshStatus();
                emit boardChanged();
                emit outcomeChanged();
                emit currentPlayerChanged();
                ++m_msgReceived;
                emit mpStatusChanged();
            }
        });

    m_logos->delivery_module.on("connectionStateChanged",
        [this](const QString& /*eventName*/, const QVariantList& data) {
            if (!m_mpEnabled) return;
            m_mpConnected = data.size() > 0 && !data[0].toString().isEmpty();
            emit mpStatusChanged();
        });

    m_logos->delivery_module.on("messageError",
        [this](const QString& /*eventName*/, const QVariantList& data) {
            if (!m_mpEnabled) return;
            m_mpError = data.size() >= 3 ? data[2].toString() : QStringLiteral("send failed");
            emit mpStatusChanged();
        });

    m_mpHandlersRegistered = true;
}

void TictactoeBackend::broadcastMove(int row, int col, int player)
{
    if (!m_mpEnabled || !m_mpConnected) return;
    tictactoe::GameMessage msg;
    auto* move = msg.mutable_move();
    move->set_row(static_cast<uint32_t>(row));
    move->set_col(static_cast<uint32_t>(col));
    move->set_player(static_cast<uint32_t>(player));

    std::string serialized;
    if (!msg.SerializeToString(&serialized)) {
        setMpError("protobuf serialization failed");
        return;
    }
    QString payload = QString::fromLatin1(
        QByteArray(serialized.data(), static_cast<int>(serialized.size())).toBase64());

    LogosAPIClient* client = m_logosAPI ? m_logosAPI->getClient("delivery_module") : nullptr;
    if (!client) { setMpError("delivery_module client unavailable"); return; }
    QVariant v = client->invokeRemoteMethod("delivery_module", "send",
                                            m_contentTopic, payload, kDeliveryTimeout);
    if (!v.isValid()) { setMpError("send: no response"); return; }
    ++m_msgSent;
    emit mpStatusChanged();
}

void TictactoeBackend::broadcastNewGame()
{
    if (!m_mpEnabled || !m_mpConnected) return;
    tictactoe::GameMessage msg;
    msg.set_new_game(true);

    std::string serialized;
    if (!msg.SerializeToString(&serialized)) {
        setMpError("protobuf serialization failed");
        return;
    }
    QString payload = QString::fromLatin1(
        QByteArray(serialized.data(), static_cast<int>(serialized.size())).toBase64());

    LogosAPIClient* client = m_logosAPI ? m_logosAPI->getClient("delivery_module") : nullptr;
    if (!client) { setMpError("delivery_module client unavailable"); return; }
    QVariant v = client->invokeRemoteMethod("delivery_module", "send",
                                            m_contentTopic, payload, kDeliveryTimeout);
    if (!v.isValid()) { setMpError("send: no response"); return; }
    ++m_msgSent;
    emit mpStatusChanged();
}

void TictactoeBackend::setMpError(const QString& err)
{
    m_mpError = err;
    qWarning() << "TictactoeBackend:" << err;
    emit mpStatusChanged();   // 3=error
}
