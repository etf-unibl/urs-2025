# Laboratorijska vježba 3

Cilj laboratorijske vježbe je da se studenti upoznaju sa izradom *bare-metal* aplikacije
na *embedded* platformi, kao i sa strukturom ELF fajla dobijenog nakon povezivanja
korišćenjem odgovarajuće linker skripte i postupkom njegovog učitavanja na ciljnu
platformu preko dostupnog JTAG interfejsa za debagovanje.

Nakon uspješno realizovane vježbe studenti će biti sposobni da:
1. konfigurišu i koriste *Ashling RiscFree IDE for Intel FPGAs* softversko razvojno okruženje
za razvoj i debagovanje *bare-metal* aplikacija bazirano na *Eclipse* okruženju,
2. prilagode linker skriptu u skladu sa ograničenjima ciljne platforme,
3. koriste *binutils* alate za inspekciju i interpretaciju izvršnog fajla u ELF formatu i
4. koriste JTAG interfejs prilikom debagovanja *bare-metal* aplikacije.

## Preduslovi za izradu vježbe

Za uspješno izvođenje laboratorijske vježbe potrebno je sljedeće:

- razvojna ploča *DE1-SoC* sa pripadajućim napajanjem i kablom za povezivanje računara sa
*USB Blaster* interfejsom na ploči,
- instaliran *Quartus Prime 23.1 Lite Edition* softver sa uključenim pratećim softverskim alatima
(*Ashling RiscFree IDE for Intel FPGAs* i alati za programiranje ploče sa pripadajućim drajverima) i
- instaliran ARM *toolchain* za kompajliranje *bare-metal* aplikacija.

## Instalacija neophodnog softvera

### Instalacija *Quartus Prime 24.1 Lite Edition* softvera

