# Laboratorijska vježba 1 

Cilj laboratorijske vježbe je da se studenti upoznaju sa *Crosstool-NG* alatom za
automatizovano generisanje *toolchain*-a za kroskompajliranje *embedded* aplikacija.

Nakon uspješno realizovane vježbe studenti će biti sposobni da:
1. preuzmu i instaliraju *Crosstool-NG* alat,
2. odaberu, konfigurišu i generišu *toolchain* u skladu sa postavljenim zahtjevima i
3. kroskompajliraju jednostavnu C aplikaciju korišćenjem generisanog *toolchain*-a.

## Preduslovi za izradu vježbe

Da bi se izbjegli eventualni problemi prilikom instalacije softverskih paketa, prvo je
potrebno primijeniti posljednja ažuriranja u bazi softverskih paketa distribucije
operativnog sistema unošenjem sljedećih komandi u terminalu:

```
sudo apt update
sudo apt dist-upgrade
```

Instalirajte sljedeće softverske pakete:

```
sudo apt install build-essential git autoconf bison flex texinfo help2man gawk \
	libtool-bin libncurses5-dev unzip
```

## Preuzimanje i instalacija *Crosstool-NG* alata

*Crosstool-NG* ćemo preuzeti preko u formi `git` repozitorijuma i prebaciti se na
najnoviju stabilnu verziju:

```
git clone https://github.com/crosstool-ng/crosstool-ng
cd crosstool-ng/
git checkout crosstool-ng-1.26.0
```

S obzirom da smo *Crosstool-NG* preuzeli kao `git` repozitorijum, prvo trebamo da
generišemo `configure` skriptu sljedećom komandom:

```
./bootstrap
```

Sljedeći korak je sama instalacija alata, tj. njegovo kompajliranje na razvojnoj
platformi (PC računar). Za potrebe laboratorijske vježbe, softver ćemo instalirati
lokalno:

```
./configure --enable-local
make
```

Da bismo potvrdili da je alat uspješno instaliran, pokrenućemo `help` komandu

```
./ct-ng help
```

koja nam izlistava sve dostupne opcije.

## Konfiguracija i generisanje *toolchain*-alat

Nakon što smo uspješno instalirali *Crosstool-NG*, možemo da pristupimo konfiguraciji
željenog *toolchain*-a. U tom smislu, potrebno je da izaberemo odgovarajuću arhitekturu,
kao i njene karakteristike, te standardnu C biblioteku i ispravne verzije ostalih
komponenata.

*Crosstool-NG* već ima određeni broj predefinisanih konfiguracija koje nazivamo *samples*.
Dobra praksa je da se identifikuje najbliža predefinisana konfiguracija za željenu
arhitekturu i da se ona koristi kao osnova za dalje modifikacije. Na taj način, broj
izmjena u konfiguraciji svodimu na minimalnu mjeru.

Da bismo prikazali listu predefinisanih konfiguracija, koristimo komandu:

```
./ct-ng list-samples
```

U datoj listi identifikujte konfiguraciju koja je najbliža ciljnoj platformi koju ćemo
koristiti na vježbama (DE1-SoC razvojna ploča), a to je ARMv7 arhitektura sa dvojezgrenim
Cortex-A9 procesorom. Odabraćemo ovu konfiguraciju kao osnovu za dalje izmjene.

```
./ct-ng arm-cortexa9_neon-linux-gnueabihf
```

Potvrdite da je izabrana odgovarajuća konfiguracijom komandom `./ct-ng show-config`.
Trebalo bi da dobijete sljedeći ispis:

```
[L..X]   arm-cortexa9_neon-linux-gnueabihf
    Languages       : C,C++
	OS              : linux-6.4
	Binutils        : binutils-2.40
	Compiler        : gcc-13.2.0
	C library       : glibc-2.38
	Debug tools     : gdb-13.2
	Companion libs  : expat-2.5.0 gettext-0.21 gmp-6.2.1 isl-0.26 libiconv-1.16 mpc-1.2.1
mpfr-4.2.1 ncurses-6.4 zlib-1.2.13 zstd-1.5.5
	Companion tools :
```

