/**
 * @file mainwindow.h
 * @brief Plik nagłówkowy dla klas MainWindow i Station.
 * @author Adam Fedorowicz
 * @date 2025-04-21
 *
 * Ten plik definiuje klasy MainWindow i Station używane w aplikacji monitorowania jakości powietrza.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QObject>
#include <QGeoCoordinate>
#include <QQmlListProperty>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QJsonDocument>
#include <QDateTime>
#include <QDir>

/**
 * @class Station
 * @brief Reprezentuje stację monitorowania jakości powietrza.
 *
 * Ta klasa przechowuje właściwości stacji, takie jak jej identyfikator, nazwa, lokalizacja i status wyszukiwania.
 */
class Station : public QObject {
    Q_OBJECT
    Q_PROPERTY(int stationId READ stationId CONSTANT)
    Q_PROPERTY(QString stationName READ stationName CONSTANT)
    Q_PROPERTY(QString cityName READ cityName CONSTANT)
    Q_PROPERTY(QString address READ address CONSTANT)
    Q_PROPERTY(double lat READ lat CONSTANT)
    Q_PROPERTY(double lon READ lon CONSTANT)
    Q_PROPERTY(bool isSearched READ isSearched NOTIFY isSearchedChanged)

public:
    /**
     * @brief Konstruktor obiektu Station.
     * @param id Identyfikator stacji.
     * @param name Nazwa stacji.
     * @param city Nazwa miasta.
     * @param addr Adres stacji.
     * @param latitude Współrzędna szerokości geograficznej.
     * @param longitude Współrzędna długości geograficznej.
     * @param searched Status wyszukiwania.
     * @param parent Rodzic QObject.
     */
    Station(int id, QString name, QString city, QString addr, double latitude, double longitude, bool searched, QObject *parent = nullptr)
        : QObject(parent), m_stationId(id), m_stationName(name), m_cityName(city), m_address(addr), m_lat(latitude), m_lon(longitude), m_isSearched(searched) {}

    /**
     * @brief Pobiera identyfikator stacji.
     * @return Identyfikator stacji.
     */
    int stationId() const { return m_stationId; }

    /**
     * @brief Pobiera nazwę stacji.
     * @return Nazwa stacji.
     */
    QString stationName() const { return m_stationName; }

    /**
     * @brief Pobiera nazwę miasta.
     * @return Nazwa miasta.
     */
    QString cityName() const { return m_cityName; }

    /**
     * @brief Pobiera adres stacji.
     * @return Adres stacji.
     */
    QString address() const { return m_address; }

    /**
     * @brief Pobiera szerokość geograficzną.
     * @return Współrzędna szerokości geograficznej.
     */
    double lat() const { return m_lat; }

    /**
     * @brief Pobiera długość geograficzną.
     * @return Współrzędna długości geograficznej.
     */
    double lon() const { return m_lon; }

    /**
     * @brief Pobiera status wyszukiwania.
     * @return True, jeśli stacja jest oznaczona jako wyszukana.
     */
    bool isSearched() const { return m_isSearched; }

    /**
     * @brief Ustawia status wyszukiwania.
     * @param searched Nowy status wyszukiwania.
     */
    void setIsSearched(bool searched) {
        if (m_isSearched != searched) {
            m_isSearched = searched;
            emit isSearchedChanged();
        }
    }

signals:
    /**
     * @brief Sygnał emitowany, gdy zmieni się status wyszukiwania.
     */
    void isSearchedChanged();

private:
    int m_stationId;          ///< Identyfikator stacji.
    QString m_stationName;    ///< Nazwa stacji.
    QString m_cityName;       ///< Nazwa miasta.
    QString m_address;        ///< Adres stacji.
    double m_lat;             ///< Szerokość geograficzna.
    double m_lon;             ///< Długość geograficzna.
    bool m_isSearched;        ///< Status wyszukiwania.
};

/**
 * @class MainWindow
 * @brief Główna klasa zarządzająca aplikacją monitorowania jakości powietrza.
 *
 * Odpowiada za pobieranie danych z API, zarządzanie stacjami, sensorami i danymi archiwalnymi.
 */
class MainWindow : public QObject {
    Q_OBJECT
    Q_PROPERTY(QGeoCoordinate mapCenter READ mapCenter NOTIFY mapCenterChanged)
    Q_PROPERTY(QQmlListProperty<Station> stations READ stations NOTIFY stationsChanged)
    Q_PROPERTY(QQmlListProperty<Station> allStations READ allStations NOTIFY allStationsChanged)
    Q_PROPERTY(QVariantList sensors READ sensors NOTIFY sensorsChanged)
    Q_PROPERTY(QVariantMap sensorData READ sensorData NOTIFY sensorDataChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QVariantList archivedStations READ archivedStations NOTIFY archivedStationsChanged)

public:
    /**
     * @brief Konstruktor obiektu MainWindow.
     * @param parent Rodzic QObject.
     */
    explicit MainWindow(QObject *parent = nullptr);

    /**
     * @brief Pobiera centrum mapy.
     * @return Współrzędne centrum mapy.
     */
    QGeoCoordinate mapCenter() const { return m_mapCenter; }

