# Budowanie projektu
Każdy skrypt uruchamiamy z poziomu głównego katalogu projektu.
Jeżeli host posiada narzędzie CMake, można użyć skryptu `./scripts/build.sh`,
w przeciwnym wypadku, lub gdy pierwszy skrypt nie zadziała — co nie powinno się zdażyć,
należy uruchomić skrypt `./scripts/poormans-build.sh`.
W obu przypadkach zostanie utworzony katalog `./build`, a w nim plik wykonywalny `aggregation-server`.
Żeby uruchomić serwer, wystarczy uruchomić właśnie ten plik.

Można dodatkowo wygenerować dokumentację do projektu — po jego zbudowaniu,
w katalogu `./build` należy wykonać polecenie `make doc`.

Można też uruchomić automatyczne testy za pomocą skryptu `./scripts/test.sh`.

Kod źródłowy znajduje się oczywiście w katalogu `./src`.
Wydaje się on być zgodny ze standardem POSIX — przed użyciem każdej
zewnętrznej funkcji/struktury/pliku sprawdzałem tą zgodność.
Jednakże, jedynymi systemami na których kompilowałem projekt są:
* `Linux 4.14.113-1-MANJARO #1 SMP PREEMPT Sun Apr 21 12:03:01 UTC 2019 x86_64 GNU/Linux`
* `Linux 4.4.27-1 #1 SMP Sat Oct 22 15:03:31 CEST 2016 x86_64 AMD_Opteron(tm)_Processor_6386_SE PLD Linux`