> [!TIP]
> Komandom `./ct-ng show-<config_name>` možemo da prikažemo konfiguraciju
bilo koje od podržanih konfiguracija koje ste dobili u listi.

Da bismo dodatno podesili određene opcije u konfiguraciji, pokrećemo konfiguracioni
meni komandom

```
./ct-ng menuconfig
```

Potrebno je da napravite sljedeće izmjene:

- U okviru **Paths and misc options**:
	- omogućite opciju **Try feature marked as EXPERIMENTAL** i
	- isključite opciju **Render the toolchain read-only**
- U okviru **Target options**:
	- postavite **cortex-a9** za **Emit assembly for CPU** opciju ako već nije uneseno
	- unesite **neon** za **Use specific FPU** opciju ako već nije uneseno
- U okviru **Toolchain options**:
	- postavite **Tuple's vendor string** opciju na **etfbl**
	- postavite **Tuple's alias** opciju na **arm-linux** (ovo nam omogućava da koristimo
	kraći umjesto punog prefiksa za *toolchain* alate, npr. **arm-linux-gcc** umjesto
	**arm-etfbl-linux-gnueabihf-gcc**)
- U okviru **Operating System**:
	- odaberite 6.1.35 u **Version of linux** opciji
- U okviru **C-library**:
	- odaberite **glibc** u **C library** opciji ukoliko već nije odabrana i
	- odaberite verziju 2.38 u **Version of glibc** opciji
    - uključite opciju **Enable obsolete libcrypt**
- U okviru **C compiler**:
	- postavite **Version of gcc** opciju na 13.2.0 (ako to već nije odabrano)
	- potvrdite da je omogućena podrška za C++

Možete istražiti i druge opcije da biste se srodili sa mogućnostima koje nudi
*Crosstool-NG*, a nakon toga je potrebno sačuvati izmjene u konfiguraciji i izaći
iz menija (opcije *Save* i *Exit*). Trenutna konfiguracija će biti sačuvana u
fajlu pod nazivom `.config` u `crosstool-ng-1` folderu.

Nakon što ste sačuvali konfiguraciju, možete provjeriti da li je sve podešeno kako
korišćenjem komande `./ct-ng show-config`, koja bi trebala da prikaže sljedeće:

```
[l..X]   arm-etfbl-linux-gnueabihf
    Languages       : C,C++
	OS              : linux-6.1.35
	Binutils        : binutils-2.40
	Compiler        : gcc-13.2.0
	C library       : glibc-2.38
	Debug tools     : gdb-13.2
	Companion libs  : expat-2.5.0 gettext-0.21 gmp-6.2.1 isl-0.26 libiconv-1.16 mpc-1.2.1
mpfr-4.2.1 ncurses-6.4 zlib-1.2.13 zstd-1.5.5
	Companion tools :
```

Ako je sve postavljeno kako treba, možete generisati *toolchain* korišćenjem komande

```
./ct-ng build
```

Ova komanda će preuzeti sve potrebne softverske pakete (potrebna je internet konekcija),
raspakovati i ih i prekompajlirati u odgovarajuće izvršne fajlove. Podrazumijevano,
*toolchain* se instalira na lokaciju `$HOME/x-tools`, tako da bi u konkretnom slučaju
trebalo da vidite folder `$HOME/x-tools/arm-etfbl-linux-gnueabihf`. Alati *toolchain*-a
se nalaze u podfolderu `bin` u kojem se nalaze i simbolički linkovi ka alatima sa kraćim
prefiksom koji ste definisali kao *alias* u konfiguraciji.

Za generisanje novog *toolchain*-a potrebno je prvo obrisati trenutne artifakte komandom
`./ct-ng distclean`. Ova komanda neće obrisati prethodno generisani *toolchain* koji je
instaliran u `$HOME/x-tools/` folderu.

## Korišćenje *toolchain*-a

