import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    function cellText(v) {
        if (v === 1) return "X"
        if (v === 2) return "O"
        return ""
    }
    function cellColor(v) {
        if (v === 1) return "#4a9eff"
        if (v === 2) return "#ff6b6b"
        return "#ffffff"
    }

    function mpStatusText() {
        switch (backend.mpStatus) {
            case 0:  return "Multiplayer off"
            case 1:  return "Connecting…"
            case 2:  return "Connected — sent " + backend.mpMessagesSent + ", received " + backend.mpMessagesReceived
            case 3:  return "Error: " + backend.mpError
            default: return ""
        }
    }
    function mpStatusColor() {
        switch (backend.mpStatus) {
            case 1:  return "#f0883e"   // connecting — amber
            case 2:  return "#56d364"   // connected — green
            case 3:  return "#f85149"   // error — red
            default: return "#8b949e"   // off — grey
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        Text {
            Layout.alignment: Qt.AlignHCenter
            text: "Tic-tac-toe"
            font.pixelSize: 24
            font.bold: true
            color: "#ffffff"
        }

        GridLayout {
            Layout.alignment: Qt.AlignHCenter
            columns: 3
            rowSpacing: 8
            columnSpacing: 8

            Repeater {
                model: 9
                delegate: Rectangle {
                    id: cellRect
                    required property int index
                    width: 80
                    height: 80
                    color: "#2a2a2a"
                    radius: 6
                    border.color: "#444"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: root.cellText(backend.board[cellRect.index])
                        font.pixelSize: 48
                        font.bold: true
                        color: root.cellColor(backend.board[cellRect.index])
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            const r = Math.floor(cellRect.index / 3)
                            const c = cellRect.index % 3
                            backend.play(r, c)
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 48
            color: "#1a2d1a"
            radius: 8
            Text {
                anchors.centerIn: parent
                text: backend.status
                color: "#ffffff"
                font.pixelSize: 15
            }
        }

        Button {
            Layout.alignment: Qt.AlignHCenter
            text: "New Game"
            onClicked: backend.newGame()
        }

        Rectangle {
            Layout.fillWidth: true
            height: 32
            color: "#161616"
            radius: 6
            border.color: root.mpStatusColor()
            border.width: 1
            Text {
                anchors.centerIn: parent
                text: root.mpStatusText()
                color: root.mpStatusColor()
                font.pixelSize: 13
            }
        }
    }
}
