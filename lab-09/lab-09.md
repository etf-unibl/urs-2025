# Laboratorijska vježba 9

Cilj laboratorijske vježbe je da se studenti upoznaju sa postupkom debagovanja i *tracing*-a u
sistemima za ugrađene sisteme bazirane na *Linux* operativnom sistemu.

Nakon uspješno realizovane vježbe studenti će biti sposobni da:
1. debaguju *embedded* aplikacije korišćenjem `gdb` alata,
2. koriste `strace` alatku za praćenje sistemskih poziva,
3. urade osnovno profilisanje izvršavanja programa u sistemu pomoću `perf` alata i
4. koriste `ftrace` infrastrukturu za praćenje i analiziranje ponašanja sistema.

## Preduslovi za izradu vježbe

Za uspješno izvođenje laboratorijske vježbe potrebno je sljedeće:

- razvojna ploča *DE1-SoC* sa pripadajućim napajanjem i kablom za povezivanje računara sa
UART interfejsom na ploči,
- pripremljen *toolchain* za kroskompajliranje sofvera za ciljnu platformu,
- pripremljeno *Buildroot* okruženje za *DE1-SoC* ploču,
- SD kartica i
- *Ethernet* mrežni interfejs na razvojnoj platformi i mrežni kabl za povezivanje sa
pločom.

## Priprema *Buildroot* okruženja za debagovavanje i profilisanje

Pozicionirajte se u *Buildroot* folder i pokrenite `make menuconfig` komandu. Izmjenite konfiguraciju tako što
ćete postaviti sljedeće opcije:

- U okviru **Toochain**:
	- uključite opciju **Copy gdb server to the Target**
- U okviru **Build options**:
	- uključite opciju **build packages with debugging symbols**
    - uključite opciju **strip target binaries** ako već nije uključena
- U okviru **Kernel**:
    - uključite opciju **Linux Kernel Tools**&rarr;**perf**
- U okviru **Target packages**:
	- uključite opciju **Debugging, profiling and benchmark**&rarr;**strace**

> [!NOTE]
> Može se desiti da je za *root* fajl sistem potrebno povećati veličinu. Ukoliko se pojavi greška ovog tipa
prilikom generisanja slike, povećajte vrijednost **Filesystem images**&rarr;**exact size** (npr. na 200).

Sačuvajte ovako izmijenjenu konfiguraciju i pokrenite komandu `make`. Po završetku generisanja, kopirajte novu
sliku na SD karticu.

Sačuvajte prethodne izmjene na granu u okviru repozitorijuma kursa.

## Debagovanje korišćenjem GDB alata

Preuzmite izvorni kod testne aplikacije koji se nalazi u folderu `lab-09/hello-gdb` u repozitorijumu kursa.
Kroskompajlirajte ovu aplikaciju sa uključenim informacijama neophodnim za debagovanje (opcija `-g` kompajlera).
Na ciljnu platformu ćemo kopirati stripovanu verziju ovog izvršnog fajla (korišćenjem `arm-linux-strip` alatke).
Međutim, na razvojnoj platformi treba da zadržimo verziju koja sadrži informacije neophodne za debagovanje.

Prebacite stripovanu verziju generisanog izvršnog fajla u folder `/home` na ciljnoj platformi. Jedan način da
to uradite je korišćenjem *Buildroot overlay* koncepta. Međutim, s obzirom da imamo osposobljenu SSH konekciju
preko `dropbear` servera, možemo direktno da koristimo SCP protokol za kopiranje fajla na ciljnu platformu
korišćenjem sljedeće komande:

```
scp hello-gdb root@192.168.21.100:/home
```

Pokrenite ploču, prebacite se u `/home` folder na ciljnoj platformi i pokrenite izvršavanje programa sa argumentom
u komandnoj liniji koji sadrži jedan karakter kao što je ilustrovano ispod.

```
# ./hello-gdb a
Segmentation fault (core dumped)
```

Jasno je da u programu postoji defekt koji rezultuje *Segmentation fault* greškom. Pokrenućemo sesiju za debagovanje
programa kako bismo otkrili uzrok problema.

Prvi korak je pokretanje `gdbserver` programa na ciljnoj platformi. Koristićemo mrežnu konekciju i definisati port
10000 za komunikaciju. U isto vrijeme ćemo specificirati i program koji debagujemo sa argumentima komandne linije.

```
# gdbserver :10000 ./hello-gdb a
Process ./hello-gdb created; pid = 177
Listening on port 10000
```

Sada je na razvojnoj mašini potrebno pokrenuti GDB klijent za ciljnu platformu kojem se proslljeđuje izvršni fajl
koji sadrži informacije neophodne za debagovanje (u datom slučaju GDB klijent pokrećemo unutar foldera u kojem se
nalazi pomenuti izvršni fajl).

