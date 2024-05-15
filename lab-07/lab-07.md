# Laboratorijska vježba 7

Cilj laboratorijske vježbe je da se studenti upoznaju sa pristupima koji omogućavaju
upravljanje uređajima koji su dio ugrađenog sistema korišćenjem infrastrukture
*Linux* operativnog sistema.

Nakon uspješno realizovane vježbe studenti će biti sposobni da:
1. napišu jednostavan drajver uređaja koji upravlja uređajem direktno
iz korisničkog prostora,
2. omoguće u *Linux* konfiguraciji relevantne drajvere za upravljanje uređajima
kao kernel module i kontrolišu uređaje iz korisničkog prostora preko ovih drajvera i
3. prilagode *device tree* za ciljnu platformu nakon dodavanja novih uređaja.

## Preduslovi za izradu vježbe

Za uspješno izvođenje laboratorijske vježbe potrebno je sljedeće:

- razvojna ploča *DE1-SoC* sa pripadajućim napajanjem i kablom za povezivanje računara sa
UART interfejsom na ploči,
- preformatirana SD kartica sa instaliranim *bootloader*-om,
- *Ethernet* mrežni interfejs na razvojnoj platformi i mrežni kabl za povezivanje sa
pločom,
- konfigurisan *U-Boot* za automatsko učitavanje kernela i *device tree* fajla preko TFTP protokola i
- pripremljen NFS *root* fajl sistem.

## Upravljanje uređajima direktno iz korisničkog prostora

*Linux* kernel omogućava kontrolisanje uređaja privilegovanim korisnicima direktno
iz korisničkog prostora. Naravno, s obzirom da ovaj operativni sistem koristi
virtuelnu memoriju, nije moguće direktno koristiti fizičke adrese memorijski
mapiranih registara uređaja, već moramo da mapiramo ove adrese u virtuelni adresni
prostor naše aplikacije.

Prilikom montiranja *device* fajl sistema, jedan od virtuelnih fajlova, koji nam
omogućava pristup fizičkoj memoriji je `/dev/mem`. Upravo ovaj virtuelni fajl koristimo
da mapiramo adresni prostor fizičke memorije u virtuelni adresni prostor aplikacije,
što nam omogućava direktnu kontrolu uređaja iz korisničkog prostora. Mapiranje se
svodi na otvaranje pomenutog virtuelnog fajla

```
int fd = open("/dev/mem", (O_RDWR | O_SYNC));
```

