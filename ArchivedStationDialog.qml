/**
 * @file ArchivedStationDialog.qml
 * @brief Okno dialogowe wyświetlające zarchiwizowane informacje o stacji.
 * @author Adam Fedorowicz
 * @date 2025-04-21
 *
 * Ten plik definiuje okno dialogowe, które wyświetla szczegóły zarchiwizowanej stacji,
 * w tym wykresy danych sensorów i statystyki.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15

/**
 * @class Window
 * @brief Okno dialogowe dla zarchiwizowanych informacji o stacji.
 *
 * To okno jest modalne i wyświetla zarchiwizowane dane o stacji, w tym wykresy danych sensorów
 * i statystyki.
 */
Window {
    id: dialog
    modality: Qt.ApplicationModal
    title: "Zarchiwizowane informacje o stacji"
    width: Screen.width
    height: Screen.height
    minimumWidth: 700
    minimumHeight: 550
    visible: false

    /// @property int stationId Identyfikator stacji.
    property int stationId: 0
    /// @property string cityName Nazwa miasta stacji.
    property string cityName: ""
    /// @property string street Ulica, na której znajduje się stacja.
    property string street: ""
    /// @property string number Numer budynku stacji.
    property string number: ""
    /// @property string saveDate Data zapisu danych.
    property string saveDate: ""

    /// @property var selectedSensors Obiekt przechowujący wybrane sensory.
    property var selectedSensors: ({})
    /// @property var colors Tablica kolorów do rysowania wykresów.
    property var colors: ["#4CAF50", "#FF0000", "#0000FF", "#FFA500", "#800080", "#00CED1"]

    /**
     * @brief Debugowanie dostępności danych przy tworzeniu dialogu.
     *
     * Wyświetla w konsoli informacje o dostępnych sensorach i danych sensorów.
     */
    Component.onCompleted: {
        console.log("ArchivedStationDialog otwarty z sensorami:", JSON.stringify(mainWindow.sensors))
        console.log("ArchivedStationDialog otwarty z danymi sensorów:", JSON.stringify(mainWindow.sensorData))
    }

    /**
     * @brief Nagłówek okna.
     *
     * Wyświetla informacje o stacji, takie jak jej identyfikator, miasto, adres i data zapisu.
     */
    Rectangle {
        id: header
        width: parent.width
        height: 50
        color: "#4CAF50"

        Text {
            anchors.centerIn: parent
            text: "Stacja nr " + stationId + ", " + cityName + ", " + street + (number ? " " + number : "") + " (Zarchiwizowano: " + saveDate + ")"
            color: "white"
            font.pixelSize: 20
            font.bold: true
            width: parent.width - 20
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }
    }

    /**
     * @brief Główny układ okna.
     *
     * Zawiera listę parametrów do wyboru oraz sekcję z wykresem i szczegółowymi danymi.
     */
    Row {
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 10
        spacing: 10

        Column {
            width: parent.width * 0.4
            height: parent.height
            spacing: 10

            /**
             * @brief Nagłówek listy parametrów.
             */
            Text {
                id: paramsHeader
                width: parent.width
                text: "Wybierz mierzony parametr:"
                font.pixelSize: 16
                color: "#333"
                wrapMode: Text.WordWrap
            }

            /**
             * @brief Lista dostępnych parametrów do wyboru.
             */
            ListView {
                id: paramsList
                width: parent.width
                height: parent.height - paramsHeader.height - parent.spacing
                spacing: 10
                clip: true
                model: mainWindow.sensors

                delegate: Rectangle {
                    width: parent.width
                    height: 40
                    color: checkBox.checked ? "#C8E6C9" : (mouseArea.containsMouse ? "#e0e0e0" : "#f0f0f0")
                    radius: 5

                    Row {
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 5

                        /**
                         * @brief Pole wyboru dla parametru.
                         */
                        CheckBox {
                            id: checkBox
                            text: modelData.paramName
                            font.pixelSize: 14
                            /**
                             * @brief Obsługuje zmianę stanu pola wyboru.
                             *
                             * Dodaje lub usuwa sensor z listy wybranych sensorów i odświeża wykres.
                             */
                            onCheckedChanged: {
                                if (checked) {
                                    selectedSensors[modelData.sensorId] = modelData.paramName
                                } else {
                                    delete selectedSensors[modelData.sensorId]
                                }
                                chartCanvas.requestPaint()
                            }
                        }

                        /**
                         * @brief Wskaźnik koloru dla wybranego parametru.
                         */
                        Rectangle {
                            visible: checkBox.checked
                            width: 12
                            height: 12
                            radius: 6
                            anchors.verticalCenter: parent.verticalCenter
                            color: {
                                var sensorId = modelData.sensorId
                                var selectedIds = Object.keys(selectedSensors)
                                var index = selectedIds.indexOf(sensorId.toString())
                                return index >= 0 && index < colors.length ? colors[index] : "transparent"
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
                        /**
                         * @brief Przełącza stan pola wyboru po kliknięciu.
                         */
                        onClicked: {
                            checkBox.checked = !checkBox.checked
                        }
                    }
                }
            }
        }

        Column {
            width: parent.width * 0.6
            height: parent.height
            spacing: 10

            /**
             * @brief Kontener dla wykresu danych.
             */
            Rectangle {
                width: parent.width
                height: parent.height * 0.6
                color: "#f0f0f0"
                border.color: "black"
                border.width: 1
                anchors.rightMargin: 20

                /**
                 * @brief Płótno do rysowania wykresu danych sensorów.
                 */
                Canvas {
                    id: chartCanvas
                    anchors.fill: parent
                    anchors.margins: 50

                    /**
                     * @brief Rysuje wykres danych sensorów.
                     *
                     * Funkcja rysuje siatkę, osie, linie danych sensorów oraz etykiety osi i czasu.
                     */
                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.clearRect(0, 0, width, height)
                        ctx.strokeStyle = "black"
                        ctx.lineWidth = 2
                        ctx.font = "14px Arial"

                        // Rysowanie siatki
                        ctx.strokeStyle = "#d3d3d3"
                        ctx.lineWidth = 1
                        for (var x = 0; x <= width; x += width / 10) {
                            ctx.beginPath()
                            ctx.moveTo(x, 0)
                            ctx.lineTo(x, height)
                            ctx.stroke()
                        }
                        for (var y = 0; y <= height; y += height / 10) {
                            ctx.beginPath()
                            ctx.moveTo(0, y)
                            ctx.lineTo(width, y)
                            ctx.stroke()
                        }

                        // Rysowanie osi
                        ctx.strokeStyle = "black"
                        ctx.lineWidth = 2
                        ctx.beginPath()
                        ctx.moveTo(0, height)
                        ctx.lineTo(width, height)
                        ctx.moveTo(0, 0)
                        ctx.lineTo(0, height)
                        ctx.stroke()

                        var sensorData = mainWindow.sensorData
                        var selectedSensorIds = Object.keys(selectedSensors)
                        if (selectedSensorIds.length === 0 || Object.keys(sensorData).length === 0) {
                            ctx.fillText("Wybierz mierzone parametry aby wyświetlić odczyty.", width / 2 - 150, height / 2)
                            return
                        }

                        // Obliczanie wartości min i max oraz zbieranie punktów czasu
                        var globalMaxValue = -Number.MAX_VALUE
                        var globalMinValue = Number.MAX_VALUE
                        var allDataPoints = []
                        var timePoints = []

                        for (var i = 0; i < selectedSensorIds.length; i++) {
                            var sensorId = selectedSensorIds[i]
                            var data = sensorData[sensorId]
                            if (!data) {
                                console.log("Brak danych dla sensora:", sensorId)
                                continue
                            }

                            for (var j = 0; j < data.length; j++) {
                                var value = data[j].value
                                if (value !== null && !isNaN(value)) {
                                    globalMaxValue = Math.max(globalMaxValue, value)
                                    globalMinValue = Math.min(globalMinValue, value)
                                }
                                if (i === 0) {
                                    timePoints.push(data[j].date)
                                }
                            }
                            allDataPoints.push(data)
                        }

                        console.log("Punkty czasowe:", timePoints.length, "Minimalna wartość:", globalMinValue, "Maksymalna wartość:", globalMaxValue, "Wybrane sensory:", selectedSensorIds)

                        if (globalMaxValue === globalMinValue) {
                            globalMaxValue += 1
                            globalMinValue -= 1
                        }
                        if (timePoints.length === 0) {
                            ctx.fillText("Brak danych czasowych.", width / 2 - 100, height / 2)
                            return
                        }

                        // Rysowanie przerywanych linii dla północy
                        ctx.strokeStyle = "black"
                        ctx.lineWidth = 1
                        ctx.setLineDash([5, 5])
                        for (var i = 0; i < timePoints.length; i++) {
                            var time = timePoints[i].split(" ")[1]
                            if (time === "00:00:00") {
                                var x = ((timePoints.length - 1 - i) / (timePoints.length - 1)) * width
                                ctx.beginPath()
                                ctx.moveTo(x, 0)
                                ctx.lineTo(x, height)
                                ctx.stroke()
                            }
                        }
                        ctx.setLineDash([])

                        // Rysowanie wykresów
                        for (var s = 0; s < selectedSensorIds.length && s < colors.length; s++) {
                            var sensorId = selectedSensorIds[s]
                            var data = sensorData[sensorId]
                            if (!data) continue

                            ctx.strokeStyle = colors[s]
                            ctx.lineWidth = 2
                            ctx.beginPath()
                            var firstPoint = true
                            for (var i = 0; i < data.length; i++) {
                                var value = data[i].value
                                if (value === null || isNaN(value)) continue
                                var x = ((data.length - 1 - i) / (data.length - 1)) * width
                                var y = height - ((value - globalMinValue) / (globalMaxValue - globalMinValue)) * height
                                if (firstPoint) {
                                    ctx.moveTo(x, y)
                                    firstPoint = false
                                } else {
                                    ctx.lineTo(x, y)
                                }
                            }
                            ctx.stroke()
                        }

                        // Rysowanie etykiet czasu (co 4 godziny) i daty (tylko pod przerywanymi liniami)
                        ctx.fillStyle = "black"
                        var lastHour = -1
                        for (var i = 0; i < timePoints.length; i++) {
                            var dateTime = timePoints[i].split(" ")
                            var date = dateTime[0].split("-")
                            var time = dateTime[1]
                            var hour = parseInt(time.split(":")[0])
                            var minutes = time.split(":")[1]

                            var x = ((timePoints.length - 1 - i) / (timePoints.length - 1)) * width
                            var timeLabel = (hour < 10 ? "0" + hour : hour) + ":" + minutes
                            var dateLabel = date[2] + "." + date[1] + "." + date[0].slice(2)

                            // Wyświetlanie godziny co 4 godziny
                            if (hour % 4 === 0 && hour !== lastHour) {
                                ctx.save()
                                ctx.translate(x, height - 10)
                                ctx.rotate(-Math.PI / 4)
                                ctx.fillText(timeLabel, 0, 0)
                                ctx.restore()
                                lastHour = hour
                            }

                            // Wyświetlanie daty tylko pod przerywanymi liniami (00:00:00)
                            if (time === "00:00:00") {
                                ctx.fillText(dateLabel, x - 20, height - 5)
                            }
                        }

                        // Rysowanie etykiet wartości na osi Y
                        ctx.fillStyle = "black"
                        for (var i = 0; i <= 5; i++) {
                            var value = globalMinValue + (i * (globalMaxValue - globalMinValue) / 5)
                            var y = height - (i * height / 5)
                            if (!isNaN(value)) {
                                ctx.fillText(value.toFixed(1), +10, y - 5)
                            }
                        }

                        // Rysowanie podpisów osi
                        ctx.fillStyle = "black"
                        ctx.fillText("Czas", width / 2, height + 60)
                        ctx.save()
                        ctx.translate(50, height / 2)
                        ctx.rotate(-Math.PI / 2)
                        ctx.fillText("Wartość (µg/m³)", 0, 0)
                        ctx.restore()
                    }
                }
            }

            /**
             * @brief Kontener dla szczegółowych danych.
             */
            Rectangle {
                width: parent.width
                height: parent.height * 0.35
                color: "#f0f0f0"
                border.color: "black"
                border.width: 1
                anchors.rightMargin: 20

                Column {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10

                    /**
                     * @brief Nagłówek sekcji szczegółowych danych.
                     */
                    Rectangle {
                        width: parent.width
                        height: 30
                        color: "#4CAF50"

                        Text {
                            anchors.centerIn: parent
                            text: "Szczegółowe dane archiwalne:"
                            color: "white"
                            font.pixelSize: 16
                            font.bold: true
                        }
                    }

                    /**
                     * @brief Selektor parametrów do szczegółowego wyświetlania.
                     */
                    ComboBox {
                        id: paramSelector
                        width: parent.width
                        height: 40
                        model: mainWindow.sensors
                        textRole: "paramName"
                        font.pixelSize: 14
                        /**
                         * @brief Aktualizuje wyświetlane statystyki po zmianie parametru.
                         */
                        onCurrentIndexChanged: {
                            latestValueText.text = Qt.binding(function() {
                                if (currentIndex < 0) return ""
                                var sensorId = mainWindow.sensors[currentIndex].sensorId
                                var data = mainWindow.sensorData[sensorId]
                                if (!data || data.length === 0) return "Brak danych"
                                var latest = data[0]
                                return "Ostatni odczyt: " + (latest.value !== null ? latest.value.toFixed(2) : "Brak") + " µg/m³ (" + latest.date + ")"
                            })
                        }
                    }

                    /**
                     * @brief Wyświetla statystyki wybranego parametru.
                     */
                    Column {
                        width: parent.width
                        spacing: 5
                        visible: paramSelector.currentIndex >= 0

                        Text {
                            id: latestValueText
                            width: parent.width
                            font.pixelSize: 14
                            color: "#4CAF50"
                            wrapMode: Text.WordWrap
                            /**
                             * @brief Wyświetla ostatni odczyt sensora.
                             *
                             * Pokazuje wartość i datę ostatniego odczytu dla wybranego sensora.
                             */
                            text: {
                                if (paramSelector.currentIndex < 0) return ""
                                var sensorId = mainWindow.sensors[paramSelector.currentIndex].sensorId
                                var data = mainWindow.sensorData[sensorId]
                                if (!data || data.length === 0) return "Brak danych"
                                var latest = data[0]
                                return "Ostatni odczyt: " + (latest.value !== null ? latest.value.toFixed(2) : "Brak") + " µg/m³ (" + latest.date + ")"
                            }
                        }

                        Text {
                            width: parent.width
                            font.pixelSize: 14
                            wrapMode: Text.WordWrap
                            /**
                             * @brief Wyświetla średnią wartość sensora.
                             *
                             * Oblicza i wyświetla średnią wartość dla danych sensora.
                             */
                            text: {
                                if (paramSelector.currentIndex < 0) return ""
                                var sensorId = mainWindow.sensors[paramSelector.currentIndex].sensorId
                                var data = mainWindow.sensorData[sensorId]
                                if (!data || data.length === 0) return "Średnia wartość: Brak danych"
                                var sum = 0
                                var count = 0
                                for (var i = 0; i < data.length; i++) {
                                    if (data[i].value !== null && !isNaN(data[i].value)) {
                                        sum += data[i].value
                                        count++
                                    }
                                }
                                var avg = count > 0 ? (sum / count).toFixed(2) : "Brak"
                                return "Średnia wartość: " + avg + " µg/m³"
                            }
                        }

                        Text {
                            width: parent.width
                            font.pixelSize: 14
                            wrapMode: Text.WordWrap
                            /**
                             * @brief Wyświetla minimalną wartość sensora.
                             *
                             * Oblicza i wyświetla minimalną wartość dla danych sensora.
                             */
                            text: {
                                if (paramSelector.currentIndex < 0) return ""
                                var sensorId = mainWindow.sensors[paramSelector.currentIndex].sensorId
                                var data = mainWindow.sensorData[sensorId]
                                if (!data || data.length === 0) return "Minimalna wartość: Brak danych"
                                var min = Number.MAX_VALUE
                                for (var i = 0; i < data.length; i++) {
                                    if (data[i].value !== null && !isNaN(data[i].value)) {
                                        min = Math.min(min, data[i].value)
                                    }
                                }
                                return "Minimalna wartość: " + (min !== Number.MAX_VALUE ? min.toFixed(2) : "Brak") + " µg/m³"
                            }
                        }

                        Text {
                            width: parent.width
                            font.pixelSize: 14
                            wrapMode: Text.WordWrap
                            /**
                             * @brief Wyświetla maksymalną wartość sensora.
                             *
                             * Oblicza i wyświetla maksymalną wartość dla danych sensora.
                             */
                            text: {
                                if (paramSelector.currentIndex < 0) return ""
                                var sensorId = mainWindow.sensors[paramSelector.currentIndex].sensorId
                                var data = mainWindow.sensorData[sensorId]
                                if (!data || data.length === 0) return "Maksymalna wartość: Brak danych"
                                var max = -Number.MAX_VALUE
                                for (var i = 0; i < data.length; i++) {
                                    if (data[i].value !== null && !isNaN(data[i].value)) {
                                        max = Math.max(max, data[i].value)
                                    }
                                }
                                return "Maksymalna wartość: " + (max !== -Number.MAX_VALUE ? max.toFixed(2) : "Brak") + " µg/m³"
                            }
                        }
                    }
                }
            }
        }
    }

    /**
     * @brief Połączenia z MainWindow.
     *
     * Reaguje na zmiany danych sensorów, aktualizując wykres i statystyki.
     */
    Connections {
        target: mainWindow
        /**
         * @brief Aktualizuje wykres i statystyki po zmianie danych sensorów.
         *
         * Po otrzymaniu sygnału sensorDataChanged, odświeża płótno wykresu oraz tekst wyświetlający ostatni odczyt.
         */
        function onSensorDataChanged() {
            console.log("Dane sensorów zmienione, żądanie odświeżenia wykresu i statystyk")
            chartCanvas.requestPaint()
            latestValueText.text = Qt.binding(function() {
                if (paramSelector.currentIndex < 0) return ""
                var sensorId = mainWindow.sensors[paramSelector.currentIndex].sensorId
                var data = mainWindow.sensorData[sensorId]
                if (!data || data.length === 0) return "Brak danych"
                var latest = data[0]
                return "Ostatni odczyt: " + (latest.value !== null ? latest.value.toFixed(2) : "Brak") + " µg/m³ (" + latest.date + ")"
            })
        }

        /**
         * @brief Reaguje na załadowanie danych archiwalnych.
         *
         * Po otrzymaniu sygnału archivedDataLoaded, odświeża wykres, aby wyświetlić załadowane dane.
         */
        function onArchivedDataLoaded() {
            console.log("Załadowano dane archiwalne, odświeżanie wykresu")
            chartCanvas.requestPaint()
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