```
arm-linux-gdb hello-gdb
```

> [!NOTE]
> GDB klijent je dio ranije generisanog *toolchain*-a, pa je potrebno eskportovati putanju do njega prije
korišćenja.


Nakon pokretanja, dobijamo informaciju o uspješno učitanom fajlu i simbolima za debagovanje, nakon čega možemo
početi sa procesom debagovanja.

```
GNU gdb (crosstool-NG 1.26.0) 13.2
Copyright (C) 2023 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "--host=x86_64-build_pc-linux-gnu --target=arm-etfbl-linux-gnueabihf".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<https://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word"...
Reading symbols from hello-gdb...
(gdb) 
```

Prvi korak je da se povežemo sa serverom na ciljnoj platformi korišćenjem IP adrese ploče i prethodno definisanog
porta.

```
(gdb) target remote 192.168.21.100:10000
Remote debugging using 192.168.21.100:10000
Reading symbols from /home/mknezic/x-tools/arm-etfbl-linux-gnueabihf/arm-etfbl-linux-gnueabihf/sysroot/lib/ld-linux-armhf.so.3...
0xb6f67a78 in _start ()
   from /home/mknezic/x-tools/arm-etfbl-linux-gnueabihf/arm-etfbl-linux-gnueabihf/sysroot/lib/ld-linux-armhf.so.3
```

Nakon toga, postavljamo lokaciju na kojoj se nalazi `sysroot`. S obzirom da koristimo *Buildroot*, možemo da postavimo
`<path-to/buildroot>/output/staging` koji predstavlja simbolički link do `sysroot` foldera kreiran od strane *Buildroot*-a.

```
(gdb) set sysroot /home/mknezic/buildroot/output/staging
warning: .dynamic section for "/home/mknezic/buildroot/output/staging/lib/ld-linux-armhf.so.3" is not at the expected address (wrong library or version mismatch?)
Reading symbols from /home/mknezic/buildroot/output/staging/lib/ld-linux-armhf.so.3...
Reading symbols from /home/mknezic/buildroot/output/staging/lib/ld-linux-armhf.so.3...
```

Jedna od osnovnih komandi pri debagovanju je postavljanje prekidne tačke (*brakpoint*) komandom `break` ili `b`. Prekidnu tačku
možemo da postavimo na određenu lokaciju u kodu (npr. broj linije u fajlu izvornog koda) ili na ulazu u neku funkciju. U našem
slučaju znamo da imamo `main` funkciju, pa ćemo tu postaviti prekidnu tačku.

```
(gdb) b main
Breakpoint 1 at 0x104a8: file hello-gdb.c, line 8.
```

Sada možemo nastaviti izvršavanje programa (komanda `continue` ili `c`)

```
gdb) c
Continuing.
```

> [!NOTE]
> Kod *native* debagovanja prvo moramo pokrenuti izvršavanje programa komandom `run`. Međutim, kada koristimo udaljeno
debagovanje, program je već pokrenut na ciljnoj platformi kada smo pokrenuli `gdbserver` i odmah je zaustavljeno njegovo
izvršavanje nakon učitavanja, tako da je dovoljno samo da nastavimo sa izvršavanjem.

Očekivano, program se zaustavlja na prekdinoj tački koju smo definisali.

```
Breakpoint 1, main (argc=1, argv=0xbeb46dc4) at hello-gdb.c:8
8	    if ((argc == 1) || (argc > 2))
```

Ako ponovo nastavimo sa izvršavanjem

```
(gdb) c
Continuing.
```

dobijamo sljedeću poruku koja nam informiše o tome u kojoj liniji u kodu se nalazi defekt koji dovodi do *Segmentation fault*
greške.

```
Program received signal SIGSEGV, Segmentation fault.
main (argc=2, argv=0xbe9dadc4) at hello-gdb.c:25
25	            msg[28] = argv[1][0];
```

Jasno je da u liniji 25 pokušavamo da pristupimo 28. elementu niza `msg`. Međutim, on je definisan kao konstantni string, pa je
to uzrok problema.

Nekada je korisno da se dobije trag o pozivima funkcija komandom `backtrace` ili `bt`.

```
(gdb) bt
#0  main (argc=2, argv=0xbe9dadc4) at hello-gdb.c:2
```

U našem slučaju imamo samo poziv `main` funkcije, pa su informacije o stanju steka ograničene.

Takođe, možemo da prikazujemo sadržaj varijabli, npr.

```
(gdb) print msg
$1 = 0x105f4 "Hello GDB World! I received x.\n"
```

pri čemu dobijamo njen sadržaj (`"Hello GDB World! I received x.\n"`), kao i adresu u memoriji (`0x105f4`).

