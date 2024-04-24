# Laboratorijska vježba 6

Cilj laboratorijske vježbe je da se studenti upoznaju sa postupkom kreiranja *root*
fajl sistema ugrađenog sistema zasnovanog na *Linux* operativnom sistemu.

Nakon uspješno realizovane vježbe studenti će biti sposobni da:
1. razumiju i kreiraju strukturu minimalnog *root* fajl sistema,
2. preuzmu, konfigurišu i kroskompajliraju *BusyBox* skup alata,
3. instaliraju i konfigurišu NFS server,
4. naprave minimalnu konfiguraciju `init` programa sa skriptom za automatsko montiranje
virtuelnih fajl sistema i
5. kreiraju i koriste *initramfs*.

## Preduslovi za izradu vježbe

Za uspješno izvođenje laboratorijske vježbe potrebno je sljedeće:

- razvojna ploča *DE1-SoC* sa pripadajućim napajanjem i kablom za povezivanje računara sa
UART interfejsom na ploči,
- preformatirana SD kartica sa instaliranim *bootloader*-om,
- *Ethernet* mrežni interfejs na razvojnoj platformi i mrežni kabl za povezivanje sa
pločom i
- konfigurisan *U-Boot* za automatsko učitavanje kernela i *device tree* fajla preko TFTP protokola.

## Kreiranje inicijalne strukture *root* fajl sistema

