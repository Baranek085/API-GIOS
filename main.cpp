/**
 * @file main.cpp
 * @brief Główny punkt wejścia aplikacji monitorowania jakości powietrza.
 * @author Adam Fedorowicz
 * @date 2025-04-21
 *
 * Ten plik zawiera funkcję główną, która inicjalizuje aplikację Qt
 * i konfiguruje silnik QML.
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "mainwindow.h"

/**
 * @brief Główna funkcja aplikacji.
 * @param argc Liczba argumentów wiersza poleceń.
 * @param argv Tablica argumentów wiersza poleceń.
 * @return Kod wyjścia aplikacji.
 *
 * Inicjalizuje aplikację Qt, tworzy instancję MainWindow,
 * konfiguruje kontekst QML i ładuje główny plik QML.
 */
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Utworzenie instancji MainWindow
    MainWindow mainWindow;

    QQmlApplicationEngine engine;
    // Udostępnienie MainWindow w QML jako "mainWindow"
    engine.rootContext()->setContextProperty("mainWindow", &mainWindow);

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    /**
     * @brief Ładuje główny plik QML i obsługuje błędy ładowania.
     *
     * Połączenie sygnału objectCreated z funkcją sprawdzającą, czy obiekt QML został utworzony.
     * Jeśli obiekt nie zostanie utworzony, aplikacja zakończy działanie z kodem błędu.
     */
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl)
                             QCoreApplication::exit(-1);
                     }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