nakon čega je potrebno da mapiramo relevantno područje fizičke memorije (opseg adresa
koje čine registre uređaja koji kontrolišemo) u virtuelni adresni prostor aplikacije
pomoću funkcije [`mmap()`](https://man7.org/linux/man-pages/man2/mmap.2.html).

Prototip ove funkcije ima sljedeći izgled:

```
void *mmap(void addr[.length], size_t length, int prot, int flags,
           int fd, off_t offset);
```

Prvi argument obično postavljamo na `NULL`, čime prepuštamo kernelu da odluči od koje
virtuelne adrese će biti mapirano dato područje. Argument `length` definiše veličinu
adresnog područja koje mapiramo (mora biti veće od 0). Nivo zaštite mapiranog memorijskog
područja se definiše argumentom `prot`, koji mora biti usklađen sa flegovima koje smo
koristili pri otvaranju fajla (npr. ako smo otvorili `/dev/mem` sa flegom `O_RDWR`, onda
ovdje specificiramo flegove `(PROT_READ | PROT_WRITE)`). Na raspolganju su flegovi
`PROT_EXEC`, `PROT_READ`, `PROT_WRITE` i `PROT_NONE`, pri čemu se više flegova može
kombinovati logičkim OR operatorom. Nakon nivoa zaštite mapiranog memorijskog područja
slijedi argument `flags` koji sadrži jedan ili više flegova koji specificiraju način
mapiranja i pristupa ovom području od strane drugih procesa. U našen slučaju, dovoljno
će biti da specificiramo `MAP_SHARED` u ovom argumentu. Konačno, preko argumenta `fd`
prosljeđujemo fajl deskriptor prethodno otvorenog virtuelnog uređaja (`/dev/mem`), a
`offset` sadrži pomjeraj relevantnog područja u fizičkoj memoriji u odnosu na početnu
adresu (najčešće, to je bazna adresa memorijskog područja u kojem se nalaze registri
uređaja).

Funkcija nam vraća pokazivač `void*`, tj. virtuelnu adresu koja je mapirana sa baznom
adresom uređaja u fizičkom adresnom prostoru. Kastovanjem na odgovarajući tip podatka
(koji odgovara veličini adresiranog registra) i dereferenciranjem pokazivača, možemo
da pristupimo željenom registru. Naravno, na ovu adresu je potrebno dodati pomjeraj
registra unutar memorijskog područja uređaja.

Kada završimo sa radom, potrebno je poništiti prethodno mapiranje pozivom funkcije
`munmap()` kojoj prosljeđujemo pokazivač koji nam je vratila funkcija `mmap()` i
veličinu mapiranog adresnog područja. Konačno, potrebno je zatvoriti otvoreni virtuelni
fajl.

U ovom dijelu vježbe je potrebno da modifikujete polazni skeleton programa `blinky.c`,
koji se nalazi u folderu `lab-07/blinky`, tako da obezbijedite naizmjeničnu promjenu
stanje LED diode na DE1-SoC ploči. U tom smislu, potrebno je prvo da proučite strukturu
datog programa, a zatim da dodate svoj kod na linijama sa `/* TODO : Add your code here */`
komentarom. Kroskompajlirajte ovaj program i kopirajte ga u `/home` direktorijum *root*
fajl sistema, a zatim testirajte na ploči. Realizovano rješenje predajte na granu u
okviru repozitorijuma kursa.

> [!TIP]
> Pomjeraj unutar memorijskog područja GPIO kontrolera za odgovarajuće registre možete
da pronađete u [Cyclone V registarskoj mapi](https://www.intel.com/content/www/us/en/programmable/hps/cyclone-v/hps.html).

Proširite prethodni program sa funkcionalnošću koja omogućava režim rada LED diode (blinkanje
uključeno ili isključeno) pomoću tastera HPS_KEY, slično kao što je rađeno za slučaj *baremetal*
aplikacije. S obzirom da se sada nalazite u okruženju sa operativnim sistemom koji podržava
višenitno izvršavanje, pokušajte da razdvojite čitanje stanja tastera i kontrolu rada LED
diode u dvije zasebne niti. U tu svrhu možete koristiti funkcije iz biblioteke `libpthread`
(POSIX *thread*). Kao pomoć možete koristiti [POSIX thread tutorijal](https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/www/pthreads.html)
za realizaciju zadatka. Novi program možete nazvati `blinky-button.c`.

> [!TIP]
> Nemojte zaboraviti da koristite *mutex* objekte za zaštitu resursa koje koristi više niti
(npr. globalne varijable).

Kao i u prethodnom slučaju, testirajte funkcionalnost programa na ploči i, kada budete zadovoljni,
predajte rješenje na granu u okviru repozitorijuma.

Iz prethodnih primjera smo vidjeli da uređaje možemo kontrolisati direktno iz korisničkog prostora.
Iako smo to ilustrovali na primjeru jednostavnog GPIO kontrolera, pristup se može generalizovati
za bilo koji uređaj. Međutim, ovaj način implementacije nije pogodan iz više razloga. Kao prvo,
da bismo kontrolisali uređaje, moramo da budemo logovani kao privilegovani korisnici, što nije
dobra ideja. Drugo, za kompleksnije uređaje, aplikacije brzo postaju veoma kompleksne, a kod
težak za održavanje. Istina, za neke magistrale poput I2C i SPI, *Linux* obezbjeđuje odgovarajuću
infrastrukturu kojom se apstrahuju kontroleri magistrale, što olakšava komuikaciju sa uređajima
priključenim na datu magistralu. Međutim, to i dalje nije idealno rješenje. Konačno, ovako napisane
aplikacije i drajveri nisu integrisani sa drugim komponenatama *Linux* sistema, što otežava njihovu
generalizaciju i eksploataciju od strane drugih softverskih komponenata.

Uzimajući u obzir sve prethodno, jasno je da je neophodno koristiti drugačiji pristup, a to su
drajveri realizovani kao kernel moduli kojima iz korisničkog prostora pritupiti preko standardizovanih
interfejsa. Ovo će biti naša glavna tema u drugom dijelu ove vježbe.

## Upravljanje uređajima iz korisničkog prostora preko kernel drajvera

Iako smo u prethodnom dijelu vježbe uspješno kontrolisali LED diodu i čitali stanje tastera direktno
iz korisničkog prostora, ovaj pristup se ne uklapa u *Linux* infrastrukturu na odgovarajući način.
Da bismo mogli da iskoristimo infrastrukturu operativnog sistema u njegovom punom kapacitetu, kao i
da bi omogućili generalizovani pristup uređajima iz korisničkog prostora preko odgovarajućih interfejsa,
potrebno je da koristimo drajvere implementirane u vidu kernel modula.

Prvo trebamo eksportovati *toolchain* i preći u folder sa izvornim kodom *Linux* jezgra.

```
source <urs-repo>/scripts/set-environment.sh
cd linux-socfpga
```

Prvo je potrebno da u konfiguraciji kernela omogućimo podršku za GPIO tastere, kako bi oni bili
vidljivi od strane kernela kao standardni ulazni uređaji (kao što su miš i tastatura). Da bi to postigli,
potrebno je da omogućimo opciju `CONFIG_KEYBOARD_GPIO` koja se nalazi u
**Device Drivers**&rarr;**Input device support**&rarr;**Keyboards**&rarr;**GPIO Buttons**.

U ovom dijelu vježbe ćemo takođe koristiti i akcelerometar ADXL345 koji se nalazi na DE1-SoC ploči, pa
uključujemo i opciju `CONFIG_ADXL345_I2C` u konfiguraciji (s obzirom da je ovaj uređaj povezan na I2C
magistralu). Ovu opciju možemo da pronađemo u
**Device Drivers**&rarr;**Industrial I/O support**&rarr;**Accelerometers**&rarr;**Analog Devices ADXL345 3-Axis Digital Accelerometer I2C Driver**,
s tim što ćemo drajver definisati kao modul (`<M>`) u konfiguraciji, a ne da bude ugrađen u sliku kernela.

Nakon konfiguracije, možemo da ponovo kroskompajliramo kernel komandom `make`. Po završetku kroskompajliranja
ostaje da prekopiramo novu sliku kernela u folder TFTP servera

```
sudo cp arch/arm/boot/zImage /srv/tftp/
```

i instaliramo nove kernel module u NFS *root* fajl sistem

```
make INSTALL_MOD_PATH=/path/to/rootfs modules_install
```

Konačno, možemo sačuvati trenutnu konfiguraciju kernela kao predefinisanu za DE1-SoC ploču

```
make savedefconfig
mv defconfig arch/arm/configs/de1_soc_defconfig
```

i predajte ovako napravljenu konfiguraciju na granu u okviru repozitorijuma kursa.

Sada ćemo pokrenuti DE1-SoC razvojnu ploču i provjeriti funkcionalnost novih drajvera koje smo prethodno
omogućili u konfiguraciji. Od interesa nam je drajver za akcelerometar ADXL345 kompajliran kao kernel modul.
Da bi učitali ovaj kernel modul, potrebno je da pronađemo njegov *alias*, koji se nalazi u fajlu
`modules.alias`. Izlistavanjem sadržaja ovog fajla dobijamo sljedeće informacije (prikazan je samo dio
listinga, jer u sistemu imamo i druge module koji nisu relevantni u našen slučaju):

```
~ # cat /lib/modules/6.1.38-etfbl-lab+/modules.alias
# Aliases extracted from modules themselves.
alias platform:altera_ilc altera_ilc
alias of:N*T*Caltr,ilc-1.0C* altera_ilc
alias of:N*T*Caltr,ilc-1.0 altera_ilc
[...]
alias i2c:adxl375 adxl345_i2c
alias i2c:adxl345 adxl345_i2c
alias of:N*T*Cadi,adxl375C* adxl345_i2c
alias of:N*T*Cadi,adxl375 adxl345_i2c
alias of:N*T*Cadi,adxl345C* adxl345_i2c
alias of:N*T*Cadi,adxl345 adxl345_i2c
alias acpi*:ADS0345:* adxl345_i2c
```

Iz prikazane liste se jasno vidi da naš drajver ima *alias* `i2c:adxl345`, koji koristimo pri učitavanju
kernel modula komandom `modprobe`.

```
~ # modprobe i2c:adxl345
[  107.540801] adxl345_i2c 0-0053: error -EREMOTEIO: Error reading device ID
[  107.547658] adxl345_i2c: probe of 0-0053 failed with error -121
```

Jasno je da pri učitavanju kernel modula sistem prijavljuje grešku. To je zato što ovaj uređaj nije ispravno
opisan u *device tree* strukturi za opis hardvera. Prema tome, sljedeći korak je da prilagodimo *device tree*
fajl tako da odgovara našoj platformi.

### Prilagođenje *device tree* fajla ciljne platforme

U prethodnim vježbama, *Linux* kernelu smo prosljeđivali `socfpga_cyclone5_sockit.dtb`, koji je opisuje hardver
*Terasic SoCkit* razvojne ploče. Iako ova razvojna ploča koristi isti SoC (*Cyclone V*), njen hardver se razlikuje
od DE1-SoC ploče.

Prvi korak u prilagođavanju *device tree* fajla je pravljenje kopije polaznog fajla (u našem slučaju to je izvorni
fajl prethodno pomenutog `socfpga_cyclone5_sockit.dtb` fajla)

```
cd arch/arm/boot/dts
cp socfpga_cyclone5_sockit.dts socfpga_cyclone5_de1_soc.dts
```

Da bi mogli kompajlirati ovaj novi fajl, potrebno je da ga uključimo u *build* sistem *Linux*-a tako što editujemo
`Makefile` fajl unutar `arch/arm/boot/dts` foldera i dodajemo `socfpga_cyclone5_de1_soc.dtb` na kraju varijable
`dtb-$(CONFIG_ARCH_INTEL_SOCFPGA)`, tako da njen sadržaj ima sljedeći izgled:

```
dtb-$(CONFIG_ARCH_INTEL_SOCFPGA) += \
	socfpga_arria5_socdk.dtb \
	socfpga_arria10_chameleonv3.dtb \
	socfpga_arria10_socdk_nand.dtb \
	socfpga_arria10_socdk_qspi.dtb \
	socfpga_arria10_socdk_sdmmc.dtb \
	socfpga_cyclone5_chameleon96.dtb \
	socfpga_cyclone5_mcvevk.dtb \
	socfpga_cyclone5_socdk.dtb \
	socfpga_cyclone5_de0_nano_soc.dtb \
	socfpga_cyclone5_sockit.dtb \
	socfpga_cyclone5_socrates.dtb \
	socfpga_cyclone5_sodia.dtb \
	socfpga_cyclone5_vining_fpga.dtb \
	socfpga_vt.dtb \
	socfpga_cyclone5_de1_soc.dtb
```

Naredni korak je editovanje samog `socfpga_cyclone5_de1_soc.dts` fajla. Otvorite fajl u bilo kojem tekstualnom
editoru i prvo promijenite `model` stavku koja se nalazi u *root* čvoru. Postavite da bude `"Terasic DE1-SoC"`.

Zatim pronađite čvor `leds` i u njemu obrišite sve podčvorove osim `hps_led0`. Promijenite naziv ovog podčvora
tako da bude `hps_led` umjesto `hps_led0`, a onda postavite `label` na `"hps_led"` umjesto `"hps:blue:led0"`.
Ovo će biti identifikator koji označava našu LED diodu u *sysfs* fajl sistemu. Vrijednosti ostalih karakteristika
podčvora možete zadržati kao što jesu.

> [!NOTE]
> Karakteristikom `gpios` definišemo port na kojem se nalazi LED dioda (u datom slučaju, to je `portb` koji odgovara
GPIO1 kontroleru) i broj pina u datom portu (24 u našem slučaju, koji odgovara pinu HPS_GPIO53). Treći broj određuje
dodatne karakteristike pina i nije značajan u našem slučaju, pa možemo da zadržimo vrijednost 0.

Pronađite čvor `gpio-keys` i u njemu obrišite sve podčvorove osim `hps_hkey0`. Promijenite naziv ovog podčvora iz
`hps_hkey0` u `hps_key`. Definišite `label` karakteristiku da takođe bude `"hps_key"`. Konačno, postavite `gpios`
karakteristiku da bude `<&portb 25 0>` (GPIO1 kontroler, pin 25, koji odgovara pinu HPS_GPIO54) i `linux,code`
karakteristiku na 16 (što predstavlja kod tastera Q).

> [!NOTE]
> Kompletnu listu kodova ulaznog podsistema *Linux*-a možete da vidite u [izvornom kodu](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h).

Ostaje još da modifikujemo opis I2C kontrolera na koji je priključen akcelereometar ADXL345. Iz [električne šeme](docs/de1-soc-schematic.pdf)
DE1-SoC ploče (strane 5 i 25) možemo da vidimo da je akcelerometar fizički povezan na I2C0 kontroler, tako da je potrebno
da umjesto `i2c1` čvora, referenciramo `i2c0`. Osim toga, postavljamo i maksimalnu I2C frekvenciju na 100kHz za ovaj uređaj
dodavanjem karakteristike `clock-frequency = <100000>` odmah ispod `status = "okay"`. Kompletan opis ima sljedeći izgled:

```
&i2c0 {
	status = "okay";
	clock-frequency = <100000>;

	accel1: accelerometer@53 {
		compatible = "adi,adxl345";
		reg = <0x53>;

		interrupt-parent = <&portc>;
		interrupts = <3 2>;
	};
};
```

Karakteristika `reg` ima vrijednost 0x53, jer je to I2C adresa ovog uređaja.

> [!NOTE]
> Iz električne šeme DE1-SoC ploče (strana 25) vidimo da je adresa akcelerometra definisana kao 0xA6/0xA7. Međutim,
ovo je 8-bitna verzija adrese koja se dobija pomjeranjem 7-bitne adrese jednu bit poziciju ulijevo. Ako 8-bitnu
vrijednost adrese (0xA6 ili 0xA7) pomjerimo jednu bit poziciju udesno, dobijamo 7-bitnu adresu uređaja, tj. 0x53.

Postavljanjem `status` na `"okay"` omogućavamo I2C0 kontroler.

Ostaje još deifinicija prekida (`interrupt-parent`) kojom je definisano da je pin na kojem uređaj generiše prekid
povezan na `portc`, što odgovara GPIO2 kontroleru. Karakteristikom `interrupts` definišemo pin u okviru ovog GPIO
kontrolera (3, što odgovara pinu HPS_GPIO61) i tip prekida (2 označava da se radi o opadajućoj ivici).

Ostaje još da potpuno obrišemo čvor `&qspi` jer DE1-SoC ploča ne koristi uređaje na QSPI magistarli i možemo da
sačuvamo izmjene, kompajliramo *device tree* fajl i kopiramo ga u folder TFTP servera.

```
cd <linux-src-root>
make dtbs
sudo cp arch/arm/boot/dts/socfpga_cyclone5_de1_soc.dtb /srv/tftp/
```

Pokrenemo ploču i prekinemo proces podizanja sistema, da bismo u *bootloader*-u izmjenili naziv *device tree* fajla
(npr. korišćenjem `editenv bootcmd`). Nakon toga sačuvamo izmjene u okruženju (`saveenv`) i restartujemo ploču.

Nakon ponovnog podizanja operativnog sistema, možemo potvrditi da je učitan novi *device tree* fajl inspekcijom
loga prilikom podizanja *Linux* operativnog sistema.

```
Deasserting all peripheral resets
[    0.000000] Booting Linux on physical CPU 0x0
[    0.000000] Linux version 6.1.38-etfbl-lab+ (mknezic@Ubuntu-Dev) (arm-linux-gcc (crosstool-NG 1.26.0) 13.2.0, GNU ld (crosstool-NG 1.26.0) 2.40) #11 SMP Fri May 10 15:23:56 CEST 2024
[    0.000000] CPU: ARMv7 Processor [413fc090] revision 0 (ARMv7), cr=10c5387d
[    0.000000] CPU: PIPT / VIPT nonaliasing data cache, VIPT aliasing instruction cache
[    0.000000] OF: fdt: Machine model: Terasic DE1-SoC
[    0.000000] Memory policy: Data cache writealloc
[...]
```

Pri samom pokretanju kernela, *Machine model* bi sada trebalo da bude *Terasic DE1-SoC*, čime potvrđujemo da smo
učitali ispravan *device tree* fajl.

Nakon što ste potvrdili da se sistem učitava kako treba sa novim *device tree* fajlom, možete da predate ovako
izmijenjen *device tree* fajl na granu u repozitorijumu kursa.

### Upravljanje LED uređajem iz korisničkog prostora

Sada kada imamo prilagođen *device tree* fajl, možemo da demonstriramo kontrolu uređaja iz korisničkog prostora
preko drajvera implementrianih kao kernel moduli. Prvo ćemo početi sa LED uređajima.

Nakon što ste pokrenuli sistem na DE1-SoC ploči, izlistajte sve LED uređaje koje je detektovao sistem (folder
`/sys/class/leds/`).

```
~ # ls /sys/class/leds/
hps_led
```

Prikazuje se samo jedan uređaj (`hps_led`) čiji naziv odgovara labeli koju smo definisalu u *device tree* fajlu.
Ako prikažemo sadržaj ovog foldera

```
~ # ls /sys/class/leds/hps_led/
brightness      max_brightness  subsystem       uevent
device          power           trigger
```

dobijamo određeni broj fajlova i foldera, preko kojih možemo da upravljamo uređajem, odnosno pristupimo dodatnim
opcijama za njegovu konfiguraciju. Na primjer, ako prikažemo sadržaj fajla `trigger`, dobijamo veći broj podržanih
opcija koje definišu ponašanje LED diode (više o značenju pojedinih opcija možete pronaći u [dokumentaciji](https://docs.kernel.org/leds/leds-class.html)).

```
~ # cat /sys/class/leds/hps_led/trigger
[none] kbd-scrolllock kbd-numlock kbd-capslock kbd-kanalock kbd-shiftlock kbd-altgrlock kbd-ctrllock kbd-altlock kbd-shiftllock kbd-shiftrlock kbd-ctrlllock kbd-ctrlrlock timer cpu cpu0 cpu1 mmc0
```

Trenutno je LED uređaj konfigurisan sa `none` opcijom u virtuelnom fajlu `trigger`. Ova opcija podrazumijeva da
stanje LED diode kontroliše direktno korisnik. Trenutno stanje možemo da dobijemo prikazom sadržaja vrituelnog
fajla `brightness` koji se nalazi u folderu LED uređaja.

```
~ # cat /sys/class/leds/hps_led/brightness
0
```

> [!NOTE]
> U zavisnosti od toga na koji pin je priključena LED dioda, ovaj parametar može podržava samo vrijednosti 0
(uključena LED dioda) ili 1 (isključena LED dioda), odnosno da može da se postavi opseg vrijednosti (ako je
LED dioda povezana na PWM pin). Podržani opseg vrijednosti kreće od 0 do maksimalnog osvjetljenja definisanog
u virtuelnom fajlu `max_brightness`.

Promjenom vrijednosti u fajlu `brightness`, korisnik može da kontroliše stanje LED diode.

```
~ # echo 1 > /sys/class/leds/hps_led/brightness
~ # echo 0 > /sys/class/leds/hps_led/brightness
```

Promjenom `trigger` opcije, možemo da sistemu prepustimo kontrolu LED uređaja na definisan način. Na primjer,
jedan od načina rada je `timer`, čijom aktivacijom

```
~ # echo timer > /sys/class/leds/hps_led/trigger
```

dobijamo dodatna dva parametra u folderu LED uređaja (`delay_off` i `delay_on`). Takođe, trebalo bi da vidite
da se sada LED dioda na ploči automtski uključuje i isključuje svakih pola sekunde.

```
~ # ls /sys/class/leds/hps_led/
brightness      device          subsystem
delay_off       max_brightness  trigger
delay_on        power           uevent
```

Parametri `delay_off` i `delay_on` definišu koliko dugo će LED dioda biti isključena, odnosno uključena, respektivno,
izraženo u milisekundama. Na primjer, ako pročitamo ove parametre

```
~ # cat /sys/class/leds/hps_led/delay_on
500
~ # cat /sys/class/leds/hps_led/delay_off
500
```

vidimo da su oba postavljena na vrijednost 500, što odgovara 0.5 sekunde (500ms).

Eksperimentišite promjenom vrijednosti ovih parametara i pratite ponašanje LED diode na ploči.

> [!NOTE]
> Iako iz naših aplikacija možemo upravljati LED uređajem preko relevantnih fajlova u *sysfs* sistemu, najčešće
koristimo biblioteke poput [*libgpiod*](https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git) u tu svrhu.

### Pristup tasterima iz korisničkog prostora

U prethodnim koracima smo omogućili *GPIO Buttons* darjver koji registruje tastere opisane u *device tree* fajlu u
kao ulazni uređaj *Linux*-a. Potvrdite da je naš taster korektno registrovan u ovoj infrastrukturi korišćenjem
komande `dmesg`.

```
~ # dmesg | grep input
[    1.803791] input: gpio-keys as /devices/platform/gpio-keys/input/input0
```

Ako izlistamo sadržaj foldera `/sys/class/input` koji prikazuje ulazne uređaje, dobijamo

```
~ # ls /sys/class/input/
event0  input0
```

Osim toga, taster kao ulazni uređaj se registruje u *device* fajl sistemu

```
~ # ls /dev/input/
event0
```

Svaki put kada pritisnemo taster, u infrastrukturi ulaznih *Linux* uređaja emituje se događaj sa odgovarajućim
ulaznim kodom koji smo definisali u *device tree* fajlu za dati taster. Ovo možemo pratiti čitanjem virtuelnog
fajla `/dev/input/event0`

```
~ # cat /dev/input/event0 | od -x
0000000 0b08 0000 c052 0008 0001 0010 0000 0000
0000020 0b08 0000 c052 0008 0000 0000 0000 0000
0000040 0b08 0000 8066 000c 0001 0010 0001 0000
0000060 0b08 0000 8066 000c 0000 0000 0000 0000
0000100 0b09 0000 2eb2 000c 0001 0010 0000 0000
0000120 0b09 0000 2eb2 000c 0000 0000 0000 0000
0000140 0b0a 0000 4684 0005 0001 0010 0001 0000
0000160 0b0a 0000 4684 0005 0000 0000 0000 0000
0000200 0b0b 0000 da01 000a 0001 0010 0000 0000
0000220 0b0b 0000 da01 000a 0000 0000 0000 0000
0000240 0b0c 0000 ee6c 0002 0001 0010 0001 0000
0000260 0b0c 0000 ee6c 0002 0000 0000 0000 0000
0000300 0b0c 0000 bdd7 000e 0001 0010 0000 0000
0000320 0b0c 0000 bdd7 000e 0000 0000 0000 0000
0000340 0b0d 0000 3c8f 0002 0001 0010 0001 0000
0000360 0b0d 0000 3c8f 0002 0000 0000 0000 0000
0000400 0b0d 0000 2b60 000d 0001 0010 0000 0000
0000420 0b0d 0000 2b60 000d 0000 0000 0000 0000
0000440 0b0e 0000 d213 0003 0001 0010 0001 0000
0000460 0b0e 0000 d213 0003 0000 0000 0000 0000
```

S obzirom da se radi o čitavom frejmvorku, dbijemo značajno veći broj informacija koje treba interpretirati na
odgovarajući način. Zasad je dovoljno da potvrdite da se podaci pojavljuju svaki put kada pritisnete taster.
Kasnije ćemo vidjeti na koji način možemo interpretirati ove podatke.


### Rad sa akcelerometrom ADXL345

Na kraju vježbe ćemo demonstrirati rad sa akcelerometrom ADXL345 sa kojim se na ploči komunicira preko I2C
magistrale.

U sklopu *BusyBox* alata smo već selektovali *i2c-tools* skup alata za manipulaciju I2C uređajima iz korisničkog
prostora. U ovom skupu alata se nalazi i alatka [`i2cdetect`](https://manpages.debian.org/unstable/i2c-tools/i2cdetect.8.en.html)
koja nam omogućava detekciju I2C uređaja na magistrali. Prvo ćemo izlistati sve I2C uređaje u sistemu.

```
~ # i2cdetect -l
i2c-0   i2c             Synopsys DesignWare I2C adapter         I2C adapter
```

Vidimo da nam je na raspolaganju samo `i2c-0` kontroler, pa ćemo detektovati prisustvo svih uređaja na uređaju 0.

```
~ # i2cdetect -r 0
i2cdetect: WARNING! This program can confuse your I2C bus
Continue? [y/N] y
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
50: -- -- -- 53 -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
70: -- -- -- -- -- -- -- --
```

Vidimo da je detektovan uređaj sa adresom 0x53, odnosno akcelerometar. Ovaj uređaj pripada IIO (Industrial I/O) podsistemu.
Međutim, ako izlistamo sve uređaje koji se trenutno nalaze u ovom podsistemu

```
~ # ls /sys/bus/iio/devices/
```

vidjećemo da nije detektovan nijedan uređaj. To je zato što još uvijek nismo učitali kernel modul drajvera uređaja. Ako
učitamo ovaj kernel modul komandom `modprobe` i potvrdimo da su svi relevantni kernel moduli učitani komandom `lsmod`

```
~ # modprobe i2c:adxl345
~ # lsmod
adxl345_i2c 16384 0 - Live 0xbf008000
adxl345_core 16384 1 adxl345_i2c, Live 0xbf000000
```

možemo ponovo pokrenuti `i2cdetect` da provjerimo stanje na magistrali.

```
~ # i2cdetect -r 0
i2cdetect: WARNING! This program can confuse your I2C bus
Continue? [y/N] y
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
50: -- -- -- UU -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
70: -- -- -- -- -- -- -- --
```

Vidimo da se sada na adresi 0x53 nalazi vrijednost `UU`, što nam govori da uređaj sa datom adresom koristi neki drajver.
Ovo možemo potvrditi izlistavanjem uređaja u IIO podsistemu

```
~ # ls /sys/bus/iio/devices/
iio:device0
```

Vidimo da je sada prisutan uređaj `iio:device0`. Izlistaćemo sadržaj njegovog foldera.

```
ls /sys/bus/iio/devices/iio:device0/
in_accel_sampling_frequency   name
in_accel_scale                of_node
in_accel_x_calibbias          power
in_accel_x_raw                sampling_frequency_available
in_accel_y_calibbias          subsystem
in_accel_y_raw                uevent
in_accel_z_calibbias          waiting_for_supplier
in_accel_z_raw
```

Kao i u slučaju LED uređaja, na raspolaganju imamo određeni broj fajlova i foldera, preko kojih možemo da pristupimo
podacima i konfigurišemo uređaj. Pročitaćemo sadržaj fajla `in_accel_scale`.

```
~ # cat /sys/bus/iio/devices/iio:device0/in_accel_scale
0.038300
```

Ovo je faktor kojim treba da pomnožimo pročitane *raw* vrijednosti da bismo dobili stvarnu vrijednost ubrzanja.

Ubrzanje po svakoj od osa dobijamo ako pročitamo fajlove `in_accel_x_raw`, `in_accel_y_raw` i `in_accel_z_raw`.

```
~ # cat /sys/bus/iio/devices/iio:device0/in_accel_x_raw
-10
~ # cat /sys/bus/iio/devices/iio:device0/in_accel_y_raw
-2
~ # cat /sys/bus/iio/devices/iio:device0/in_accel_z_raw
293
```

Množenjem sa prethodno pomenutim faktorom skaliranja, dobijamo stvarne vrijednosti ubrzanja. U našem slučaju je
uređaj montiran tako da je z-osa usmjerena prema zemlji, tako da bi u mirnom stanju ubrzanje na ovoj osi trebalo
da iznosi 9.81 što odgovara gravitaciji Zemlje.

> [!NOTE]
> Možete primjetiti da se množenjem ne dobija ispravna vrijednost gravitacije. Razlog leži u tome što senzor nije
kalibrisan, pa izmjerene vrijednosti po osama imaju određena odstupanja.

Po završetku rada, kernel modul možemo da uklonimo komandom `modprobe`.

```
~ # modprobe -r i2c:adxl345
```

> [!NOTE]
> Kao i kod prethodnih uređaja, u aplikacijama možemo da otvorimo relevantne fajlove u *sysfs* fajl sistemu i da
na taj način pristupamo uređaju. Međutim, najčešće koristimo biblioteke koje eksportuju funkcionalnosti uređaja
kroz određeni skup funkcija. Na primjer, za akcelereometar možemo da koristimo [*libiio*](https://analogdevicesinc.github.io/libiio/) biblioteku.