*Linux* kernel za podizanje sistema zahtjeva postojanje *root* fajl sistema koji
ima odgovarajuću strukturu foldera definisanu [Filesystem Hierarchy Standard (FHS)](https://refspecs.linuxfoundation.org/fhs.shtml)
specifikacijom. Većina foldera i fajlova kreira se prilikom instalacije *Busybox*
alata i prilikom montiranja virtuelnih fajl sistema. Međutim, prije svega, moramo
da napravimo incijalnu strukturu ovog fajl sistema. Prvo ćemo u `home` direktorijumu
korisnika kreirati krovni direktorijum pod nazivom `rootfs`.

```
cd ~
mkdir rootfs/bin/busybox
```

Ovaj direktorijum nazivamo još i *staging* direktorijum.

Zatim ćemo unutar ovog direktorijuma kreirati određeni broj standardnih foldera.

```
cd rootfs/
mkdir dev proc sys etc home lib tmp var
mkdir -p var/log usr/lib
```

Struktura `rootfs` direktorijuma treba da ima sljedeći izgled:

```
rootfs/
├── dev
├── etc
├── home
├── lib
├── proc
├── sys
├── tmp
├── usr
│   └── lib
└── var
    └── log
```

Sljedeći korak je instalacija alatki koje su neophodne za rad sa *Linux* sistemom.
U tu svrhu najbolje je koristiti *BusyBox* projekat.

## *Busybox*: konfiguracija i kompajliranje

Prvo ćemo preuzeti *BusyBox* izvorni kod i prebaciti se na stabilnu verziju `1_36_stable`:

```
git clone https://git.busybox.net/busybox
cd busybox/
git checkout 1_36_stable
```

Zatim ćemo postaviti podrazumijevanu konfiguraciju koja uključuje alate koje tipično koristi
većina korisnika.

```
make defconfig
```

Za dodatnu konfiguraciju pokrećemo meni za konfiguraciju *BusyBox* opcija.

```
make menuconfig
```

Zasad ćemo u okviru **Settings** pod kategorijom **Build Options** definisati opciju
**Cross compiler prefix** tako da odgovara prefiksu našeg kompajlera (npr. `arm-linux-`),
a pod kategorijom **Installation Options** definisati opciju **Destination path for 'make install'**
tako da pokazuje na lokaciju foldera u kojem se nalazi naš *root* fajl sistem (npr. `../rootfs`).
 
Prije kompajliranja, potrebno je da eksportujemo putanju do *toolchain*-a korišćenjem skripte koju
smo koristili u ranijim vježbama.

> [!NOTE]
> Pomenuta skripta takođe postavlja i prefiks kompajlera u varijabli `CROSS_COMPILE`, tako da
podešavanje opcije **Cross compiler prefix** nije obavezno u našem slučaju.

Sada je dovoljno pokrenuti komandu `make` za kroskompajliranje *BusyBox* projekta. Po završetku
kompajliranja, komandom `make install` instaliramo konfigurisane alate na lokaciju specificiranu
u opciji **Destination path for 'make install'**. Dovoljno je da definišemo lokaciju odredišnog
foldera, a *BusyBox* će kreirati sve neophodne foldere ukoliko već nisu kreirani.

> [!TIP]
> *BusyBox* kompajlira sve alate u jedan binarni fajl koji se kopira na lokaciju `rootfs/bin/busybox`.
Svi ostali fajlovi su zapravo simbolički linkovi koji pokazuju na ovaj binarni fajl, što možete
provjeriti npr. komandom `ls -l rootfs/bin/`.

Sada kada smo instalirali sve neophodne alate za pokretanje i rad sa *Linux* sistemom, potrebno je
da ih dostavimo ciljnoj platformi. Jedan način je da kompletan *root* fajl sistem kopiramo na EXT4
particiju na SD kartici, ali daleko felksibilniji i brži način je korišćenjem NFS protokola.

## Instalacija i konfiguracija NFS servera

Prvo je potrebno instalirati NFS server. Na *Ubuntu 22.04* distribuciji koristimo sljedeću komandu:

```
sudo apt install nfs-kernel-server
```

Nakon instalacije, server će se automatski pokrenuti, ali njegov status možete provjeriti komandom

```
sudo service nfs-kernel-server status
```

Ekvivalentno, stanje servera možete kontrolisati komandama `start`, `stop` i `restart`.

Da bi naš *staging* direktorijum bio vidljiv na ciljnoj platformi, potrebno je da ga dodamo
u listu eksportovanih foldera NFS servera. U tom smislu, trebamo editovati fajl `/etc/exports`
tako što ćemo dodati liniju

```
/home/<user>/rootfs <client_ip_addr>(rw,no_root_squash,no_subtree_check)
```

pri čemu `<client_ip_addr>` treba zamijeniti sa stvarnom IP adresom klijenta (ciljna platforma),
npr. 192.168.21.200, a `<user>` sa stvarnim korisničkim imenom.

> [!IMPORTANT]
> Svi parametri moraju da se nalaze u jednoj liniji, a između IP adrese i NFS parametara navedenih
u zagradi ne smije da se nalazi prazan prostor (čak ni razmak).

> [!TIP]
> Umjesto IP adrese može da se stavi `*`, čime se dozvoljava pristup NFS serveru svim klijentima koji
se nalaze u istoj mreži. Ovo je korisna opcija ukoliko više klijenata koristi isti *root* fajl sistem.
U našem slučaju je bolje specificirati IP adresu klijenta.

Konačno, da bi se promjene u `/etc/exports` fajlu propagirale do server, potrebno ga je restartovati
ili pokrenuti sljedeću komandu:

```
sudo exportfs -r
```

## Podizanje *Linux* kernela korišćenjem NFS *root* fajl sistema

Prije podizanja sistema, potrebno je potvrditi da su u kernelu uključene opcije koje omogućavaju
podizanje sistema preko NFS servera. U tom smislu, provjerite konfiguraciju kernela i potvrdite da su
omogućene sljedeće opcije:

- `CONFIG_NFS_FS`: opcija **File systems**&rarr;**Network File Systems**&rarr;**NFS client support** u konfiguraciji
koja omogućava podršku za komunikaciju sa NFS klijentom,
- `CONFIG_ROOT_NFS`: opcija **File systems**&rarr;**Network File Systems**&rarr;**Root file system on NFS** u konfiguraciji
koja omogućava podizanje sistema kada se *root* fajl sistem nalazi na NFS serveru i
- `CONFIG_IP_PNP`: opcija **Networking support**&rarr;**Networking options**&rarr;**IP: kernel level autoconfiguration**
u konfiguraciji koja omogućava dodjeljivanje IP adrese tokom rane faze podizanja sistema, prije montiranja *root*
fajl sistema.

Takođe, da bi omogućili podršku za montiranje `devtmpfs` virtuelnog fajl sistema koji eksportuje korisničkom prostoru
interfejs ka uređajima u sistemu, potrebno je da omogućimo sljedeće opcije u konfiguraciji kernela:

- `CONFIG_DEVTMPFS`: opcija **Device Drivers**&rarr;**Generic Driver Options**&rarr;**Maintain a devtmpfs filesystem to mount at /dev**
u konfiguraciji koja kreira instancu `devtmpfs` fajl sistema u ranoj fazi podizanja sistema i
- `CONFIG_DEVTMPFS_MOUNT`: opcija **Device Drivers**&rarr;**Generic Driver Options**&rarr;**Automount devtmpfs at /dev, after the kernel mounted the rootfs**
u konfiguraciji koja govori kernelu da automatski montira `devtmpfs` virtuelni fajl sistem nakon montiranja *root*
fajl sistema.

Ukoliko neka od prethodnih opcija nije omogućena u konfiguraciji kernela, potrebno je da je omogućite, a zatim da
ponovo kroskompajlirate kernel.

Ostaje još da promijenimo *U-Boot* varijablu `bootargs` preko koje prosljeđujemo argumente komandne linije kernelu.
Povežite serijski terminal na ploči sa PC računarom, napajanje ploče i Ethernet kabl, a zatim uključite ploču.
Prekinite proces podizanja sistema kako biste dobili pristup *U-Boot* konzoli, a zatim dodajte sljedeće argumente
(npr. korišćenjem komande `editenv bootargs`)

```
root=/dev/nfs nfsroot=<server_ip>:/home/<user>/rootfs,nfsvers=3,tcp rw
```

gdje su: `<user>` korisničko ime, `<server_ip>` IP adresa NFS servera i `<client_ip>` IP adresa ploče.

Nakon editovanja, komandna linija bi trebalo da ima izgled ekvivalentan sljedećem primjeru:

```
=> printenv bootargs
bootargs=console=ttyS0,115200n8 ip=192.168.21.200 root=/dev/nfs nfsroot=192.168.21.101:/home/mknezic/rootfs,nfsvers=3,tcp rw
```

Sačuvajte izmjene komandom `saveenv`

```
=> saveenv
Saving Environment to MMC... Writing to MMC(0)... OK
```

a zatim ponovo pokrenite sistem.

*Linux* kernel bi trebalo da prepozna i montira *root* fajl sistem sa NFS servera, a nakon njega i `devtmpfs` virtuelni
fajl sistem, što potvrđujemo sljedećim logovima:

```
[    6.240581] VFS: Mounted root (nfs filesystem) on device 0:14.
[    6.248162] devtmpfs: mounted
```

Međutim, kada pokuša da pokrene `init` program, nastaju problemi, što rezultuje *kernel panic* porukom sljedećeg sadržaja.

```
[    6.292948] Kernel panic - not syncing: No working init found.  Try passing init= option to kernel. See Linux Documentation/admin-guide/init.rst for guidance.
```

U čemu je problem? *BusyBox* binarni fajl je dinamički linkovan, a fajlovi neophodni za njegovo učitavanje i povezivanje
nisu dostupni u našem *root* fajl sistemu. Ovaj problem ćemo privremeno riješiti tako da statički kompajliramo
*BusyBox* alat. U tom smislu, vratite se u *BusyBox* konfiguraciju i omogućite opciju **Build static binary (no shared libs)**
koja se nalazi pod **Settings** u okviru **Build Options** kategorije. Zatim ponovo kompajlirajte projekat i instalirajte
fajlove komandаma `make` i `make install`.

Restartujte ploču sa ovako ažuriranim *root* fajl sistemom i sada bi trebalo da *Linux* može da podigne sistem i
da omogući pristup konzoli (nakon što pritisnete taster `Enter` po završetku procesa podizanja sistema).

Pokušajte da pokrećete različite komande u konzoli kako biste potvrdili da imate funkcionalan *Linux*
operativni sistem na *embedded* platformi.

Veličinu statički kompajliranog `busybox` fajla možemo da provjerimo komandom `ls -l`, kao što je
dato ispod.

```
~ # ls -l /bin/busybox
-rwxr-xr-x    1 1000     1000       2029128 Apr 23  2024 /bin/busybox
```

Vidimo da ovako kompajliran fajl ima veličinu od oko 2MB, što nije zanemarljivo.

Potvrdite da je *device* fajl sistem montiran kako treba izlistavanjem sadržaja direktorijuma `/dev`.
Trebalo bi da se izlista veliki broj fajlova koji odgovaraju uređajima detektovanim u sistemu.

Međutim, ako izlistate sadržaj druga dva virtuelna fajl sistema (folderi `/proc` i `/sys`), vidjećete
da su potpuno prazni. Neki programi zavise od ovih fajl sistema. Na primjer, pokrenite program `ps`
za izlistavanje trenutno aktivnih procesa. Dobijate praznu listu iako je jasno da to ne može biti slučaj,
jer sigurno imamo aktivan određeni broj procesa.

Da bi sve ispravno funkcionisalo u sistemu, treba da montiramo ova dva sistema. Prvo ćemo montirati
*procfs* fajl sistem.

```
~ # mount -t proc nodev /proc/
```

Pokrenite komandu `ps` nakon montiranja i vidjećete da sada dobijamo listu aktivnih procesa.

Na sličan način montiramo *sysfs* fajl sistem.

```
~ # mount -t sysfs nodev /sys/
```

Ako sada izlistamo sadržaj foldera `/sys`, vidjećemo da on više nije prazan. Ovdje, na primjer,
imamo mogućnost kontrolisanja LED dioda na ploči. Pristup ovoj funkcionalnosti se nalazi u
sljedećem folderu:

```
~ # ls /sys/devices/platform/leds/leds/
hps:blue:led0  hps:blue:led1  hps:blue:led2  hps:blue:led3
```

Jasno je da LED diode koje se ovdje nalaze ne postoje na našoj ploči. To je zato što koristimo
*device tree* druge ploče. Ovaj problem ćemo adresirati u narednim vježbama.

Jasno je da ručno montiranje virtuelnih fajlova na prethodno opisan način nije praktično. Zbog
toga koristimo infrastrukturu koju nudi `init` program koji dolazi u okviru *BusyBox* skupa alata.

Prvo trebamo kreirati konfiguracioni fajl `/etc/inittab` u okviru *staging* direktorijuma.

```
cd ~/rootfs
touch etc/inittab
```

Ovaj fajl čita `init` program i na osnovu njegovog sadržaja pokreće specificirane servise.
Nakon kreiranja, editujemo fajl tako što dodamo sljedeće dvije linije:

```
::sysinit:/etc/init.d/rcS
::askfirst:-/bin/ash
```

Prva linija pokreće skriptu `rcS` u sklopu inicijalizacije sistema, dok druga aktivira *shell*
program.

Inicijalizaciona skripta se nalazi u folderu `init.d`, pa prvo kreiramo ovaj folder, a zatim i fajl.

```
mkdir etc/init.d
touch etc/init.d/rcS
```

Ovo je obična *shell* skripta koja može da sadrži proizvoljne komande. U našem slučaju ćemo je
koristiti da automatski montiramo *procfs* i *sysfs* virtuelne fajl sisteme. Editujte `rcS`
fajl i doajte sljedeće linije:

```
#!/bin/sh
mount -t proc nodev /proc/
mount -t sysfs nodev /sys/
```

> [!NOTE]
> S obzirom da se radi o *shell* skripti, prva linija mora da definiše program koji će
izvršiti skriptu (u datom slučaju to je `/bin/sh`).

Sada još samo ostaje da definišemo skriptu kao izvršni fajl sljedećom komandom:

```
chmod +x etc/init.d/rcS
```

Restartujte sistem i, nakon uspješnog podizanja, verifikujte da se virtuelni fajl sistemi sada
automatski montiraju prethodno kreiranom skriptom bez intervencije korisnika.

Bez isključivanja ploče, kopirajte program `hello` iz prve laboratorijske vježbe u `home` direktorijum
*root* fajl sistema na razvojnoj platformi.

```
cd <path_to_urs2024_repo>
cp lab-01/hello/hello ~/rootfs/home
```

Provjerite sada u konzoli ploče da li se nalazi prethodno kopirani fajl na lokaciji `/home/`. Vidimo
kako elegantno možemo da nadograđujemo sadržaj *root* fajl sistema kada koristimo NFS protokol u
fazi razvoja.

```
~ # cd /home/
/home # ls
hello
```

Međutim, ako pokušamo da pokrenemo ovaj fajl, nećemo uspjeti iz razloga što je fajl dinamički linkovan
i zavisi od programa za dinamičko linkovanje i dinamičkih biblioteka.

Da bismo provjerili od kojih biblioteka zavisi naš program, možemo da iskoristimo skriptu `list-libs.sh`
koja se nalazi u `scripts` folderu repozitorijuma kursa.

```
scripts/list-libs.sh lab-01/hello/hello
      [Requesting program interpreter: /lib/ld-linux-armhf.so.3]
 0x00000001 (NEEDED)                     Shared library: [libc.so.6]
```

Vidimo da u *root* fajl sistem treba da prekopiramo fajlove `ld-linux-armhf.so.3` i `libc.so.6` da bismo
mogli da uspješno da izvršimo dati program.

Relevantne biblioteke se nalaze u *sysroot* folderu *toolchain*-a koji smo koristili za kroskompajliranje
programa. S obzirom da skripta `set-environment.sh` koju smo ranije pozvali eksportuje putanju do navedenog
foldera kroz varijablu `$SYSROOT`, kopiranje možemo da obavimo korišćenjem ove varijable na sljedeći način:

```
cd ~/rootfs
cp -a $SYSROOT/lib/ld-linux-armhf.so.3 lib
cp -a $SYSROOT/lib/libc.so.6 lib
```

> [!TIP]
> Da bi potvrdili da je ova varijabla zaista ispravno postavljena u trenutnoj konzoli, možete da koristite
komandu `echo $SYSROOT`.

Ako prikažemo detaljne informacije o fajlu `ld-linux-armhf.so.3` korišćenjem komande `file` zaključujemo da
ovaj fajl sadrži dodatne informacije neophodne za debagovanje, koje nisu neophodne na ciljnoj platformi.

```
file lib/ld-linux-armhf.so.3 
lib/ld-linux-armhf.so.3: ELF 32-bit LSB shared object, ARM, EABI5 version 1 (SYSV), dynamically linked, with debug_info, not stripped
```

Prema tome, možemo redukovati veličinu fajla korišćenjem `arm-linux-strip` alata.

```
arm-linux-strip lib/ld-linux-armhf.so.3
```

Nakon skidanja suvišnih informacija, dobijamo sljedeće informacije

```
file lib/ld-linux-armhf.so.3 
lib/ld-linux-armhf.so.3: ELF 32-bit LSB shared object, ARM, EABI5 version 1 (SYSV), dynamically linked, stripped
```

i nešto manju veličinu fajla (što možete provjeriti komandom `ls -l`).

Ako sada u konzoli ciljne platforme na serijskom terminalu ponovo pokušamo da izvršimo program, sve će da
funkcioniše kako treba.

```
~ # cd /home/
/home # ./hello
Hello, World!
```

Ovaj koncept možemo da iskoristimo i za redukovanje veličine `busybox` binarnog fajla. Ponovo konfigurišite
*BusyBox* sa isključenom opcijom za statičko linkovanje, kroskompajlirajte i instalirajte u *root* fajl sistem.

Ako pokrenemo skriptu za prikazivanje zavisnosti, dobijamo sljedeće:

```
cd <path_to_urs2024_repo>
scripts/list-libs.sh ~/rootfs/bin/busybox 
      [Requesting program interpreter: /lib/ld-linux-armhf.so.3]
 0x00000001 (NEEDED)                     Shared library: [libm.so.6]
 0x00000001 (NEEDED)                     Shared library: [libresolv.so.2]
 0x00000001 (NEEDED)                     Shared library: [libc.so.6]
```

Prekopirajte identifikovane fajlove u `lib` folder u *staging* direktorijumu i ponovo pokrenite sistem. Potvrdite
da se sistem uspješno podiže i da je veličina `busybox` binarnog fajla sada značajno manja (trebalo bi da bude
nešto preko 1MB).

Za ispravan rad kernela, nekada su neophodni kernel moduli. Iako to u našen slučaju nije nužno,
instaliraćemo kernel module koji su kompajlirani u skladu sa konfiguracijom. Potrebno je da
pređemo u folder sa izvornim kodom kernela i pokrenemo komandu za instalaciju modula, pri
čemu specificiramo naš *staging* direktorijum kao odredište.

```
cd <path_to_kernel_src>
make INSTALL_MOD_PATH=/home/<user>/rootfs modules_install
```

Ako sada izlistamo sadržaj `/lib` foldera na ciljnoj platformi, vidjećemo da se u njemu nalazi folder `modules`
koji sadrži instalirane kernel module u različitim podfolderima.

> [!TIP]
> Instalacija kernel modula može da se obavi bez restartovanja ploče, kao u slučaju korisničkih aplikacija.

## Kreiranje *initramfs* fajl sistema

Za podizanje *Linux* sistema sa *initramfs* podrškom, prvo trebamo potvrditi da je omogućena opcija `CONFIG_BLK_DEV_INITRD`
u konfiguraciji kernela koja se nalazi u **General setup**&rarr;**Initial RAM filesystem and RAM disk (initramfs/initrd) support**.

Ukoliko opcije nije omogućena, potrebno je omogućiti je i ponovo kompajlirati kernel.

Sada kreiramo kompresovanu CPIO arhivu našeg *staging* direktorijuma, koja predstavlja *initramfs*.

```
cd ~/rootfs
find . | cpio -H newc -o --owner root:root > ../initramfs.cpio
cd ..
gzip initramfs.cpio
```

Na ovaj način dobijamo fajl `initramfs.cpio.gz` u koji je spakovan kompletan *root* fajl sistem koji
smo prethodno kreirali. Opcijom `--owner root:root` dodjeljujemo vlasništvo nad svim fajlovima
`root` korisniku.

Da bi *U-Boot* mogao da učita ovaj fajl, potrebno je da ga konvertujemo u odgovarajuću sliku
sljedećom komandom:

```
mkimage -n "Ramdisk Image" -A arm -O linux -T ramdisk -C gzip -d initramfs.cpio.gz uInitramfs
```

Prekopirajte sljedeće fajlove na FAT32 particiju na SD karticu:

- kompresovanu sliku kernela (`zImage`),
- binarnu verziju *device tree* fajla (`socfpga_cyclone5_sockit.dtb`) i
- kompresovanu *initramfs* sliku (`uInitramfs`).

Stavite karticu u ploču i pokrenite sistem. Prekinite proces podizanja, tako da dobijete pristup
*U-Boot* konzoli. Koristite komande za učitavanje fajlova sa SD kartice iz prethodnih vježbi
da biste učitali relevantne fajlove (kernel sliku na lokaciju 0x01000000, DTB na lokaciju
0x02000000 i *initramfs* na lokaciju 0x03000000).

Redefinišite `bootargs` varijablu sljedećom komandom:

```
setenv bootargs console=ttyS0,115200n8 rdinit=/sbin/init
```

Prethodnom komandom definišemo lokaciju `init` programa koji kernel treba da izvrši pri podizanju
sistema (`rdinit=` argument komandne linije).

> [!TIP]
> Kada se koristi *initramfs* podrazumijevano se pokreće `/init` program za inicijalizaciju. Argumentom
komandne linije `rdinit=` možemo da redefinišemo ovu podrazumijevanu opciju i da specificiramo stvarnu
lokaciju `init` programa. Alternativno, mogli smo da napravimo simbolički link `/init` tako da
pokazuje na `/sbin/init`. U tom slučaju, prethodno pomenuti argument komandne linije nije potrebno
definisati.

Pokrenite proces podizanja sistema komandom

```
bootz 0x01000000 0x03000000 0x02000000
```

Kernel bi trebalo da inicijalizuje sistem i prikaže korisničku konzolu kao i u primjeru sa NFS
serverom.

Ako sada izlistate sadržaj foldera `/dev`, primjetićete da se u njemu nalazi samo jedan fajl
(`console`). To je zato što pri korišćenju *initramfs* pristupa kernel ignoriše konfiguracionu
opciju za automatsko montiranje `devtmpfs` fajl sistema, pa *device* fajl sistem nije montiran.
Ako pokrenete komandu za montiranje fajl sistema

```
mount -t devtmpfs nodev /dev
```

trebalo bi da dobijete listu uređaja kao što je to bio slučaj ranije.

Sada napravite izmjene u skripti `rcS` tako što ćete dodati prethodno navedenu komandu za montiranje
`devtmpfs` fajl sistema ispod postojećih. Ponovo kreirajte sliku *initramfs* fajl sistema na prethodno
opisan način i pokrenite sistem ponovo sa novom slikom. Potvrdite da se sada *device* fajl sistem
montira automatski.
