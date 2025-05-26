
# Problem jedzących filozofów

## Opis problemu

Problem jedzących filozofów to klasyczny problem synchronizacji w programowaniu. Kilku filozofów siedzi wokół okrągłego stołu i na zmianę myśli oraz je. Każdy z nich potrzebuje dwóch widelców (lewego i prawego), aby móc jeść. Współdzielenie widelców prowadzi do problemów, takich jak zakleszczenie czy zagłodzenie.

## Struktura projektu

Projekt składa się z dwóch głównych klas:
* `Waiter` - synchronizuje dostęp do widelców i zarządza kolejką oczekujących filozofów.
* `Philisopher` - reprezentuje pojedynczego filozofa, który w pętli wykonuje nieustannie operacje myślenia i jedzenia.

## Wątki i ich reprezentacja

W projekcie wykorzystano wielowątkowość poprzez bibliotekę `std::thread`. 
* Każdy filozof jest osobnym wątkiem wykonującycm cykliczne operacje.
* Mutex (`std::mutex`) zapewnia synchronizowaną dostepność do widelców i operacji wyjścia strumienia, aby uniknąć nakładania tekstu i zakleszczenia.
* Główna funckja `main()` inicjalizuje filozofów i uruchamia wątki.

## Sekcje krytyczne i ich rozwiązanie

1. Dostep do widelców (`requestForks` i `releaseForks`)
* Problem: Filozofowie mogą probować jednoczesnie podnieść te same widelce, co może prowadzić do zakleszczenia.
* Rozwiązanie: Mutex w klasie `Waiter` blokuje dostęp do tablicy `forks`.
2. Zagłodzenie
* Problem: Filozofowie mogą nie mieć możliwości podniesienia dwóch widelców na raz przez dłuższy czas.
* Rozwiązanie: Kolejka FIFO (`waitingQueue`) gwarantuje, że filozofowie dostają widelce w kolejności zgłoszeń.
3. Wyjście do konsoli (`std::cout`):
* Problem: Kilka wątków może jednocześnie pisać do `std::cout`, co prowadzi do pomieszania tekstu.
* Rozwiązanie: Dodatkowy mutex `coutMutex` zapewnia, że tylko jeden wątek w danym momencie wypisuje na ekran.

## Instrukcja uruchomienia

### Linux

1. Skompiluj program
```
g++ -o main main.cpp
```
2. Uruchom program, podając liczbę filozofów (np. 5):
```
./main 5
```
3. Program działa w nieskończonej pętli. Aby go zakończyć użyj `Ctrl + C`.

### Windows

1.Skompiluj program (MinGW)
```
g++ -o main main.cpp
```
2. Uruchom program, podając liczbę filozofów (np. 5):
```
main 5
```
3. Program działa w nieskończonej pętli. Aby go zakończyć użyj `Ctrl + C`.