    /**
     * @brief Pobiera listę wyszukanych stacji.
     * @return Lista stacji w formacie QQmlListProperty.
     */
    QQmlListProperty<Station> stations() {
        return QQmlListProperty<Station>(this, &m_stations);
    }

    /**
     * @brief Pobiera listę wszystkich stacji.
     * @return Lista wszystkich stacji w formacie QQmlListProperty.
     */
    QQmlListProperty<Station> allStations() {
        return QQmlListProperty<Station>(this, &m_allStations);
    }

    /**
     * @brief Pobiera listę sensorów.
     * @return Lista sensorów jako QVariantList.
     */
    QVariantList sensors() const { return m_sensors; }

    /**
     * @brief Pobiera dane sensorów.
     * @return Mapa danych sensorów jako QVariantMap.
     */
    QVariantMap sensorData() const { return m_sensorData; }

    /**
     * @brief Pobiera komunikat statusu.
     * @return Aktualny komunikat statusu.
     */
    QString status() const { return m_status; }

    /**
     * @brief Pobiera listę zapisanych stacji.
     * @return Lista zapisanych stacji jako QVariantList.
     */
    QVariantList archivedStations() const { return m_archivedStations; }

public slots:
    /**
     * @brief Wyszukuje stacje w podanym mieście.
     * @param city Nazwa miasta.
     */
    void searchCity(const QString &city);

    /**
     * @brief Pobiera sensory dla stacji.
     * @param stationId Identyfikator stacji.
     */
    void fetchSensors(int stationId);

    /**
     * @brief Pobiera dane dla sensora.
     * @param sensorId Identyfikator sensora.
     */
    void fetchSensorData(int sensorId);

    /**
     * @brief Aktualizuje status wyszukiwania stacji.
     * @param stationId Identyfikator stacji.
     * @param isSearched Nowy status wyszukiwania.
     */
    void updateStationSearchStatus(int stationId, bool isSearched);

    /**
     * @brief Usuwa dane dla sensora.
     * @param sensorId Identyfikator sensora.
     */
    void removeSensorData(int sensorId);

    /**
     * @brief Zapisuje dane stacji.
     * @param stationId Identyfikator stacji.
     * @param cityName Nazwa miasta.
     * @param address Adres stacji.
     */
    void saveStationData(int stationId, const QString &cityName, const QString &address);

    /**
     * @brief Ładuje zapisane dane stacji.
     * @param stationId Identyfikator stacji.
     * @param saveDate Data zapisu.
     */
    void loadArchivedStationData(int stationId, const QString &saveDate);

private slots:
    /**
     * @brief Obsługuje odpowiedź API geokodowania.
     * @param reply Odpowiedź sieciowa.
     * @param searchedCity Wyszukiwane miasto.
     */
    void onGeocodeReply(QNetworkReply *reply, const QString &searchedCity);

    /**
     * @brief Obsługuje odpowiedź API dla stacji.
     * @param reply Odpowiedź sieciowa.
     */
    void onStationsReply(QNetworkReply *reply);

    /**
     * @brief Obsługuje odpowiedź API dla sensorów.
     * @param reply Odpowiedź sieciowa.
     */
    void onSensorsReply(QNetworkReply *reply);

    /**
     * @brief Obsługuje odpowiedź API dla danych sensora.
     * @param reply Odpowiedź sieciowa.
     * @param sensorId Identyfikator sensora.
     */
    void onSensorDataReply(QNetworkReply *reply, int sensorId);

private:
    /**
     * @brief Ładuje listę zapisanych stacji.
     */
    void loadArchivedStations();

    QGeoCoordinate m_mapCenter;              ///< Centrum mapy.
    QList<Station*> m_stations;              ///< Lista wyszukanych stacji.
    QList<Station*> m_allStations;           ///< Lista wszystkich stacji.
    QVariantList m_sensors;                  ///< Lista sensorów.
    QVariantMap m_sensorData;                ///< Dane sensorów.
    QString m_status;                        ///< Komunikat statusu.
    QVariantList m_archivedStations;         ///< Lista zapisanych stacji.
    QNetworkAccessManager *m_networkManager; ///< Menedżer sieci.

signals:
    /**
     * @brief Sygnał emitowany, gdy zmieni się centrum mapy.
     */
    void mapCenterChanged();

    /**
     * @brief Sygnał emitowany, gdy zmieni się lista wyszukanych stacji.
     */
    void stationsChanged();

    /**
     * @brief Sygnał emitowany, gdy zmieni się lista wszystkich stacji.
     */
    void allStationsChanged();

    /**
     * @brief Sygnał emitowany, gdy zmieni się lista sensorów.
     */
    void sensorsChanged();

    /**
     * @brief Sygnał emitowany, gdy zmieni się dane sensorów.
     */
    void sensorDataChanged();

    /**
     * @brief Sygnał emitowany, gdy zmieni się komunikat statusu.
     */
    void statusChanged();

    /**
     * @brief Sygnał emitowany, gdy zmieni się lista zapisanych stacji.
     */
    void archivedStationsChanged();

    /**
     * @brief Sygnał emitowany, gdy dane archiwalne zostaną załadowane.
     */
    void archivedDataLoaded();
};

#endif // MAINWINDOW_H