Tokom debagovanja, možemo da prikazujemo i ostale korisne informacije komandom `info`. Npr. ako želimo da prikažemo
trenutno definisane tačke prekida, koristimo sljedeću komandu:

```
(gdb) info breakpoints
Num     Type           Disp Enb Address    What
1       breakpoint     keep y   0x000104a8 in main at hello-gdb.c:8
	breakpoint already hit 1 time
```

Ekvivalentno, možemo da dobijemo informacije o nitima koje se trenutno izvršavaju

```
(gdb) info threads
  Id   Target Id                        Frame 
* 1    Thread 196.196 "hello-gdb.strip" main (argc=2, argv=0xbe9dadc4)
    at hello-gdb.c:25
```

odnosno o dinamičkim bibliotekema koje program koristi

```
(gdb) info sharedlibrary
From        To          Syms Read   Shared Object Library
0xb6f10ac0  0xb6f2faf0  Yes         /home/mknezic/x-tools/arm-etfbl-linux-gnueabihf/arm-etfbl-linux-gnueabihf/sysroot/lib/ld-linux-armhf.so.3
0xb6dae240  0xb6eca108  Yes         /home/mknezic/x-tools/arm-etfbl-linux-gnueabihf/arm-etfbl-linux-gnueabihf/sysroot/lib/libc.so.6
```

Po završetku debagovanja, naredbom `quit` završavamo sesiju (s obzirom da u našem slučaju prgram nije adekvatno terminisan
potrebno je da odaberemo `y` da završimo njegovo izvršavanje).

```
(gdb) quit
A debugging session is active.

	Inferior 1 [process 196] will be killed.

Quit anyway? (y or n) y
```

Popravite prethodno uočeni defekt, prekompajlirajte program i potvrdite da se izvršava bez greške na ciljnoj platformi.

Prije prelaska na sljedeći dio vježbe, sačuvajte sve izmjene na granu u repozitorijumu kursa.

## Praćenje sistemskih poziva pomoću `strace` alata

Alatka `strace` nam omogućava praćenje sistemskih poziva pri izvršavanju nekog programa. Iskoristićemo prethodni `hello-gdb`
program da ilustrujemo način korišćenja ove alatke.

Pokrenite ploču i prebacite se u folder u kojem se nalazi `hello-gdb` program. Pokrenite njegovo izvršavanje uz korišćenje
`strace` alatke.

```
# strace ./hello-gdb a
execve("./hello-gdb", ["./hello-gdb", "a"], 0xbecb2dd4 /* 13 vars */) = 0
brk(NULL)                               = 0x13000
mmap2(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0xb6f7e000
access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_LARGEFILE|O_CLOEXEC) = -1 ENOENT (No such file or directory)
openat(AT_FDCWD, "/lib/libc.so.6", O_RDONLY|O_LARGEFILE|O_CLOEXEC) = 3
read(3, "\177ELF\1\1\1\0\0\0\0\0\0\0\0\0\3\0(\0\1\0\0\0\234*\2\0004\0\0\0"..., 512) = 512
statx(3, "", AT_STATX_SYNC_AS_STAT|AT_NO_AUTOMOUNT|AT_EMPTY_PATH, STATX_BASIC_STATS, {stx_mask=STATX_BASIC_STATS|STATX_MNT_ID, stx_attributes=0, stx_mode=S_IFREG|0755, stx_size=12060864, ...}) = 0
mmap2(NULL, 1577984, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0xb6dfc000
mmap2(0xb6f59000, 110592, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x15d000) = 0xb6f59000
mmap2(0xb6f74000, 37888, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0xb6f74000
close(3)                                = 0
set_tls(0xb6f7f0a0)                     = 0
set_tid_address(0xb6f7ec08)             = 153
set_robust_list(0xb6f7ec0c, 12)         = 0
rseq(0xb6f7f080, 0x20, 0, 0xe7f5def3)   = 0
mprotect(0xb6f59000, 106496, PROT_READ) = 0
mprotect(0x11000, 4096, PROT_READ)      = 0
mprotect(0xb6fa5000, 8192, PROT_READ)   = 0
ugetrlimit(RLIMIT_STACK, {rlim_cur=8192*1024, rlim_max=RLIM_INFINITY}) = 0
statx(1, "", AT_STATX_SYNC_AS_STAT|AT_NO_AUTOMOUNT|AT_EMPTY_PATH, STATX_BASIC_STATS, {stx_mask=STATX_BASIC_STATS|STATX_MNT_ID, stx_attributes=0, stx_mode=S_IFCHR|0600, stx_size=0, ...}) = 0
ioctl(1, TCGETS, {c_iflag=ICRNL|IXON|IXOFF|IUTF8, c_oflag=NL0|CR0|TAB0|BS0|VT0|FF0|OPOST|ONLCR, c_cflag=B115200|CS8|CREAD|HUPCL|CLOCAL, c_lflag=ISIG|ICANON|ECHO|ECHOE|ECHOK|ECHOKE, ...}) = 0
getrandom("\x06\xcc\xb1\xb4", 4, GRND_NONBLOCK) = 4
brk(NULL)                               = 0x13000
brk(0x34000)                            = 0x34000
write(1, "Hello GDB World! I received a.\n", 31Hello GDB World! I received a.
) = 31
exit_group(0)                           = ?
+++ exited with 0 +++
```

