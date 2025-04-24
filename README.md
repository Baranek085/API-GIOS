Stacje Pomiarowe


Opis projektu
Stacje Pomiarowe to aplikacja desktopowa stworzona w celu monitorowania jakości powietrza w Polsce.
Umożliwia użytkownikom wyszukiwanie stacji pomiarowych Głównego Inspektoratu Ochrony Środowiska (GIOŚ) według miast,
wyświetlanie danych z sensorów na wykresach oraz zapisywanie i przeglądanie danych archiwalnych.
Aplikacja została zbudowana przy użyciu frameworka Qt z wykorzystaniem C++ i QML.

Główne funkcjonalności

Wyszukiwanie stacji: Wyszukiwanie stacji pomiarowych w wybranym mieście przy użyciu API Nominatim i API GIOŚ.
Mapa interaktywna: Wyświetlanie lokalizacji stacji na mapie opartej na OpenStreetMap z możliwością przybliżania i przesuwania.
Wykresy danych: Prezentacja danych z sensorów w formie wykresów z uwzględnieniem wartości minimalnych, maksymalnych i średnich.
Archiwizacja danych: Zapisywanie danych stacji w plikach JSON oraz przeglądanie zarchiwizowanych danych.
Testy jednostkowe: Wdrożone testy jednostkowe dla kluczowych komponentów aplikacji przy użyciu Qt Test.

Wymagania

System operacyjny: Windows, Linux lub macOS
Qt: Wersja 6.8.3 lub nowsza
Kompilator: MinGW 64-bit (dla Windows) lub kompatybilny z Qt
Zależności: Qt Core, Qt GUI, Qt Network, Qt QML, Qt Quick, Qt Positioning, Qt Location, Qt Test

Instalacja

Sklonuj repozytorium:
git clone https://github.com/Baranek085/API-GIOS.git
cd API-GIOS


Zainstaluj Qt: Pobierz i zainstaluj Qt z oficjalnej strony lub menedżera pakietów (np. apt dla Ubuntu).

Skompiluj projekt:
qmake project.pro
make


Uruchom aplikację:
./stacje_pomiarowe


Uruchom testy jednostkowe (opcjonalnie):
qmake "CONFIG += test" project.pro
make
./tst_mainwindow



Struktura projektu

main.cpp: Główny punkt wejścia aplikacji, konfiguruje silnik QML i inicjalizuje klasę MainWindow.

mainwindow.h / mainwindow.cpp: Klasa MainWindow zarządza logiką aplikacji, w tym pobieraniem danych z API i obsługą archiwizacji.

StationDialog.qml: Okno dialogowe wyświetlające szczegóły stacji, wykresy i statystyki.

ArchivedDataDialog.qml: Okno dialogowe do przeglądania listy zarchiwizowanych danych.

ArchivedStationDialog.qml: Okno dialogowe wyświetlające szczegóły zarchiwizowanych danych stacji.

main.qml: Główny interfejs użytkownika z mapą, listą stacji i paskiem wyszukiwania.

tst_mainwindow.cpp: Testy jednostkowe dla klas MainWindow i Station.

project.pro: Plik konfiguracyjny projektu Qt.


Użycie

Wyszukiwanie miasta:

Wpisz nazwę miasta w polu wyszukiwania i kliknij "Szukaj".
Aplikacja wyświetli listę stacji w wybranym mieście lub najbliższą stację, jeśli żadna nie zostanie znaleziona.


Przeglądanie danych:

Kliknij stację na liście lub znacznik na mapie, aby otworzyć okno dialogowe z szczegółami.
Wybierz parametry (np. PM10, PM2.5) do wyświetlenia na wykresie.
Przeglądaj statystyki, takie jak aktualny odczyt, średnia, minimum i maksimum.


Zapisywanie danych:

W oknie dialogowym stacji kliknij "Zapisz dane", aby zapisać dane do pliku JSON.


Przeglądanie archiwum:

Kliknij przycisk "Archiwum", aby otworzyć listę zapisanych danych.
Wybierz zestaw danych, aby wyświetlić szczegóły w oknie dialogowym.



API
Aplikacja korzysta z następujących API:

GIOŚ API: Pobieranie danych o stacjach, sensorach i pomiarach (https://api.gios.gov.pl/pjp-api/rest).
Nominatim API: Geokodowanie miast (https://nominatim.openstreetmap.org).

Autorzy

Adam Fedorowicz: Główny programista, implementacja logiki aplikacji i interfejsu QML.

Licencja
Projekt jest dostępny na licencji MIT. Szczegóły w pliku LICENSE.
Kontakt
W razie pytań lub problemów skontaktuj się przez GitHub Issues.
