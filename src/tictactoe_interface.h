#ifndef TICTACTOE_INTERFACE_H
#define TICTACTOE_INTERFACE_H

#include <QObject>
#include <QString>
#include "interface.h"

class TictactoeInterface : public PluginInterface
{
public:
    virtual ~TictactoeInterface() = default;
};

#define TictactoeInterface_iid "org.logos.TictactoeInterface"
Q_DECLARE_INTERFACE(TictactoeInterface, TictactoeInterface_iid)

#endif // TICTACTOE_INTERFACE_H
