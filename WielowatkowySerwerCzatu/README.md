# Projekt 2 - Systemy Operacyjne 2 - SERWER CZATOWY

## 1. Opis
Niniejszy projekt przedstawia działanie serwera czatowego napisanego w C++ z użyciem biblioteki Winsock2 oraz mechanizmów wielowątkowości (`thread`, `mutex`). Celem projektu było stworzenie serwera umożliwiającego komunikację tekstową między wieloma klientami w czasie rzeczywistym poprzez TCP. Dodatkowo została zaimplementowana identyfikacja użytkowników za pomocą nazw i losowo przypisanych kolorów.

## 2. Technologie i biblioteki
-	Język programowania: C++
-	Platforma: Windows
-	Biblioteki:
	- `winsock2.h` – do obsługi gniazd (socketów) TCP/IP
	- `thread`, `mutex` – do obsługi wielowątkowości
	- `vector`, `map`, `string`, `algorithm`, `random` – kontenery STL i inne funkcjonalności standardowej biblioteki C++

## 3. Opis Działania Serwera
#### 1. Inicjalizacja
-	Uruchamia Winsock, tworzy i wiąże gniazdo TCP, nasłuchuje na porcie 7000.
#### 2. Wątek wejścia (serverInputThread)
-	Odbiera komendy z konsoli:
-	`/exit` zamyka serwer.
-	Inne wiadomości rozgłasza do klientów z prefiksem `[Server]`.
#### 3.  Obsługa połączeń
-	Główna pętla akceptuje klientów i uruchamia osobny wątek dla każdego (`handleClient`).
#### 4. Obsługa klienta (handleClient)
-	Dodaje klienta do listy, przypisuje nazwę i kolor.
-	W pętli odbiera wiadomości:
-	W przypadku rozłączenia — usuwa dane.
-	Inaczej — przekazuje sformatowaną wiadomość do broadcast.
#### 5. Rozgłaszanie wiadomości (broadcast)
-	Wysyła wiadomości do wszystkich klientów, wyświetla je w konsoli.
#### 6. Kolory użytkowników
-	Funkcja getRandomColorCode() przypisuje losowy kolor ANSI nowym klientom.
#### 7. Wielowątkowość
-	Mutexy chronią wspólne zasoby (lista klientów, konsola).

## 4. Opis działania klienta
#### 1. Połączenie z serwerem
-	Podaje się IP i nazwę.
-	Tworzy się socket i łączy z serwerem.
-	Wysyłana jest nazwa, uruchamiany jest wątek odbiorczy.
#### 2. Obsługa wejścia i UI
-	Klawisze sterują czatem:
-	Strzałki – przewijanie.
-	Enter – wysyłanie lub /exit.
-	Inne znaki – edycja wiadomości.
-	Zmiany odświeżają interfejs (drawUI).
#### 3. Odbiór wiadomości (receiveMessages)
-	W osobnym wątku:
-	Odbiera dane od serwera.
-	Dodaje je do bufora czatu.
-	Informuje o zerwaniu połączenia.
#### 4. Wyświetlanie UI (drawUI)
-	Czyści ekran, rysuje czat i pole wejścia.
-	Dostosowuje się do rozmiaru terminala.


## 5. Sekcje krytyczne i ich rozwiązanie

W serwerze zastosowano dwa mutexy do zabezpieczenia sekcji krytycznych:
1.	`clientsMutex` – chroni: listę klientów (clients), mapę nazw (clientNames), mapę kolorów (clientColors). Dzięki temu unikamy konfliktów przy jednoczesnym dołączaniu, rozłączaniu i wysyłaniu wiadomości.
 2.	`printMutex` – zabezpiecza wypisywanie na konsolę (std::cout), by uniknąć nakładającego się tekstu z wielu wątków.

## 6. Instrukcja uruchomienia

**Windows**

1.	Skompiluj server.cpp i client.cpp
```bash
g++ -Wall -std=c++17 -o server.exe server.cpp -lws2_32
g++ -Wall -std=c++17 -o client.exe client.cpp -lws2_32
```
 2.	Uruchom server i clienta
```bash
server

client
```
	
W przypadku klienta trzeba podać adres IP serwera, lub pozostawić puste dla localhost, następnie nazwę użytkownika bez polskich znaków.

3.	Server i klient działa w nieskończonej pętli. Aby zakończyć użyj **Ctrl + C** lub komendy **/exit**.

## 7. Wnioski
Zaimplementowany serwer czatowy stanowi podstawę dla systemów komunikacji sieciowej, efektywnie wykorzystując mechanizmy gniazd oraz wielowątkowość do obsługi wielu jednoczesnych połączeń. Projekt ten demonstruje praktyczne zastosowanie synchronizacji wątków za pomocą mutexów, co jest kluczowe dla stabilności i niezawodności aplikacji działających równolegle. Aplikacja w prosty sposób realizuje funkcjonalność czatu tekstowego z obsługą wielu klientów i kolorową identyfikacją użytkowników.
