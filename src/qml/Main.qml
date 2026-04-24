import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    readonly property var backend: logos.module("tictactoe")
    property bool ready: false

    readonly property string statusText: backend ? backend.status : ""
    property var cells: [0, 0, 0, 0, 0, 0, 0, 0, 0]
    property string errorText: ""

    Connections {
        target: logos
        function onViewModuleReadyChanged(moduleName, isReady) {
            if (moduleName === "tictactoe") {
                root.ready = isReady && root.backend !== null
                if (root.ready) root.refresh()
            }
        }
    }
    Component.onCompleted: {
        root.ready = root.backend !== null && logos.isViewModuleReady("tictactoe")
        if (root.ready) root.refresh()
    }

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

    function refresh() {
        if (!backend) return
        for (let i = 0; i < 9; ++i) {
            const idx = i
            const r = Math.floor(i / 3)
            const c = i % 3
            logos.watch(backend.cell(r, c),
                function(v) {
                    const arr = root.cells.slice()
                    arr[idx] = v
                    root.cells = arr
                },
                function(e) { root.errorText = String(e) }
            )
        }
    }

    function play(r, c) {
        if (!backend) return
        root.errorText = ""
        logos.watch(backend.play(r, c),
            function(_) { root.refresh() },
            function(e) { root.errorText = String(e) }
        )
    }

    function newGame() {
        if (!backend) return
        root.errorText = ""
        logos.watch(backend.newGame(),
            function(_) { root.refresh() },
            function(e) { root.errorText = String(e) }
        )
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

        Text {
            Layout.alignment: Qt.AlignHCenter
            text: root.ready ? "" : "Connecting to backend…"
            color: "#f0883e"
            font.pixelSize: 12
            visible: !root.ready
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
                        text: root.cellText(root.cells[cellRect.index])
                        font.pixelSize: 48
                        font.bold: true
                        color: root.cellColor(root.cells[cellRect.index])
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        enabled: root.ready
                        onClicked: {
                            const r = Math.floor(cellRect.index / 3)
                            const c = cellRect.index % 3
                            root.play(r, c)
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 48
            color: root.errorText ? "#3d1a1a" : "#1a2d1a"
            radius: 8
            Text {
                anchors.centerIn: parent
                text: root.errorText || root.statusText || (root.ready ? "Ready" : "…")
                color: root.errorText ? "#f85149" : "#ffffff"
                font.pixelSize: 15
            }
        }

        Button {
            Layout.alignment: Qt.AlignHCenter
            text: "New Game"
            enabled: root.ready
            onClicked: root.newGame()
        }
    }
}
