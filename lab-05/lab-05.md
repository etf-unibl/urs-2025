# Laboratorijska vježba 5

Cilj laboratorijske vježbe je da se studenti upoznaju sa konfiguracijom, kroskompajliranjem i
procesom podizanja *Linux* jezgra na platformi ugrađenog računarskog sistema.

Nakon uspješno realizovane vježbe studenti će biti sposobni da:
1. preuzmu, konfigurišu i kroskompajliraju *Linux* kernel,
2. koriste *U-Boot* komande za pokretanje procesa podizanja *Linux* kernela i
3. kreiraju *U-Boot* skriptu za automatsko pokretanje procesa podizanja *Linux* kernela.

## Preduslovi za izradu vježbe

Za uspješno izvođenje laboratorijske vježbe potrebno je sljedeće:

- razvojna ploča *DE1-SoC* sa pripadajućim napajanjem i kablom za povezivanje računara sa
*USB Blaster* interfejsom na ploči,
- preformatirana SD kartica sa instaliranim *bootloader*-om iz prethodne vježbe i
- *Ethernet* mrežni interfejs na razvojnoj platformi i mrežni kabl za povezivanje sa
pločom.

## Konfiguracija i kompajliranje *Linux* kernela

Prvo je potrebno da preuzmemo *Linux* izvorni kod. Za potrebe ove laboratorijske vježbe ćemo
koristiti repozitorijum kompanije *Altera*, koji predstavlja *fork* repozitorijuma zvaničnog
*Linux* projekta. Nakon kloniranja repozitorijuma, potrebno je da se prebacimo na verziju
`socfpga-6.1.38-lts`:

```
git clone https://github.com/altera-opensource/linux-socfpga.git
cd linux-socfpga
git checkout socfpga-6.1.38-lts
```

> [!IMPORTANT]
> Ova verzija *Linux* kernela je usklađena sa prethodno kreiranim *toolchain*-om. Važno je
napomenuti da, ukoliko želite da promijenite verziju *Linux* kernela, možda ćete morati
da kreirate i novi *toolchain*. Kao što smo ranije napomenuli, interfejs kernela je uvijek
kompatibilan unazad, što znači da bi nova verzija kernela u većini slučajeva trebala da
radi kako treba sa starijim *toolchain*-om.

Za ispravnu konfiguraciju i kroskompajliranje *Linux* kernela, potrebno je da eksportujemo
putanju do kroskompajlera i da postavimo varijablu `CROSS_COMPILE` tako da odgovara prefiksu našeg
kroskompajlera, te da ispravno definišemo arhitekturu (varijabla `ARCH`). Kao i ranije,
koristimo skriptu `set-environment.sh` iz `scripts` foldera u repozitorijumu kursa.

Da se upoznate sa opcijama koje nudi *Kbuild* sistem i da prikažete predefinisane konfiguracije
za odabranu arhitekuru, koristite komandu `make help`.

Najznačajnije komande sa opisom su prikazane u listingu datom ispod.

```
Cleaning targets:
  clean		  - Remove most generated files but keep the config and
                    enough build support to build external modules
  mrproper	  - Remove all generated files + config + various backup files
  distclean	  - mrproper + remove editor backup and patch files

Configuration targets:
  menuconfig	  - Update current config utilising a menu based program
  xconfig	  - Update current config utilising a Qt based front-end
  oldconfig	  - Update current config utilising a provided .config as base
  defconfig	  - New config with default from ARCH supplied defconfig
  savedefconfig   - Save current config as ./defconfig (minimal config)

Other generic targets:
  all		  - Build all targets marked with [*]
* vmlinux	  - Build the bare kernel
* modules	  - Build all modules
  modules_install - Install all modules to INSTALL_MOD_PATH (default: /)
  kernelrelease	  - Output the release version string (use with make -s)
  kernelversion	  - Output the version stored in Makefile (use with make -s)

Devicetree:
* dtbs             - Build device tree blobs for enabled boards
  dtbs_install     - Install dtbs to /boot/dtbs/6.1.38-etfbl-lab+

Architecture specific targets (arm):
* zImage        - Compressed kernel image (arch/arm/boot/zImage)
  Image         - Uncompressed kernel image (arch/arm/boot/Image)
  uImage        - U-Boot wrapped zImage
```

