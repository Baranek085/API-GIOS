/**
 * @file mainwindow.cpp
 * @brief Implementacja klasy MainWindow.
 * @author Adam Fedorowicz
 * @date 2025-04-21
 *
 * Ten plik zawiera implementację klasy MainWindow, która obsługuje
 * główną logikę pobierania i zarządzania danymi o jakości powietrza.
 */

#include "mainwindow.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <QDir>

/**
 * @brief Konstruktor obiektu MainWindow.
 * @param parent Rodzic QObject.
 *
 * Inicjalizuje centrum mapy na Poznań, ustawia domyślny komunikat statusu
 * i pobiera wszystkie stacje z API.
 */
MainWindow::MainWindow(QObject *parent)
    : QObject(parent),
    m_mapCenter(52.4064, 16.9252), // Domyślnie Poznań
    m_status("Wprowadź nazwę miasta i kliknij Szukaj"),
    m_networkManager(new QNetworkAccessManager(this))
{
    // Pobierz wszystkie stacje przy starcie
    QNetworkRequest request(QUrl("https://api.gios.gov.pl/pjp-api/rest/station/findAll"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onStationsReply(reply);
    });

    // Załaduj zapisane dane archiwalne
    loadArchivedStations();
}

/**
 * @brief Wyszukuje stacje w podanym mieście.
 * @param city Nazwa miasta do wyszukania.
 *
 * Wysyła żądanie do API Nominatim w celu geokodowania miasta i aktualizuje centrum mapy.
 */
void MainWindow::searchCity(const QString &city)
{
    m_status = "Wyszukiwanie: " + city + "...";
    emit statusChanged();

    QUrlQuery query;
    query.addQueryItem("q", city);
    query.addQueryItem("format", "json");
    query.addQueryItem("limit", "1");

    QUrl url("https://nominatim.openstreetmap.org/search");
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "ControlStationsApp/1.0");
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, city]() {
        onGeocodeReply(reply, city);
    });
}

/**
 * @brief Pobiera sensory dla stacji.
 * @param stationId Identyfikator stacji.
 *
 * Wysyła żądanie do API GIOŚ w celu pobrania sensorów dla danej stacji.
 */
void MainWindow::fetchSensors(int stationId)
{
    QUrl url(QString("https://api.gios.gov.pl/pjp-api/rest/station/sensors/%1").arg(stationId));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onSensorsReply(reply);
    });
}

/**
 * @brief Pobiera dane dla sensora.
 * @param sensorId Identyfikator sensora.
 *
 * Wysyła żądanie do API GIOŚ w celu pobrania danych dla określonego sensora.
 */
void MainWindow::fetchSensorData(int sensorId)
{
    QUrl url(QString("https://api.gios.gov.pl/pjp-api/rest/data/getData/%1").arg(sensorId));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, sensorId]() {
        onSensorDataReply(reply, sensorId);
    });
}

/**
 * @brief Aktualizuje status wyszukiwania stacji.
 * @param stationId Identyfikator stacji.
 * @param isSearched Nowy status wyszukiwania.
 *
 * Aktualizuje właściwość isSearched dla określonej stacji.
 */
void MainWindow::updateStationSearchStatus(int stationId, bool isSearched)
{
    for (Station *station : m_allStations) {
        if (station->stationId() == stationId) {
            station->setIsSearched(isSearched);
        }
    }
    emit allStationsChanged();
}

/**
 * @brief Usuwa dane dla sensora.
 * @param sensorId Identyfikator sensora.
 *
 * Usuwa dane powiązane z określonym identyfikatorem sensora.
 */
void MainWindow::removeSensorData(int sensorId)
{
    m_sensorData.remove(QString::number(sensorId));
    emit sensorDataChanged();
}

/**
 * @brief Zapisuje dane stacji do pliku.
 * @param stationId Identyfikator stacji.
 * @param cityName Nazwa miasta.
 * @param address Adres stacji.
 *
 * Zapisuje dane stacji i sensorów do pliku JSON.
 */
