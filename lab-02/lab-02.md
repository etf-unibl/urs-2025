# Laboratorijska vježba 2

Cilj laboratorijske vježbe je da se studenti upoznaju sa postupkom (kros)kompajliranja
aplikacija u C programskom jeziku, kao i sa alatima za automatizaciju postupka
generisanja binarnih fajlova za ciljnu *embedded* platformu.

Nakon uspješno realizovane vježbe studenti će biti sposobni da:
1. kroskompajliraju i linkuju programe sa dinamičkim i statičkim bibliotekama
korišćenjem odgovarajućeg *toolchain*-a,
2. automatizuju proces kroskompajliranja korišćenjem *make* alata,
3. koriste *autotools* skup alata za kroskompajliranje *third-party* biblioteka i
4. samostalno kreiraju *CMake* pravila za automatizovano kroskompajliranje
*embedded* aplikacija.

## Preduslovi za izradu vježbe

Za uspješno izvođenje laboratorijske vježbe potrebno je ispunjeno sljedeće:

- kloniran je repozitorijum laboratorijskih vježbi i
- na razvojnoj platformi je instaliran *CMake* softverski paket sa minimalnom verzijom 3.0.

## Manuelno (kros)kompajliranje aplikacija i biblioteka

Ukoliko već niste, pređite u folder druge laboratorijske vježbe (`lab-02`)

```
cd lab-02
```

i upoznajte se sa strukturom ovog foldera. Kao što možete primjetiti, dati folder sadrži
sljedeće podfoldere i fajlove:

```
lab-02
│   lab-02.md
├───app
│   │   Makefile
│   ├───inc
│   │       test.h
│   ├───lib
│   │       Makefile
│   └───src
│           app.c
│           print.c
│           sum.c
└───sqlite-test
        sqlite-test.c
```

Fajl `lab-02.md` predstavlja ovaj tekst u *markdown* formatu. Uz njega, imamo dva foldera:
`app`, u kojem se nalaze resursi neophodni za (kros)kompajliranje aplikacije u ovom dijelu
vježbe, i `sqlite-test`, koji sadrži aplikaciju za testiranje *SQlite3* biblioteke u narednim
dijelovima laboratorijske vježbe.

Unutar `app` foldera, u podfolderu `src` nalaze se fajlovi sa izvornim kodom aplikacije (`app.c`)
i biblioteke funkcija (`print.c` i `sum.c`) koju ćemo zvati `test`. Podfolder `inc` sadrži
*header* fajl `test.h` sa definisanim prototipovima funkcija iz biblioteke. Fajl pod nazivom
`Makefile` ćemo koristiti u narednoj sekciji laboratorijske vježbe za automatizovano (kros)kompajliranje
korišćenjem *make* alata.

Cilj ovog dijela vježbe je da demonstriramo proceduru (kros)kompajliranja biblioteke i aplikacije.
Radi efikasnijeg rada, preporuka je da se prvo obavi kompajliranje korišćenjem *native* kompajlera
(`gcc`), kako bi se moglo obaviti testiranje rada programa na razvojnoj platformi (i otklonili
eventualni nedostaci), nakon čega je postupak potrebno ponoviti korišćenjem kroskompajlera iz
*toolchain*-a koji je generisan u prvoj laboratorijskoj vježbi. Testiranje rezultata kroskompajliranja
obavljamo na ciljnoj platformi tako što kopiramo izvršni fajl aplikacije, kao i ostali artifakte
(npr. dinamičku biblioteku) koji su neophodni za njegovo izvršavanje. U tom smislu, neophodno je
kopirati relevantne fajlove na odgovarajuću lokaciju u *root filesystem* na SD kartici.

> [!IMPORTANT]
> Prije postupka kroskompajliranja, potrebno je eksportovati putanju do kroskompajlera,
kao što je objašnjeno u prvoj laboratorijskoj vježbi, a u tu svrhu može da se koristi i skripta iz
pomenute vježbe, koja je dostupna u okviru repozitorijuma (folder `scripts`).

### Statičko linkovanje

Prvo ćemo ilustrovati korake za generisanje statičke bibloteke sa kojom ćemo zatim linkovati našu
aplikaciju. Kompajliramo fajlove sa izvornim kodom funkcija koje su dio *test* biblioteke:

```
cd app
gcc -c src/print.c -I./inc
gcc -c src/sum.c -I./inc
```