Takođe, možemo da vidimo da je za 32-bitnu ARM arhitekturu podržan veliki broj predefinisanih
konfiguracija, od kojih su neke:

```
...
  omap1_defconfig             - Build for omap1
  omap2plus_defconfig         - Build for omap2plus
  orion5x_defconfig           - Build for orion5x
  oxnas_v6_defconfig          - Build for oxnas_v6
  palmz72_defconfig           - Build for palmz72
  pcm027_defconfig            - Build for pcm027
  pleb_defconfig              - Build for pleb
  pxa168_defconfig            - Build for pxa168
  pxa255-idp_defconfig        - Build for pxa255-idp
  pxa3xx_defconfig            - Build for pxa3xx
  pxa910_defconfig            - Build for pxa910
  pxa_defconfig               - Build for pxa
  qcom_defconfig              - Build for qcom
  realview_defconfig          - Build for realview
  rpc_defconfig               - Build for rpc
  s3c2410_defconfig           - Build for s3c2410
  s3c6400_defconfig           - Build for s3c6400
  s5pv210_defconfig           - Build for s5pv210
  sama5_defconfig             - Build for sama5
  sama7_defconfig             - Build for sama7
  shannon_defconfig           - Build for shannon
  shmobile_defconfig          - Build for shmobile
  simpad_defconfig            - Build for simpad
  socfpga_defconfig           - Build for socfpga
  sp7021_defconfig            - Build for sp7021
  spear13xx_defconfig         - Build for spear13xx
  spear3xx_defconfig          - Build for spear3xx
  spear6xx_defconfig          - Build for spear6xx
  spitz_defconfig             - Build for spitz
...
```

Za *Cyclone V* platformu, najbliža predefinisana konfiguracija je `socfpga_defconfig`,
pa se ona preporučuje kao polazna za konfiguraciju *Linux* kernela. Aktiviramo je komandom:

```
make socfpga_defconfig`
```

Sada možemo pokrenuti komandu `make menuconfig` kako bismo prilagodili opcije u konfiguraciji
kernela.

> [!TIP]
> Alternativno, za komforniji interfejs, možete koristiti `make xconfig`. Za korišćenje ovog tipa
konfiguracionog menija, potrebno je da imate instaliran Qt5 softverski paket na razvojnoj platformi.
U slučaju *Ubuntu 22.04* distribucije, možete da koristite komandu `sudo apt install qtbase5-dev`
za instalaciju ovog softverskog paketa.

Prvo ćemo definisati lokalnu verziju kernela tako što postavljamo opciju **Local version - append to kernel release**
(parametar `CONFIG_LOCALVERSION`), koja se nalazi u sklopu kategorije **General setup**,
na *-etfbl-lab*. Takođe, u istoj kategoriji, isključite opciju **Automatically append version information to the version string**
ako je omogućena.

U sklopu kategorije **Boot options**, pronađite opciju **Default kernel command string** i
postavite neki proizvoljan string (npr. *console*). Konačno, ispod ove opcije će se pojaviti, nova
stavka pod nazivom **Kernel command line type**, koju trebate postaviti na **Use bootloader kernel arguments if available**.
Na ovaj način definišemo da kernel dobija argumente komandne linije isključivo od *bootloader*-a
(konfiguracioni parametar `CONFIG_CMDLINE_FROM_BOOTLOADER`).

> [!NOTE]
> Ukoliko opcija **Default kernel command string** nije postavljena, nećete moći da odaberete
`CONFIG_CMDLINE_FROM_BOOTLOADER` parametar, a kernel će da koristi argumente komandne linije
definisane u okviru `chosen` čvora unutar *Device Tree* strukture.

Nakon što smo konfigurisali prethodno pomenute parametre kernela, možemo pokrenuti proces
kroskompajliranja komandom `make`:

```
make -j 4
```

> [!NOTE]
> Da bi ubrzali proces kroskompajliranja, komandi `make` možete proslijediti broj paralelnih
niti (opcija `-j N`, gdje je `N` broj paralelnih niti). Optimalan broj paralelnih niti odgovara
dvostrukom broju jezgara procesora. Prethodna komanda pretpostavlja da na raspolaganju imamo
procesor sa dva jezgra.

> [!NOTE]
> Komanda `make` generiše fajlove koji su označeni `*` u listi opcija koju dobijamo komandom
`make help`. Podrazumijevano, generiše se samo kompresovana slika kernela `zImage`. Ukoliko
je potrebno da generišete nekompresovanu sliku i nekompresovanu sliku sa *U-Boot* zaglavljem
(koja se koristi prilikom pokretanja sistema komandom `bootm`), potrebno je da koristite
komande `make Image` i `make uImage`, respektivno.

Nakon kompajliranja, provjerite verziju kernela komandom

```
make kernelversion
```

koja bi trebalo da prikaže `6.1.38`. Slično, komandom

```
make kernelrelease
```

prikazujemo lokalnu verziju kernela. Kao rezultat, trebalo bi da dobijete verziju kernela na
koju je dodan string definisan u parametru `CONFIG_LOCALVERSION`, tj. `6.1.38-etfbl-lab+`.

Kao rezultat kompajliranja dobijamo određeni broj fajlova raspoređenih na različitim lokacijama
unutar izvornog koda kernela. Za ovu laboratorijsku vježbu najznačajniji su: kompresovana slika
kernela (`zImage`) koja se u datom slučaju nalazi u folderu `arch/arm/boot`, te binarna verzija
*Device Tree* strukture koja opisuje hardver sistema (tzv. *Device Tree Blob* sa ektenzijom
`.dtb`) koja je smještena u folderu `arch/arm/boot/dts`.

S obzirom da želimo da *U-Boot* učita sliku kernela i odgovarajući `.dtb` fajl preko mreže,
potrebno je kopirati pomenute fajlove u folder `/srv/tftp` koji je vidljiv na ciljnoj platformi
preko TFTP protkola.

Prvo kopiramo kompresovanu binarnu sliku kernela sljedećom komandom.

```
sudo cp arch/arm/boot/zImage /srv/tftp/
```

Zatim kopiramo fajl `socfpga_cyclone5_sockit.dtb` koji najbliže odgovara *DE1-SoC* ploči
koju koristimo u laboratorijskoj vježbi.

```
sudo cp arch/arm/boot/dts/socfpga_cyclone5_sockit.dtb /srv/tftp/
```

> [!NOTE]
> Ukoliko student nije u mogućnosti da učita pomenute fajlove preko mreže, potrebno je
iste kopirati na FAT32 particiju na prethodno kreiranoj SD kartici.

## Pokretanje *Linux* jezgra iz *U-Boot* konzole

Nakon što su relevantni fajlovi kopirani u folder eksportovan preko TFTP protokola
(odnosno na FAT32 particiju ukoliko podižemo sistem sa SD kartice), potrebno je da
povežemo napajanje ploče, te USB serijski kabl i mrežni kabl sa razvojnom platformom.
Uključenjem napajanja, pokreće se SPL i *U-Boot*, nakon čega prvo postavljamo varijablu
*U-Boot* okruženja `bootargs` koja sadrži argumente koji se kernelu prosljeđuju kroz
komandnu liniju.

```
setenv bootargs 'console=ttyS0,115200n8;'
```

Da bi trajno sačuvali ovu varijablu, potrebno je da sačuvamo trenutno okruženje u
stalnu memoriju.

```
=> saveenv
Saving Environment to MMC... Writing to MMC(0)... OK
```

Nakon toga, koristimo `ping` komandu da testiramo mrežnu konekciju.

```
=> ping 192.168.21.101
Speed: 1000, full duplex
Using ethernet@ff702000 device
host 192.168.21.101 is alive
```

Ukoliko postoji komunikacija preko mrežnog interfejsa, možemo inicirati učitavanje
kompresovane binarne slike jezgra u RAM memoriju na lokaciju 0x01000000.

```
=> tftp 0x01000000 zImage
Speed: 1000, full duplex
Using ethernet@ff702000 device
TFTP from server 192.168.21.101; our IP address is 192.168.21.100
Filename 'zImage'.
Load address: 0x1000000
Loading: #################################################################
         #################################################################
         #################################################################
         #################################################################
         #################################################################
         #################################################################
         ########
         3.1 MiB/s