Nakon što smo instalirali željeni *toolchain*, možemo početi da ga koristimo za
kompajliranje aplikacija za ciljnu platformu. Za ovo je potrebno da eksportujemo putanju
do `bin` foldera *toolchain*-a u kojem se nalaze svi neophodni alati. Za to možemo da
koristimo komandu

```
export PATH=$HOME/x-tools/arm-etfbl-linux-gnueabihf/bin:$PATH
```

Nakon eksportovanja, možemo potvrditi da je *toolchain* dostupan provjerom verzije kompajlera:

```
arm-linux-gcc --version
```

Trebalo bi da se prikaže sljedeći ispis:

```
arm-linux-gcc (crosstool-NG 1.26.0) 13.2.0
Copyright (C) 2023 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

Ukoliko dobijete sljedeću poruku

```
arm-linux-gcc: command not found
```

onda to znači da niste ispravno eksportovali putanju do *toolchain* alata.

> [!IMPORTANT]
> Putanja do *toolchain* alata eksportovana prethodno navedenom komandom
biće aktuelna samo dok je aktivna trenutna sesija terminala. Kada zatvorite konzolu
(odnosno ako otvorite novu konzolu), potrebno je ponovo da unesete ovu komandu. Zbog
toga, iz praktičnih razloga, možete koristiti skriptu pod nazivom `set-environment.sh`
koja je dostupna u okviru repozitorijuma (folder `scripts`) koju treba pozvati pri
svakom pokretanju terminala komandom `source ./set-environment.sh`. Ova skripta
postavlja još neke varijable (`CROSS_COMPILE`, `ARCH` i `SYSROOT`) koje ćemo koristiti
u drugim vježbama.

Detaljnije informacije o *toolchain*-u možemo dobiti pomoću *verbose* opcije:

```
arm-linux-gcc -v
```

Konačno, sljedećom komandom možemo da prikažemo putanju do *sysroot* foldera u kojem
su smješteni *header* fajlovi (`<sysroot>/usr/include/`) i biblioteke (`<sysroot>/lib/`).

```
arm-linux-gcc -print-sysroot
```

Sada ćemo pokazati kako korišćenjem generisanog *toolchain*-a možemo da kroskompajliramo
neku aplikaciju koja je napisana u C programskom jeziku.

Prvo je potrebno klonirati repozitorijum kursa, a zatim preći u folder laboratorijske
vježbe u kojem se nalazi izvorni kod pokaznog programa `hello.c`.

```
git clone https://github.com/etf-unibl/urs-2024
cd urs-2024/lab-01/hello
```

Sljedećom komandom krokompajliramo izvorni kod u izvršni fajl pod nazivom `hello`.

```
arm-linux-gcc hello.c -o hello
```

Ispravnost formata izvršnog fajla potvrđujemo korišćenjem komande `file`

```
file hello
```

koja bi trebalo da prikaže sljedeću poruku ukoliko je kroskompajliranje prošlo
kako treba.

```
hello: ELF 32-bit LSB executable, ARM, EABI5 version 1 (SYSV), dynamically linked,
interpreter /lib/ld-linux-armhf.so.3, for GNU/Linux 3.2.0, with debug_info, not
stripped
```

Ukoliko se u prikazanoj poruci navodi neka druga arhitektura, onda je potrebno
provjeriti da li je *toolchain* ispravno konfigurisan.

Takođe, ako pokušamo da izvršimo ovaj fajl na razvojnoj platformi

```
./hello
```

dobićemo sljedeću poruku, jer format izvršnog fajla nije podržan na x86 arhitekturi.

```
bash: ./hello: cannot execute binary file: Exec format error
```

Zavisnost kompajliranog programa od dinamičkih biblioteka provjeravamo komandom

```
arm-linux-readelf -a hello | grep "Shared library"
```

koja nam prikazuje sljedeću poruku

```
 0x00000001 (NEEDED)                     Shared library: [libc.so.6]