Inspekcijom prikazanih informacija možemo da dobijemo osjećaj o tome koji sve sistemski pozivi su se koristili pri izvršavanju datog
programa.

Alatku `strace` možemo koristiti i za osnovno profilisanje u kontekstu vremena izvršavanja sistemskih poziva (opcija `-c`). sljedeći
primjer ilustruje profilisanje programa koji se izvršavaju pokretanjem komande `grep linux /usr/lib/* > /dev/null`.

```
# strace -c grep linux /usr/lib/* > /dev/null
% time     seconds  usecs/call     calls    errors syscall
------ ----------- ----------- --------- --------- ----------------
 98.55    0.157597           1     99862        11 read
  0.43    0.000688           9        74         1 openat
  0.37    0.000585           1       472           write
  0.18    0.000288          41         7           mmap2
  0.16    0.000261           3        74           statx
  0.13    0.000210           2        73           close
  0.10    0.000161          40         4           mprotect
  0.02    0.000037           7         5           brk
  0.01    0.000019          19         1           getrandom
  0.01    0.000014          14         1           ugetrlimit
  0.01    0.000013          13         1           set_tls
  0.01    0.000012          12         1           getuid32
  0.01    0.000011          11         1         1 ioctl
  0.01    0.000009           9         1           set_tid_address
  0.01    0.000009           9         1           set_robust_list
  0.01    0.000009           9         1           rseq
  0.00    0.000000           0         1           execve
  0.00    0.000000           0         1         1 access
------ ----------- ----------- --------- --------- ----------------
100.00    0.159923           1    100581        14 total
```

Iz datog primjera možemo da zaključimo da je dominantno korišćen `read` sistemski poziv. Takođe, možemo da dobijemo i druge
korisne informacije o izvršavanju različitih sistemskih poziva.

## Alati za profilisanje sistema

### Osnovno profilisanje pomoću `top` alatke

Prvi korak u profilisanju sistema je identifikacija aplikacija koje zauzimaju najviše resursa u sistemu. U tom smislu je
najlakše koristiti alatku `top`. Pokretanjem ove komande na ciljnoj platformi dobijamo izvještaj sličan primjeru prikazanom
ispod.

```
Mem: 62736K used, 963336K free, 396K shrd, 3300K buff, 32792K cached
CPU:   0% usr   0% sys   0% nic 100% idle   0% io   0% irq   0% sirq
Load average: 0.00 0.02 0.00 1/57 185
  PID  PPID USER     STAT   VSZ %VSZ %CPU COMMAND
  171     2 root     IW       0   0%   0% [kworker/0:0-eve]
   97     1 root     S    18700   2%   0% /usr/lib/systemd/systemd-udevd
   67     1 root     S    16744   2%   0% /usr/lib/systemd/systemd-journald
  114     1 systemd- S    15316   1%   0% /usr/lib/systemd/systemd-timesyncd
    1     0 root     S     8364   1%   0% {systemd} /sbin/init
  111     1 systemd- S     7256   1%   0% /usr/lib/systemd/systemd-networkd
  113     1 systemd- S     6500   1%   0% /usr/lib/systemd/systemd-resolved
  127     1 dbus     S     4504   0%   0% /usr/bin/dbus-daemon --system --addres
  131     1 root     S     2908   0%   0% -sh
  185   131 root     R     2908   0%   0% top
  132     1 root     S     2432   0%   0% /usr/sbin/dropbear -F -R
   17     2 root     SW       0   0%   0% [migration/1]
    8     2 root     IW<      0   0%   0% [kworker/0:0H-mm]
   50     2 root     IW<      0   0%   0% [kworker/1:2H-kb]
   35     2 root     IW       0   0%   0% [kworker/u4:2-ev]
   32     2 root     IW       0   0%   0% [kworker/1:1-eve]
   13     2 root     IW       0   0%   0% [rcu_sched]
   33     2 root     IW<      0   0%   0% [kworker/1:1H-kb]
   44     2 root     IW       0   0%   0% [kworker/0:2-eve]
   23     2 root     IW       0   0%   0% [kworker/0:1-mm_]
```

