/**
 * @file main.qml
 * @brief Główny plik QML aplikacji monitorowania jakości powietrza.
 * @author Adam Fedorowicz
 * @date 2025-04-21
 *
 * Ten plik QML definiuje główne okno aplikacji, w tym widok mapy,
 * listę stacji i funkcjonalność wyszukiwania.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtLocation 5.15
import QtPositioning 5.15

/**
 * @class ApplicationWindow
 * @brief Główne okno aplikacji.
 *
 * Ten komponent reprezentuje główne okno aplikacji, zawierające
 * nagłówek, pasek wyszukiwania, mapę, listę stacji oraz stopkę.
 */
ApplicationWindow {
    id: root
    visible: true
    visibility: Window.Maximized
    minimumWidth: 400
    minimumHeight: 600
    title: "Stacje Pomiarowe"

    /// @property int highlightedStationId Identyfikator aktualnie podświetlonej stacji.
    property int highlightedStationId: -1

    /**
     * @brief Sekcja nagłówka okna.
     *
     * Wyświetla tytuł aplikacji w nagłówku.
     */
    Rectangle {
        id: header
        width: parent.width
        height: 50
        color: "#4CAF50"

        Text {
            anchors.centerIn: parent
            text: "Stacje Pomiarowe"
            color: "white"
            font.pixelSize: 20
            font.bold: true
        }
    }

    /**
     * @brief Główny obszar treści z paskiem wyszukiwania, tekstem statusu, mapą i listą stacji.
     */
    Column {
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: footer.top
        anchors.margins: 10
        spacing: 10

        Row {
            width: parent.width
            spacing: 10

            /**
             * @brief Pole tekstowe do wprowadzania nazwy miasta.
             */
            TextField {
                id: cityInput
                width: parent.width - searchButton.width - archiveButton.width - 20
                height: 40
                placeholderText: "Wpisz nazwę miasta"
                font.pixelSize: 16
                onAccepted: {
                    mainWindow.searchCity(text)
                }
            }

            /**
             * @brief Przycisk do uruchomienia wyszukiwania miasta.
             */
            Button {
                id: searchButton
                text: "Szukaj"
                width: 100
                height: 40
                font.pixelSize: 14
                onClicked: {
                    mainWindow.searchCity(cityInput.text)
                }
            }

            /**
             * @brief Przycisk do otwierania dialogu z danymi archiwalnymi.
             */
            Button {
                id: archiveButton
                text: "Archiwum"
                width: 100
                height: 40
                font.pixelSize: 14
                onClicked: {
                    var component = Qt.createComponent("qrc:/ArchivedDataDialog.qml");
                    if (component.status === Component.Ready) {
                        var dialog = component.createObject(root);
                        dialog.open();
                    } else {
                        console.log("Error loading ArchivedDataDialog.qml: " + component.errorString());
                    }
                }
            }
        }

        /**
         * @brief Wyświetla aktualny komunikat statusu.
         */
        Text {
            id: statusText
            text: mainWindow.status
            font.pixelSize: 16
            color: "#333"
            width: parent.width
            wrapMode: Text.WordWrap
        }

        Row {
            width: parent.width
            height: parent.height - statusText.height - cityInput.height - 30
            spacing: 10

            /**
             * @brief Kontener dla listy stacji.
             */
            Column {
                width: parent.width * 0.4
                height: parent.height
                spacing: 10

                /**
                 * @brief Tekst nagłówka listy stacji.
                 */
                Text {
                    id: listHeader
                    width: parent.width
                    text: stationList.model.count === 0 ? "Wyszukaj stację aby rozpocząć" : "Wybierz stację z listy aby wyświetlić szczegóły"
                    font.pixelSize: 16
                    color: "#333"
                    wrapMode: Text.WordWrap
                }

                /**
                 * @brief Lista wyszukanych stacji.
                 */
                ListView {
                    id: stationList
                    width: parent.width
                    height: parent.height - listHeader.height - parent.spacing
                    spacing: 10
                    clip: true

                    model: mainWindow.stations

                    delegate: Rectangle {
                        width: parent.width
                        height: 120
                        color: modelData.stationId === root.highlightedStationId ? "#e0e0e0" : (mouseArea.containsMouse ? "#e0e0e0" : "#f0f0f0")
                        radius: 5

                        /**
                         * @brief Obsługuje interakcje myszą z elementami listy stacji.
                         */
                        MouseArea {
                            id: mouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            z: 1
                            onEntered: {
                                root.highlightedStationId = modelData.stationId
                            }
                            onExited: {
                                root.highlightedStationId = -1
                            }
                            onClicked: {
                                mainWindow.fetchSensors(modelData.stationId)
                                var component = Qt.createComponent("qrc:/StationDialog.qml");
                                if (component.status === Component.Ready) {
                                    var address = modelData.address ? modelData.address : "Brak danych";
                                    var street = address === "Brak danych" ? "Brak danych" : address.split(" ")[0];
                                    var number = address === "Brak danych" ? "" : (address.split(" ").length > 1 ? address.split(" ")[1] : "");
                                    var dialog = component.createObject(root, {
                                        "stationId": modelData.stationId,
                                        "cityName": modelData.cityName,
                                        "street": street,
                                        "number": number
                                    });
                                    dialog.open();
                                }
                            }
                        }

                        /**
                         * @brief Wyświetla szczegóły stacji w elemencie listy.
                         */
                        Column {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 5
                            z: 0

                            Text {
                                text: "<b>Nazwa:</b> " + modelData.stationName
                                font.pixelSize: 14
                            }
                            Text {
                                text: "<b>ID:</b> " + modelData.stationId
                                font.pixelSize: 14
                            }
                            Text {
                                text: "<b>Współrzędne:</b> " + modelData.lat + ", " + modelData.lon
                                font.pixelSize: 14
                            }
                            Text {
                                text: "<b>Adres:</b> " + (modelData.address ? modelData.address : "Brak danych")
                                font.pixelSize: 14
                            }
                        }
                    }
                }
            }

            /**
             * @brief Kontener dla widoku mapy.
             */
            Rectangle {
                id: mapContainer
                width: parent.width * 0.6 + 4
                height: parent.height + 4
                border.color: "black"
                border.width: 1
                color: "transparent"

                /**
                 * @brief Komponent mapy wyświetlający lokalizacje stacji.
                 */
                Map {
                    id: map
                    width: parent.width - 4
                    height: parent.height - 4
                    anchors.centerIn: parent
                    plugin: Plugin {
                        name: "osm"
                        PluginParameter {
                            name: "osm.mapping.highdpi_tiles"
                            value: "true"
                        }
                        PluginParameter {
                            name: "osm.useragent"
                            value: "ControlStationsApp/1.0"
                        }
                    }
                    center: QtPositioning.coordinate(mainWindow.mapCenter.latitude, mainWindow.mapCenter.longitude)
                    zoomLevel: 8

                    Connections {
                        target: mainWindow
                        function onMapCenterChanged() {
                            map.center = QtPositioning.coordinate(mainWindow.mapCenter.latitude, mainWindow.mapCenter.longitude)
                        }
                    }

                    /**
                     * @brief Obsługuje interakcje z mapą (przeciąganie, przybliżanie).
                     */
                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton
                        property point lastPos: Qt.point(0, 0)
                        property bool dragging: false

                        onPressed: {
                            if (mouse.button === Qt.LeftButton) {
                                lastPos = Qt.point(mouse.x, mouse.y)
                                dragging = true
                                mouse.accepted = true
                            }
                        }

                        onPositionChanged: {
                            if (dragging) {
                                var deltaX = mouse.x - lastPos.x
                                var deltaY = mouse.y - lastPos.y
                                var center = map.center
                                var deltaLat = deltaY * 0.003 * Math.pow(2, 8 - map.zoomLevel)
                                var deltaLon = -deltaX * 0.003 * Math.pow(2, 8 - map.zoomLevel) / Math.cos(center.latitude * Math.PI / 180)
                                map.center = QtPositioning.coordinate(center.latitude + deltaLat, center.longitude + deltaLon)
                                lastPos = Qt.point(mouse.x, mouse.y)
                                mouse.accepted = true
                            }
                        }

                        onReleased: {
                            if (mouse.button === Qt.LeftButton) {
                                dragging = false
                                mouse.accepted = true
                            }
                        }

                        onWheel: {
                            if (wheel.angleDelta.y > 0) {
                                map.zoomLevel += 0.5
                            } else if (wheel.angleDelta.y < 0) {
                                map.zoomLevel -= 0.5
                            }
                            wheel.accepted = true
                        }
                    }

                    /**
                     * @brief Wyświetla znaczniki stacji na mapie.
                     */
                    MapItemView {
                        model: mainWindow.allStations
                        delegate: MapQuickItem {
                            coordinate: QtPositioning.coordinate(modelData.lat, modelData.lon)
                            anchorPoint.x: marker.width / 2
                            anchorPoint.y: marker.height

                            sourceItem: Item {
                                width: marker.width
                                height: marker.height

                                Rectangle {
                                    id: marker
                                    width: (modelData.isSearched || modelData.stationId === root.highlightedStationId) ? 16 : 8
                                    height: width
                                    color: modelData.stationId === root.highlightedStationId ? "#4CAF50" : (modelData.isSearched ? "#FF0000" : "#0000FF")
                                    radius: width / 2
                                }

                                /**
                                 * @brief Obsługuje interakcje myszą ze znacznikami stacji.
                                 */
                                MouseArea {
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onEntered: {
                                        root.highlightedStationId = modelData.stationId
                                    }
                                    onExited: {
                                        root.highlightedStationId = -1
                                    }
                                    onClicked: {
                                        mainWindow.fetchSensors(modelData.stationId)
                                        var component = Qt.createComponent("qrc:/StationDialog.qml");
                                        if (component.status === Component.Ready) {
                                            var address = modelData.address ? modelData.address : "Brak danych";
                                            var street = address === "Brak danych" ? "Brak danych" : address.split(" ")[0];
                                            var number = address === "Brak danych" ? "" : (address.split(" ").length > 1 ? address.split(" ")[1] : "");
                                            var dialog = component.createObject(root, {
                                                "stationId": modelData.stationId,
                                                "cityName": modelData.cityName,
                                                "street": street,
                                                "number": number
                                            });
                                            dialog.open();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /**
     * @brief Sekcja stopki okna.
     *
     * Wyświetla informację o źródle danych.
     */
    Rectangle {
        id: footer
        width: parent.width
        height: 30
        anchors.bottom: parent.bottom
        color: "#f0f0f0"

        Text {
            anchors.centerIn: parent
            text: "Dane dostarczane przez Główny Inspektorat Ochrony Środowiska"
            font.pixelSize: 12
            color: "#333"
        }
    }
}