void MainWindow::saveStationData(int stationId, const QString &cityName, const QString &address)
{
    // Znajdź stację w liście wszystkich stacji
    Station *station = nullptr;
    for (Station *s : m_allStations) {
        if (s->stationId() == stationId) {
            station = s;
            break;
        }
    }

    if (!station) {
        m_status = "Błąd: Stacja o ID " + QString::number(stationId) + " nie znaleziona.";
        emit statusChanged();
        return;
    }

    // Utwórz obiekt JSON
    QJsonObject jsonObj;
    jsonObj["stationId"] = stationId;
    jsonObj["stationName"] = station->stationName();
    jsonObj["cityName"] = cityName;
    jsonObj["address"] = address;
    jsonObj["latitude"] = station->lat();
    jsonObj["longitude"] = station->lon();
    jsonObj["saveDate"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    // Dodaj dane sensorów
    QJsonArray sensorsArray;
    for (const QVariant &sensorVariant : m_sensors) {
        QVariantMap sensorInfo = sensorVariant.toMap();
        int sensorId = sensorInfo["sensorId"].toInt();
        QString paramName = sensorInfo["paramName"].toString();

        QJsonObject sensorObj;
        sensorObj["sensorId"] = sensorId;
        sensorObj["paramName"] = paramName;

        // Dodaj pomiary
        QVariantList data = m_sensorData[QString::number(sensorId)].toList();
        QJsonArray measurementsArray;
        for (const QVariant &dataPoint : data) {
            QVariantMap dataMap = dataPoint.toMap();
            QJsonObject measurementObj;
            measurementObj["date"] = dataMap["date"].toString();
            measurementObj["value"] = dataMap["value"].toDouble();
            measurementsArray.append(measurementObj);
        }
        sensorObj["measurements"] = measurementsArray;
        sensorsArray.append(sensorObj);
    }
    jsonObj["sensors"] = sensorsArray;

    // Utwórz dokument JSON
    QJsonDocument jsonDoc(jsonObj);

    // Wygeneruj nazwę pliku na podstawie ID stacji i znacznika czasu
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString filename = QString("station_%1_%2.json").arg(stationId).arg(timestamp);

    // Określ ścieżkę katalogu
    QDir dir("C:/Users/max08/OneDrive/Pulpit/AirAPI/build/Desktop_Qt_6_8_3_MinGW_64_bit-Release");
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QString filepath = dir.filePath(filename);

    // Zapisz do pliku
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly)) {
        m_status = "Błąd: Nie można otworzyć pliku do zapisu: " + filepath;
        emit statusChanged();
        return;
    }

    file.write(jsonDoc.toJson(QJsonDocument::Indented));
    file.close();

    m_status = "Dane zapisano do pliku: " + filepath;
    emit statusChanged();

    // Odśwież listę zapisanych stacji
    loadArchivedStations();
}

/**
 * @brief Ładuje listę zapisanych plików JSON.
 *
 * Wyszukuje pliki JSON w określonym katalogu i ładuje ich metadane do listy zapisanych stacji.
 */
void MainWindow::loadArchivedStations()
{
    m_archivedStations.clear();

    QDir dir("C:/Users/max08/OneDrive/Pulpit/AirAPI/build/Desktop_Qt_6_8_3_MinGW_64_bit-Release");
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    dir.setNameFilters(QStringList() << "station_*.json");

    QFileInfoList fileList = dir.entryInfoList();
    for (const QFileInfo &fileInfo : fileList) {
        QFile file(fileInfo.absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "Nie można otworzyć pliku:" << fileInfo.absoluteFilePath();
            continue;
        }

        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();

        if (doc.isNull() || !doc.isObject()) {
            qDebug() << "Nieprawidłowy format JSON w pliku:" << fileInfo.absoluteFilePath();
            continue;
        }

        QJsonObject jsonObj = doc.object();
        QVariantMap stationData;
        stationData["stationId"] = jsonObj["stationId"].toInt();
        stationData["cityName"] = jsonObj["cityName"].toString();
        stationData["address"] = jsonObj["address"].toString();
        stationData["saveDate"] = jsonObj["saveDate"].toString();
        m_archivedStations.append(stationData);
    }

    emit archivedStationsChanged();
}

/**
 * @brief Ładuje zapisane dane stacji.
 * @param stationId Identyfikator stacji.
 * @param saveDate Data zapisu.
 *
 * Wyszukuje i ładuje dane stacji z pliku JSON na podstawie identyfikatora i daty zapisu.
 */
