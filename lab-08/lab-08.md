# Laboratorijska vježba 8

Cilj laboratorijske vježbe je da se studenti upoznaju sa *build* sistemima za ugrađene
sisteme bazirane na *Linux* operativnom sistemu.

Nakon uspješno realizovane vježbe studenti će biti sposobni da:
1. instaliraju i konfigurišu *build* sistem zasnovan na *Buildroot* i *Yocto*
projektu,
2. generišu binarnu sliku sistema koja se može kopirati na SD karticu,
3. prilagode infrastrukturu *build* sistema dodavanjem specifičnih konfiguracionih
i drugih fajlova i
4. dodaju nove softverske komponente u postojeći  *build* sistem.

## Preduslovi za izradu vježbe

Za uspješno izvođenje laboratorijske vježbe potrebno je sljedeće:

- razvojna ploča *DE1-SoC* sa pripadajućim napajanjem i kablom za povezivanje računara sa
UART interfejsom na ploči,
- pripremljen *toolchain* za kroskompajliranje sofvera za ciljnu platformu,
- SD kartica i
- *Ethernet* mrežni interfejs na razvojnoj platformi i mrežni kabl za povezivanje sa
pločom.

## Generisanje sistema korišćenjem *Buildroot* alata

U ovom dijelu vježbe ćemo se upoznati sa prvim *build* sistemom pod nazivom *Buildroot*.
Prvo je potrebno da preuzmemo repozitorijum ovog *build* sistema i da se prebacimo
na odgovarajuću granu:

```
git clone https://gitlab.com/buildroot.org/buildroot.git
cd buildroot
git checkout 2024.2
```