Instalacioni fajl za *Quartus Prime 24.1 Lite Edition* se može preuzeti sa
[zvaničnog linka](https://www.intel.com/content/www/us/en/software-kit/795188/intel-quartus-prime-lite-edition-design-software-version-23-1-for-windows.html)
kompanije *Intel*. Prilikom preuzimanja, izaberite operativni sistem (podržani su *Windows* i *Linux*) koji
koristite na svom računaru. Ukoliko koristite *Linux* na virtuelnoj mašini, preporučuje se da preuzmete
instalaciju za *Windows* ako je to *host* operativni sistem.

Kada preuzmete instalacioni fajl, instalacija je automatizovana i slična instalaciji drugih aplikacija.
Vodite računa da je potrebno da obavezno instalirate i prateće alate (*Ashling RiscFree IDE for Intel FPGAs* i
alat za programiranje ploče). Ukoliko ste u nedoumici oko izbora pojedinih opcija pri instalaciji,
posavjetujte se sa predmetnim asistentom ili nastavnikom.

> [!NOTE]
> Svi primjeri prikazani u nastavku su bazirani na *Windows* instalaciji *Quartus Prime 24.1 Lite Edition*
softverskog paketa, ali koraci i izgled korisničkog interfejsa je ekvivalentan i u *Linux* okruženju.

Podrazumijevana lokacija na kojoj će softver biti instaliran u *Windows* okruženju je `C:\intelFPGA_lite\23.1std`.
Iako ovo može da se redefiniše prilikom instalacije, najbolje je da zadržite preporučenu putanju za instalaciju.
U nastavku ćemo koristiti relativne putanje u odnosu na instalacioni folder *Quartus Prime* softvera, koji ćemo
referencirati sa `<QUARTUS_INSTALL_DIR>`.

### Instalacija ARM *toolchain*-a za kompajliranje *bare-metal* aplikacija

Prekompajliran *toolchain* za *bare-metal* aplikacije možemo preuzeti sa
[zvaničnog linka](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) kompanije ARM. Za ovu vježbu
ćemo koristiti verziju 13.2Rel1. U tom smislu, potrebno je da preuzmete *AArch32 bare-metal target (arm-none-eabi)*
varijantu.

Ako koristite *Windows*, potrebno je da preuzmete neki od fajlova iz *Windows (mingw-w64-i686) hosted cross toolchains*
sekcije (prikazano na slici). Postoje dvije opcije: (1) instalacioni fajl sa ekstenzijom `.exe` ili (2)
kompresovana arhiva sa ekstenzijom `.zip`. Pokretanjem instalacionog fajla pokrećete instalacioni program
koji sam raspakuje fajlove na definisanu lokaciju, dok kod druge opcije trebate ručno da raspakujete arhivu
i kopirate fajlove na željenu lokaciju.

![arm-toolchain-windows](figs/arm-toolchain-windows.PNG)

U slučaju da koristite *Linux* na razvojnom računaru, preuzmite i raspakujte arhivu sa ekstenzijom `.tar.xz` u okviru
sekcije *x86_64 Linux hosted cross toolchains* (vidi sliku ispod).

![arm-toolchain-linux](figs/arm-toolchain-linux.PNG)

Ovako preuzet *toolchain* potrebno je smjestiti/instalirati na lokaciju `<QUARTUS_INSTALL_DIR>/riscfree/toolchain/Arm`.

> [!IMPORTANT]
> S obzirom da *toolchain* sadrži korisne alate koje ćemo koristiti tokom vježbe, preporučujemo da
dodate putanju do `bin` foldera u sistemsku varijablu `Path` *Windows* okruženja. Alternativno, ako
koristite *Linux*, potrebno je da dodate ovu putanju u sistemsku varijablu `PATH`.

## Kreiranje projekta za jednostavnu *bare-metal* aplikaciju

Ukoliko to već niste uradili, povucite posljednje izmjene sa repozitorijuma

```
git checkout main
git pull
```

Pokrenite *Ashling RiscFree IDE for Intel FPGAs* iz menija. Prilikom prvog pokretanja, možda ćete morati da dozvolite
pristup softveru u okviru podešavanja *Firewall*-a kao što je prikazano na sljedećoj slici:

![firewall-settings](figs/firewall-settings.PNG)

Nakon toga, pojaviće se prozor u kojem je potrebno odabrati putanju do radnog prostora, kao što je prikazano
na slici ispod.

![launcher-set-workspace](figs/launcher-set-workspace.PNG)

U našen slučaju, potrebno je da, preko dugmeta *Browse*, odaberete putanju do foldera treće vježbe (`lab-03`),
a zatim da kliknete na *Launch*. Pojaviće se glavni prozor radnog okruženja koji će biti kao na sljedećoj slici.

![start-window](figs/start-window.PNG)

Sada je potrebno da kreiramo projekat za našu testnu *bare-metal* aplikaciju. Prvo odaberemo opciju
**File**&rarr;**New**&rarr;**Project...** u glavnom meniju, nakon čega se pojavljuje prozor za izbor projekta.

![01-create-project](figs/project/01-create-project.PNG)

Odaberemo *C/C++ Project* i kliknemo da dugme *Next*. U sljedećem prozoru definišemo šablon projekta.

![02-create-c-project](figs/project/02-create-c-project.PNG)

U našem slučaju, potrebno je odabrati opciju *C Managed Build* i kliknuti na dugme *Next*. Sljedeći prozor
omogućava definisanje imena projekta (`test-baremetal`) i izbor polaznog kostura i *toolchain*-a. U datom
slučaju, biramo *Empty Project* kao tip i *Cross GCC* za *toolchain*, kao što je prikazano na sljedećoj
slici

![03-project-name](figs/project/03-project-name.PNG)

> [!IMPORTANT]
> Vodite računa da naziv projekta bude `test-baremetal` (što je i naziv foldera u koji su smješteni izvorni
fajlovi projekta), kao i da je selektovana opcija *Use default location*. Na taj način biće automatski
odabrana ispravna lokacija projekta.

a zatim kliknemo na dugme *Next*. U sljedećem prozoru selektujemo obje podržane konfiguracije (*Debug* i
*Release*) i kliknemo dugme *Next*.

![04-project-configurations](figs/project/04-project-configurations.PNG)

Konačno, potrebno je da definišemo prefiks i putanju do korišćenog *toolchain*-a.

![05-toolchain-path-and-prefix](figs/project/05-toolchain-path-and-prefix.PNG)

Za naš *toolchain*, prefiks je `arm-none-eabi-`, a putanju selektujemo preko *Browse* dugmeta i
postavljamo na lokaciju `bin` foldera prethodno instaliranog *toolchain*-a. Nakon što smo sve podesili
kako treba, klikom na dugme *Finish* završavamo proces kreiranja projekta.

Prije kompajliranja projekta, potrebno je da konfigurišemo određene opcije za *toolchain*. U tom smislu
potrebno je desnim klikom miša na folder projekta odabrati opciju *Properties* iz padajućeg menija,
a zatim odabrati opciju **C/C++ Build**&rarr;**Settings**.

Prvo je potrebno da odaberete opciju *[ All configurations ]* u okviru *Configuration* polja kako bi izmjene
koje napravite bile primjenjene na obje konfiguracije (*Debug* i *Release*).

> [!NOTE]
> U okviru **Cross Settings** podešavanja možete da promijenite naziv prefiksa i putanje za *toolchain*, na primjer,
ako ste nešto pogrešno definisali pri kreiranju projekta ili ako želite da promijenite *toochain* u projektu.

U sekciji **Cross GCC Compiler**&rarr;**Miscellaneous**, u okviru polja *Other flags*, definišemo dodatne
kompajlerske flegove. U našem slučaju ćemo dodati `-mcpu=cortex-a9` kako bismo definisali tip procesora
(Cortex-A9) na ciljnoj platformi, kao što je ilustrovano na slici ispod.

![01-compiler-flags](figs/toolchain/01-compiler-flags.PNG)

U okviru sekcije **Cross GCC Linker**&rarr;**General** selektujemo opcije *Do not use standard start files (-nostartfiles)*
i *No startup or default libs (-nostdlib)* (vidi sliku). Ovo je potrebno jer ćemo u potpunosti da kontrolišemo proces
pokretanja sistema, a takođe nećemo koristiti standardnu biblioteku (čak ni u redukovanoj varijanti). Ovo
je tipično podešavanje za *bare-metal* aplikacije.

![02-linker-startup-files](figs/toolchain/02-linker-startup-files.PNG)

Zatim, u okviru sekcije **Cross GCC Linker**&rarr;**Miscellaneous**, podešavamo linkerske flegove. Kao i za
kompajler, postavljamo tip procesora, ali dodatno definišemo i skriptu koja će biti korišćena Prilikom
povezivanja korisničkih modula (opcija `-T../test.ld`). Sadržaj polja *Linker flags* sa ovim flegovima
prikazan je na sljedećoj slici.

![03-linker-flags](figs/toolchain/03-linker-flags.PNG)

Konačno, podešavamo tip procesora ta asemblerske flegove u sekciji **Cross GCC Assembler**&rarr;**General**
kao što je prikazano na slici.

![04-assembler-flags](figs/toolchain/04-assembler-flags.PNG)

Nakon što smo definisali željene opcije za *toolchain* potrebno je da ih sačuvamo klikom na dugme
*Apply and Close*, nakon čega će se pojaviti prozor kao na sljedećoj slici.

![05-rebuild-index](figs/toolchain/05-rebuild-index.PNG)

Klikom na dugme *Rebuild Index* omogućavamo da prethodne promjene budu ugrađene u naš projekat.

Sada je projekat spreman za kompajliranje i generisanje izvršnog fajla koji ćemo učitati na ciljnu
platformu. Međutim, prvo ćemo da analiziramo sadržaj linkerske skripte koja govori linkeru gdje
se smještaju pojedine sekcije objektnih fajlova.

Otvorite fajl `test.ld` koji predstavlja linkersku skriptu projekta. U prvoj liniji definisana je
organizacija memorije kako je dato ispod.

```
MEMORY
{
  program     (rx)  : ORIGIN = 0xffff0000, LENGTH = 16K
  data        (rw)  : ORIGIN = 0xffff4000, LENGTH = 16K
}
```

Kao što možemo da vidimo, imamo dva memorijska područja:

- `program` sa *read-only* pristupom (fleg `r`) i oznakom da sadrži instrukcije (fleg `x`),
čija je početna adresa `0xffff0000` (ovo je početna adresa *on-chip* SRAM memorije *Cyclone V*
platforme kapaciteta 64kB), a veličina 16kB;
- `data` koja ima pristup za čitanje i upis (flegovi `rw`) koja počinje od adrese `0xffff4000`
(odmah nakon 16kB `program` memorijskog područja) i takođe je veličine 16kB.

Nakon definisanja memorijskih područja slijedi definicija sekcija programa u okviru `SECTIONS`
segmenta linker skripte.

Prvo je definisana sekcija `.text`, koja sadrži instrukcije (sadržane unutar `.text` sekcija
ulaznih objektnih fajlova) i konstantne podatke (smještene unutar `.rodata` sekcija ulaznih
objektnih fajlova). Kao što može da se vidi iz isječka datog ispod, prvo se smještaju sve
`.text` ulazne sekcije, nakon kojih slijede `.rodata` sekcije. Ovako definisana izlazna sekcija
se smješta u memorijsko područje `program` (konstrukcija `> program` na kraju sekcije).

```
.text :
{
    *(.text*)
    *(.rodata*)
} > program
```

Sljedeća sekcija definiše neinicijalizovane globalne i statičke varijable (`.bss`) koji se
smještaju u `data` memorijsko područje, kao što je prikazano u sljedećem isječku.

```
.bss (NOLOAD) :
{
    __bss_start__ = .;
    *(.bss*)
    *(COMMON)
    __bss_end__ = .;
} > data
```

Ovdje skrećemo pažnju na nekoliko bitnih elemenata. Prvo, sekciju označavamo `NOLOAD` direktivom,
jer se radi o sekciji u koju nije predviđeno učitavanje podataka iz LMA područja koje sadrži
inicijalne vrijednosti (za razliku od sekcije sa inicijalizovanim podacima). Drugo, osim `.bss`
ulaznih sekcija, koje se prve smještaju, imamo i `COMMON` ulazne sekcije. Ove sekcije se mogu
automatski generisati prilikom kompajliranja i sadrže zajedničke simbole koji se tipično smještaju
unutar `.bss` sekcije. Konačno, ovdje definišemo i simbole `__bss_start__` i `__bss_end__` koji
označavaju početak i kraj `.bss` sekcije. Ovi simboli se eksportuju od strane linkera, tako da su
globalno vidljivi i mogu se korsititi za pristup ovoj sekciji iz programa ako je to potrebno
(npr. za postavljanje neinicijalizovanih podataka na nulu pri startovanju sistema).

Sljedeća sekcija je `.data` koja sadrži inicijalizovane globalne i statičke varijable. Ova sekcija
se takođe nalazi u `data` memorijskom području.

```
.data :
{
    __data_start__ = .;
    *(.data*);
    __data_end__ = .;
} > data
```

Kao što vidimo u prethodnom isječku, ova sekcija sadrži sve `.data` ulazne sekcije. Slično kao
kod `.bss` sekcije, definišemo simbole koji označavaju početak (`__data_start__`) i kraj
(`__data_end__`) ove sekcije.

Posljednja sekcija koju definišemo je sekcija rezervisana za stek memoriju (`.stack`). Ovo je
takođe `NOLOAD` sekcija, koja treba da ima poravnanje `ALIGN(8)` za datu arhitekturu.

```
.stack (NOLOAD):
{
    . = ALIGN(8);
    . = . + __stack_size;
    . = ALIGN(8);
    __stack_start__ = .;
} > data
```

Iz datog isječka vidimo da je veličina stek memorije određena je simbolom `__stak_size` čiju
vrijednost smo ranije definisali tako da iznosi 8kB:

`__stack_size = 0x2000;`

Na kraju stek sekcije, definišemo simbol `__stack_start__` koji odgovara adresi vrha steka.
Ovaj simbol koristimo u inicijalizacionom kodu za postavljanje početne vrijednosti `SP` registra.

Konačno, na kraju linker skripte definišemo simbol `_end` koji označava posljednju lokaciju
korišćene memorije.

`_end = . ;`

S obzirom da smo postavili opcije `-nostartfiles` i `-nostdlib` prilikom konfigurisanja linkera,
naša aplikacija ne sadrži podrazumijevani *startup* kod, pa je potrebno da ga dodamo. Iz tog
razloga se u projektu nalazi i minimalan *startup* kod napisan u asemblerskom jeziku (fajl
`startup.S`). Ovaj kod sadrži samo nekoliko instrukcija, kao što je prikazano u isječku ispod.

```assembly
_start:
    ldr     r1, =__stack_start__
    mov sp,r1
    bl main
    b .
```

Uloga ovog koda je da postavi pokazivač vrha steka (registar `SP`) na vrijednost definisanu u
linker skripti (simbol `__stack_start__`). Ovo je neophodno jer pozivi svih funkcija u C programu
zavise od ispravno definisanog steka.

Nakon postavljanja `SP` registra, imamo bezuslovni skok na simbol `main` (instrukcija `bl main`)
koji predstavlja *main* funkciju naše aplikacije.

> [!NOTE]
> Nakon instrukcije skoka na simbol `main`, slijedi instrukcija `b .` koja predstavlja beskonačnu
petlju. Ova instrukcija nije neophodna u našem slučaju, jer C program već sadrži beskonačnu petlju.
Međutim, dobra je praksa da se doda ova funkcija u inicijalizacioni kod za slučaj da se izađe iz
*main* funkcije glavnog programa, kako bi se dalje izvršavanje zadržalo u definisanom području.
U suprotnom, procesor bi nastavio da izvršava podatke koji slijede kao instrukcije, što dovodi do
nepredvidivog ponašanja sistema.

Sada konačno možemo da kompajliramo našu aplikaciju. Prvo desnim klikom na folder projekta iz
padajućeg menija biramo opciju **Build Configurations**&rarr;**Set Active**&rarr;**Debug** da
bismo odabrali konfiguraciju koja uključuje simbole potrebne za debagovanje, a zatim
kompajliramo projekat desnim klikom na folder projekta i izborom opcije **Build Project** iz
padajućeg menija.

> [!IMPORTANT]
> Ukoliko promijenimo linker skriptu i pokrenemo kompajliranje projekta, dobićemo poruku da nije
ništa promijenjeno. To je zbog toga što se linker skripta ne vidi kao fajl sa izvornim kodom, pa
se izmjene ignorišu od strane *build* sistema. Da bi ove promjene bile aktuelizovane u izvršnom
fajlu, potrebno je prvo da očistimo projekat izborom opcije **Clean Project** iz padajućeg menija.

Kompajliranjem dobijamo izvršni fajl u ELF formatu koji se smješta u *Debug* folder unutar projekta.
Prije izvršavanja samog programa, prvo ćemo se upoznati sa strukturom ovog fajla. U tu svrhu nam
služi `objdump` alatka. Ako pređemo u `Debug` folder i izvršimo komandu

```
arm-none-eabi-objdump -h test-baremetal
```

dobijamo ispis koji prikazuje listu sekcija programa.

```
test-baremetal:     file format elf32-littlearm

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .text         000000c4  ffff0000  ffff0000  00001000  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 .bss          00000008  ffff4000  ffff4000  00002000  2**2
                  ALLOC
  2 .stack        00002000  ffff4008  ffff4008  00002000  2**0
                  ALLOC
  3 .ARM.attributes 00000035  00000000  00000000  000010c4  2**0
                  CONTENTS, READONLY
  4 .comment      00000044  00000000  00000000  000010f9  2**0
                  CONTENTS, READONLY
  5 .debug_info   0000007f  00000000  00000000  0000113d  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  6 .debug_abbrev 00000064  00000000  00000000  000011bc  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  7 .debug_aranges 00000020  00000000  00000000  00001220  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  8 .debug_macro  00000ad7  00000000  00000000  00001240  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  9 .debug_line   00000068  00000000  00000000  00001d17  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
 10 .debug_str    00002e02  00000000  00000000  00001d7f  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
 11 .debug_frame  00000028  00000000  00000000  00004b84  2**2
                  CONTENTS, READONLY, DEBUGGING, OCTETS
```

Iz ovog ispisa vidimo da program ima tri sekcije (ako izuzmemo sekcije koje sadrže simbole
neophodne za debagovanje):

- `.text` koja sadrži instrukcije i konstante, čija veličina je `0xc4` i koja počinje od
adrese `0xffff0000` (početak `program` memorijskog područja);
- `.bss` koja sadrži neinicijalizovane globalne i statičke varijable, čija je veličina `0x8`
i koja počinje od adrese `0xffff4000` (početak `data` memorijskog područja);
- `.stack` koja predstavlja stek memoriju, veličine je `0x2000` i čija je početna adresa
`0xffff4008` (kraj `.bss` sekcije sa poravnanjem `ALIGN(8)).

Prvo što treba primjetiti da nema `.data` sekcije iako u programu imamo inicijalizovanu globalnu
varijablu `counter`. Razlog je u tome što je varijabla inicijalizovana na 0, pa će kompajler
smjestiti i ovu varijablu u `.bss` sekciju sa ostalim varijablama koje bi inicijalno trebalo da
se postave na 0. To možemo potvrditi ako izlistamo sekcije objektnog fajla `test-baremetal.o`
korišćenjem komande `arm-none-eabi-objdump -h test-baremetal.o`.

```
test-baremetal.o:     file format elf32-littlearm

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .group        0000000c  00000000  00000000  00000034  2**2
                  CONTENTS, READONLY, GROUP, LINK_ONCE_DISCARD
  1 .text         000000b0  00000000  00000000  00000040  2**2
                  CONTENTS, ALLOC, LOAD, RELOC, READONLY, CODE
  2 .data         00000000  00000000  00000000  000000f0  2**0
                  CONTENTS, ALLOC, LOAD, DATA
  3 .bss          00000008  00000000  00000000  000000f0  2**2
                  ALLOC
  4 .debug_info   0000007f  00000000  00000000  000000f0  2**0
                  CONTENTS, RELOC, READONLY, DEBUGGING, OCTETS
  5 .debug_abbrev 00000064  00000000  00000000  0000016f  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  6 .debug_aranges 00000020  00000000  00000000  000001d3  2**0
                  CONTENTS, RELOC, READONLY, DEBUGGING, OCTETS
  7 .debug_macro  00000011  00000000  00000000  000001f3  2**0
                  CONTENTS, RELOC, READONLY, DEBUGGING, OCTETS
  8 .debug_macro  00000ac6  00000000  00000000  00000204  2**0
                  CONTENTS, RELOC, READONLY, DEBUGGING, OCTETS
  9 .debug_line   00000068  00000000  00000000  00000cca  2**0
                  CONTENTS, RELOC, READONLY, DEBUGGING, OCTETS
 10 .debug_str    00002e02  00000000  00000000  00000d32  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
 11 .comment      00000045  00000000  00000000  00003b34  2**0
                  CONTENTS, READONLY
 12 .debug_frame  00000028  00000000  00000000  00003b7c  2**2
                  CONTENTS, RELOC, READONLY, DEBUGGING, OCTETS
 13 .ARM.attributes 00000033  00000000  00000000  00003ba4  2**0
                  CONTENTS, READONLY
```

Kao što možemo da vidimo, sekcija `.data` postoji ali je njena veličina 0, pa se ne smješta
u izlazni fajl. Da smo imali neku varijablu čija je inicijalna vrijednost različita od 0, ona
bi bila smještena u ovu sekciju. Ovo možemo potvrditi definisanjem statičke varijable u
programu, npr. `static int sum = 10` nakon `int b = 5`. Ako ako nakon kompajliranja ponovo
izlistamo sekcije programa, dobijamo

```
test-baremetal:     file format elf32-littlearm

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .text         000000c4  ffff0000  ffff0000  00001000  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 .bss          00000008  ffff4000  ffff4000  00002000  2**2
                  ALLOC
  2 .data         00000004  ffff4008  ffff4008  00002008  2**2
                  CONTENTS, ALLOC, LOAD, DATA
  3 .stack        00002004  ffff400c  ffff400c  0000200c  2**0
                  ALLOC
  4 .ARM.attributes 00000035  00000000  00000000  0000200c  2**0
                  CONTENTS, READONLY
  5 .comment      00000044  00000000  00000000  00002041  2**0
                  CONTENTS, READONLY
  6 .debug_info   0000008c  00000000  00000000  00002085  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  7 .debug_abbrev 00000075  00000000  00000000  00002111  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  8 .debug_aranges 00000020  00000000  00000000  00002186  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  9 .debug_macro  00000ad7  00000000  00000000  000021a6  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
 10 .debug_line   00000068  00000000  00000000  00002c7d  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
 11 .debug_str    00002e02  00000000  00000000  00002ce5  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
 12 .debug_frame  00000028  00000000  00000000  00005ae8  2**2
                  CONTENTS, READONLY, DEBUGGING, OCTETS
```

odakle vidimo da je kreirana `.data` sekcija veličine `0x4`. Komandom
`arm-none-eabi-objdump -t test-baremetal` prikazujemo tabelu simbola

```
test-baremetal:     file format elf32-littlearm

SYMBOL TABLE:
ffff0000 l    d  .text  00000000 .text
ffff4000 l    d  .bss   00000000 .bss
ffff4008 l    d  .data  00000000 .data
ffff400c l    d  .stack 00000000 .stack
00000000 l    d  .ARM.attributes        00000000 .ARM.attributes
00000000 l    d  .comment       00000000 .comment
00000000 l    d  .debug_info    00000000 .debug_info
00000000 l    d  .debug_abbrev  00000000 .debug_abbrev
00000000 l    d  .debug_aranges 00000000 .debug_aranges
00000000 l    d  .debug_macro   00000000 .debug_macro
00000000 l    d  .debug_line    00000000 .debug_line
00000000 l    d  .debug_str     00000000 .debug_str
00000000 l    d  .debug_frame   00000000 .debug_frame
00000000 l    df *ABS*  00000000 startup.o
ffff0000 l       .text  00000000 _start
00000000 l    df *ABS*  00000000 test-baremetal.c
ffff4008 l     O .data  00000004 sum.0
ffff4008 g       .data  00000000 __data_start__
00002000 g       *ABS*  00000000 __stack_size
ffff4000 g       .bss   00000000 __bss_start__
ffff400c g       .data  00000000 __data_end__
ffff4008 g       .bss   00000000 __bss_end__
ffff4000 g     O .bss   00000004 counter
ffff0014 g     F .text  000000b0 main
ffff4004 g     O .bss   00000004 result
ffff6010 g       .stack 00000000 _end
ffff6010 g       .stack 00000000 __stack_start__
```

iz koje se jasno vidi da se varijabla `sum` nalazi u `.data` sekciji.

> [!TIP]
> Lista simbola u kompaktnijoj formi može se prikazati komandom `arm-none-eabi-nm test-baremetal`.

Ono što možemo primjetiti u prikazu liste sekcija iz prethodnog slučaja je da je VMA i LMA
adrese sekcije `.data` imaju istu vrijednost, što znači da nije definisano odakle se učitavaju
inicijalne vrijednosti varijabli koje se nalaze u sekciji. Ovo možemo ispraviti ako kraj sekcije
izmjenimo tako da bude `> data AT > program` umjesto `> data`. Na ovaj način govorimo linkeru
da se inicijalne vrijednosti učitavaju iz `program` memorijskog područja.

Ako ponovo prekompajliramo projekat (prethodno moramo da ga očistimo, jer smo napravili izmjene
samo u linker skripti) i ponovo izlistamo sekcije, dobijamo

```
test-baremetal:     file format elf32-littlearm

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .text         000000c4  ffff0000  ffff0000  00001000  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 .bss          00000008  ffff4000  ffff4000  00003000  2**2
                  ALLOC
  2 .data         00000004  ffff4008  ffff00c4  00002008  2**2
                  CONTENTS, ALLOC, LOAD, DATA
  3 .stack        00002004  ffff400c  ffff00c8  0000200c  2**0
                  ALLOC
  4 .ARM.attributes 00000035  00000000  00000000  0000200c  2**0
                  CONTENTS, READONLY
  5 .comment      00000044  00000000  00000000  00002041  2**0
                  CONTENTS, READONLY
  6 .debug_info   0000008c  00000000  00000000  00002085  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  7 .debug_abbrev 00000075  00000000  00000000  00002111  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  8 .debug_aranges 00000020  00000000  00000000  00002186  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  9 .debug_macro  00000ad7  00000000  00000000  000021a6  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
 10 .debug_line   00000068  00000000  00000000  00002c7d  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
 11 .debug_str    00002e01  00000000  00000000  00002ce5  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
 12 .debug_frame  00000028  00000000  00000000  00005ae8  2**2
                  CONTENTS, READONLY, DEBUGGING, OCTETS
```

Odavde jasno vidimo da je vrijednost LMA sada `0xffff00c4` koja se nalazi u `program`
memorijskom području.

> [!IMPORTANT]
> Kao što ćemo vidjeti, prethodna intervencija u linker skripti nije dovoljna da incijalne
vrijednosti budu postavljene kako treba. Za tako nešto je potrebno proširiti *startup* kod
koji će da prekopira vrijednosti sa LMA na VMA adresu sekcije (slično, varijable `.bss` u sekciji
je potrebno inicijalno postaviti na 0). U tu svrhu se mogu iskoristiti simboli definisani u
linker skripti koji označavaju početak i kraj sekcije.

Sada ćemo da pokrenemo izvršavanje programa korišćenjem JTAG interfejsa za debagovanje. Prvo
povežite *DE1-SoC* ploču sa napajanjem i razvojni računar USB kablom sa *USB Blaster* interfejsom
za programiranje i debagovanje, kao što je prikazano na slici ispod.

![board-usb-blaster](figs/board-usb-blaster.jpg)

Uključite napajanje ploče i pokrenite izvršavanje u *Debug* modu tako što ćete desnim klikom
na folder projekta iz padajućeg menija izabrati opciju **Debug As**&rarr;**Ashling Arm Hardware Debugging**.
Pojaviće se prozor za izbor i konfiguraciju interfejsa za debagovanje kao na slici ispod.

![debugger-autodetect](figs/debugger-autodetect.PNG)

U polju *Debug probe* trebalo bi da se nalazi *DE-SoC [USB-1]*. Ukoliko to nije slučaj, provjerite
da je napajanje ploče uključeno i kliknite na *Refresh* dugme s desne strane. Zatim u sekciji
*Target Configuration* kliknite na *Auto-detect Scan Chain* za detekciju uređaja. Konačno, kliknite
dugme *Debug* koje će biti omogućeno nakon detekcije. Pojaviće se prozor sa sljedeće slike.

![switch-perspective](figs/switch-perspective.PNG)

U ovom prozoru kliknite na dugme *Switch* da se prebacite u *Debug* perspektivu nakon čega će
program da se učita u memoriju i možete početi sa debagovanjem.

Alternativno, izvršavanje u *debug* modu može da se pokrene klikom na dugme za debagovanje u
paleti sa alatkama koja je prikazana na slici ispod.

![run-toolbar](figs/run-toolbar.PNG)

Dugme *Run* iz ove palete se tipično koristi za izvršavanje programa koji su kompajlirani sa
*Release* konfiguracijom.

Nakon pokretanja, program će automatski da se zaustavi na početku *main* funkcije, a korisnik
može da izvrši inspekciju različitih objekata programa u nizu prozora koji su mu na raspolaganju
u okviru softverskog okruženja. Jedan od takvih prozora je *Variables*, koji prikazuje trenutno
stanje lokalnih varijabli (vidi sliku ispod). Ukoliko ne vidite ovaj prozor, trebate ga omogućiti
opcijom **Window**&rarr;**Show View**&rarr;**Variables** iz glavnog menija.

![variable-view](figs/variable-view.PNG)

Inicijalno, lokalne varijable (`a` i `b`) će imati neke slučajne vrijednosti. Međutim, kako
izvršavanje programa napreduje, ove varijable će biti postavljene na očekivane vrijednosti
nakon što se izvrše instrukcije za njihovu inicijalizaciju.

Program možemo izvršavati korak po korak ili tako da se izvršava do sljedeće prekidne tačke. U
tom smislu možemo da koristimo paletu alatki za kontrolu izvršavanja programa prikazanu na slici.

![debugging-control-toolbar](figs/debugging-control-toolbar.PNG)

Najznačajnije opcije za kontrolu izvršavanja programa su:

1. *Resume* kojom nastavljamo izvršavanje programa koji je zaustavljen u nekoj prekidnoj tački
(aktivno samo dok je suspendovano izvršavanje programa),
2. *Suspend* kojom pauziramo izvršavanje programa (aktivno samo dok se program izvršava),
3. *Terminate* za prekidanje izvršavanja programa i izlazak iz *Debug* moda,
4. *Step Into* za izvršavanje instrukcija korak po korak na način da se pri nailasku na poziv
funkcije izvršavanje nastavlja korak po korak i unutar funkcije i
5. *Step Over* za izvršavanje instrukcija korak po korak, pri čemu se po nailasku na poziv
funkcije izvršava kompletna funkcija i vraća rezultat, tj. funkcija se tretira kao jedna atomska
instrukcija.

Sada koristite opciju *Step Into* da izvršite prve dvije instrukcije (inicijalizacija varijabli
`a` i `b`). Kao rezultat, u prozoru varijabli će da se promijeni stanje varijabli, kao što je
ilustrovano na sljedećoj slici.

![local-variable-change](figs/local-variable-change.PNG)

Sada, prije nego što izvršite instrukciju za inicijalizaciju globalne varijable `result`, prikažite
prozor globalnih varijabli tako što ćete odabrati opciju **Window**&rarr;**Show View**&rarr;**Other...**,
a zatim u prikazanom prozoru odabrati opciju **Debug**&rarr;**Global Variables View** (vidi sliku).

![show-global-variables](figs/show-global-variables.PNG)

Da bi prikazali globalne varijable, potrebno je prikazati prozor za dodavanje globalnih varijabli
koji se aktivira klikom na dugme prikazano na slici ispod.

![select-global-variables](figs/select-global-variables.PNG)

U prikazanom prozoru odaberemo željene varijable i kliknemo na dugme *OK*, kao što je ilustrovano
na sljedećoj slici.

![add-global-variables](figs/add-global-variables.PNG)

Inicijalno, varijable će imati slučajne vrijednosti (vidi sliku ispod), jer ih *startup* kod
nije postavio na 0, odnosno nije kopirao inicijalne vrijednosti iz `program` memorijskog područja.

![global-variables-initial](figs/global-variables-initial.PNG)

Međutim, kada se njihova vrijednost promijeni u kodu (npr. izvršavanjem naredne instrukcije
opcijom *Step Into*), to se automatski reflektuje u prikazanom prozoru. Na sljedećoj slici
vidimo da se varijablа `result` postavlja na 0.

![global-variables-changed](figs/global-variables-changed.PNG)

Postavite sada prekidnu tačku na liniju sa instrukcijom dodjele nove vrijednosti `result`
unutar beskonačne petlje kao što je prikazano na datoj slici.

![add-breakpoint](figs/add-breakpoint.PNG)

Zatim pokrenite izvršavanje programa do sljedeće prekidne tačke (opcija *Resume* iz palete
sa alatkama). Izvršavanje programa će da se zaustavi u prethodno definisanoj prekidnoj tački,
a globalna varijabla `counter` će takođe da promijeni vrijednosti kako je dato na slici ispod.

![global-variables-both-changed](figs/global-variables-both-changed.PNG)

Eksperimentišite sa izvršavanjem programa na ovaj način i pratite kako se mijenjaju vrijednosti
lokalnih i globalnih varijabli nakon svakog zasutavljanja u prekidnoj tački.

Statičku varijablu `sum` možemo posmatrati u okviru prozora *Expressions* tako što ćemo
kliknuti na polje *Add new expression* i unijeti naziv varijable.

Iz prethodne analize izvršavanja programa, jasno je da inicijalne vrijednosti posmatranih
globalnih i statičkih varijabli ne odgovaraju onim koje su postavljene u programu. Da bi ovo
ispravili, moramo da modifikujemo *startup* kod.

Ispod je data modifikacija fajla `startup.S` na način da se prvo inicijalizuje `.data` sekcija,
nakon čega slijedi postavljanje svih varijabli unutar `.bss` sekcije na 0. Konačno, prelazi se
na prethodni segment koda za inicijalizaciju `SP` registra, čime je proces incijalizacije
kompletiran.

```
_start:
    /* Initialize .data section */
    ldr r0, =__text_end__
    ldr r1, =__data_start__
    ldr r2, =__data_end__
    sub r3, r2, r1

    /* Handle the case when .data section size is 0 */
    cmp r3, #0
    beq init_bss

copy_data:
    ldrb r4, [r0], #1
    strb r4, [r1], #1
    subs r3, r3, #1
    bne copy_data

init_bss:
    /* Initialize .bss section  */
    ldr r0, =__bss_start__
    ldr r1, =__bss_end__
    sub r2, r1, r0

    /* Handle the case when .bss section size is 0 */
    cmp r2, #0
    beq init_stack
    mov r4, #0

zero_bss:
    strb r4, [r0], #1
    subs r2, r2, #1
    bne zero_bss

init_stack:
    ldr r1, =__stack_start__
    mov sp, r1
    bl main
    b .
```

U okviru prethodnog asemblerskog programa se koriste simboli koji određuju početak i kraj
sekcija definisanih u linker skripti. Jedini simbol koji nedostaje u ovoj skripti je `__text_end__`
koji označava lokaciju od koje se nalaze inicijalne vrijednosti varijabli unutar `.data`
sekcije (kraj `.text` sekcije), odnosno LMA adresu sekcije. Ovu adresu možete pribaviti
korišćenjem direktive `LOADADDR` ili korišćenjem operatora `.` na kraju `.text` sekcije.

Sada modifikujte *startup* kod korišćenjem asemblerskog programa datog iznad i dodajte
nedostajuće simbole u linker skripti, a zatim ponovo kompajlirajte program i učitajte
korišćenjem JTAG interfejsa za debagovanje. Potvrdite da inicijalne vrijednosti statičkih i
globalnih varijabli sada imaju očekivane vrijednosti.

> [!TIP]
> Svaki put kada ponovo učitavate program na ploču, potrebno je da isključite napajanje
na ploči ili da pritisnete taster za *hard reset* ploče (`HPS RST`) kako bi se ploča
postavila u ispravno incijalno stanje.

Po završetku ovog dijela vježbe, predajte sve modifikacije na prethodno kreiranu granu
za treću vježbu u repozitorijumu.

## *Hello World* aplikacija za *embedded* sisteme: *LED Blinking*

U drugom dijelu vježbe, demonstriraćemo izradu najjednostavnije aplikacije u *embedded*
svijetu, koja predstavlja *Hello World* primjer u ovom domenu, a to je kontrola LED
diode na ploči. U tu svrhu ćemo iskoristiti `HPS_LED` koja je povezana na GPIO53 pin
*Cyclone V* čipa, što možemo da vidimo iz šeme same ploče (vidi sliku).

![gpio-schematic](figs/gpio-schematic.PNG)

Sa slike takođe vidimo da je na pin GPIO54 povezan korisnički taster `HPS_KEY` čije stanje
možemo pročitati ako se ovaj pin postavi kao digitalni ulaz.

Lokacija korisničkog tastera i LED diode na ploči, prikazana je na sljedećoj slici.

![board-button-led](figs/board-button-led.jpg)

Kreirajte projekat na sličan način kao u prethodnom dijelu vježbe, s tim što naziv projekta
treba da bude `blinky`. Nakon što ste kreirali projekat, kompajlirajte ga da dobijete
izvršni fajl u ELF formatu. Izvršićemo inspekciju dobijenog fajla Komandom

```
cd Debug
arm-none-eabi-objdump -d blinky
```

Ova komanda nam prikazuje disasembliran kod sa asemblerskim instrukcijama i simboličkim
labelama koji se nalaze u `.text` sekciji. Parcijalan prikaz relevatnih dijelova ovog koda
dat je ispod.

```
blinky:     file format elf32-littlearm


Disassembly of section .text:

ffff0000 <spin>:
ffff0000:       e52db004        push    {fp}            @ (str fp, [sp, #-4]!)
ffff0004:       e28db000        add     fp, sp, #0
ffff0008:       e24dd00c        sub     sp, sp, #12
ffff000c:       e50b0008        str     r0, [fp, #-8]
ffff0010:       ea000000        b       ffff0018 <spin+0x18>
ffff0014:       e320f000        nop     {0}
ffff0018:       e51b3008        ldr     r3, [fp, #-8]
ffff001c:       e2432001        sub     r2, r3, #1
ffff0020:       e50b2008        str     r2, [fp, #-8]
ffff0024:       e3530000        cmp     r3, #0
ffff0028:       1afffff9        bne     ffff0014 <spin+0x14>
ffff002c:       e320f000        nop     {0}
ffff0030:       e320f000        nop     {0}
ffff0034:       e28bd000        add     sp, fp, #0
ffff0038:       e49db004        pop     {fp}            @ (ldr fp, [sp], #4)
ffff003c:       e12fff1e        bx      lr

ffff0040 <main>:
ffff0040:       e92d4800        push    {fp, lr}
ffff0044:       e28db004        add     fp, sp, #4
ffff0048:       e24dd008        sub     sp, sp, #8
ffff004c:       e3a03000        mov     r3, #0
ffff0050:       e50b3008        str     r3, [fp, #-8]
ffff0054:       eb00013f        bl      ffff0558 <board_init>
[...]
ffff071c:       e28bd000        add     sp, fp, #0
ffff0720:       e49db004        pop     {fp}            @ (ldr fp, [sp], #4)
ffff0724:       e12fff1e        bx      lr

ffff0728 <_start>:
ffff0728:       e59f1008        ldr     r1, [pc, #8]    @ ffff0738 <_start+0x10>
ffff072c:       e1a0d001        mov     sp, r1
ffff0730:       ebfffe42        bl      ffff0040 <main>
ffff0734:       eafffffe        b       ffff0734 <_start+0xc>
ffff0738:       ffff6008        .word   0xffff6008

ffff073c <iocsr_scan_chain>:
ffff073c:       300c0300 00000000 0ff00000 00000000     ...0............
ffff074c:       000300c0 00008000 00080000 18060000     ................
ffff075c:       18000000 00018060 00020000 00004000     ....`........@..
ffff076c:       200300c0 10000000 00000000 00000040     ... ........@...
ffff077c:       00010000 00002000 10018060 06018000     ..... ..`.......
ffff078c:       06000000 00010018 00006018 00001000     .........`......
ffff079c:       0000c030 00000000 03000000 0000800c     0...............
ffff07ac:       00c0300c 00000800                       .0......
```

Ono što odmah možemo primjetiti je da se na adresi `0xffff0000` nalazi labela `<spin>` koja
odgovara funkciji `spin()` definisanoj u fajlu `blinky.c`. Međutim, nakon izvršavanja
inicijalne konfiguracije hardvera od strane *BootROM* koda na platformi, podrazumijevano
se prelazi na izvršavanje instrukcija počevši od početne adrese *on-chip* SRAM memorije
(`0xffff0000`). To znači da će se prvo izvršiti instrukcije iz `spin()` funkcije umjesto
*startup* instrukcija za inicijalizaciju `SP` registra. Kao posljedicu imamo da program uopšte
ne može da se izvrši jer će doći do *exception*-a zbog neispravno konfigurisane stek memorije.
Šta se desilo i zašto ovaj problem nismo imali u prethodnom primjeru?

Ulazne sekcije prilikom kreiranja izlaznih sekcija se obično sortiraju prema nazivu fajla,
a kako je naziv `startup.S` u alfabetu poslije naziva `blinky.c`, tako će i njegove `.text`
sekcije biti smještene pri kraju. Ovaj problem nismo imali u prethodnom primjeru jer se naziv
`test-baremetal.c` nalazi poslije `startup.S`. Zaista, ako izlistate izvršni fajl iz prethodnog
primjera, vidjećete da je prva instrukcija koja se izvršava od adrese `0xffff0000` označena
labelom `<_start>` što je korektno.

Da bismo riješili ovaj problem, moramo eksplicitno da navedemo naziv objektnog fajla prilikom
kreiranja `.text` sekcije. U tu svrhu je potrebno modifikovati tekst sekciju tako da ima
sljedeći izgled.

```
.text :
{
    *startup.o (.text)
    *(.text*)
    *(.rodata*)
} > program
```

Na ovaj način linkeru govorimo da želimo da ulazna `.text` sekcija iz objektnog fajla
`startup.o` bude locirana na početku izlazne `.text` sekcije. Ako ponovo prekompajliramo
projekat i prikažemo disasembliran kod, dobijamo sljedeće.

```
blinky:     file format elf32-littlearm


Disassembly of section .text:

ffff0000 <_start>:
ffff0000:       e59f1008        ldr     r1, [pc, #8]    @ ffff0010 <_start+0x10>
ffff0004:       e1a0d001        mov     sp, r1
ffff0008:       eb000011        bl      ffff0054 <main>
ffff000c:       eafffffe        b       ffff000c <_start+0xc>
ffff0010:       ffff6008        .word   0xffff6008

ffff0014 <spin>:
ffff0014:       e52db004        push    {fp}            @ (str fp, [sp, #-4]!)
ffff0018:       e28db000        add     fp, sp, #0
ffff001c:       e24dd00c        sub     sp, sp, #12
ffff0020:       e50b0008        str     r0, [fp, #-8]
ffff0024:       ea000000        b       ffff002c <spin+0x18>
ffff0028:       e320f000        nop     {0}
ffff002c:       e51b3008        ldr     r3, [fp, #-8]
ffff0030:       e2432001        sub     r2, r3, #1
ffff0034:       e50b2008        str     r2, [fp, #-8]
ffff0038:       e3530000        cmp     r3, #0
ffff003c:       1afffff9        bne     ffff0028 <spin+0x14>
ffff0040:       e320f000        nop     {0}
ffff0044:       e320f000        nop     {0}
ffff0048:       e28bd000        add     sp, fp, #0
[...]
fff0734:       e49db004        pop     {fp}            @ (ldr fp, [sp], #4)
ffff0738:       e12fff1e        bx      lr

ffff073c <iocsr_scan_chain>:
ffff073c:       300c0300 00000000 0ff00000 00000000     ...0............
ffff074c:       000300c0 00008000 00080000 18060000     ................
ffff075c:       18000000 00018060 00020000 00004000     ....`........@..
ffff076c:       200300c0 10000000 00000000 00000040     ... ........@...
ffff077c:       00010000 00002000 10018060 06018000     ..... ..`.......
ffff078c:       06000000 00010018 00006018 00001000     .........`......
ffff079c:       0000c030 00000000 03000000 0000800c     0...............
ffff07ac:       00c0300c 00000800                       .0......
```

Na taj način smo popravili pomenuti problem i izvršavanje programa počinje sa
odgovarajućim instrukcijama.

Glavni program u okviru fajla `blinky.c` inicijalizuje lokalnu varijablu `led_state` čija
vrijednost određuje stanje LED diode (0 znači da je dioda isključena, dok 1 znači da je ona
uključena). Nakon toga se poziva funkcija `board_init()` koja je implementirana u okviru
fajla `board_init.c`. Njena implementacija ima sljedeći izgled:

```
void board_init()
{
    // Freeze the IO banks (force safe values during configuration)
    sysmgr_vioctrl_freeze_req();
    // Configure IOCSR for bank 7A
    board_iocsr_config();
    // Configure pinmux for GPIO1
    board_pinmux_config();
    // Unfreeze the IO banks so configured IOCSR values take place
    sysmgr_vioctrl_thaw_req();
    // Configure reset manager (reset WD0 and release GPIO1 reset)
    board_reset_config();
}
```

Implementacija svake funkcije koja se poziva nalazi se u istom fajlu, a detalji o registrima
kojima se u tom smislu pristupa mogu da se vide u okviru specifikacije *Cyclone V* čipa
i njegove [mape registara](https://www.intel.com/content/www/us/en/programmable/hps/cyclone-v/hps.html).
Svaka funkcija je detaljno propraćena komentarima, pa se studentima preporučuje da izdvoje
malo vremena i analiziraju dati kod.

U osnovi, inicijalizacija ploče se svodi na zamrzavanje ulazno-izlaznih banki čipa
prije konfiguracije (funkcijа `sysmgr_vioctrl_freeze_req()`), tj. postavljamo pinove u
bezbjedno stanje. Nakon toga, preko *Scan Manager* periferije postavljamo konfiguraciju pinova
čipa (funkcija `board_iocsr_config()`) za banku 7A koja je nama relevantna (na njoj se nalaze
GPIO pinovi na koje su povezani `HPS_LED` i `HPS_KEY`). Ovom konfiguracijom se postavljaju
fizičke karakteristike pinova (jačina struje, standard napona, konfiguracija
*pull-up*/*pull-down* otpornika itd.).

Slijedi *pinumux* konfiguracija za GPIO1 modul koji koristimo u našem programu (funkcija
`board_pinmux_config()`) koja sa fizičkim pinovima povezuje izlaze GPIO1 kontrolera (GPIO53 i
GPIO54) kako bismo mogli da kontrolišemo stanje LED diode i čitamo stanje tastera.

Poslije konfiguracije ulazno-izlaznih pinova, odmrzavamo ulazno-izlazne banke (funkcija
`sysmgr_vioctrl_thaw_req()` kako bi se prethodno postavljena konfiguracija aktuelizovala na
fizičkim pinovima.

Konačno, pozivamo funkciju `board_reset_config()` koja resetuje *watchdog* tajmer (kako bismo
izbjegli uzastopno restovanje ploče zbog njegovog preteka) i izvodi iz reset stanja GPIO1 modul,
kako bismo mogli da pristupimo njegovim registrima.

Nakon inicijalizacije ploče, slijedi postavljanje smjera `HPS_LED` pina tako da on bude izlazni
(funkcija `set_gpio_dir()`). Na raspolaganju su i druge funkcije za manipulaciju sa GPIO pinovima
na GPIO1 kontroleru, `write_gpio()` i `read_gpio()`, koje omogućavaju postavljanje vrijednosti
izlaznog pina i čitanje stanja ulaznog pina, respektivno. Sve pomenute funkcije implementirane
su u fajlu `gpio.c`.

Konačno, program ulazi u beskonačnu petlju u kojoj otprilike svakih pola sekunde mijenja stanje
LED diode. Funkcija `spin()` je obična *busy-wait* funkcija koja blokira izvršavanje programa
(izvršava *nop* instrukcije) za predefinisani vremenski interval (u datom slučaju, pola sekunde
odgovara vrijednosti 250000 koju prosljeđujemo funkciji).

Nakon osnovnog pregleda datog koda, prelazimo na učitavanje programa i njegovo izvršavanje
na ciljnoj platformi. Kao i u prethodnom primjeru, koristićemo JTAG interfejs za debagovanje.
S obzirom da u ovom primjeru pristupamo registrima periferija, bilo bi zgodno da imamo na
raspolaganju komforan prikaz registara periferija. U tu svrhu možemo u konfiguraciju debagera
da učitamo SVD fajl sa definicijom svih registara platforme.

U prozoru za selekciju i podešavanje JTAG interfejsa (možete ga prikazati desnim klikom miša
na folder projekta, a zatim iz padajućeg menija odabrati opciju **Debug As**&rarr;**Debug Configurations...**),
pod tabulatorom *SVD Path* definišemo putanju do SVD fajla kao što je prikazano na slici ispod.

![svd-path](figs/svd-path.PNG)

U našem slučaju potrebno je kliknuti na dugme *Browse* a zatim učitati fajl sa lokacije
`<QUARTUS_INSTALL_DIR>/riscfree/xsvd/cyclonev_hps.svd`.

Sada možete pokrenuti debagovanje (npr. klikom na dugme *Debug*). Nakon inicijalizacije interfejsa
i učitavanja programa, njegovo izvršavanje će se zaustaviti na početku funkcije *main*. Sada
postavite prekidnu tačku na liniju `write_gpio(HPS_LED, led_state);` i omogućite prikaz željenih
periferija (npr. `rstmgr`, `sysmgr` i `gpio1`) kao što je ilustrovano na sljedećoj slici.

![peripheral-register-selection](figs/peripheral-register-selection.PNG)

Sada možete da izaberete periferiju iz liste i da posmatrate kako se mijenjaju pojedini registri.

Pokrenite izvršavanje programa prethodno definisane prekidne tačke (dugme *Resume*), a kada se
izvršavanje programa suspenduje u prekdinoj tački, posmatrajte kako se mijenjaju pojedini regstri
periferija. Jedan takav prikaz za GPIO1 skup registara je dat na slici ispod.

![gpio1-register-view](figs/gpio1-register-view.PNG)

Eksperimentišite sa izvršavanjem programa i pratite stanje registara periferija, kao i fizičko
LED diode na ploči. Trebalo bi da se LED dioda naizmjenično uključuje i isključuje svaki put
kada nastavite sa izvršavanjem programa.

> [!WARNING]  
> U okviru *Ashling RiscFree IDE for Intel FPGAs* okruženja postoji defekt koji uzrokuje da
argumenti koji se prosljeđuju funkciji pri izvršavanju u režimu korak po korak uvijek imaju
vrijednost 0, tako da ovakvim izvršavanjem program neće ispravno raditi (periferije neće biti
ispravno inicijalizovane). Prema tome, preporučujemo da program debagujete korišćenjem
isključivo prekidnih tačaka.

Zaustavite izvršavanje programa i isključite ploču. Obrišite prethodno definisanu prekidnu
tačku ili je onemogućite u *Breakpoints* prozoru, a zatim ponovo uključite ploču i pokrenite
izvršavanje programa bez prekidnih tačaka. Nakon što kliknete *Resume* poslije inicijalnog
suspendovanja programa na početku *main* funkcije, na ploči bi trebalo alternativno da se
uključuje i isključuje LED dioda sa intervalom od otprilike pola sekunde između promjene stanja.

Modifikujte dati program tako da tasterom `HPS_KEY` kontrolišete rad blinkajuće diode. Potrebno
je da inicijalno po pokretanju programa LED dioda blinka kao u izvornom primjeru, a kada korisnik
pritisne taster da se blinkanje zaustavi. Ponovnim pritiskom na taster ponovo aktiviramo blinkanje
i tako u krug. Za čitanje stanja tastera možete koristiti funkcije implementirane u `gpio.c` fajlu.

Na kraju, predajte sve modifikacije na granu vježbe u repozitorijumu.
