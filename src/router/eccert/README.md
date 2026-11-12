# eccert – minimales P-256-Schlüssel-/CSR-/Zertifikatstool

Kleiner Ersatz für `openssl req/x509` auf Routern mit wenig Flash. Nur ECDSA P-256 mit SHA-256,
nur libc (kein stdio, kein malloc) plus ein ECC-Kern:

| Backend | Quelle | Lizenz | Eigenschaften |
|---|---|---|---|
| **p256-m** (`BACKEND=p256m`) | TF-PSA-Crypto `drivers/p256-m`, Commit `4674a53` (2026-11-05), unverändert | Apache-2.0 OR GPL-2.0-or-later | konstante Laufzeit, löscht Geheimnisse, kleiner; Wycheproof fehlerfrei |
| **micro-ecc** (`BACKEND=uecc`) | github.com/kmackay/micro-ecc, Commit `541b3a7` (2024-11-13) | BSD-2-Clause | schneller (~2×), nicht zeitkonstant; lehnt einige Randfälle fälschlich ab |

Das Makefile nimmt p256-m, wenn `p256-m/` vorhanden ist, sonst micro-ecc. `eccert.c` ist für beide identisch.

## Bedienung

```
eccert genkey [-out key.pem]
eccert pub  -key key.pem [-out pub.pem]
eccert req  -key key.pem [-subj /CN=..] [-san ..] [-out csr.pem]
eccert cert (-key key.pem | -csr req.pem) [-subj /C=DE/O=../CN=..]
            [-san DNS:x,IP:1.2.3.4] [-days N (0=nie)] [-ca [-pathlen N]]
            [-cakey ca.key -cacert ca.pem] [-out cert.pem]
```

```
eccert genkey -out https.key
eccert cert -key https.key -subj /CN=192.168.1.1 -out https.pem        # selbstsigniert, SAN automatisch

eccert genkey -out ca.key
eccert cert -key ca.key -ca -pathlen 0 -subj "/O=Firma/CN=Router CA" -days 7300 -out ca.pem
eccert genkey -out srv.key
eccert req  -key srv.key -subj /CN=router.lan -san "DNS:router.lan,IP:192.168.1.1" -out srv.csr
eccert cert -csr srv.csr -cakey ca.key -cacert ca.pem -days 825 -out srv.pem
```

## Zufall und Signaturen

- **Schlüsselerzeugung:** `getrandom()` – blockiert, bis der Kernel-CRNG initialisiert ist. Erst wird
  nicht-blockierend gefragt; ist der Pool noch nicht bereit, erscheint
  `eccert: warning: waiting for kernel entropy pool` und das Tool wartet.
- **Alte Kernel (< 3.17, kein `getrandom()`):** vor dem Lesen von `/dev/urandom` wird per `poll()`
  gewartet, bis `/dev/random` lesbar ist (gleiches Verfahren wie libsodium), mit derselben Warnung.
  Zum Testen dieses Pfads: `CFLAGS="-Os -DECCERT_NO_GETRANDOM"`.