```

iz koje se jasno vidi da program zavisi samo od glavne C biblioteke `libc.so.6`.

Slično, sljedećom komandom provjeravamo zavisnost od *runtime* linkera:

```
arm-linux-readelf -a hello | grep "program interpreter"
```

Kao rezultat, dobijamo poruku

```
      [Requesting program interpreter: /lib/ld-linux-armhf.so.3]
```
što znači da naš program neće moći da se izvrši na ciljnoj platformi ako se na njoj
ne nalaze i fajlovi `libc.so.6` i `ld-linux-armhf.so.3`.

Za prikazivanje ovih informacija, u okviru repozitorijuma (folder `scripts`) možete
pronaći skriptu pod nazivom `list-libs.sh` kojoj je potrebno proslijediti naziv
programa za koji želimo da prikažemo date informacije, npr.

```
./scripts/list-libs.sh ./lab-01/hello/hello
```

## Kopiranje izvršnog fajla na ciljnu platformu

Za potrebe vježbe, pripremljen je *image* za SD karticu (fajl pod nazivom
`sd-card-lab.img` koji možete pronaći na *Google Classroom* stranici kursa) u okviru
kojeg se nalazi sve što je neophodno za pokretanje *Linux* sistema na DE1-SoC ploči,
uključujući i *root filesystem* u okviru kojeg se nalaze sve biblioteke od kojih zavisi
pokretanje kroskompajliranog programa.

Ovaj *image* je prvo potrebno kopirati na SD karticu. Na *Windows* operativnom sistemu
možete koristiti softver [*balenaEtcher*](https://etcher.balena.io/) koji ima veoma
intuitivan grafički interfejs. U okviru *Linux* operativnog sistema, nakon ubacivanja
SD kartice u čitač, možete koristiti sljedeću komandu:

```
sudo dd if=sd-card-lab.img of=/dev/sdX bs=1M
```

gdje je `/dev/sdX` ime uređaja koji predstavlja SD karticu (npr. `/dev/sdb`).

> [!CAUTION]
> Obratite posebnu pažnju na to da izaberete ispravan uređaj, jer
slično ime može da ima glavni hard disk računara na kojem radite. Pomenuta komanda
može da uzrokuje brisanje kompletnog fajl sistema računara na kojem radite ako
izaberete pogrešan uređaj.

> [!NOTE]
Ukoliko koristite virtuelnu mašinu ili ako računar koji koristite nema
čitač za SD kartice, možete koristiti USB čitač kartica koji će da se nalazi u
laboratoriji.

Nakon što ste pripremili SD karticu, ostaje još da prekopirate izvršni fajl koji ste
kroskompajlirali na particiju SD kartice na kojoj se nalazi *root filesystem* (veća
particija). Izvršni fajl kopirajte u folder `/home/root/`.

Odjavite SD karticu sa razvojnog računara i umetnite je u SD slot na DE1-SoC ploči.
Povežite napajanje i USB/UART serijski kabl sa USB portom računara a zatim pokrenite
program terminal koji podržava serijsku vezu (npr. [*TeraTerm*](https://teratermproject.github.io/index-en.html) ili
[*PuTTY*](https://www.putty.org/) na *Windows* platformi, odnosno [`minicom`](https://linux.die.net/man/1/minicom)
ili [`picocom`](https://linux.die.net/man/8/picocom) na *Linux* platformi.

Nakon uključenja napajanja, trebalo bi da se prikažu logovi na serijskom terminalu
iz kojih s emože vidjeti faze podizanja operativnog sistema. Na kraju će biti ponuđeno
da se ulogujete u sistem

```
Welcome to DE1-SoC
socfpga login:
```

Unesite korisničko ime `root` i trebalo bi da imate pristup sistemu, pri čemu ćete biti
inicijalno pozicionirani u direktorijumu `/home/root/`. Izlistajte fajlove komandom `ls` i
trebalo bi da bude prisutan izvršni fajl. Pokrenite ga da biste verfikovali da je
kroskompajliranje bilo uspješno.