void MainWindow::loadArchivedStationData(int stationId, const QString &saveDate)
{
    QDir dir("C:/Users/max08/OneDrive/Pulpit/AirAPI/build/Desktop_Qt_6_8_3_MinGW_64_bit-Release");
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    dir.setNameFilters(QStringList() << QString("station_%1_*.json").arg(stationId));

    QFileInfoList fileList = dir.entryInfoList();
    for (const QFileInfo &fileInfo : fileList) {
        QFile file(fileInfo.absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly)) {
            m_status = "Błąd: Nie można otworzyć pliku: " + fileInfo.absoluteFilePath();
            emit statusChanged();
            continue;
        }

        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();

        if (doc.isNull() || !doc.isObject()) {
            m_status = "Błąd: Nieprawidłowy format JSON w pliku: " + fileInfo.absoluteFilePath();
            emit statusChanged();
            continue;
        }

        QJsonObject jsonObj = doc.object();
        if (jsonObj["saveDate"].toString() != saveDate) {
            continue;
        }

        // Zaktualizuj centrum mapy
        double lat = jsonObj["latitude"].toDouble();
        double lon = jsonObj["longitude"].toDouble();
        m_mapCenter = QGeoCoordinate(lat, lon);

        // Wyczyść istniejące sensory i dane sensorów
        m_sensors.clear();
        m_sensorData.clear();
        emit sensorsChanged();
        emit sensorDataChanged();

        // Przygotuj nowe dane
        QVariantList newSensors;
        QVariantMap newSensorData;
        QJsonArray sensorsArray = jsonObj["sensors"].toArray();
        for (const QJsonValue &sensorValue : sensorsArray) {
            QJsonObject sensorObj = sensorValue.toObject();
            QVariantMap sensorInfo;
            sensorInfo["sensorId"] = sensorObj["sensorId"].toInt();
            sensorInfo["paramName"] = sensorObj["paramName"].toString();
            newSensors.append(sensorInfo);

            // Ładuj dane sensorów
            QVariantList sensorDataList;
            QJsonArray measurementsArray = sensorObj["measurements"].toArray();
            for (const QJsonValue &measurementValue : measurementsArray) {
                QJsonObject measurementObj = measurementValue.toObject();
                QVariantMap dataPoint;
                dataPoint["date"] = measurementObj["date"].toString();
                dataPoint["value"] = measurementObj["value"].toDouble();
                sensorDataList.append(dataPoint);
            }
            newSensorData[QString::number(sensorInfo["sensorId"].toInt())] = sensorDataList;
        }

        // Zaktualizuj dane
        m_sensors = newSensors;
        m_sensorData = newSensorData;

        // Emituj sygnały
        emit mapCenterChanged();
        emit sensorsChanged();
        emit sensorDataChanged();

        m_status = QString("Załadowano dane archiwalne dla stacji %1 z datą %2.").arg(stationId).arg(saveDate);
        emit statusChanged();

        emit archivedDataLoaded();
        break;
    }
}

/**
 * @brief Obsługuje odpowiedź API geokodowania.
 * @param reply Odpowiedź sieciowa.
 * @param searchedCity Wyszukiwane miasto.
 *
 * Przetwarza odpowiedź z API Nominatim, aktualizuje centrum mapy i wyszukuje stacje.
 */