Odavde možemo da zaključimo koji procesi su najzastupljeniji u sistemu, kao i njihove identifikatore (PID) za dalju
detaljniju analizu.

### Korišćenje `perf` alatke

Alatka `perf` nam omogućava praćenje i profilisanje kako na sistemskom nivou tako i na nivou aplikacija. Osim toga,
ona sadrži infrastrukturu koja omogućava analizu različith hardverskih i softverskih događaja.

Osnovnu statistiku na nivou sistema u trajanju od 5 sekundi dobijamo ako pokrenemo sljedeću komandu na ciljnoj platformi.

```
# perf stat sleep 5

 Performance counter stats for 'sleep 5':

              3.46 msec task-clock                       #    0.001 CPUs utilized
                 1      context-switches                 #  289.410 /sec
                 0      cpu-migrations                   #    0.000 /sec
                55      page-faults                      #   15.918 K/sec
           2749002      cycles                           #    0.796 GHz
           1208886      instructions                     #    0.44  insn per cycle
             92000      branches                         #   26.626 M/sec
             39147      branch-misses                    #   42.55% of all branches

       5.004386710 seconds time elapsed

       0.004796000 seconds user
       0.000000000 seconds sys
```

Kao što možemo da vidimo, dobijamo korisne informacije o ponašanju kompletnog sistema.

Takođe, možemo i da logujemo informacije dobijene prfilisanjem određenog događaja koji se specificira opcijom
`-e` u komandnoj liniji. Npr. ako želimo da snimimo aktivnost svih događaja koji počinju sa `sched:sched_process_`
u trajanju od 5 sekundi, onda možemo da koristimo sljedeću komandu:

```
# perf record -e 'sched:sched_process_*' -a sleep 5
[ perf record: Woken up 1 times to write data ]
[ perf record: Captured and wrote 0.026 MB perf.data (2 samples) ]
```

Snimljene rezultata nakon toga možemo prikazati komandom `perf report`.

```
# perf report
# To display the perf.data header info, please use --header/--header-only option
#
#
# Total Lost Samples: 0
#
# Samples: 1  of event 'sched:sched_process_exit'
# Event count (approx.): 1
#
# Overhead  Command  Shared Object      Symbol
# ........  .......  .................  ..................................
#
   100.00%  sleep    [kernel.kallsyms]  [k] __traceiter_sched_process_exit


# Samples: 1  of event 'sched:sched_process_exec'
# Event count (approx.): 1
#
# Overhead  Command  Shared Object      Symbol
# ........  .......  .................  ..................................
#
   100.00%  sleep    [kernel.kallsyms]  [k] __traceiter_sched_process_exec
```

Listu svih podržanih događaja dobijamo komandom `perf list`.

```
#perf list

List of pre-defined events (to be used in -e or -M):

  branch-instructions OR branches                    [Hardware event]
  branch-misses                                      [Hardware event]
  cache-misses                                       [Hardware event]
  cache-references                                   [Hardware event]
  cpu-cycles OR cycles                               [Hardware event]
  instructions                                       [Hardware event]
  stalled-cycles-backend OR idle-cycles-backend      [Hardware event]
  stalled-cycles-frontend OR idle-cycles-frontend    [Hardware event]

  alignment-faults                                   [Software event]
  bpf-output                                         [Software event]
  cgroup-switches                                    [Software event]
[...]
```

