/**
 * @file ArchivedDataDialog.qml
 * @brief Okno dialogowe do przeglądania zarchiwizowanych danych stacji.
 * @author Adam Fedorowicz
 * @date 2025-04-24
 *
 * Ten plik definiuje okno dialogowe, które wyświetla listę zarchiwizowanych danych stacji
 * i umożliwia użytkownikowi wybranie zestawu danych do wyświetlenia w ArchivedStationDialog.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
/**
 * @class Window
 * @brief Okno dialogowe dla zarchiwizowanych danych.
 *
 * To okno jest modalne i wyświetla listę zapisanych zestawów danych stacji,
 * z możliwością załadowania wybranych danych.
 */
Window {
    id: dialog
    modality: Qt.ApplicationModal
    title: "Zarchiwizowane dane stacji"
    width: 600
    height: 400
    minimumWidth: 500
    minimumHeight: 300
    visible: false

    /// @property var archivedStationDialog Obiekt dialogu szczegółów zarchiwizowanej stacji.
    property var archivedStationDialog: null

    /**
     * @brief Inicjalizuje dialog szczegółów zarchiwizowanej stacji.
     *
     * Tworzy instancję ArchivedStationDialog przy starcie.
     */
    Component.onCompleted: {
        archivedStationDialog = Qt.createQmlObject('import QtQuick.Controls 2.15; ArchivedStationDialog {}', dialog)
    }

    /**
     * @brief Nagłówek okna.
     */
    Rectangle {
        id: header
        width: parent.width
        height: 50
        color: "#4CAF50"

        Text {
            anchors.centerIn: parent
            text: "Zarchiwizowane dane stacji"
            color: "white"
            font.pixelSize: 20
            font.bold: true
        }
    }

    /**
     * @brief Lista zarchiwizowanych danych.
     */
    ListView {
        id: archivedList
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 10
        clip: true
        model: mainWindow.archivedStations

        delegate: Rectangle {
            width: parent.width
            height: 60
            color: mouseArea.containsMouse ? "#e0e0e0" : "#f0f0f0"
            radius: 5

            Row {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                /**
                 * @brief Wyświetla informacje o zarchiwizowanej stacji.
                 */
                Text {
                    width: parent.width * 0.7
                    text: "Stacja " + modelData.stationId + ", " + modelData.cityName + ", " + modelData.address + " (" + modelData.saveDate + ")"
                    font.pixelSize: 14
                    wrapMode: Text.WordWrap
                }

                /**
                 * @brief Przycisk do załadowania danych.
                 */
                Button {
                    width: 100
                    height: 40
                    text: "Załaduj"
                    font.pixelSize: 14
                    /**
                     * @brief Ładuje wybrane dane i otwiera dialog szczegółów.
                     */
                    onClicked: {
                        mainWindow.loadArchivedStationData(modelData.stationId, modelData.saveDate)
                        archivedStationDialog.stationId = modelData.stationId
                        archivedStationDialog.cityName = modelData.cityName
                        archivedStationDialog.street = modelData.address
                        archivedStationDialog.number = ""
                        archivedStationDialog.saveDate = modelData.saveDate
                        archivedStationDialog.open()
                    }
                }
            }

            /**
             * @brief Obsługuje interakcje myszą z elementem listy.
             */
            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
            }
        }
    }

    /**
     * @brief Połączenia z MainWindow.
     *
     * Reaguje na zmiany listy zarchiwizowanych stacji.
     */
    Connections {
        target: mainWindow
        /**
         * @brief Aktualizuje listę po zmianie zarchiwizowanych stacji.
         *
         * Po otrzymaniu sygnału archivedStationsChanged, odświeża model ListView.
         */
        function onArchivedStationsChanged() {
            archivedList.model = mainWindow.archivedStations
        }
    }

    /**
     * @brief Otwiera okno dialogowe.
     *
     * Ustawia widoczność okna na true, co powoduje jego wyświetlenie.
     */
    function open() {
        visible = true
    }
}