Dobijamo objektne fajlove `print.o` i `sum.o` koje zatim arhiviramo u jedan fajl koji predstavlja
statičku biblioteku `libtest.a`, a zatim ovako kreiranu biblioteku prebacujemo u `lib` folder:

```
ar rcs libtest.a print.o sum.o
mv libtest.a lib
```

Konačno, možemo pristupiti kompajliranju i linkovanju aplikacije `app.c` koja koristi ovu biblioteku.

```
gcc -o app src/app.c -L./lib -I./inc -ltest
```

Dobijeni izvršni fajl `app` možemo izvršiti na razvojnoj platformi jer je kompajliran *native*
kompajlerom. Očekivani rezultat izvršavanje je sljedeći tekst:

```
Printing a debug message.
        Sum of array elements is 15.
```

Ako pokrenemo komandu `file app` vidjećemo poruku *dynamically linked* iako smo aplikaciju statički
linkovali sa bibliotekom. Pokretanjem skripte iz repozitorijuma za prikaz zavisnosti fajla od
drugih biblioteka

```
../../scripts/list-libs.sh app
```

vidjećemo da izvršni fajl i dalje zavisi od standardne biblioteke i dinamičkog linkera, ali da ne
zavisi od biblioteke `libtest`, što znači da ona **jeste** statički linkovana sa programom.

Da dobijemo potpuno statički linkovan program, koristimo dodatni linker fleg `-static`:

```
gcc -o app src/app.c -static -L./lib -I./inc -ltest
```

Ako ponovo izvršimo komandu `file app` vidjećemo poruku *statically linked*, što znači da je program
u potpunosti statički linkovan. Ovo možemo potvrditi i skriptom `list-libs.sh` koja ne bi trebalo
da prikaže zavisnost od bilo koje biblioteke.

Da biste prešli na sljedeći korak, obrišite sve artifakte dobijene kompajliranjem (objektne fajlove,
biblioteku i izvršni fajl).

```
rm app *.o lib/libtest.a
```

### Dinamičko linkovanje

Ponovićemo korake iz prethodne sekcije, ali ovaj put ćemo dinamički linkovati program sa bibliotekom.
Prvo trebamo da kompajliramo izvorne fajlove biblioteke. S obzirom da ćemo koristiti dinamičko
linkovanje, fajlovi se moraju kompajlirati sa `-fPIC` kompajlerskim flegom:

```
gcc -c -fPIC src/print.c -I./inc
gcc -c -fPIC src/sum.c -I./inc
```

Nakon toga, dobijene objektne fajlove povezujemo u jedinstvenu dinamičku biblioteku:

```
gcc -shared print.o sum.o -o libtest.so
mv libtest.so lib
```

nakon čega možemo da kompajliramo i linkujemo i samu aplikaciju:

```
gcc -o app src/app.c -L./lib -I./inc -ltest
```

Ako pokrenemo skriptu `list-libs.sh` nad dobijenim fajlom, vidjećemo da on zavisi i od *test* biblioteke.
Pokretanjem izvršnog fajla, dobićemo sljedeću grešku:

```
./app: error while loading shared libraries: libtest.so: cannot open shared object
file: No such file or directory
```

To je zato što se generisana biblioteka ne nalazi u očekivanim sistemskim folderima, tako da dinamički
linker ne može da je učita. Ovo se može riješiti eksportovanjem sistemske varijable `LD_LIBRARY_PATH`:

```
export LD_LIBRARY_PATH=./lib
```

Konačno, obrišite sve artifakte dobijene kompajliranjem kao i u prethodnom slučaju prije prelaska na
sljedeći korak.

```
rm app *.o lib/libtest.so
```

### Kroskompajliranje

U ovom dijelu vježbe potrebno je ponoviti prethodno opisane korake za statičko i dinamičko linkovanje
programa ali tako da se program kroskompajlira za ciljnu platformu na DE1-SoC razvojnoj ploči korišćenjem
*toolchain*-a generisanog u prvoj laboratorijskoj vježbi.

U tu svrhu, prvo je potrebno eksportovati sistemsku varijablu `PATH` tako da sadrži putanju do alata koji
su sastavni dio pomenutog *toolchain*-a. To se može učiniti komandom

```
export PATH=$HOME/x-tools/arm-etfbl-linux-gnueabihf/bin:$PATH
```

ali možemo da iskoristimo i postojeću skriptu (`set-environment.sh` iz foldera `scripts`) u ovu svrhu.