done
Bytes transferred = 5829984 (58f560 hex)
```

Nakon toga, učitavamo i `socfpga_cyclone5_sockit.dtb` fajl počevši od adrese
0x02000000.

```
=> tftp 0x02000000 socfpga_cyclone5_sockit.dtb
Speed: 1000, full duplex
Using ethernet@ff702000 device
TFTP from server 192.168.21.101; our IP address is 192.168.21.100
Filename 'socfpga_cyclone5_sockit.dtb'.
Load address: 0x2000000
Loading: ##
         678.7 KiB/s
done
Bytes transferred = 27808 (6ca0 hex)
```

Konačno, možemo da pokrenemo proces podizanja kernela korišćenjem komande `bootz`,
jer se radi o kompresovanoj slici kernela.

`bootz 0x01000000 - 0x02000000`

Nakon toga bi trebalo da počne izvršavanje koda kernela i inicijalizacija sistema.
Ispod je prikazana sekvenca logova koju bi trebalo da dobijete na serijskom terminalu
(veći dio je izostavljen zbog uštede prostora).

```
Kernel image @ 0x1000000 [ 0x000000 - 0x58f560 ]
## Flattened Device Tree blob at 02000000
   Booting using the fdt blob at 0x2000000
Working FDT set to 2000000
   Loading Device Tree to 09ff6000, end 09fffc9f ... OK
Working FDT set to 9ff6000

Starting kernel ...