> [!NOTE]
> Detaljnije informacije o opcijama `perf` komande mogu se pronaći u zvaničnim *man* stranicama ([perf(1)](https://www.man7.org/linux/man-pages/man1/perf.1.html)
i [perf-record(1)](https://man7.org/linux/man-pages/man1/perf-record.1.html)).

### Korišćenje `ftrace` alata

Alat `ftrace` omogućava praćenje funkcija i događaja unutar kernela. Ukoliko je ova opcija omogućena u konfiguraciji kernela,
prilikom pokretanja sistema se montira virtuelni fajl sistem (*debugfs*) koji sadrži interfejs ka ovoj infrastrukturi. Tačka
montiranja ovog fajl sistema je `/sys/kernel/debug/tracing`

Listu dostupnih *tracer*-a možemo dobiti sljedećom komandom na ciljnoj platformi

```
# cat /sys/kernel/debug/tracing/available_tracers
function_graph function nop
```

Da bismo omogućili određeni *tracer*, potrebno je da ga upišemo u fajl `current_tracer`.

```
# echo function > /sys/kernel/debug/tracing/current_tracer
```

Nakon toga uključujemo *tracing* (fajl `tracing_on`). Pokrećemo `sleep` za interval u kojem želimo da pratimo sistem (npr. 1 sekunda),
nakon čega je potrebno da opet isključimo *tracing*.

```
# echo 1 > /sys/kernel/debug/tracing/tracing_on
# sleep 1
# echo 0 > /sys/kernel/debug/tracing/tracing_on
```

Po završetku, rezultate prikazujemo izlistavanjem fajla `trace`.

```
# cat /sys/kernel/debug/tracing/trace
# tracer: function
#
# entries-in-buffer/entries-written: 144256/572648   #P:2
#
#                                _-----=> irqs-off/BH-disabled
#                               / _----=> need-resched
#                              | / _---=> hardirq/softirq
#                              || / _--=> preempt-depth
#                              ||| / _-=> migrate-disable
#                              |||| /     delay
#           TASK-PID     CPU#  |||||  TIMESTAMP  FUNCTION
#              | |         |   |||||     |         |
          <idle>-0       [001] ..s..  1403.386442: call_timer_fn <-run_timer_softirq
          <idle>-0       [001] ..s..  1403.386443: tcp_orphan_update <-call_timer_fn
          <idle>-0       [001] ..s..  1403.386445: tcp_orphan_count_sum <-tcp_orphan_update
          <idle>-0       [001] ..s..  1403.386446: mod_timer <-call_timer_fn
          <idle>-0       [001] ..s..  1403.386448: lock_timer_base <-__mod_timer
          <idle>-0       [001] ..s..  1403.386449: _raw_spin_lock_irqsave <-lock_timer_base
          <idle>-0       [001] d.s..  1403.386451: detach_if_pending <-__mod_timer
          <idle>-0       [001] d.s..  1403.386452: calc_wheel_index <-__mod_timer
          <idle>-0       [001] d.s..  1403.386453: enqueue_timer <-__mod_timer
          <idle>-0       [001] d.s..  1403.386455: _raw_spin_unlock_irqrestore <-__mod_timer
          <idle>-0       [001] ..s..  1403.386456: _raw_spin_lock_irq <-run_timer_softirq
          <idle>-0       [001] ..s..  1403.386457: run_rebalance_domains <-__do_softirq
          <idle>-0       [001] ..s..  1403.386459: update_blocked_averages <-run_rebalance_domains
          <idle>-0       [001] d.s..  1403.386460: raw_spin_rq_lock_nested <-update_blocked_averages
          <idle>-0       [001] d.s..  1403.386461: _raw_spin_lock <-raw_spin_rq_lock_nested
          <idle>-0       [001] d.s..  1403.386462: update_rq_clock <-update_blocked_averages
          <idle>-0       [001] d.s..  1403.386464: update_rq_clock.part.0 <-update_blocked_averages
          <idle>-0       [001] d.s..  1403.386465: update_rt_rq_load_avg <-update_blocked_averages
[...]
```

S obzirom da se prati svaka funkcija u kernelu, jasno je da ćemo dobiti ogromnu količinu informacija.

Na sličan način možemo da dobijemo informacije o pozivu svake funkcije i vremenu njenog izvršavanja, ako postavimo
`function_graph` kao `current_tracer`.

Podesite ovaj *tracer* i ponovite profilisanje. Sada sadržaj `trace` fajl ima izgled poput litinga datog ispod.

```
# cat /sys/kernel/debug/tracing/trace
# tracer: function_graph
#
# CPU  DURATION                  FUNCTION CALLS
# |     |   |                     |   |   |   |
 1) + 25.010 us   |              } /* load_balance */
 1) + 68.730 us   |            } /* run_rebalance_domains */
 1) + 72.980 us   |          } /* __do_softirq */
 1) + 81.310 us   |        } /* irq_exit */
 1) # 9906.360 us |      } /* arch_cpu_idle */
 1) # 9910.880 us |    } /* default_idle_call */
 1)               |    ledtrig_cpu() {
 1)   1.770 us    |      led_trigger_event();
 1)   1.780 us    |      led_trigger_event();
 1) + 10.210 us   |    }
 1)               |    arch_cpu_idle_enter() {
 1)               |      ledtrig_cpu() {
 1)   1.700 us    |        led_trigger_event();
 1)   1.640 us    |        led_trigger_event();
 1)   9.500 us    |      }
 1)   2.240 us    |      arm_heavy_mb();
 1) + 17.970 us   |    }
 1)               |    default_idle_call() {
 1)               |      arch_cpu_idle() {
 1)   3.460 us    |        irq_enter();
 1)   ==========> |
 1)               |        gic_handle_irq() {
 1)               |          generic_handle_domain_irq() {
 1)   1.710 us    |            __irq_resolve_mapping();
 1)               |            handle_percpu_devid_irq() {
 1)               |              twd_handler() {
 1)               |                hrtimer_interrupt() {
 1)   2.690 us    |                  ktime_get_update_offsets_now();
 1)               |                  __hrtimer_run_queues() {
 1)               |                    tick_sched_timer() {
 1)   2.480 us    |                      ktime_get();
 1)               |                      update_process_times() {
 1)   1.670 us    |                        hrtimer_run_queues();
 1)               |                        rcu_sched_clock_irq() {
 1)   1.750 us    |                          rcu_is_cpu_rrupt_from_idle();
 1)   1.740 us    |                          rcu_qs();
 1)   1.750 us    |                          rcu_is_cpu_rrupt_from_idle();
 1)   1.760 us    |                          rcu_is_cpu_rrupt_from_idle();
 1) + 18.220 us   |                        }
 1)               |                        scheduler_tick() {
 1)   2.040 us    |                          update_rq_clock.part.0();
 1)   1.780 us    |                          calc_global_load_tick();
 1)   1.720 us    |                          perf_event_task_tick();
 1)               |                          trigger_load_balance() {
 1)               |                            raise_softirq() {
 1)   1.710 us    |                              __raise_softirq_irqoff();
 1)   5.590 us    |                            }
 1)   9.610 us    |                          }
 1) + 27.480 us   |                        }
 1)   1.830 us    |                        run_posix_cpu_timers();
 1) + 61.550 us   |                      }
 1)   1.780 us    |                      profile_tick();
 1)   1.780 us    |                      hrtimer_forward();
 1) + 78.460 us   |                    }
 1)   1.910 us    |                    enqueue_hrtimer();
 1) + 88.250 us   |                  }
 1)               |                  hrtimer_update_next_event() {
 1)   1.720 us    |                    __hrtimer_next_event_base.constprop.0();
 1)   1.820 us    |                    __hrtimer_next_event_base.constprop.0();
 1)   9.650 us    |                  }
 1)               |                  tick_program_event() {
 1)               |                    clockevents_program_event() {
 1)   2.520 us    |                      ktime_get();
 1)   7.180 us    |                    }
 1) + 11.310 us   |                  }
 1) ! 124.040 us  |                }
 1) ! 128.200 us  |              }
 1) ! 132.790 us  |            }
 1) ! 140.770 us  |          }
 1) ! 145.070 us  |        }
[...]
```

Vidimo da za svaku funkciju imamo informaciju o vremenu izvršavanja.

Iz prethodnih slučajeva smo mogli da zaključimo da dobijamo ogromnu količinu informacija, tako da postoji potreba
da filtriramo dobijene rezultate i prikažemo samo informacije o određenoj funkciji ili događaju. Alatka `ftrace`
ovo podržava kroz filtriranje.

Radi lakšeg rada, prebacićemo se u folder `/sys/kernel/debug/tracing`.

```
cd /sys/kernel/debug/tracing
```

Prikazaćemo dostupne opcije za filtriranje funkcija (fajl `available_filter_functions`).

```
# cat available_filter_functions
handle_fiq_as_nmi
gic_handle_irq
__do_softirq
__traceiter_initcall_level
__traceiter_initcall_start
__traceiter_initcall_finish
initcall_blacklisted
trace_initcall_finish_cb
do_one_initcall
do_one_initcall
match_dev_by_label
match_dev_by_uuid
rootfs_init_fs_context
name_to_dev_t
wait_for_initramfs
wait_for_initramfs
calibration_delay_done
calibrate_delay
vfp_enable
vfp_dying_cpu
vfp_starting_cpu
vfp_raise_sigfpe
vfp_cpu_pm_notifier
vfp_raise_exceptions
VFP_bounce
vfp_sync_hwstate
vfp_notifier
vfp_flush_hwstate
vfp_preserve_user_clear_hwstate
vfp_restore_user_hwstate
vfp_single_fneg
vfp_single_fabs
vfp_single_fcpy
vfp_compare
vfp_single_fcmpe
vfp_single_fcmp
vfp_propagate_nan
vfp_single_multiply
vfp_single_fcmpez
vfp_single_ftoui
vfp_single_ftouiz
vfp_single_ftosi
vfp_single_ftosiz
vfp_single_fcmpz
vfp_single_add
vfp_single_fcvtd
[...]
```

Kao što možemo da vidimo, postoji ogroman broj funkcija. Međutim, možemo da koristimo džokere. Npr. ako želimo da
filtriramo samo funkcije koje počinju sa `tcp` (npr. ako pratimo ponašanje neke mrežne aplikacije), onda možemo
postaviti `ftrace` filter (fajl `set_ftrace_filter`) na sljedeći način:

```
# echo "tcp*" > set_ftrace_filter
# echo function > current_tracer
# echo 1 > tracing_on
```

Nakon što smo postavili *tracer* i pokrenuli praćenje, možemo da eksperimentišmo sa nekom mrežnom aplikacijom (npr.
povezivanje sa pločom preko SSH veze). Nakon toga, zaustavljamo praćenje

```
# echo 0 > tracing_on
```

i prikazujemo rezultate

```
# cat trace
# tracer: function
#
# entries-in-buffer/entries-written: 4195/4195   #P:2
#
#                                _-----=> irqs-off/BH-disabled
#                               / _----=> need-resched
#                              | / _---=> hardirq/softirq
#                              || / _--=> preempt-depth
#                              ||| / _-=> migrate-disable
#                              |||| /     delay
#           TASK-PID     CPU#  |||||  TIMESTAMP  FUNCTION
#              | |         |   |||||     |         |
          <idle>-0       [001] ..s..  2146.156373: tcp_orphan_update <-call_timer_fn
          <idle>-0       [001] ..s..  2146.156377: tcp_orphan_count_sum <-tcp_orphan_update
          <idle>-0       [001] ..s..  2146.266369: tcp_orphan_update <-call_timer_fn
          <idle>-0       [001] ..s..  2146.266372: tcp_orphan_count_sum <-tcp_orphan_update
          <idle>-0       [001] ..s..  2146.386369: tcp_orphan_update <-call_timer_fn
          <idle>-0       [001] ..s..  2146.386370: tcp_orphan_count_sum <-tcp_orphan_update
          <idle>-0       [001] ..s..  2146.496368: tcp_orphan_update <-call_timer_fn
          <idle>-0       [001] ..s..  2146.496369: tcp_orphan_count_sum <-tcp_orphan_update
          <idle>-0       [001] .ns..  2146.606374: tcp_orphan_update <-call_timer_fn
          <idle>-0       [001] .ns..  2146.606376: tcp_orphan_count_sum <-tcp_orphan_update
          <idle>-0       [001] ..s..  2146.716368: tcp_orphan_update <-call_timer_fn
          <idle>-0       [001] ..s..  2146.716370: tcp_orphan_count_sum <-tcp_orphan_update
          <idle>-0       [001] ..s..  2146.836371: tcp_orphan_update <-call_timer_fn
          <idle>-0       [001] ..s..  2146.836374: tcp_orphan_count_sum <-tcp_orphan_update
          <idle>-0       [001] ..s..  2146.946369: tcp_orphan_update <-call_timer_fn
          <idle>-0       [001] ..s..  2146.946373: tcp_orphan_count_sum <-tcp_orphan_update
          <idle>-0       [000] .ns..  2147.046378: tcp_write_timer <-call_timer_fn
          <idle>-0       [000] .ns..  2147.046382: tcp_write_timer_handler <-tcp_write_timer
          <idle>-0       [000] .ns..  2147.046384: tcp_mstamp_refresh <-tcp_write_timer_handler
          <idle>-0       [000] .ns..  2147.046386: tcp_retransmit_timer <-tcp_write_timer
          <idle>-0       [000] .ns..  2147.046389: tcp_fastopen_active_detect_blackhole <-tcp_retransmit_timer
          <idle>-0       [000] .ns..  2147.046392: tcp_enter_loss <-tcp_retransmit_timer
          <idle>-0       [000] .ns..  2147.046393: tcp_mark_skb_lost <-tcp_enter_loss
          <idle>-0       [000] .ns..  2147.046396: tcp_set_ca_state <-tcp_enter_loss
          <idle>-0       [000] .ns..  2147.046398: tcp_retransmit_skb <-tcp_retransmit_timer
          <idle>-0       [001] ..s..  2147.056375: tcp_orphan_update <-call_timer_fn
          <idle>-0       [001] ..s..  2147.056378: tcp_orphan_count_sum <-tcp_orphan_update
          <idle>-0       [001] ..s..  2147.176368: tcp_orphan_update <-call_timer_fn
          <idle>-0       [001] ..s..  2147.176369: tcp_orphan_count_sum <-tcp_orphan_update
          <idle>-0       [001] ..s..  2147.296369: tcp_orphan_update <-call_timer_fn
[...]
```

Iz datih rezultata jasno vidimo da se prikazuju samo funkcije vezane za TCP komunikaciju.

Na kraju, kada završimo sa radom, dobra je praksa da se trenutni *tracer* postavi na `nop`.

```
echo nop > current_tracer
```


Više informacija u drugim opcijama koje nudi `ftrace` infrastruktura može se pronaći u zvaničnoj dokumentaciji
ovo alata ([Documentation/trace/ftrace.txt](https://www.kernel.org/doc/Documentation/trace/ftrace.txt)).