void MainWindow::onGeocodeReply(QNetworkReply *reply, const QString &searchedCity)
{
    if (reply->error() != QNetworkReply::NoError) {
        m_status = "Błąd wyszukiwania: " + reply->errorString();
        emit statusChanged();
        reply->deleteLater();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonArray results = doc.array();

    if (results.isEmpty()) {
        m_status = "Nie znaleziono miasta.";
        emit statusChanged();
        reply->deleteLater();
        return;
    }

    QJsonObject result = results.first().toObject();
    double lat = result["lat"].toString().toDouble();
    double lon = result["lon"].toString().toDouble();

    m_mapCenter = QGeoCoordinate(lat, lon);
    emit mapCenterChanged();

    // Wyczyść listę wyszukanych stacji
    m_stations.clear();
    // Resetuj flagę isSearched dla wszystkich stacji
    for (Station *station : m_allStations) {
        station->setIsSearched(false);
    }

    // Znajdź stacje w dokładnie wyszukanym mieście
    QString normalizedSearchedCity = searchedCity.toLower().simplified();
    for (Station *station : m_allStations) {
        QString stationCity = station->cityName().toLower().simplified();
        if (stationCity == normalizedSearchedCity) {
            Station *searchedStation = new Station(
                station->stationId(),
                station->stationName(),
                station->cityName(),
                station->address(),
                station->lat(),
                station->lon(),
                true,
                this
                );
            m_stations.append(searchedStation);
            updateStationSearchStatus(station->stationId(), true);
        }
    }

    if (m_stations.isEmpty()) {
        // Znajdź najbliższą stację
        Station *closestStation = nullptr;
        double minDistance = std::numeric_limits<double>::max();
        QGeoCoordinate cityCoord(lat, lon);

        for (Station *station : m_allStations) {
            QGeoCoordinate stationCoord(station->lat(), station->lon());
            double distance = cityCoord.distanceTo(stationCoord);
            if (distance < minDistance) {
                minDistance = distance;
                closestStation = station;
            }
        }

        if (closestStation) {
            Station *searchedStation = new Station(
                closestStation->stationId(),
                closestStation->stationName(),
                closestStation->cityName(),
                closestStation->address(),
                closestStation->lat(),
                closestStation->lon(),
                true,
                this
                );
            m_stations.append(searchedStation);
            updateStationSearchStatus(closestStation->stationId(), true);

            // Wycentruj mapę na najbliższej stacji
            m_mapCenter = QGeoCoordinate(closestStation->lat(), closestStation->lon());
            emit mapCenterChanged();

            m_status = QString("Nie znaleziono stacji w %1. Najbliższa stacja znajduje się w %2.").arg(searchedCity, closestStation->cityName());
        } else {
            m_status = QString("Nie znaleziono stacji w %1.").arg(searchedCity);
        }
    } else {
        m_status = QString("Znaleziono %1 stacji w %2.").arg(m_stations.count()).arg(searchedCity);
    }

    emit stationsChanged();
    emit statusChanged();
    reply->deleteLater();
}

/**
 * @brief Obsługuje odpowiedź API dla stacji.
 * @param reply Odpowiedź sieciowa.
 *
 * Przetwarza odpowiedź z API GIOŚ w celu wypełnienia listy wszystkich stacji.
 */
void MainWindow::onStationsReply(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        m_status = "Błąd pobierania stacji: " + reply->errorString();
        emit statusChanged();
        reply->deleteLater();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonArray stations = doc.array();

    m_allStations.clear();
    for (const QJsonValue &value : stations) {
        QJsonObject obj = value.toObject();
        int id = obj["id"].toInt();
        QString name = obj["stationName"].toString();
        QString city = obj["city"].toObject()["name"].toString();
        QString address = obj["addressStreet"].toString();
        double lat = obj["gegrLat"].toString().toDouble();
        double lon = obj["gegrLon"].toString().toDouble();

        m_allStations.append(new Station(id, name, city, address, lat, lon, false, this));
    }

    emit allStationsChanged();
    reply->deleteLater();
}

/**
 * @brief Obsługuje odpowiedź API dla sensorów.
 * @param reply Odpowiedź sieciowa.
 *
 * Przetwarza odpowiedź z API GIOŚ w celu wypełnienia listy sensorów.
 */
void MainWindow::onSensorsReply(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        m_sensors.clear();
        emit sensorsChanged();
        reply->deleteLater();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonArray sensors = doc.array();

    m_sensors.clear();
    for (const QJsonValue &sensorValue : sensors) {
        QJsonObject obj = sensorValue.toObject();
        QJsonObject param = obj["param"].toObject();
        QString paramName = param["paramName"].toString();
        int sensorId = obj["id"].toInt();
        QVariantMap sensorInfo;
        sensorInfo["paramName"] = paramName;
        sensorInfo["sensorId"] = sensorId;
        m_sensors.append(sensorInfo);
    }

    emit sensorsChanged();
    reply->deleteLater();
}

/**
 * @brief Obsługuje odpowiedź API dla danych sensora.
 * @param reply Odpowiedź sieciowa.
 * @param sensorId Identyfikator sensora.
 *
 * Przetwarza odpowiedź z API GIOŚ w celu zapisania danych sensora.
 */
void MainWindow::onSensorDataReply(QNetworkReply *reply, int sensorId)
{
    if (reply->error() != QNetworkReply::NoError) {
        m_sensorData.remove(QString::number(sensorId));
        emit sensorDataChanged();
        reply->deleteLater();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonObject obj = doc.object();
    QJsonArray values = obj["values"].toArray();

    QVariantList sensorDataList;
    for (const QJsonValue &value : values) {
        QJsonObject dataPoint = value.toObject();
        QString date = dataPoint["date"].toString();
        QVariant dataValue = dataPoint["value"].toVariant();
        QVariantMap data;
        data["date"] = date;
        data["value"] = dataValue;
        sensorDataList.append(data);
    }

    qDebug() << "ID sensora:" << sensorId << "Punkty danych:" << sensorDataList.size();
    m_sensorData[QString::number(sensorId)] = sensorDataList;
    emit sensorDataChanged();
    reply->deleteLater();
}