Deasserting all peripheral resets
[    0.000000] Booting Linux on physical CPU 0x0
[    0.000000] Linux version 6.1.38-etfbl-lab+ (mknezic@Ubuntu-Dev) (arm-linux-gcc (crosstool-NG 1.26.0) 13.2.0, GNU ld (crosstool-NG 1.26.0) 2.40) #5 SMP Mon Apr 15 13:38:28 CEST 2024
[    0.000000] CPU: ARMv7 Processor [413fc090] revision 0 (ARMv7), cr=10c5387d
[    0.000000] CPU: PIPT / VIPT nonaliasing data cache, VIPT aliasing instruction cache
[    0.000000] OF: fdt: Machine model: Terasic SoCkit
[    0.000000] Memory policy: Data cache writealloc
[    0.000000] Zone ranges:
[    0.000000]   Normal   [mem 0x0000000000000000-0x000000002fffffff]
[    0.000000]   HighMem  [mem 0x0000000030000000-0x000000003fffffff]
[    0.000000] Movable zone start for each node
[    0.000000] Early memory node ranges
[    0.000000]   node   0: [mem 0x0000000000000000-0x000000003fffffff]
[    0.000000] Initmem setup node 0 [mem 0x0000000000000000-0x000000003fffffff]
[    0.000000] percpu: Embedded 15 pages/cpu s29780 r8192 d23468 u61440
[    0.000000] Built 1 zonelists, mobility grouping on.  Total pages: 260608
[    0.000000] Kernel command line: console=ttyS0,115200n8;
[    0.000000] Dentry cache hash table entries: 131072 (order: 7, 524288 bytes, linear)
[    0.000000] Inode-cache hash table entries: 65536 (order: 6, 262144 bytes, linear)
[    0.000000] mem auto-init: stack:all(zero), heap alloc:off, heap free:off
[    0.000000] Memory: 1025044K/1048576K available (9216K kernel code, 820K rwdata, 1948K rodata, 1024K init, 168K bss, 23532K reserved, 0K cma-reserved, 262144K highmem)
[    0.000000] SLUB: HWalign=64, Order=0-3, MinObjects=0, CPUs=2, Nodes=1
...
[    1.820800] /dev/root: Can't open blockdev
[    1.824914] VFS: Cannot open root device "(null)" or unknown-block(0,0): error -6
[    1.832418] Please append a correct "root=" boot option; here are the available partitions:
[    1.840763] 0100            8192 ram0
[    1.840773]  (driver?)
[    1.846880] 0101            8192 ram1
[    1.846888]  (driver?)
[    1.850666] mmc_host mmc0: Bus speed (slot 0) = 50000000Hz (slot req 50000000Hz, actual 50000000HZ div = 0)
[    1.853012] Kernel panic - not syncing: VFS: Unable to mount root fs on unknown-block(0,0)
[    1.853023] CPU: 0 PID: 1 Comm: swapper/0 Not tainted 6.1.38-etfbl-lab+ #5
[    1.853034] Hardware name: Altera SOCFPGA
[    1.853046]  unwind_backtrace from show_stack+0x18/0x1c
[    1.853078]  show_stack from dump_stack_lvl+0x40/0x4c
[    1.853101]  dump_stack_lvl from panic+0x110/0x328
[    1.853124]  panic from mount_block_root+0x17c/0x214
[    1.853147]  mount_block_root from prepare_namespace+0x158/0x194
[    1.853166]  prepare_namespace from kernel_init+0x20/0x138
[    1.853186]  kernel_init from ret_from_fork+0x14/0x2c
[    1.853203] Exception stack(0xf0831fb0 to 0xf0831ff8)
[    1.853213] 1fa0:                                     00000000 00000000 00000000 00000000
[    1.853222] 1fc0: 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
[    1.853231] 1fe0: 00000000 00000000 00000000 00000000 00000013 00000000
[    1.862692] CPU1: stopping
[    1.862699] CPU: 1 PID: 32 Comm: kworker/1:1 Not tainted 6.1.38-etfbl-lab+ #5
[    1.862710] Hardware name: Altera SOCFPGA
[    1.862715] Workqueue: events_freezable mmc_rescan
[    1.862735]  unwind_backtrace from show_stack+0x18/0x1c
[    1.862758]  show_stack from dump_stack_lvl+0x40/0x4c
[    1.862777]  dump_stack_lvl from do_handle_IPI+0x2a8/0x2dc
[    1.862797]  do_handle_IPI from ipi_handler+0x20/0x28
[    1.862814]  ipi_handler from handle_percpu_devid_irq+0x94/0x1e0
[    1.862834]  handle_percpu_devid_irq from generic_handle_domain_irq+0x30/0x40
[    1.862858]  generic_handle_domain_irq from gic_handle_irq+0x7c/0x90
[    1.862879]  gic_handle_irq from generic_handle_arch_irq+0x34/0x44
[    1.862897]  generic_handle_arch_irq from call_with_stack+0x18/0x20
[    1.862917]  call_with_stack from __irq_svc+0x98/0xb0
[    1.862931] Exception stack(0xf0919c18 to 0xf0919c60)
[    1.862939] 9c00:                                                       60000093 2ea60000
[    1.862950] 9c20: 00000000 c0d76448 0000055c c0ed0248 00000001 c0ed03a0 f0919d2b 0000006f
[    1.862961] 9c40: c123abc0 60000013 00000000 f0919c68 c0173bb4 c0173bb8 60000013 ffffffff
[    1.862967]  __irq_svc from console_emit_next_record.constprop.0+0x1d8/0x2e0
[    1.862986]  console_emit_next_record.constprop.0 from console_unlock+0x118/0x250
[    1.863005]  console_unlock from vprintk_emit+0x19c/0x218
[    1.863023]  vprintk_emit from dev_vprintk_emit+0x128/0x14c
[    1.863041]  dev_vprintk_emit from dev_printk_emit+0x34/0x60
[    1.863055]  dev_printk_emit from __dev_printk+0x5c/0x78
[    1.863069]  __dev_printk from _dev_info+0x48/0x74
[    1.863083]  _dev_info from dw_mci_setup_bus+0x204/0x210
[    1.863110]  dw_mci_setup_bus from dw_mci_set_ios+0x174/0x220
[    1.863136]  dw_mci_set_ios from mmc_sd_init_card+0x1ec/0x894
[    1.863167]  mmc_sd_init_card from mmc_attach_sd+0xe0/0x180
[    1.863193]  mmc_attach_sd from mmc_rescan+0x254/0x31c
[    1.863214]  mmc_rescan from process_one_work+0x1f4/0x4d0
[    1.863239]  process_one_work from worker_thread+0x5c/0x558
[    1.863266]  worker_thread from kthread+0xd8/0xf4
[    1.863293]  kthread from ret_from_fork+0x14/0x2c
[    1.863309] Exception stack(0xf0919fb0 to 0xf0919ff8)
[    1.863317] 9fa0:                                     00000000 00000000 00000000 00000000
[    1.863327] 9fc0: 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
[    1.863335] 9fe0: 00000000 00000000 00000000 00000000 00000013 00000000
[    2.168638] ---[ end Kernel panic - not syncing: VFS: Unable to mount root fs on unknown-block(0,0) ]---
```

Prva stvar koju možemo primjetiti je da verzija kernela (`Linux version 6.1.38-etfbl-lab+`) odgovara
verziji dobijenoj nakon kroskompajliranja. Takođe, dobijamo i informacije o mašini na kojoj je kernel
kroskompajliran, kao i o tome koji je *toolchain* korišćen.

Dalje, ako pretražite log i nađete `Kernel command line`, vidjećete da argumenti komandne linije
odgovaraju onim koji su postavljeni u varijabli `bootargs`.

Konačno, vidimo da kernel prijavljuje `Kernel panic`, jer nije uspio da montira korjeni fajl sistem što
dovodi do prekida procesa podizanja sistema. Ovo je očekivano ponašanje, a grešku ćemo otkloniti
u sljedećoj vježbi kada kreiramo odgovarajući *root* fajl sistem.

Ako fajlove učitavate sa SD kartice, potrebno je da koristite sljedeću sekvencu komandi:

```
mmc rescan
fatload mmc 0:1 0x01000000 zImage
fatload mmc 0:1 0x02000000 socfpga_cyclone5_sockit.dtb
bootz 0x01000000 - 0x02000000
```

## Automatizacija procesa podizanja *Linux* kernela korišćenjem *U-Boot* skripti

Jasno je da nije praktično unositi prethodnu sekvencu komandi pri svakom pokretanju sistema, iako je sam
proces vrlo jednostavan. Potrebno je da automatizujemo proces tako da *U-Boot* automatski izvrši
prethodne komande svaki put kada pokrenemo sistem, bez interakcije sa korisnikom.

Prvo ćemo redefinisati varijablu `bootdelay` i postaviti je na vrijednost 5, tako da imamo dovoljno
vremena da prekinemo proces automatskog podizanja sistema i pređemo u *U-Boot* konzolu ukoliko
je to nephodno.

```
setenv bootdelay 5
```

Nakon toga ćemo prethodne komande objediniti u mini skriptu koja treba da bude smještena u varijablu
`bootcmd` (svaka komanda treba da bude odvojena sa `;`). U našem slučaju, skripta ima sljedeći
izgled. 

```
setenv bootcmd 'ping $serverip; tftp 0x01000000 zImage; tftp 0x02000000 socfpga_cyclone5_sockit.dtb; bootz 0x01000000 - 0x02000000;'
```

> [!NOTE]
> Iako komanda `ping` ovdje nije obavezna, pokazuje se da se inicijalizacija mrežnog interfejsa brže obavlja
ukoliko koristimo ovu komandu. Takođe, skrećemo pažnju da se kod ove komande referencira prethodno definisana
varijabla `serverip` u kojoj se čuva IP adresa TFTP servera.

Ostaje još da sačuvamo trenutno okruženje

```
=> saveenv
Saving Environment to MMC... Writing to MMC(0)... OK
```

i da restartujemo sistem. Nakon odbrojavanja 5 sekundi (definisano varijablom `bootdelay`) komande iz
skripte bi trebalo automatski da se izvrše, što rezultuje podizanjem sistema. Ukoliko korisnik želi,
može da prekine proces podizanja sistema pritiskom bio kojeg tastera prije isteka vremena od 5 sekundi
i biće postavljen u *U-Boot* konzolu.

> [!NOTE]
> Prilikom editovanja varijabli *U-Boot* okruženja, preporučujemo da koristite komandu `editenv` za
interaktivno editovanje sadržaja varijable ukoliko trebate samo da promijenite jedan dio (npr. adrese
lokacija na koje će se učitati kernel i `.dtb` fajl u RAM memoriju ili nazive ovih fajlova). Na taj
način značajno ubrzavate proces rada.