Potvrdite da na razvojnom računaru imate instalirane sve [obavezne softverske pakete](https://buildroot.org/downloads/manual/manual.html#requirement-mandatory).

Za DE1-SoC ploču ne postoji predefinisana konfiguracija, pa ćemo kreirati resurse neophodne
za ovu ploču. Prvo je potrebno kreirati direktorijum u kojem će se nalaziti potrebni resursi

```
mkdir -p board/terasic/de1soc_cyclone5
```

u koji kopiramo `defconfig` i DTS fajlove koje smo kreirali u prethodnoj vježbi:

```
cp <path-to-linux-src>/arch/arm/configs/de1_soc_defconfig board/terasic/de1soc_cyclone5/
cp <path-to-linux-src>/arch/arm/boot/dts/socfpga_cyclone5_de1_soc.dts board/terasic/de1soc_cyclone5/
```

Osim prethodna dva fajla, potrebno je da pripremimo i fajl kojim opisujemo strukturu
SD kartice koja je pogodna za našu platformu. Kao polaznu osnovu, možemo da uzmemo
fajl `genimage.cfg` platforme `socrates_cyclone5` koja se nalazi u folderu
`board/altera/socrates_cyclone5`.

```
cp board/altera/socrates_cyclone5/genimage.cfg board/terasic/de1soc_cyclone5/
```

Ovako kopiran `genimage.cfg` fajl ćemo editovati i promijeniti naziv DTB fajla u `boot.vfat`
sekciji. Potrebno je da zamijenimo naziv ovog fajla i da definišemo `socfpga_cyclone5_de1_soc.dtb`.

> [!TIP]
> Konfiguracioni fajl kojim opisujemo strukturu SD kartice ima veliki broj drugih opcija. Na
repozitorijumu [*genimage*](https://github.com/pengutronix/genimage) projekta možete pronaći više
informacija o svim podržanim opcijama i svim mogućnostima koje ovaj softverski alat nudi.

Sljedeći fajl koji ćemo kopirati sa iste lokacije je `boot-env.txt`:

```
cp board/altera/socrates_cyclone5/boot-env.txt board/terasic/de1soc_cyclone5/
```

Ovaj fajl opisuje *U-Boot* okruženje (umjesto podrazumijevanog sadržaja koji je ugrađen
u sam izvorni kod *bootloader*-a). Prilagodićemo sadržaj ovog fajla tako da odgovara
okruženju naše platforme. U tom smislu editujte kopirani `boot-env.txt` tako da postavite
adrese na koje se učitava kernel (varijabla `linux_load_address`) i DTB fajl (varijabla
`linux_dtb_load_address`), kao i naziv DTB fajla (varijabla `linux_dtb`). Za prethodno pomenute
adrese možete koristiti vrijednosti iz prethodnih vježbi. Osim toga, potrebno je još da postavimo
`rw` tip za *root* fajl sistem (umjesto `ro`) u `bootargs` varijabli. Konačno, obrišite sljedeću
liniju u ovom fajlu:

```
source_env=fatload mmc 0:1 0x2000000 boot.scr; source 0x2000000
```

Sada možemo da pristupimo konfiguraciji *build* sistema. Našu konfiguraciju ćemo bazirati na
postojećoj, kako bismo smanjili broj neophodnih izmjena. Uzećemo `terasic_de10nano_cyclone5_defconfig`
kao polaznu konfiguraciju, a onda pokrenuti `make menuconfig` (ili `make xconfig` ako korsitite grafički
interfejs) za dodatno prilagođavanje određenih opcija.

```
make terasic_de10nano_cyclone5_defconfig
make menuconfig
```

U konfiguraciji napravite sljedeće izmjene:

- U okviru **Toochain**:
	- postavite **Toolchain type** opciju na **External toolchain**
    - postavite **Toolchain** opciju na **Custom toolchain**
    - postavite **Toolchain origin** opciju na **Pre-installed toolchain**
    - postavite **Toolchain path** tako da odgovara putanji korišćenog
    *toolchain*-a (relativno u odnosu na `buildroot` folder)
    - ostavite opciju **Toolchain prefix** kako jeste (**$(ARCH)-linux**)
    - ostavite opciju **External toolchain gcc version** kako jeste (**13x**)
    - postavite **External toolchain kernel headers series** opciju na **6.1.x**
    - postavite **External toolchain C library** opciju na **glibc**
    - uključite opcije **Toolchain has SSP support?** i **Toolchain has C++ support?**
    (ako nisu već uključene) i
	- isključite opciju **Toolchain has RPC support?**
- U okviru **Build options**:
	- postavite **Location to save buildroot config** opciju na **<path-to-buildroot>/configs/terasic_de1soc_cyclone5_defconfig**
    ili ostavite prazno polje (`<path-to-buildroot>` treba zamijeniti apsolutnom putanjom do `buildroot` direktorijuma)
- U okviru **System configuration**:
	- postavite **System hostname** opciju na **etfbl**
	- postavite **System banner** opciju na **Welcome to DE1-SoC on ETFBL**
    - odaberite **systemd** u okviru opcije **Init system** i
    - izmijenite **Extra arguments** opciju tako da bude **-c board/terasic/de1soc_cyclone5/genimage.cfg**
- U okviru **Kernel**:
	- postavite **Custom repository version** opciju na **socfpga-6.1.38-lts**
    - postavite **Kernel configuration** opciju na **Using a custom (def)config file**
    - postavite **Configuration file path** opciju na **board/terasic/de1soc_cyclone5/de1_soc_defconfig**
    - obrišite sadržaj opcije **In-Tree Device Tree Source file names**
    - postavite **Out-of-tree Device Tree Source file names** opciju na **board/terasic/de1soc_cyclone5/socfpga_cyclone5_de1_soc.dts** i
    - uključite opciju **Linux Kernel Tools**&rarr;**iio**
- U okviru **Target packages**:
	- uključite opciju **Hardware handling**&rarr;**evtest**
	- uključite opciju **Libraries**&rarr;**Hardware handling**&rarr;**libgpiod** i
    - uključite opciju **Libraries**&rarr;**Hardware handling**&rarr;**install tools**
- U okviru **Bootloaders**:
	- isključite opciju **Disable barebox** i uključite opciju **U-Boot**
	- zadržite vrijednost opcije **U-Boot version** (**2024.01**)
    - postavite **U-Boot configuration** opciju na **Using an in-tree board defconfig file**
    - postavite **Board defconfig** opciju na **socfpga_de1_soc**
    - uključite opciju **U-Boot needs dtc**
    - u okviru **U-Boot binary format** isključite opciju **u-boot.bin** i uključite opciju **u-boot.img**
    - uključite opciju **Install U-Boot SPL binary image** i
    - uključite opciju **CRC image for Altera SoC FPGA (mkpimage)**
- U okviru **Host utilities**:
	- uključite opciju **host u-boot tools**
    - uključite opciju **Environment image**
    - postavite **Source files for environment** opciju na **board/terasic/de1soc_cyclone5/boot-env.txt** i
	- postavite **Size of environment** opciju na 8192

U prethodnim opcijama smo odabrali *toolchain* koji smo ranije generisali da bismo uštedjeli vrijeme
pri generisanju artifakata *build* sistema. Osim toga, prilagodili smo opcije vezane za verziju kernela,
kao i naše prilagođene fajlove za njegovu konfiguraciju i *Device Tree* za opis hardvera, kao i za korišćeni
*bootloader* (*U-Boot* u našem slučaju). Konačno, prilagodili smo neke sistemske opcije, uključili neke
dodate softverske pakete koje želimo da se nalaze u *root* fajl sistemu i definisali opcije za generisanje
prilagođenog okruženja za *U-Boot* u sklopu **Host utilities** opcija.

Sada ostaje da sačuvamo konfiguraciju i pokrenemo proces generisanja artifakata komandom `make`.

> [!NOTE]
> Ovako konfigurisano *Buildroot* okruženje zahtjeva oko 6.2GB prostora na disku, a potrebno je oko pola
sata za generisanje artifakata (testirano na virtuelnoj mašini sa dva jezgra procesora i 3GB radne memorije).

Po završetku, svi relevantni fajlovi će da se nalaze u folderu `<buildroot-folder>/output/images`. Dovoljno
je da prekopirate sliku SD kartice `sdcard.img` sljedećom komandom:

```
cd output/images
sudo dd if=sdcard.img of=/dev/sdb bs=1M
```

> [!IMPORTANT]
> Prije korišćenja komande `dd` potrebno je da demontirate fajl sisteme particija SD kartice (ukoliko su montirane).
Putanju do foldera koji predstavlja tačku montiranja particija SD kartice možete prikazati komandom `lsblk`. Kao i
ranije, vodite računa da ovom komandom možete napraviti štetu na fajl sistemu razvojnog računara ako ne specificirate
odgovarajući uređaj.

Ovako pripremljenu SD karticu umetnite u slot na DE1-SoC ploči i pokrenite izvršavanje.

Prvo ćemo verifikovati ispravnost *U-Boot* okruženja. U tom smislu, prekinite proces podizanja sistema i
prikažite sadržaj kompletnog okruženja (komanda `printenv`).

```
=> printenv
bootargs=console=ttyS0,115200 root=/dev/mmcblk0p3 rw rootwait
bootcmd=run linux_load; bootz ${linux_load_address} - ${linux_dtb_load_address}
bootdelay=1
bootmode=sd
fdtcontroladdr=3bf8e640
fpgatype=cv_se_a5
linux_dtb=socfpga_cyclone5_de1_soc.dtb
linux_dtb_load_address=0x02000000
linux_load=mmc rescan; fatload mmc 0:1 ${linux_load_address} zImage; fatload mmc 0:1 ${linux_dtb_load_address} ${linux_dtb}
linux_load_address=0x01000000
stderr=serial
stdin=serial
stdout=serial
ver=U-Boot 2024.01 (May 18 2024 - 15:46:26 +0200)

Environment size: 527/8188 bytes
```

Iz ispisa datog iznad možemo da zaključimo da je okruženje u skladu sa našom konfiguracijom. Restartujte ploču
i pustite da se pokrenem *Linux* operativni sistem.

Nakon procesa podizanja i pokretanja servisa, trebalo bi da dobijete sljedeću poruku:

```
Welcome to DE1-SoC on ETFBL
etfbl login: root
#
```

Ovo nam ukazuje da su naše modifikacije uključene kako treba, pa možemo da se logujemo na sistem sa `root`
korisničkim imenom (nije potrebno unositi šifru, jer je nismo postavili u konfiguraciji).

Prvo što ćemo provjeriti je lista učitanih kernel modula korišćenjem komande `lsmod`.

```
# lsmod
Module                  Size  Used by
adxl345_i2c            16384  0
adxl345_core           16384  1 adxl345_i2c
```

Zanimljivo je da je kernel modul za ADXL345 uređaj učitan automatski, za razliku od slučaja u prethodnoj
vježbi kada smo to morali da uradimo ručno. Zašto je to tako?

Razlika je u tome što smo u *Buildroot* konfiguraciji (u sklopu **System configuration** postavki) postavili
`systemd` kao **Init system**. Ovaj program koristi `udev` alatku za automatsko prepoznavanje prisutnih uređaja
(na osnovu *alias*-a) i učitavanje odgovarajućih drajvera pri pokretanju sistema, što nije bio slučaj sa
`init` programom koji je dio *BusyBox*-a. Prisustvo ADXL345 uređaja možemo dodatno da potvrdimo komandom
`lsiio`.

```
# lsiio
Device 000: adxl345
```

> [!NOTE]
> Alatka `lsiio` ranije nije bila dostupna. Sada smo je instalirali u *root* fajl sistem uključenjem opcije
**Linux Kernel Tools**&rarr;**iio** pri podešavanju opcija kernela.

Kao dio *root* fajl sistema, uključili smo i alatku `evtest` koja nam omogućava komforniji rad sa tasterima,
prekidačima i drugim ulaznim uređajima u sistemu. Ako sada pokrenemo ovu alatku i pritišćemo taster HPS_KEY
na ploči, vidjećemo da se dobijaju informacije koje lako možemo interpretirati i koje odgovaraju postavkama
u *Device Tree* fajlu za ovaj taster.

```
# evtest
No device specified, trying to scan all of /dev/input/event*
Available devices:
/dev/input/event0:      gpio-keys
Select the device event number [0-0]: 0
Input driver version is 1.0.1
Input device ID: bus 0x19 vendor 0x1 product 0x1 version 0x100
Input device name: "gpio-keys"
Supported events:
  Event type 0 (EV_SYN)
  Event type 1 (EV_KEY)
    Event code 16 (KEY_Q)
Properties:
Testing ... (interrupt to exit)
Event: time 1706136552.274086, type 1 (EV_KEY), code 16 (KEY_Q), value 0
Event: time 1706136552.274086, -------------- SYN_REPORT ------------
Event: time 1706136552.544549, type 1 (EV_KEY), code 16 (KEY_Q), value 1
Event: time 1706136552.544549, -------------- SYN_REPORT ------------
Event: time 1706136555.273270, type 1 (EV_KEY), code 16 (KEY_Q), value 0
Event: time 1706136555.273270, -------------- SYN_REPORT ------------
Event: time 1706136555.566018, type 1 (EV_KEY), code 16 (KEY_Q), value 1
Event: time 1706136555.566018, -------------- SYN_REPORT ------------
Event: time 1706136556.713513, type 1 (EV_KEY), code 16 (KEY_Q), value 0
Event: time 1706136556.713513, -------------- SYN_REPORT ------------
Event: time 1706136557.327922, type 1 (EV_KEY), code 16 (KEY_Q), value 1
Event: time 1706136557.327922, -------------- SYN_REPORT ------------
Event: time 1706136557.957886, type 1 (EV_KEY), code 16 (KEY_Q), value 0
Event: time 1706136557.957886, -------------- SYN_REPORT ------------
Event: time 1706136558.560921, type 1 (EV_KEY), code 16 (KEY_Q), value 1
Event: time 1706136558.560921, -------------- SYN_REPORT ------------
Event: time 1706136559.224136, type 1 (EV_KEY), code 16 (KEY_Q), value 0
Event: time 1706136559.224136, -------------- SYN_REPORT ------------
Event: time 1706136559.965952, type 1 (EV_KEY), code 16 (KEY_Q), value 1
Event: time 1706136559.965952, -------------- SYN_REPORT ------------
```

Na ovaj način možemo vrlo jednostavno da debagujemo rad ulaznih uređaja u sistemu.

Za rad sa GPIO kontrolerom, pogodne su alatke koje su dio `libgpiod` biblioteke. Sada ćemo
ilustrovati način korišćenja nekih od njih.

Alatkom `gpiodetect` možemo da izlistamo sve dostupne i trenutno aktivne GPIO kontrolere
sa osnovnim informacijama.

```
# gpiodetect
gpiochip0 [ff708000.gpio] (29 lines)
gpiochip1 [ff709000.gpio] (29 lines)
gpiochip2 [ff70a000.gpio] (27 lines)
```

Alatka `gpioinfo` prikazuje detaljnije informacije o svakom GPIO kontroleru.

```
# gpioinfo
gpiochip0 - 29 lines:
        line   0:      unnamed       unused   input  active-high
        line   1:      unnamed       unused   input  active-high
        line   2:      unnamed       unused   input  active-high
        line   3:      unnamed       unused   input  active-high
        line   4:      unnamed       unused   input  active-high
        line   5:      unnamed       unused   input  active-high
        line   6:      unnamed       unused   input  active-high
        line   7:      unnamed       unused   input  active-high
        line   8:      unnamed       unused   input  active-high
        line   9:      unnamed       unused   input  active-high
        line  10:      unnamed       unused   input  active-high
        line  11:      unnamed       unused   input  active-high
        line  12:      unnamed       unused   input  active-high
        line  13:      unnamed       unused   input  active-high
        line  14:      unnamed       unused   input  active-high
        line  15:      unnamed       unused   input  active-high
        line  16:      unnamed       unused   input  active-high
        line  17:      unnamed       unused   input  active-high
        line  18:      unnamed       unused   input  active-high
        line  19:      unnamed       unused   input  active-high
        line  20:      unnamed       unused   input  active-high
        line  21:      unnamed       unused   input  active-high
        line  22:      unnamed       unused   input  active-high
        line  23:      unnamed       unused   input  active-high
        line  24:      unnamed       unused   input  active-high
        line  25:      unnamed       unused   input  active-high
        line  26:      unnamed       unused   input  active-high
        line  27:      unnamed       unused   input  active-high
        line  28:      unnamed       unused   input  active-high
gpiochip1 - 29 lines:
        line   0:      unnamed       unused   input  active-high
        line   1:      unnamed       unused   input  active-high
        line   2:      unnamed       unused   input  active-high
        line   3:      unnamed       unused   input  active-high
        line   4:      unnamed       unused   input  active-high
        line   5:      unnamed       unused   input  active-high
        line   6:      unnamed       unused   input  active-high
        line   7:      unnamed       unused   input  active-high
        line   8:      unnamed       unused   input  active-high
        line   9:      unnamed       unused   input  active-high
        line  10:      unnamed       unused   input  active-high
        line  11:      unnamed       unused   input  active-high
        line  12:      unnamed       unused   input  active-high
        line  13:      unnamed       unused   input  active-high
        line  14:      unnamed       unused   input  active-high
        line  15:      unnamed       unused   input  active-high
        line  16:      unnamed       unused   input  active-high
        line  17:      unnamed       unused   input  active-high
        line  18:      unnamed       unused   input  active-high
        line  19:      unnamed       unused   input  active-high
        line  20:      unnamed       unused   input  active-high
        line  21:      unnamed       unused   input  active-high
        line  22:      unnamed       unused   input  active-high
        line  23:      unnamed       unused   input  active-high
        line  24:      unnamed    "hps_led"  output  active-high [used]
        line  25:      unnamed    "hps_key"   input  active-high [used]
        line  26:      unnamed       unused   input  active-high
        line  27:      unnamed       unused   input  active-high
        line  28:      unnamed       unused   input  active-high
gpiochip2 - 27 lines:
        line   0:      unnamed       unused   input  active-high
        line   1:      unnamed       unused   input  active-high
        line   2:      unnamed       unused   input  active-high
        line   3:      unnamed       unused   input  active-high
        line   4:      unnamed       unused   input  active-high
        line   5:      unnamed       unused   input  active-high
        line   6:      unnamed       unused   input  active-high
        line   7:      unnamed       unused   input  active-high
        line   8:      unnamed       unused   input  active-high
        line   9:      unnamed       unused   input  active-high
        line  10:      unnamed       unused   input  active-high
        line  11:      unnamed       unused   input  active-high
        line  12:      unnamed       unused   input  active-high
        line  13:      unnamed       unused   input  active-high
        line  14:      unnamed       unused   input  active-high
        line  15:      unnamed       unused   input  active-high
        line  16:      unnamed       unused   input  active-high
        line  17:      unnamed       unused   input  active-high
        line  18:      unnamed       unused   input  active-high
        line  19:      unnamed       unused   input  active-high
        line  20:      unnamed       unused   input  active-high
        line  21:      unnamed       unused   input  active-high
        line  22:      unnamed       unused   input  active-high
        line  23:      unnamed       unused   input  active-high
        line  24:      unnamed       unused   input  active-high
        line  25:      unnamed       unused   input  active-high
        line  26:      unnamed       unused   input  active-high
```

Na kraju, testirajmo mrežnu konekciju našeg sistema. U tom smislu, najjednostavnije je da koristimo
alatku `ifconfig` koja nam izlistava sve dostupne i trenutno aktivne mrežne interfejse.

```
# ifconfig
lo        Link encap:Local Loopback
          inet addr:127.0.0.1  Mask:255.0.0.0
          inet6 addr: ::1/128 Scope:Host
          UP LOOPBACK RUNNING  MTU:65536  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)
```

Iz prethodnog ispisa vidimo da je na raspolaganju samo *loopback* interfejs iako znamo da na ploči imamo
na raspolaganju i fizički *ethernet* interfejs. Da je ovaj interfejs prisutan, možemo da potvrdimo
ispisom kernel logova pomoću `dmesg` komande.

```
# dmesg | grep eth
[    1.462860] socfpga-dwmac ff702000.ethernet: IRQ eth_wake_irq not found
[    1.469491] socfpga-dwmac ff702000.ethernet: IRQ eth_lpi not found
[    1.475781] socfpga-dwmac ff702000.ethernet: PTP uses main clock
[    1.482030] socfpga-dwmac ff702000.ethernet: Version ID not available
[    1.488476] socfpga-dwmac ff702000.ethernet:         DWMAC1000
[    1.493687] socfpga-dwmac ff702000.ethernet: DMA HW capability register supported
[    1.501150] socfpga-dwmac ff702000.ethernet: RX Checksum Offload Engine supported
[    1.508619] socfpga-dwmac ff702000.ethernet: COE Type 2
[    1.513827] socfpga-dwmac ff702000.ethernet: TX Checksum insertion supported
[    1.520857] socfpga-dwmac ff702000.ethernet: Enhanced/Alternate descriptors
[    1.527808] socfpga-dwmac ff702000.ethernet: Extended descriptors not supported
[    1.535089] socfpga-dwmac ff702000.ethernet: Ring mode enabled
[    1.540930] socfpga-dwmac ff702000.ethernet: device MAC address f6:ab:b5:cc:a0:0c
[    1.548396] socfpga-dwmac ff702000.ethernet: TX COE limited to 0 tx queues
[    5.738736] socfpga-dwmac ff702000.ethernet end0: renamed from eth0
```

Vidimo da je uređaj prepoznat i konfigurisan. Postavlja se pitanje zašto se ne pojavljuje u listi mrežnih
interfejsa. Razlog je u tome što `systemd` nije pronašao odgovarajući konfiguracioni fajl za mrežni interfejs.
U nastavku vježbe ćemo izmijeniti sliku *root* fajl sistema i uključiti ovaj konfiguracioni fajl.

Obratite pažnju na posljednju liniju prethodnog ispisa. Iz ove linije da je podrazumijevano ime mrežnog interfejsa
`eth0` zamijenjeno nazivom `end0`. U pomenutom konfiguracionom fajlu za konfigurisanje mrežnog interfejsa trebamo
da koristimo ovo ime.

> [!TIP]
> Mrežni interfejs možete konfigurisati komandom `ifconfig`, tako što specificirate IP adresu i mrežnu masku interfejsa,
npr. `ifconfig end0 192.168.21.100 netmask 255.255.255.0`. Međutim, pri restartovanju sistema mrežni interfejs
opet neće biti konfigurisan i svaki put ćete morati da pokrenete ovu komandu.

Konačno, ako sve radi kako treba, sačuvajte našu prilagođenu konfiguraciju kao predefinisanu pod nazivom
`terasic_de1soc_cyclone5_defconfig`. Ukoliko ste specificirali putanju pri konfiguraciji *build* sistema
(opcija **Location to save buildroot config**), dovoljno je da pokrenete komandu

```
make savedefconfig
```

i ona će biti sačuvana na definisanoj lokaciji u specificiranom fajlu. Ako ste prilikom konfiguracije ostavili
prazno polje, onda možete da definišete putanju i naziv konfiguracionog fajla u sklopu `BR2_DEFCONFIG` varijable:

```
make savedefconfig BR2_DEFCONFIG=configs/terasic_de1soc_cyclone5_defconfig
```

Nakon što ste sačuvali predefinisanu konfiguraciju, po potrebi možete da je aktivirate komandom:

```
make terasic_de1soc_cyclone5_defconfig
```

Na kraju, predajte sve prethodno kreirane fajlove na granu u sklopu repozitorijuma kursa. Prvo napravite folder
pod nazivom *buildroot* unutar foldera `lab-08`, a zatim u ovaj folderu prekopirajte sve relevantne fajlove
prateći organizaciju kao što je u izvornom `buildroot` folderu.

### Kreiranje *overlay*-a za *root* fajl sistem

Vidjeli smo da mrežni interfejs nije konfigurisan i da ga, prema tome, ne možemo koristiti za komunikaciju.
Jedan način je da ga konfigurišemo `ifconfig` komandom, međutim, ovo nije trajno rješenje. Bolji način je da
koristimo infrastrukturu `systemd` programa za incijalizaciju i da kreiramo odgovarajući fajl u kojem je
opisana konfiguracija mrežnog interfejsa. Ovaj fajl treba da se nalazi u folderu `etc/systemd/network` kako
bi program mogao da ga učita i interpretira pri pokretanju sistema. Prema tome, mogli bismo da kreiramo
fajl direktno na ciljnoj platformi, a onda da restartujemo mrežne servise. Međutim, na taj način nismo napravili
trajno rješenje, jer kreiranje nove SD kartice zahtijeva da se ponovi ovaj postupak. Bolji pristup je da
napravimo da ovaj fajl bude dio *root* fajl sistema unutar *Buildroot* infrastrukture, što se može postići
tzv. *overlay*-om. Naime, *Buildroot* dozvoljava dodavanje proizvoljnih fajlova (novih ili redefinisanje
postojećih) unutar *root* fajl sistema. Sada ćemo pokazati kako to može da se uradi.

Prvo kreiramo folder pod nazivom `rootfs-overlay` (možemo da damo i drugačiji, proizvoljan naziv), koji ćemo
smjestiti u okviru foldera naše ploče, zajedno sa prethodnim fajlovima. Nakon toga, u *overlay* folderu
kreiramo strukturu foldera koja odgovara onoj na *root* fajl sistemu, a u koje želimo da smjestimo naše
fajlove. U datom slučaju, to je folder `etc/systemd/network`. Konačno, kreiramo konfiguracioni fajl
`70-static.network` koji treba da sadrži informacije koje omogućavaju da `systemd` infrastruktura ispravno
konfiguriše mrežni interfejs. Sve prethodno je sumiranu u sljedećoj sekvenci komandi.

```
cd <buildroot-dir>
mkdir -p board/terasic/de1soc_cyclone5/rootfs-overlay
cd board/terasic/de1soc_cyclone5/rootfs-overlay/
mkdir -p etc/systemd/network
touch etc/systemd/network/70-static.network
```

U okviru kreiranog fajla definišemo parametre za konfiguraciju interfejsa.

```
[Match]
Name=end0

[Network]
Address=192.168.21.100/24
Gateway=192.168.21.1
```

U sklopu `[Match]` sekcije, referenciramo mrežni interfejs (`end0`), dok `[Network]` sekcija sadrži
relevantne parametre interfejsa (IP adresa, mrežna maska i *gateway* adresa).

> [!NOTE]
> Više informacija o konfiguracionim fajlovima i `systemd` infrastrukturi možete pronaći na sljedećoj
[web stranici](https://wiki.archlinux.org/title/Systemd).

Sada je potrebno da opet pokrenete `make menuconfig` komandu i postavite putanju do prethodno kreiranog
*overlay*-a u sklopu **System configuration**. U tom smislu postavite opciju **Root filesystem overlay directories** na
**board/terasic/de1soc_cyclone5/rootfs-overlay**.

Sačuvajte ovako izmijenjenu konfiguraciju i pokrenite komandu `make`. Po završetku generisanja, kopirajte novu
sliku na SD karticu i pokrenite sistem na DE1-SoC ploči.

Nakon logovanja, izlistajte interfejse komandom `ifconfig` i trebalo bi da se pojavi fizički mrežni interfejs čime
potvrđujemo da je konfiguracioni fajl ispravan.

```
# ifconfig
end0      Link encap:Ethernet  HWaddr F2:F3:04:61:A0:61
          inet addr:192.168.21.100  Bcast:192.168.21.255  Mask:255.255.255.0
          inet6 addr: fe80::f0f3:4ff:fe61:a061/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:33 errors:0 dropped:0 overruns:0 frame:0
          TX packets:17 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000
          RX bytes:2656 (2.5 KiB)  TX bytes:1344 (1.3 KiB)
          Interrupt:32

lo        Link encap:Local Loopback
          inet addr:127.0.0.1  Mask:255.0.0.0
          inet6 addr: ::1/128 Scope:Host
          UP LOOPBACK RUNNING  MTU:65536  Metric:1
          RX packets:80 errors:0 dropped:0 overruns:0 frame:0
          TX packets:80 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000
          RX bytes:6080 (5.9 KiB)  TX bytes:6080 (5.9 KiB)
```

> [!WARNING]
> Da bi se u interfejsu prikazala konfigurisana IP adresa, link mora da bude uspostavljen, tj. DE1-SoC ploča
mora da bude povezana mrežnim kablom sa razvojnim računarom.

Sačuvajte prethodne izmjene na granu u okviru repozitorijuma kursa kao što je ranije opisano.

### Dodavanje novog softverskog paketa u *Buildroot*

Na prethodno opisani način možemo da dodajemo korisničke konfiguracione fajlove, pa čak i prekompajlirane
programe (praktično, bilo koji fajl koji želimo da bude dio *root* fajl sistema). Međutim, nekada želimo
da dodamo novi softverski paket u *build* sistem. U nastavku ćemo ilustrovati dodavanje novog paketa u
*Buildroot* na primjeru [*CANopenLinux*](https://github.com/CANopenNode/CANopenLinux) softvera koji je dio
[*CANopenNode*](https://github.com/CANopenNode/CANopenNode) projekta (implementacija *CANopen* protokola na *Linux*
infrastrukturi).

Prvo ćemo kreirati relevantne foldere za novi softvrski paket unutar `package` foldera.

```
mkdir -p package/canopen
touch package/canopen/Config.in
touch package/canopen/canopen.mk
```

U fajlu `Config.in` specificiramo opcije koje će biti vidljive pri *Buildroot* konfiguraciji. Minimalno,
potrebno je definisati varijablu `BR_PACKAGE_<PACKAGE-NAME>` sa opcijom za selekciju paketa, njegov opis
u sklopu `help` sekcije sa linkom ka izvornom kodu softvera. U tom smislu, otvorite ovaj fajl i dodajte
sljedeće linije:

```
config BR2_PACKAGE_CANOPEN
	bool "canopen"
	help
	  This software is CANopenNode implementation on
	  Linux devices

	  https://github.com/CANopenNode/CANopenLinux
```

Da bi softverski paket bio vidljiv u konfiguraciji, potrebno je da dodate liniju

```
source "package/canopen/Config.in"
```

u krovni `Config.in` fajl koji se nalazi u `package` folderu. Ovu liniju dodajte na kraj menija pod
nazivom *Networking applications* (`menu "Networking applications"`).

Fajl `canopen.mk` definiše način na koji *Builroot* preuzima i kompajlira softverski paket, kao i komande
za instalaciju generisanih artifakata. Otvorite ovaj fajl u bilo kojem editoru teksta i prvo definišite
sljedeće varijable:

```
CANOPEN_VERSION = v4.0
CANOPEN_SITE = https://github.com/CANopenNode/CANopenLinux
CANOPEN_LICENSE = Apache-2.0
CANOPEN_LICENSE_FILES = LICENSE
CANOPEN_SITE_METHOD=git
CANOPEN_GIT_SUBMODULES = YES
```

Prva varijabla definiše verziju softvera koja odgovara validnoj vrijednosti taga na *Git* repozitorijumu
(ukoliko softver preuzimamo korišćenjem `git` alata, što je naš slučaj). Variablom `CANOPEN_SITE`
specificiramo link ka repozitorijumu. U nastavku definišemo tip licence (`CANOPEN_LICENSE`) i naziv
fajla u kojem se nalazi tekst licence (`CANOPEN_LICENSE_FILES`). Konačno, s obzirom da koristimo `git`
za preuzimanje izvornog koda, potrebno je to da specificiramo unutar varijable `CANOPEN_SITE_METHOD`.
Takođe, pošto *CANopenLinux* projekat sadrži *CANopenNode* kao *Git submodule*, potrebno je da
omogućimo podršku za to u okviru varijable `CANOPEN_GIT_SUBMODULES`.

> [!NOTE]
> Iako ovdje koristimo *Git submodule* opciju, to nije uobičajeni način za definisanje paketa. Preporučuje
se da izbjegavamo *Git submodule* unutar paketa i da pakete od koji zavisi dati softverski paket definišemo
kao zasebne pakete unutar *Buildroot* infrastrukture i da specificiramo zavisnost između softverskih paketa.

Ostaje još da definišemo komande za kompajliranje softverskog paketa i za instalaciju generisanih artifakata
u *root* fajl sistem na ciljnoj platformi. Prvo ćemo definisati komandu za kompajliranje softverskog paketa.
S obzirom da *CANopenLinux* projekat koristi *Makefile* infrastrukturu za kompajliranje (što možemo da
zaključimo pregledanjem repozitorijuma), komandu definišemo na sljedeći način:

```
define CANOPEN_BUILD_CMDS
    $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D) all
endef
```

Vidimo da se prosto poziva `make` komanda (koristimo varijablu `$(MAKE)` koja je interno definisana u *Buildroot*
infrastrukturi), s tim da redefinišemo varijable `CC` i `LD` kojima specificiramo odgovorajući kompajler (`$(TARGET_CC)`)
i linker (`$(TARGET_LD)`) za ciljnu platformu. Na kraju moramo da specificiramo da se komanda `make` poziva Iz
odgovarajućeg foldera (`-C $(@D)`). Varijabla `$(@D)` sadrži putanju do direktorijuma u koji je ekstrahovan izvorni
kod softverskog paketa.

Konačno, potrebno je da definišemo i komandu za instalaciju generisanih artifakata. Ova komanda treba da ima Sljedeći
izgled u našem slučaju.

```
define CANOPEN_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0755 $(@D)/canopend $(TARGET_DIR)/usr/bin/canopend
endef
```

U osnovi, ovom komandom kopiramo generisani binarni fajl `canopend` na odgovarajuću lokaciju u *root* fajl sistem
ciljne platforme. Pri tome, definišemo odgovarajuće permisije za ovaj fajl.

> [!TIP]
> Sve prethodno korišćene varijable su dio *Buildroot* infrastrukture. Definicije i detalje o njima možete pronaći
u fajlu `package/Makefile.in`.

Na kraju, ostaje još da definišemo koja infrastruktura se koristi za generisanje artfifakata softvera. Kako se u našem
slučaju direktno koristi `make` alat, potrebno je da definišemo `generic-package`. U tom smislu, na kraju fajla dodajte
sljedeću liniju.

```
$(eval $(generic-package))
```

> [!NOTE]
> Detaljnije informacije o kreiranju generičkih paketa sa opisom svih podržanih opcija se može pronaći u
[*Buildroot* dokumentaciji](https://buildroot.org/downloads/manual/adding-packages-generic.txt). Za pakete koji
koriste drugu infrastrukturu (*Autotools* ili *CMake*), možete konsultovati [korisničko uputstvo](https://buildroot.org/downloads/manual/manual.html#adding-packages).

Nakon što smo dodali novi softverski paket, da bi se kompajlirao i pojavio u *root* fajl sistemu ciljne platforme,
potrebno je da ga uključimo u konfiguraciji. U tom smislu je potrebno pokrenuti konfiguracioni meni i uključiti
opciju **Target packages**&rarr;**Networking applications**&rarr;**canopen**.

Aktuelizujte sadržaj SD kartice i pokrenite DE1-SoC ploču. Pokretanjem komande `canopend` potvrdite da je softver
ispravno instaliran na ciljnoj platformi.

```
# canopend --help
Usage: canopend <CAN device name> [options]

Options:
  -i <Node ID>        CANopen Node-id (1..127) or 0xFF (LSS unconfigured).
  -r                  Enable reboot on CANopen NMT reset_node command.
  -s <storage path>   Path and filename prefix for data storage files.
                      By default files are stored in current dictionary.
  -c <interface>      Enable command interface for master functionality.
                      One of three types of interfaces can be specified as:
                   1. "stdio" - Standard IO of a program (terminal).
                   2. "local-<file path>" - Local socket interface on file
                      path, for example "local-/tmp/CO_command_socket".
                   3. "tcp-<port>" - Tcp socket interface on specified
                      port, for example "tcp-60000".
                      Note that this option may affect security of the CAN.
  -T <timeout_time>   If -c is specified as local or tcp socket, then this
                      parameter specifies socket timeout time in milliseconds.
                      Default is 0 - no timeout on established connection.

See also: https://github.com/CANopenNode/CANopenNode
```

Sačuvajte prethodne izmjene na granu u okviru repozitorijuma kursa kao što je ranije opisano.

### Zadatak za samostalnu izradu

Na osnovu prethodno stečenih znanja, proširite *root* fajl sistem na DE1-SoC ploči tako da omogućite pristup
ploči preko SSH protokola. U tom smislu je potrebno uključiti `dropbear` softverski paket u okviru *Buildroot*
konfiguracije kako bismo na raspolaganju imali SSH server.

> [!NOTE]
> *Dropbear* je implementacija SSH protokola pogodna za ugrađene sisteme. Više informacija o ovom softveru možete
pronaći na [web stranici](https://matt.ucc.asn.au/dropbear/dropbear.html).

Da biste mogli da se konektujete na ploču preko SSH servera, potrebno je da generišete javni i privatni ključ
(pomoću `ssh-keygen` alata kao kod SSH pristupa *GitHub* repozitorijumu ili drugim online resursima) ili da koristite
već generisane ključeve, a zatim da javni ključ smjestite na *root* fajl sistem u fajl `/root/.ssh/authorized_keys`.
Korišćenjem znanja o kreiranju *overlay*-a, smjestite javni ključ na *root* fajl sistem ciljne platforme kako biste
omogućili pristup preko SSH protokola.

> [!TIP]
> Ukoliko ne možete da ostvarite komunikaciju sa *Dropbear* SSH serverom čak i ako ste sve podesili kako je
opisano iznad, pokušajte na ciljnoj platformi da promijenite permisije komandama `chmod -R go-rwx /root` i
`chown -R root.root /root`.

Po završetku, sačuvajte sve izmjene na granu u sklopu repozitorijuma kursa kao što je to ranije opisano.

## Generisanje sistema korišćenjem *Yocto* alata

### Preuzimanje i inicijalna konfiguracija *Yocto* projekta

U ovom dijelu vježbe ćemo se upoznati sa *Yocto build* sistemom. Prvo je potrebno provjeriti da li su instalirani
svi softverski paketi neophodni za rad sa *Yocto* okruženjem.

```
sudo apt install gawk wget git diffstat unzip texinfo gcc build-essential chrpath socat cpio python3 python3-pip \
                 python3-pexpect xz-utils debianutils iputils-ping python3-git python3-jinja2 python3-subunit zstd \
                 liblz4-tool file locales libacl1
```

Nakon instalacije zahtjevanih softverskih paketa, kreiraćemo zaseban direktorijum pod nazivom `yocto` u koji ćemo
smjestiti svu neophodnu infrastrukturu.

```
mkdir yocto
cd yocto
```

Prvi korak je kloniranje referentne *Poky* distribucije.

```
git clone -b kirkstone git://git.yoctoproject.org/poky
```

> [!WARNING]
> Pri kloniranju bilo kojeg *Yocto* sloja, važno je da odaberemo istu granu. U našem slučaju ćemo koristiti LTS
granu `kirkstone`.

Sljedeći korak je preuzimanje BSP sloja koji obezbjeđuje podršku za *IntelFPGA* platforme kojoj pripada i
*Cyclone V* koji se nalazi na DE1-SoC ploči.

```
git clone -b kirkstone git://git.yoctoproject.org/meta-intel-fpga
```

Prethodno preuzeti BSP sloj (`meta-intel-fpga`) je potrebno dodati u *Yocto* infrastrukturu, a da bismo mogli to
da uradimo, potrebno je prvo da kreiramo okruženje koje nam omogućava pristup `bitbake` komandama.

```
source poky/oe-init-build-env build-socfpga
```

Prethodna komanda kreira folder pod nazivom `build-socfpga` u kojem će se nalaziti svi artifakti dobijeni nakon
što se *build* proces završi (kao i dodatna infrastruktura neophodna za rad sa *Yocto* okruženjem). Takođe, ova
komanda nas automatski pozicionira u ovaj folder.

> [!NOTE]
> Prethodnu komandu je potrebno pokrenuti svaki put kada korisnik želi da radi sa *Yocto* okruženjem, odnosno pri
otvaranju novog terminala.

Sada možemo komandom `bitbake-layers` da dodamo `meta-intel-fpga` BSP sloj.

```
bitbake-layers add-layer ../meta-intel-fpga
```

Sljedećom komandom možemo da potvrdimo da je naš sloj dodan u listu registrovanih *Yocto* slojeva.

```
bitbake-layers show-layers
```

Nakon izvršavanja komande, trebalo bi da dobijemo sljedeći listing:

```
NOTE: Starting bitbake server...
layer                 path                                      priority
==========================================================================
meta                  /home/mknezic/yocto/poky/meta             5
meta-poky             /home/mknezic/yocto/poky/meta-poky        5
meta-yocto-bsp        /home/mknezic/yocto/poky/meta-yocto-bsp   5
meta-intel-fpga       /home/mknezic/yocto/meta-intel-fpga       6
```

Prije nego što započnemo *build* proces, moramo da definišemo nekoliko stvari unutar lokalne konfiguracije
(fajl `conf/local.conf`). Prije svega, treba da selektujemo platformu za koju želimo da generišemo artifakte
(u našem slučaju, to je `cyclone5`). Osim toga, tipično ćemo odabrati željenu verziju kernela i *U-Boot*
konfiguraciju. Jedan način je da otvorimo fajl `conf/local.conf` bilo kojim tekstualnim editorom i definišemo
neophodne varijable. Međutim, to možemo da učinimo i prostim dodavanjem sadržaja ovih varijabli na kraj fajla,
kao što je prikazano sljedećim komandama:

```
echo "MACHINE = \"cyclone5\"" >> conf/local.conf
echo "PREFERRED_PROVIDER_virtual/kernel = \"linux-socfpga-lts\"" >> conf/local.conf
echo "PREFERRED_VERSION_linux-socfpga-lts = \"5.15%\"" >> conf/local.conf
echo "UBOOT_CONFIG = \"de0-nano-soc\"" >> conf/local.conf
```

Ako sada izlistamo sadržaj lokalne konfiguracije

```
cat conf/local.conf
```

na kraju fajla bi trebalo da dobijemo sljedeći sadržaj:

```
[...]
MACHINE = "cyclone5"
PREFERRED_PROVIDER_virtual/kernel = "linux-socfpga-lts"
PREFERRED_VERSION_linux-socfpga-lts = "5.15%"
UBOOT_CONFIG = "de0-nano-soc"
```

Prethodnim varijablama definišemo platformu (varijabla `MACHINE`), recept koji želimo da koristimo za kernel
(varijabla `PREFERRED_PROVIDER_virtual/kernel`) i verziju kernela za taj recept (varijabla `PREFERRED_VERSION_linux-socfpga-lts`),
te *U-Boot* konfiguraciju (varijabla `UBOOT_CONFIG`).

Sada je sve spremno za generisanje artifakata. Odabraćemo sliku sa minimalnim skupom programa koji su neophodni za pokretanje
sistema (`core-image-minimal`).

```
bitbake core-image-minimal
```

> [!NOTE]
> Kompletno *Yocto* okruženje zahtjeva oko 45GB prostora na disku, a za generisanje svih artifakata (uključujući
*toolchain*) potrebno je nešto više od 4 sata (testirano na virtuelnoj mašini sa dva jezgra procesora i 3GB radne memorije).

Po završetku *build* procesa, u okviru foldera `tmp/deploy/images/cyclone5` možemo pronaći relevantne slike koje su pogodne
za kopiranje na SD karticu. U našem slučaju, svi fajlovi su zapakovani u okviru `core-image-minimal-cyclone5.wic` fajla čiji
sadržaj možemo komandom `dd` direktno prebaciti na SD karticu.

```
cd tmp/deploy/images/cyclone5
sudo dd if=core-image-minimal-cyclone5.wic of=/dev/sdb bs=1M
```

Nakon prebacivanja sadržaja na SD karticu, trebamo da je ubacimo u SD slot na ploči i da pokrenemo sistem. Dobićemo sljedeći
ispis.

```
U-Boot SPL 2022.04 (Nov 10 2022 - 03:07:42 +0000)
Trying to boot from MMC1


U-Boot 2022.04 (Nov 10 2022 - 03:07:42 +0000)

CPU:   Altera SoCFPGA Platform
FPGA:  Altera Cyclone V, SE/A5 or SX/C5 or ST/D5, version 0x0
BOOT:  SD/MMC Internal Transceiver (3.0V)
       Watchdog enabled
DRAM:  1 GiB
Core:  22 devices, 13 uclasses, devicetree: separate
MMC:   dwmmc0@ff704000: 0
Loading Environment from MMC... *** Warning - bad CRC, using default environment

In:    serial
Out:   serial
Err:   serial
Model: Terasic DE-0(Atlas)
Net:
Error: ethernet@ff702000 address not set.
No ethernet found.

=>
```

S obzirom da u konfiguraciji nije definisana varijabla `bootcmd` u okviru *U-Boot* okruženja, proces podizanja *Linux*-a
pokrećemo tako da eksplicitno pokrenemo skriptu `distro_bootcmd` koja će automatski pronaći i selektovati odgovarajući
način podizanja (u datom slučaju SD kartica) i pokrenuti proces podizanja sistema.

```
run distro_bootcmd
```

Konačno, kada se pojavi poruka za logovanje, korisnik se može ulogovati sa `root` korisničkim imenom (nije potrebno unositi
korisničku šifru).

### Kreiranje korisničkog sloja i izmjena postojećih recepata

Da bismo uključili podršku za DE1-SoC ploÄču, koristićemo fajlove koje smo kreirali u prethodnim vježbama (predefinisana
konfiguracija kernela i DTS fajl). Prvo ćemo napraviti izmjene u samoj konfiguraciji mašine za *Cyclone V* platformu,
koja se nalazi u fajlu `meta-intel-fpga/conf/machine/cyclone5.conf`. U tom smislu, otvorite ovaj fajl u tekstualnom
editoru i dodajte sljedeću liniju:

```
UBOOT_CONFIG[de1-soc] = "socfpga_de1_soc_defconfig"
```

Dio konfiguracionog fajla mašine treba da ima izgled

```
UBOOT_CONFIG ??= "cyclone5-socdk"
UBOOT_CONFIG[cyclone5-socdk] = "socfpga_cyclone5_defconfig"
UBOOT_CONFIG[de0-nano-soc] = "socfpga_de0_nano_soc_defconfig"
UBOOT_CONFIG[de10-nano-soc] = "socfpga_de10_nano_defconfig"
UBOOT_CONFIG[mcvevk] = "socfpga_mcvevk_defconfig"
UBOOT_CONFIG[sockit] = "socfpga_sockit_defconfig"
UBOOT_CONFIG[socrates] = "socfpga_socrates_defconfig"
UBOOT_CONFIG[sr1500] = "socfpga_sr1500_defconfig"
UBOOT_CONFIG[de1-soc] = "socfpga_de1_soc_defconfig"
```

Takođe, u istom fajlu dopunite sadržaj varijable `KERNEL_DEVICETREE` sa

```
socfpga_cyclone5_de1_soc.dtb \
```

tako da ova varijabla ima sljedeći izgled:

```
KERNEL_DEVICETREE ?= "\
			socfpga_cyclone5_socdk.dtb \
			socfpga_cyclone5_sockit.dtb \
			socfpga_cyclone5_socrates.dtb \
			socfpga_cyclone5_de0_nano_soc.dtb \
			socfpga_cyclone5_mcvevk.dtb \
			socfpga_cyclone5_sodia.dtb \
			socfpga_cyclone5_vining_fpga.dtb \
			socfpga_cyclone5_de1_soc.dtb \
			"
```

Sada ćemo kreirati korisnički sloj za *DE1-SoC* ploču koji ćemo iskoristiti da izmjenimo recepte kernela s ciljem dodavanja
prethodno pomenutih fajlova.

Pređite u folder `build-socfpga` i kreirajte novi sloj pod nazivom `meta-de1soc`.

```
bitbake-layers create-layer -p 7 ../meta-de1soc
```

> [!NOTE]
> Napominjemo da svaki put kada koristite *BitBake* komande, potrebno je da prethodno kreirate *Yocto* okruženje korišćenjem
komande `source poky/oe-init-build-env build-socfpga` u našem slučaju.

Ova komanda će kreirati inicijalnu infrastrukturu sa oglednim receptom `recipes-example` koji služi kao primjer za kreiranje
korisničkih recepata. Ovaj primjer nam neće biti potreban pa možete obrisati kompletan `recipes-example` folder.

U prethodno kreiranom sloju, napravićemo repecte koji su nam neophodni za adekvatnu konfiguraciju kernela.

```
cd ../meta-de1soc
mkdir -p recipes-kernel/linux/files
```

Kopirajte kernel konfiguraciju i DTS fajl za *DE1-SoC* ploču u folder `recipes-kernel/linux/files`. Zatim, kreirajte fajl za
proširenje postojećeg recepta za *Linux*.

```
touch recipes-kernel/linux/linux-socfpga-lts_%.bbappend
```

Otvorite ovaj fajl i u njega dodajte sljedeći sadržaj:

```
FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

do_kernel_metadata:prepend(){
 cp ${WORKDIR}/de1_soc_defconfig ${S}/arch/arm/configs
 cp ${WORKDIR}/socfpga_cyclone5_de1_soc.dts ${S}/arch/arm/boot/dts
 echo "dtb-\$(CONFIG_ARCH_INTEL_SOCFPGA) += socfpga_cyclone5_de1_soc.dtb" >> ${S}/arch/arm/boot/dts/Makefile
}

KBUILD_DEFCONFIG = "de1_soc_defconfig"
```

Prva linija omogućava da prethodno kreirani `files` folder bude vidljiv u radnom direktorijumu okruženja. Narednih nekoliko linija
realizuje kopiranje konfiguracije kernela i DTS fajla na odgovarajuće lokacija prije same konfiguracije i kompajliranja kernela.
Osim toga, modifikujemo `Makefile` tako da omogućimo *build*-anje našeg DTS fajla. Konačno, posljednjom linijom redefinišemo
varijablu `KBUILD_DEFCONFIG` kojom se selektuje predefinisana konfiguracija kernela.

Struktura foldera našeg sloja bi trebalo da ima sljedeći izgled nakon prethodnih izmjena.

```
meta-de1soc/
├── conf
│   └── layer.conf
├── COPYING.MIT
├── README
└── recipes-kernel
    └── linux
        ├── files
        │   ├── de1_soc_defconfig
        │   └── socfpga_cyclone5_de1_soc.dts
        └── linux-socfpga-lts_%.bbappend
```

Da bi ovaj sloj bio vidljiv u *build* sistemu, potrebno je da ga dodamo kao što je ranije opisano.

```
cd ../build-socfpga
bitbake-layers add-layer ../meta-de1soc
```

Konačno, da bi *root* fajl sistem sadržao i kernel module, potrebno je da u lokalnoj konfiguraciji (fajl `conf/local.conf`)
dodamo sljedeću liniju:

```
MACHINE_ESSENTIAL_EXTRA_RDEPENDS += "kernel-modules"
```

Ostaje još da pokrenemo generisanje slike

```
bitbake core-image-minimal
```

a zatim da aktuelizujemo SD karticu sa novom slikom i pokrenemo sistem na ploči.

Skrećemo pažnju da je u lokalnoj konfiguraciji i dalje selektovana *DE0-Nano-SoC* ploča. Iako smo omogućili konfiguraciju i za
*DE1-SoC* ploču koja je podržana u *U-Boot* izvornom kodu, postoje određeni problemi sa korišćenom *U-Boot* verzijom, tako da
ćemo za inicijalno podizanje ploče koristiti *DE0-Nano-SoC* konfiguraciju. Međutim, da bi u nastavku bio učitan odgovarajući
*Device Tree* fajl, potrebno je prije podizanja *Linux*-a izmijeniti `fdtfile` varijablu *U-Boot* okruženja. Npr. možemo koristiti
`editenv` komandu da postavimo vrijednost `fdtfile` varijable na `socfpga_cyclone5_de1_soc.dtb`. Nakon toga, pokrećemo proces
podizanja sistema kao i ranije, komandom `run distro_bootcmd`.

Kada se sistem podigne, ulogujte se i provjerite da li su drajveri za ADXL345 uređaj učitani kako treba i potvrdite da možete da
pristupite svim uređajima koje smo pominjali tokom vježbi preko *sysfs* fajl sistema.

### Korišćenje `recipetool` alata za proširenje postojećih recepata

U prethodnoj sekciji smo vidjeli kako možemo ručno da proširimo i izmjenimo postojeći recept. Međutim, u okviru *Yocto* infrastrukture
na raspolaganju imamo i alate koji mogu da automatizuju i olakšaju ovaj proces. Jedan takav alat je `recipetool`. Iskoristićemo ovaj
alat da prilagodimo mrežnu konfiguraciju i postavimo željenu IP adresu za mrežni interfejs.

S obzirom da *Yocto* podrazumijevano koristi *System V* inicijalizacioni sistem, kod kojeg se mrežna konfiguracija nalazi u fajlu
`/etc/network/interfaces`, prvo ćemo kreirati ovaj fajl sa željenom konfiguracijom.

Pretpostavićemo da se nalazimo u folderu `build-socfpga`.

```
touch ../myinterfaces
```

Otvorite ovaj fajl u editoru i dodajte sljedeći sadržaj:

```
# The loopback interface
auto lo
iface lo inet loopback

# Wired interface
auto eth0
iface eth0 inet static
	address 192.168.21.100
	netmask 255.255.255.0
	network 192.168.21.0
	gateway 192.168.21.1
```

Ova konfiguracija podešava *loopback* interfejs i mrežni interfejs sa statičkom adresom.

Da bismo ovaj fajl dodali u postojeći recept za mrežnu konfiguraciju, koji je sastavni dio *Poky* distribucije, najjednostavnije je
koristiti sljedeću komandu:

```
recipetool appendfile ../meta-de1soc /etc/network/interfaces ../myinterfaces
```

Komanda `appendfile` će automatski da pronađe recept koji je zadužen za generisanje fajla `/etc/network/interfaces` u *root* fajl
sistemu, a zatim generisati sve neophodne fajlove za proširenje recepta. Komanda takođe očekuje da specificiramo sloj u koji će
ovo proširenje recepta biti sačuvano (`../meta-de1soc`), kao i fajl koji želimo dodati (`../myinterfaces`).

> [!NOTE]
> Alatka `recipetool` podržava i druge varijante izmjena, kao što su `appendsrcfile` i `appendsrcfiles`. Za više informacija možete
da pokrenete `recipetool --help` komandu.

Nakon pokretanja ove komande, trebalo bi da dobijete sljedeću strukturu foldera našeg sloja:

```
meta-de1soc/
├── conf
│   └── layer.conf
├── COPYING.MIT
├── README
├── recipes-core
│   └── init-ifupdown
│       ├── init-ifupdown
│       │   └── interfaces
│       └── init-ifupdown_1.0.bbappend
└── recipes-kernel
    └── linux
        ├── files
        │   ├── de1_soc_defconfig
        │   └── socfpga_cyclone5_de1_soc.dts
        └── linux-socfpga-lts_%.bbappend
```

Odavde zaključujemo da recept zadužen za konfigurisanje mrežnog interfejsa ima naziv `init-ifupdown` i da se nalazi u `recipes-core`.

Nakon postavljanja parametara mrežnog interfejsa, ostaje još da uključimo softverski paket `dropbear` koji će nam omogućiti pristup ploči
preko SSH protokla, koji je neophodan za sljedeći dio vježbe. Ovaj softverski paket nije dio `core-image-minimal` slike, pa ga moramo
uključiti kroz lokalnu konfiguraciju. U tom smislu, u fajl lokalne konfiguracije, dodajte sljedeću liniju:

```
IMAGE_INSTALL:append = " dropbear"
```

Sada možete ponovo pokrenuti *build* da bi dobili novu sliku SD kartice.

```
bitbake core-image-minimal
```

Ponovo pokrenite sistem na ploči i potvrdite da ploči možete pristupiti preko SSH protokola.

### Kreiranje novog recepta pomoću `devtool` alata

Kao i `recipetool`, alatka `devtool` nam služi za lakšu manipulaciju receptima. Međutim, ona posjeduje dodatne opcije koje nam olakšavaju
postupak dodavanja novih softverskih paketa. Način korišćenja `devtool` alatke ćemo ilustrovati primjerom dodavanja *CANopenLinux* softverskog
paketa.

Prvo ćemo se pozicionirati u `yocto` folder i preuzeti izvorni kod sa *CANopenLinux* repozitorijuma. Uz pretpostavku da se nalazimo u `build-socfpga`
folderu, potrebno je da pokrenemo sljedeću sekvencu komandi:

```
cd ..
git clone --recurse-submodules https://github.com/CANopenNode/CANopenLinux
cd CANopenLinux
git checkout v4.0
cd ..
```

Nakon toga, kreiraćemo lokalno radno okruženje pomoću `devtool` alatke koje nam služi za kreiranje potrebne infrastrukture i testiranje ispravnosti
opisa recepta.

```
devtool create-workspace local-workspace
```

Na ovaj način smo kreirali radno okruženje pod nazivom `local-workspace` u istoimenom folderu. Ako pokrenete komandu `bitbake-layers show-layers`
vidjećete da je ovo lokalno okruženje automatski dodano u *Yocto* sistem.

```
layer                 path                                      priority
==========================================================================
meta                  /home/mknezic/yocto/poky/meta             5
meta-poky             /home/mknezic/yocto/poky/meta-poky        5
meta-yocto-bsp        /home/mknezic/yocto/poky/meta-yocto-bsp   5
meta-de1soc           /home/mknezic/yocto/meta-de1soc           7
local-workspace       /home/mknezic/yocto/local-workspace       99
meta-intel-fpga       /home/mknezic/yocto/meta-intel-fpga       6
```

To nam omogućava testiranje svih izmjena tokom rada sa `devtool` alatom.

Novi recept dodajemo komandom

```
devtool add canopen ./CANopenLinux/
```

u okviru koje definišemo ime recept i lokaciju do izvornog koda. Provjerom pomoću komande

```
tree local-workspace
```

potvrđujemo da je kreiran novi recept u lokalnom okruženju.

```
local-workspace
├── appends
│   └── canopen_git.bbappend
├── conf
│   └── layer.conf
├── README
└── recipes
    └── canopen
        └── canopen_git.bb
```

Inicijalni sadržaj ovog recepta možemo izlistati komandom

```
cat local-workspace/recipes/canopen/canopen_git.bb
```

U našem slučaju dobijamo:

```
# Recipe created by recipetool
# This is the basis of a recipe and may need further editing in order to be fully functional.
# (Feel free to remove these comments when editing.)

# WARNING: the following LICENSE and LIC_FILES_CHKSUM values are best guesses - it is
# your responsibility to verify that the values are complete and correct.
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://CANopenNode/LICENSE;md5=3b83ef96387f14655fc854ddc3c6bd57 \
                    file://LICENSE;md5=3b83ef96387f14655fc854ddc3c6bd57"

SRC_URI = "git://github.com/CANopenNode/CANopenLinux;protocol=https;branch=master"

# Modify these as desired
PV = "1.0+git${SRCPV}"
SRCREV = "87cfbac52bc67809be20ca03ef3533293e7f7c3a"

S = "${WORKDIR}/git"

# NOTE: this is a Makefile-only piece of software, so we cannot generate much of the
# recipe automatically - you will need to examine the Makefile yourself and ensure
# that the appropriate arguments are passed in.

do_configure () {
	# Specify any needed configure commands here
	:
}

do_compile () {
	# You will almost certainly need to add additional arguments here
	oe_runmake
}

do_install () {
	# NOTE: unable to determine what to put here - there is a Makefile but no
	# target named "install", so you will need to define this yourself
	:
}
```

Jasno je da određen stvari trebamo prilagoditi u samom receptu. U tom smislu otvorite fajl `canopen_git.bb` u
editoru i napravite sljedeće izmjene:

1. potpuno obrišite funkciju `do_configure ()`,
2. zadržite funkciju `do_compile ()` kakva jeste,
3. u funkciji `do_install ()` dodajte liniju

```
install -D -m 0755 ${B}/canopend ${D}/usr/bin/canopend
```

4. opciono, obrište komentare (linije koje počinju sa `#`).

Sada kompajlirajte *CANopenLinux* softverski paket u lokalnom okruženju

```
devtool build canopen
```

Ukoliko je kompajliranje prošlo bez greške, možete instalirati softver na ploču kako biste ga testirali (neophodno je
da imate SSH konekciju sa pločom).

```
devtool deploy-target canopen root@192.168.21.100
```

Povežite se sa pločom preko SSH protokola (ili serijske veze) i provjerite da li instalirani softver radi kako treba.

Kada završite sa testiranjem, opciono, možete da izbrišete instalirani softver sa ciljne platforme.

```
devtool undeploy-target canopen root@192.168.21.100
```

Ostaje još da završite proces kreiranja recepta `devtool` alatom. Međutim, `devtool` će se žaliti kako imate nekomitovane
izmjene u *CANopenLinux* repozitorijumu zbog prelaska na tag v4.0. Jedan način da se ovo prevaziđe jeste da se vratite na
*master* granu. Međutim, prije prelaska trebate da sačuvate *commit hash* vrijednost (`SRCREV` varijabla u samom receptu
ili vrijednost koju dobijete komandom `git log` u repozitorijumu).

```
cd CANopenLinux
git log (kopirajte git commit hash vrijednost)
git checkout master
cd ..
devtool finish canopen meta-de1soc/recipes-core
```

Nakon `devtool finish` komande, kreirani recept će biti prebačen u sloj na specificiranu lokaciju (`meta-de1soc/recipes-core`).
Ovo možemo potvrditi izlistavanjem sadržaja lokalnog radnog okruženja:

```
tree local-workspace
```

što daje

```
local-workspace
├── appends
├── conf
│   └── layer.conf
├── README
└── recipes
```

odnosno našeg sloja:

```
tree meta-de1soc
```

čime dobijamo sljdeću strukturu foldera:

```
meta-de1soc
├── conf
│   └── layer.conf
├── COPYING.MIT
├── README
├── recipes-core
│   ├── canopen
│   │   └── canopen_git.bb
│   └── init-ifupdown
│       ├── init-ifupdown
│       │   └── interfaces
│       └── init-ifupdown_1.0.bbappend
└── recipes-kernel
    └── linux
        ├── files
        │   ├── de1_soc_defconfig
        │   └── socfpga_cyclone5_de1_soc.dts
        └── linux-socfpga-lts_%.bbappend
```

Ako sada otvorimo `canopen_git.bb` u našem sloju, vidjećemo da je vrijednost varijable `SRCREV` promijenjena. To je zato
što smo se prebacili na *master* granu. Da bi sve bilo kako treba, umetnite *commit hash* vrijednost taga v4.0 koju ste
ranije kopirali.

Sada možemo obrisati folder `CANopenLinux`, jer nam nije više potreban.

```
rm -rf ../CANopenLinux
```

*CANopenLinux* softverski paket je dio *Yocto* infrastrukture, pa ćemo pokušati da ga *build*-amo.

```
cd build-socfpga/
bitbake -c clean canopen
bitbake canopen
```

Sistem bi trebalo da prijavi grešku sa licencom, kao što je prikazano ispod.

```
ERROR: canopen-1.0+gitAUTOINC+87cfbac52b-r0 do_populate_lic: QA Issue: canopen: LIC_FILES_CHKSUM points to an invalid file: /home/mknezic/yocto/build-socfpga/tmp/work/cortexa9t2hf-neon-poky-linux-gnueabi/canopen/1.0+gitAUTOINC+87cfbac52b-r0/git/CANopenNode/LICENSE [license-checksum]
ERROR: canopen-1.0+gitAUTOINC+87cfbac52b-r0 do_populate_lic: Fatal QA errors were found, failing task.
ERROR: Logfile of failure stored in: /home/mknezic/yocto/build-socfpga/tmp/work/cortexa9t2hf-neon-poky-linux-gnueabi/canopen/1.0+gitAUTOINC+87cfbac52b-r0/temp/log.do_populate_lic.5593
ERROR: Task (/home/mknezic/yocto/meta-de1soc/recipes-core/canopen/canopen_git.bb:do_populate_lic) failed with exit code '1'
```

Problem je u tome što *CANopenLinux* repozitorijum sadrži podmodule, koji nisu povučeni prilikom kloniranja repozitorijuma. moramo
da stavimo do znanja *Yocto* sistemu da treba da radi sa podmodulima. U tom smislu, potrebno je izmijeniti varijablu `SRC_URI` tako
da `git:/` zamijenimo sa `gitsm:/`.

Kada ponovo pokrenete *build* proces, ova greška bi trebalo da nestane. Međutim, pojavljuje se još jedna, koja ima izgled kao što je
dato ispod.

```
ERROR: canopen-1.0+gitAUTOINC+87cfbac52b-r0 do_package_qa: QA Issue: File /usr/bin/canopend in package canopen doesn't have GNU_HASH (didn't pass LDFLAGS?) [ldflags]
ERROR: canopen-1.0+gitAUTOINC+87cfbac52b-r0 do_package_qa: Fatal QA errors were found, failing task.
ERROR: Logfile of failure stored in: /home/mknezic/yocto/build-socfpga/tmp/work/cortexa9t2hf-neon-poky-linux-gnueabi/canopen/1.0+gitAUTOINC+87cfbac52b-r0/temp/log.do_package_qa.6291
ERROR: Task (/home/mknezic/yocto/meta-de1soc/recipes-core/canopen/canopen_git.bb:do_package_qa) failed with exit code '1'
```

*Yocto* sistem zahtjeva da se dodaju `LDFLAGS` pri procesu kompajliranja. Ovo popravljamo dodavanjem sljedeće linije u recept
softverskog paketa:

```
TARGET_CC_ARCH += "${LDFLAGS}"
```

Sada bi trebalo da možete da *build*-ate softverski paket bez problema.

Ostaje još da uklonimo lokalno radno okruženje iz liste slojeva

```
bitbake-layers remove-layer local-workspace
```

i da dodamo naš novi paket u listu dodatnih paketa, tj. da izmjenimo varijablu `IMAGE_INSTALL` u lokalnoj konfiguraciji na
sljedeći način:

```
IMAGE_INSTALL:append = " dropbear canopen"
```

Konačno, možemo da kreiramo novu sliku

```
bitbake core-image-minimal
```

i da je deponujemo na SD karticu za testiranje na ploči.

### Zadatak za samostalnu izradu

Potrebno je da integrišete i dodate u *Yocto* sistem softverski paket za testiranje ulaznih uređaja `evtest`.
Recept za ovaj softverski paket je dio [`meta-openembedded`](git://git.openembedded.org/meta-openembedded) repozitorijuma.
Recept se nalazi u sloju `meta-oe` u `recipes-test` grupi recepata.
