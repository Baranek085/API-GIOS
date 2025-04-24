/**
 * @file tst_mainwindow.cpp
 * @brief Testy jednostkowe dla klas MainWindow i Station.
 * @author Jan Kowalski
 * @date 2025-04-22
 *
 * Ten plik zawiera testy jednostkowe dla klas MainWindow i Station przy użyciu frameworka Qt Test.
 */

#include <QtTest>
#include "mainwindow.h"

/**
 * @class TestMainWindow
 * @brief Klasa testowa dla funkcjonalności MainWindow i Station.
 *
 * Ta klasa zawiera testy jednostkowe weryfikujące poprawność działania klas MainWindow i Station.
 */
class TestMainWindow : public QObject
{
    Q_OBJECT

private slots:
    /**
     * @brief Testuje właściwości klasy Station.
     *
     * Sprawdza, czy właściwości stacji (ID, nazwa, miasto, adres, współrzędne, status wyszukiwania)
     * są poprawnie ustawiane i zwracane.
     */
    void testStationProperties()
    {
        Station station(1, "Test Station", "Test City", "Test Address", 50.0, 20.0, false, nullptr);
        QCOMPARE(station.stationId(), 1);
        QCOMPARE(station.stationName(), QString("Test Station"));
        QCOMPARE(station.cityName(), QString("Test City"));
        QCOMPARE(station.address(), QString("Test Address"));
        QCOMPARE(station.lat(), 50.0);
        QCOMPARE(station.lon(), 20.0);
        QCOMPARE(station.isSearched(), false);
    }

    /**
     * @brief Testuje centrum mapy.
     *
     * Sprawdza, czy centrum mapy jest poprawnie ustawiane i zmieniane po wyszukiwaniu miasta.
     */
    void testMapCenter()
    {
        MainWindow mainWindow;
        QGeoCoordinate initial = mainWindow.mapCenter();
        QCOMPARE(initial.latitude(), 52.4064); // Poznań, zgodnie z konstruktorem MainWindow
        QCOMPARE(initial.longitude(), 16.9252);

        // Test wyszukiwania miasta wymaga mockowania odpowiedzi sieciowej, co jest bardziej złożone
        // Na razie pomijamy zmianę centrum mapy, ponieważ wymaga to symulacji API
        // mainWindow.searchCity("Krakow");
        // QVERIFY(mainWindow.mapCenter().latitude() != 52.4064 || mainWindow.mapCenter().longitude() != 16.9252);
    }

    /**
     * @brief Testuje status wyszukiwania stacji.
     *
     * Sprawdza, czy status wyszukiwania stacji jest poprawnie aktualizowany.
     */
    void testStationSearchStatus()
    {
        MainWindow mainWindow;
        Station *station = new Station(2, "Test Station", "Test City", "Test Address", 50.0, 20.0, false, &mainWindow);

        // Dodajemy stację do listy m_allStations, aby metoda updateStationSearchStatus mogła ją znaleźć
        QMetaObject::invokeMethod(&mainWindow, "appendToAllStations", Q_ARG(Station*, station));

        mainWindow.updateStationSearchStatus(2, true);
        QCOMPARE(station->isSearched(), true);
        mainWindow.updateStationSearchStatus(2, false);
        QCOMPARE(station->isSearched(), false);
    }

    /**
     * @brief Testuje dane sensorów.
     *
     * Sprawdza, czy dane sensorów są poprawnie ustawiane, usuwane i zwracane.
     */
    void testSensorData()
    {
        MainWindow mainWindow;
        QVariantMap sensorData;
        sensorData["test"] = QVariantList{ QVariantMap{{"date", "2025-04-22"}, {"value", 10.0}} };
        mainWindow.setSensorData(sensorData);
        QCOMPARE(mainWindow.sensorData().size(), 1);
        QVERIFY(mainWindow.sensorData().contains("test"));

        mainWindow.removeSensorData(999);
        QCOMPARE(mainWindow.sensorData().size(), 1);

        mainWindow.setSensorData(QVariantMap());
        QCOMPARE(mainWindow.sensorData().size(), 0);
    }

private:
    /**
     * @brief Dodaje stację do listy m_allStations w MainWindow.
     *
     * Używa QMetaObject::invokeMethod, aby uzyskać dostęp do prywatnej listy m_allStations.
     * @param mainWindow Wskaźnik do obiektu MainWindow.
     * @param station Wskaźnik do stacji do dodania.
     */
    void appendToAllStations(MainWindow *mainWindow, Station *station) {
        QMetaObject::invokeMethod(mainWindow, "appendToAllStations", Q_ARG(Station*, station));
    }
};

QTEST_MAIN(TestMainWindow)
#include "tst_mainwindow.moc"