- **Signaturen deterministisch nach RFC 6979:** Die Nonce k wird per HMAC-SHA256 aus privatem
  Schlüssel und Hash abgeleitet und hängt nicht mehr vom Zufall ab – eine schwache Zufallsquelle kann
  beim Signieren keinen Schlüssel mehr verraten. Gleicher Schlüssel + gleiche Daten = identische
  Signatur (CSRs sind byte-identisch reproduzierbar, auch zwischen beiden Backends).
  - p256-m holt k als einen 32-Byte-Abruf aus der Zufallsquelle; während des Signierens liefert
    diese die RFC-6979-Kandidaten. Beim Signieren findet damit überhaupt kein Zufallszugriff statt.
  - micro-ecc: dessen eigenes `uECC_sign_deterministic()` ist **nicht** RFC-6979-konform
    (Kandidat byteverdreht in Little-Endian-Worte kopiert, Hash nicht mod n reduziert; Upstream-Issue #139).
    eccert nutzt daher eigenes RFC 6979 + `uECC_sign_with_k()`. micro-ecc verwendet zusätzlich
    weiter Zufall für Seitenkanal-Blinding; schlägt der fehl, bricht eccert ab.
- Liefert die Zufallsquelle nichts, bricht eccert ab – es entsteht nie ein Schlüssel aus Fehlerdaten.

Empfehlung für den Router: Schlüssel erst beim ersten Start des Webservers erzeugen, nicht in der
ersten Boot-Sekunde; einen Hardware-RNG des SoC per `rng-tools` einspeisen, falls vorhanden.

## Weiteres Verhalten

- Schlüssel: Ausgabe SEC1 (`EC PRIVATE KEY`), Eingabe SEC1 oder PKCS#8. Dateien mit Modus 0600.
- Endzertifikat: basicConstraints CA:FALSE, keyUsage digitalSignature (beide kritisch),
  extKeyUsage serverAuth+clientAuth, SAN, SKI, AKI. CA (`-ca`): CA:TRUE [pathlen],
  keyCertSign+cRLSign+digitalSignature, SKI.
- SAN: `-san` (DNS:, IP: v4/v6, email:) > SAN aus dem CSR > automatisch aus dem CN.
- Subject: C, ST, L, O, OU, CN; ohne `-subj` `/CN=<hostname>`, bei `-csr` das Subject des CSR.
- Ungestellte Uhr: liegt die Zeit vor `ECCERT_MIN_EPOCH` (Default 2026-01-01), wird diese als
  notBefore genommen (Warnung). notBefore 5 min zurück; 64-Bit-Datumsrechnung (2038-sicher);
  `-days 0` = 99991231235959Z. Default-Laufzeit 3650 Tage.
- Prüfungen: CSR-Signatur, `-cakey` passt zu `-cacert`, Public Keys auf Kurvenzugehörigkeit.

## Größe (Bytes)

Zig/clang + musl, `-Os`, gestrippt; squashfs = Mehrbedarf im Image (`mksquashfs -comp xz -b 256k`,
BCJ für arm/arm64/x86). „dynamisch“ = gegen `libc.so` gelinkt wie im Firmware-Build.

| Arch | Linken | micro-ecc roh | p256-m roh | micro-ecc squashfs | p256-m squashfs |
|---|---|---:|---:|---:|---:|
| mips    | statisch  | 43 444 | 35 620 | 18 950 | 15 766 |
| mips    | dynamisch | 41 956 | 33 904 | 18 150 | 14 718 |
| mipsel  | statisch  | 43 476 | 35 652 | 19 066 | 15 786 |
| mipsel  | dynamisch | 41 988 | 33 936 | 18 450 | 14 914 |
| arm     | statisch  | 40 416 | 34 048 | 19 020 | 16 212 |
| arm     | dynamisch | 32 916 | 26 624 | 15 152 | 12 412 |
| aarch64 | statisch  | 34 856 | 28 816 | 18 578 | 15 586 |
| aarch64 | dynamisch | 31 792 | 25 768 | 15 542 | 12 542 |
| x86_64  | statisch  | 34 928 | 28 888 | 18 604 | 15 236 |
| x86_64  | dynamisch | 32 480 | 26 504 | 15 696 | 12 484 |

RFC 6979 und der `/dev/random`-Fallback kosten ca. 1,2–1,7 KB roh bzw. 0,6–0,75 KB im squashfs.
Mit der OpenWrt/DD-WRT-GCC-Toolchain dürfte der dynamische MIPS-Build kleiner ausfallen (Zig baut PIC).

## Bauen und testen

```
make                                     # Host
make CC=mipsel-openwrt-linux-musl-gcc OBJCOPY=mipsel-openwrt-linux-musl-objcopy
make test                                # Regressionstest gegen OpenSSL (tests/check.sh)
make rfc6979                             # RFC-6979-Vektoren + 507 Fälle gegen python-ecdsa
tests/check.sh "qemu-mipsel bin/eccert-mipsel"
```

p256-m: `-DMUL64_IS_CONSTANT_TIME` beschleunigt deutlich, aber nur setzen, wenn der 32×32→64-Multiplizierer
der Ziel-CPU nachweislich zeitkonstant ist. micro-ecc: `UECC_OPT=0` spart ~1 KB.
`OBJCOPY` entfernt die MIPS-`.pdr`-Sektion (~30 KB), die `strip -s` stehen lässt.

`bin/` enthält statische musl-Binaries (mips, mipsel, arm, aarch64, x86_64), alle mit `tests/check.sh`
unter QEMU geprüft.

## Getestet

- `tests/check.sh`: CA → CSR → Zertifikat, `openssl verify -x509_strict -purpose sslserver`, CSR-Verify,
  Public-Key-Vergleich, OpenSSL-Schlüssel (PKCS#8) und -CSRs als Eingabe, manipulierte CSRs und falscher
  CA-Schlüssel werden abgewiesen, deterministische CSRs; TLS-1.3-Handshake mit Hostname-Prüfung.
- RFC 6979 A.2.5 (P-256/SHA-256, „sample“/„test“) und 507 Fälle gegen python-ecdsa inkl. Hash ≥ n
  und Schlüssel 1/2/n−1 – beide Backends, micro-ecc mit 32- und 64-Bit-Worten.
- Backends: Wycheproof ECDSA P-256/SHA-256 und ECDH-Punktvalidierung, 3000 Kreuztests gegen OpenSSL.
  p256-m fehlerfrei; micro-ecc ohne falsche Akzeptanz, aber 2 gültige Randfall-Signaturen,
  3 ECDH-Randfälle und die Schlüssel 1, n−2, n−1 fälschlich abgelehnt.

## SBOM

- p256-m: TF-PSA-Crypto `drivers/p256-m/p256-m`, Commit 4674a5360406776eacd23dc09cec9c386e769a52.
  Keine Mbed-TLS-Advisory nennt p256-m; Scanner, die die Datei Mbed TLS zuordnen, melden ggf. dessen
  TLS-/X.509-CVEs → VEX „not_affected“.
- micro-ecc: Commit 541b3a78026420a3e369c4c9281c396b5e531113. CVE-2020-27209 (Power-Analyse)
  ist mit 1b5f5ce gefixt → VEX „fixed“; Scanner melden es wegen der NVD-Versionsangabe „1.0“ trotzdem.

Nicht enthalten: RSA, andere Kurven, CRLs, Zertifikatsanzeige, DER-Ausgabe, verschlüsselte Schlüssel.