Ponovite prethodne korake, pri čemu je programima `gcc` i `ar` potrebno dodati prefiks `arm-linux-`
(uz pretpostavku da ste pri konfiguraciji *toolchain*-a podesili ovaj prefiks kao *alias*, u suprotnom
ćete morati koristiti puni *tuple* arhitekture.

> [!NOTE]
Osim eksportovanja putanje do *toolchain* alata, skripta `set-environment.sh` podešava i
druge varijable (`CROSS_COMPILE`, `ARCH` i `SYSROOT) koje se koriste u drugim skriptama ili prilikom
automatizacije procesa kroskompajliranja (npr. skripta `list-libs.sh` koristi varijablu `CROSS_COMPILE`).

Nakon što generišete neophodne fajlove, trebate ih prekopirati na SD karticu za izvršavanje na ciljnoj
platformi. Vodite računa da je kompletan *root filesystem* "u vlasništvu" *root* korisnika, tako da će
pri kopiranju biti potrebno koristiti `sudo` privilegije.

> [!IMPORTANT]
Kod dinamičkog kompajliranja potrebno je uz izvršni fajl aplikacije prekopirati i fajl
dinamičke biblioteke (`libtest.so`). Takođe, kao i kod *native* izvršavanja, na ciljnoj platformi ćete
trebati eksportovati `LD_LIBRARY_PATH` varijablu (osim ako biblioteku kopirate u sistemske foldere).


## Automatizacija postupka (kros)kompajliranja korišćenjem *make* alata

Proces (kros)kompajlirana postaje veoma kompleksan kada imamo veliki broj izvornih fajlova koje treba
kompajlirati i linkovati. Kako bi se postupak ubrzao, razvijeni su pomoćni alati poput *make* alata, koji
omogućavaju automatizaciju ovog procesa. Koraci neophodni za proizvodnju nekog softvera se opisuju pravilima
u okviru fajla pod nazivom `Makefile` koji *make* alatka parsira i interpretira na odgovarajući način.

U `app` folderu se nalazi primjer ovog fajla za aplikaciju koju smo manuelno kompajlirali u prethodnoj sekciji
vježbe. Otvorite ovaj fajl i upoznajte se sa njegovom strukturom.

Na početku fajla definišemo sadržaj implicitnih varijabli koje sadrže flegove za kompajler i linker.

```
CFLAGS := -Wall -I./inc
LDFLAGS := -static -ltest -L./lib
```

Varijabla `CFLAGS` sadrži fleg `-Wall` koja uključuje prikazivanje dodatnih informacija prilikom kompajliranja
(uključujući i upozorenja, ne samo greške), dok opcijom `-I./inc` specificiramo lokaciju na kojoj se nalaze
*header* fajlovi koje koristi aplikacija.

U varijabli `LDFLAGS` definisani su flegovi koji opisuju proces linkovanja. Prvo, opcijom `-static` specificiramo
da želimo da se obavi statičko linkovanje. Opcija `-ltest` linkeru govori da je aplikaciju potrebno linkovati
sa bibliotekom pod nazivom *test*, a fleg `-L./lib` definiše lokaciju na kojoj se biblioteka nalazi.

U okviru `Makefile` fajla definisana su sljedeća ciljna pravila: `app`, `src/app.o`, `lib/libtest.a` i `clean`.

```
app: src/app.o lib/libtest.a
	$(CROSS_COMPILE)gcc src/app.o -o $@ $(LDFLAGS)
```

Pravilo `app` je podrazumijevano pravilo koje se aktivira komandom `make` bez argumenata. Ovo pravilo generiše
izvršni fajl `app`, pri čemu je preduslov za izvršavanje komande date ispod postojanje fajlova `src/app.o` i
`lib/libtest.a`. Vidimo da će komandom da se linkuje objektni fajl `app.o` sa bibliotekom `libtest.a`. Ovdje
treba pomenuti da se, kao prefiks `gcc` programu, koristi vrijednost varijable `CROSS_COMPILE`. Ako ova
varijabla nije definisana koristi se *native* kompajliranje. S druge strane, ako postavimo vrijednost ove
varijable tako da odgovara prefiksu kroskompajlera, koristiće se kroskompajliranje. Konačno, pri linkovanju se
koriste i flegovi definisani u `LDFLAGS` varijabli.

```
src/app.o: src/app.c
	$(CROSS_COMPILE)gcc -c $< $(CFLAGS) -o $@
```

Generisanje izvršnog fajla aplikacije nije moguće ako nije generisan njen objektni fajl u `src` folderu.
Upravo tu svrhu ima pravilo dato iznad. Kao što vidimo, komanda ispod (kros)kompajlira izvorni kod
aplikacije (`app.c`) i generiše pomenuti objektni fajl, pri čemu se pri kompajliranju koriste prethodno
definisani kompajlerski flegovi (varijabla `CFLAGS`). Takođe, vidimo da je aktiviranje ovog pravila
uslovljeno postojanjem fajla sa izvornim kodom (`app.c`) u `src` folderu.

```
lib/libtest.a:
	$(MAKE) -C lib libtest.a
```

Generisanje izvršnog fajl takođe je uslovljeno postojanjem fajl *test* statičke biblioteke `libtest.a` u
`lib` folderu. Ovo pravilo rekurzivno poziva `make libtest.a` komandu unutar `lib` foldera kao što je
dato u isječku prikazanom iznad.

```
clean:
	rm -f app src/*.o
	$(MAKE) -C lib clean
```

Pravilo `clean` briše sve artifakte generisane pri procesu generisanja izvršnog fajla aplikacije. Ovo pravilo
definišemo kao `PHONY`, jer, za razliku od prethodnih, ne generiše izlazni fajl koji odgovara nazivu pravila.

Pravilo `lib/libtest.a` izvršava komandu `make libtest.a` unutar `lib` foldera (slično imamo i kod `clean`
pravila, samo što se izvršava komanda `make clean`). Otvorite `Makefile` koji se nalazi u `lib` folderu i
upoznajte se sa njegovom strukturom.

Kao i kod prethodnog fajl, i ovdje prvo definišemo sadržaj varijabli:

```
CFLAGS := -Wall -I../inc
SRCS_DIR := ../src
```

Varijabla `CFLAGS` ima sličnu ulogu kao u prethodnom slučaju, dok varijablom `SRCS_DIR` definišemo lokaciju
foldera u kojem su smješteni fajlovi sa izvornim kodom.

```
libtest.a: print.o sum.o
	$(CROSS_COMPILE)ar rcs $@ $^
```

Pravilo `libtest.a` zavisi od fajlova `print.o` i `sum.o`, a komanda koja se aktivira koristi `ar` alatku
za kreiranje arhive statičke biblioteke.

```
%.o: $(SRCS_DIR)/%.c
	$(CROSS_COMPILE)gcc -c $< $(CFLAGS) -o $@
```

Pravilom datim iznad prosto kompajliramo sve izvorne fajlove koji se nalaze u `SRCS_DIR` folderu. Naravno,
u datom slučaju kompajliraće se samo neophodni objektni fajlovi.

Konačno, imamo pravilo za brisanje generisanje objektnih fajlova i kreirane biblioteke

```
clean:
	rm -f *.o libtest*
```

koje je takođe `PHONY` tipa zbog istih razloga kao u prethodnom slučaju.

Sada ćemo kompajlirati aplikaciju na način da je možemo izvršiti na razvojnoj platformi. Uz pretpostavku da se
nalazimo u folderu `app`, potrebno je samo pokrenuti komandu `make`. Generisaće se statički linkovan izvršni
fajl. Ako pokrenemo komandu `make clean`, svi artifakti će biti obrisani.

Da bismo kroskompajlirali aplikaciju, potrebno je da pokrenemo skriptu `set-environment.sh`

```
../../scripts/list-libs.sh app
```

a onda ponovo da pokrenemo `make` komandu. Inspekcijom dobijenog fajla pomoću `file app` možemo potvrditi da smo
dobili ispravan fajl format, a zatim ga i testirati na ciljnoj platformi. Takođe, korišćenjem skripte `list-libs.sh`
možemo potvrditi da je program statički linkovan.

U narednom koraku je potrebno da proširite date `Makefile` fajlove tako da se predvidi podrška za dinamičko
linkovanje programa. U tom smislu je potrebno uvesti varijablu `STATIC` kojom se definiše da li želimo da statički
da linkujemo program ili ne (predvidjeti da je statičko linkovanje podrazumijevano). Definisanjem vrijednosti ove
varijable prilikom pozivanja komande `make` možemo da biramo način linkovanja.

Osim toga, potrebno je predvidjeti i dodatno pravilo za instalaciju generisanog izvršnog fajla na lokaciju koja
je određena varijablom `DESTDIR`.

Sve modifikovane fajlove je potrebno predati na zasebnu granu u repozitorijumu kursa, koju student kreira prema
pravilima koji su definisani u sekciji "Predaja rezultata rada".

## Kroskompajliranje SQLite3 biblioteke pomoću *Autotools* alata

*Autotools* skup alata se koristi prilikom (kros)kompajliranja kompleksnijih projekata. Ovdje ćemo ilustrovati
njegovu primjenu na primjeru *third-party* biblioteke *SQLite3*.

> [!NOTE]
> Za korišćenje *Autotools* alata, potrebno je da imate instalirane sljedeće pakete: `pkgconf` i `pkg-config`.
> Iako bi ovi paketi trebalo da budu dostupni nakon instalacije softverskih paketa u okviru prve vježbe, nije naodmet
> da provjerite da ih imate instalirane prije izrade dijela vježbe koji slijedi.

Prvo trebamo da preuzmemo i raspakujemo arhivu sa izvornim kodom *SQLite3* biblioteke.

```
wget https://sqlite.org/2024/sqlite-autoconf-3450200.tar.gz
tar xf sqlite-autoconf-3450200.tar.gz
cd sqlite-autoconf-3450200
```

Zatim ćemo da pokrenemo skriptu za postavljanje okruženja (`set-environment.sh`), jer želimo da kroskompajliramo
ovu biblioteku.

Sljedeći korak je generisanje `Makefile` fajlova korišćenjem `configure` skripte.

```
CC=arm-linux-gcc ./configure --host=arm-linux --prefix=/usr
```

Postavljanjem varijable `CC` biramo naziv kroskompajlera (navodi se naziv kompajlera, a ne *toolchain* prefiks).
Opcijom `--host=arm-linux` izbjegavamo potencijalne greške prilikom pokretanja skripte zbog neusklađenosti
arhitekture na kojoj se izvršava kod biblioteke sa razvojnom platformom, dok opcijom `--prefix=/usr` specificiramo
da biblioteku želimo instalirati na lokaciju `<sysroot>/usr/*` umjesto na podrazumijevanu `<sysroot>/usr/local/*`
lokaciju, čime izbjegavamo eventualne probleme prilikom instalacije bez `sudo` privilegija.

Nakon što je konfiguracija uspješno završena i potrebni fajlovi generisani, pokrećemo komande za Kroskompajliranje
i instalaciju softvera.

```
make
make DESTDIR=$SYSROOT install
```

Ispravnu instalaciju možete da potvrdite inspekcijom sadržaja foldera `<sysroot>/usr/bin`, `<sysroot>/usr/lib` i
`<sysroot>/usr/lib/include`. U okviru ovih foldera bi trebalo da se nalaze fajlovi koji imaju riječ `sqlite3`
u svom nazivu.

Sada možemo kroskompajlirati testnu aplikaciju:

```
cd sqlite-test
export PKG_CONFIG_LIBDIR=$SYSROOT/usr/lib/pkgconfig
arm-linux-gcc $(pkg-config sqlite3 --cflags --libs) sqlite-test.c -o sqlite-test
```

Trebalo bi da dobijete izvršni fajl `sqlite-test` čijom provjerom pomoću skripte `list-libs.sh` možemo da potvrdimo da
je dinamički linkovan sa *SQLite3* bibliotekom.

Konačno, kopirajte izvršni fajl i fajlove biblioteke na SD karticu i testirajte izvršavanje aplikacije na ciljnoj
platformi.

> [!IMPORTANT]
Fajlove biblioteke `libsqlite3.so.0.8.6`, `libsqlite3.so.0` i `libsqlite3.so` iz *sysroot* foldera trebate
kopirati u folder `/usr/lib` na ciljnoj platformi.

## Korišćenje *CMake*-a za automatizaciju (kros)kompajliranja

Primjenu *CMake* alata za automatizovano (kros)kompajliranje prvo ćemo ilustrovati na primjeru `hello` programa iz
prve laboratorijske vježbe. U tom smislu, prekopirajte kompletan `hello` folder iz prve laboratorijske vježbe u
folder `lab-02`, a zatim u tako kopiran folder i napravite prazan fajl pod nazivom `CMakeLists.txt`.

```
cp -r ../lab-01/hello .
cd hello
touch CMakeLists.txt
```

Otvorite ovaj fajl bilo kojim editorom teksta i dodajte sljedeće tri linije:

```
cmake_minimum_required(VERSION 3.0)
project(hello)
add_executable(hello hello.c)
```

a zatim sačuvajte izmjene.

Prva linija definiše minimalnu verziju *CMake* softvera koja neophodna. U drugoj liniji definišemo naziv projekta, dok
u trećoj definišemo ciljni izvršni fajl koji želimo da generišemo, kao i naziv fajla izvornog koda.

Program ćemo kompajlirati tako da se artifakti nakon kompajliranja nalaze u zasebnom folderu pod nazivom `build`.

```
mkdir build
cd build
cmake ..
cmake --build .
```

Provjerite da li se program ispravno izvršava na razvojnoj platformi.

Da bismo kroskompajlirali program, potrebno je da prilikom pokretanja komande `cmake` specificiramo kroskompajler koji
koristimo.

```
rm -r *
cmake .. -DCMAKE_C_COMPILER=$HOME/x-tools/arm-etfbl-linux-gnueabihf/bin/arm-linux-gcc
cmake --build .
```

Komandu `rm -f *` unutar `build` foldera koristimo da bismo očistili sve fajlove dobijene na osnovu prethodne konfiguracije.
To je neophodno inače će se pojaviti greške prilikom rekonfiguracije sistema.

Sljedeći korak je (kros)kompajliranje korisničke biblioteke. U tom smislu, krirajte novi folder `app-cmake` unutar `lab-02` foldera,
a onda napravite folder pod nazivom `test` i kopirajte izvorne i *header* fajlove biblioteke u ovaj folder. Prekopirajte i izvorni
kod aplikacije u folder `app-cmake`

```
mkdir app-cmake
cd lab-02/app-cmake
mkdir test
cp ../app/inc/test.h ../app/src/print.c ../app/src/sum.c test
cp ../app/src/app.c .
```

Kreirajte dva `CMakeLists.txt` fajla, jedan u `app-cmake` folderu, a drugi u `test` folderu. Krovni `CMakeLists.txt` treba da ima iste
tri linije kao prethodnom primjeru, s tom razlikom da je potrebno da se umjesto `hello` referencira `app`. Uz ove tri linije dodaćemo 
i sljedeće za uključenje korisničke biblioteke:

```
add_subdirectory(test)
target_link_libraries(app PUBLIC test)
target_include_directories(app PUBLIC
	"${PROJECT_SOURCE_DIR}/test")
```

U okviru `CMakeLists.txt` fajla koji se nalazi u `test` folderu, potrebno je definisati samo jednu liniju:

```
add_library(test print.c sum.c)
```

Kreirajte zaseban `build` folder i kompajlirajte projekat.

```
mkdir build
cd build
cmake ..
cmake --build .
```

Provjerite da li se generisani program izvršava ispravno, a zatim ponovite sve ali tako da kroskompajlirate program. Testirajte
ovako generisan program na ciljnoj platformi.

Eksperimentišite sa tipom generisane biblioteke (statička ili dinamička) korišćenjem kvalifikatora `STATIC` i `SHARED` u okviru
`add_library()` direktive. Pokušajte da to ponovite za ciljnu platformu kroskompajliranjem.

> [!WARNING]
Kada kompajlirate program povezan sa dinamičkom bibliotekom, možete primjetiti da nije neophodno eksportovati
varijablu `LD_LIBRARY_PATH` za izvršavanje na razvojnoj platformi, jer je to riješeno na nivou *CMake* sistema. Međutim,
probajte da izvršite tako kopiran fajl sa dinamičkom bibliotekom na ciljnoj platformi. Gdje se očekuje da biblioteka bude
smještena na ciljnoj platformi?

U posljednjem koraku, potrebno je demonstrirati postupak kroskompajliranja `sqlite-test` aplikacije koja treba da se linkuje sa
*SQLite3* bibliotekom u prethodnoj sekciji vježbe. U tu svrhu iskoristite *CMake* funkciju `find_package()` koju smo pominjali
na predavanjima i *toolchain* fajl definisan u fajlu `arm_cortex_a9.cmake` koji se nalazi u folderu `toolchains` unutar `lab-02`
foldera.

> [!IMPORTANT]
*Toolchain* fajl u repozitorijumu je potrebno prilagoditi računaru koji koristi student, jer se definisane putanje
razlikuju od onih koje su navedene.

Na kraju, predajte sve kreirane i modifikovane fajlove na prethodno kreiranu granu u repozitorijumu.
