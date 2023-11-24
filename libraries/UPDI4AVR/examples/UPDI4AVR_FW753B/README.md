# UPDI4AVR_FW753B (obsolete)

この UPDI4AVR ファームウェアは、以下のバージョンを示す。

This UPDI4AVR firmware shows the following versions:

```sh
# avrdude -vv

JTAG ICE mkII sign-on message:
Communications protocol version: 1
M_MCU:
  boot-loader FW version:        1
  firmware version:              2.1
  hardware version:              1
S_MCU:
  boot-loader FW version:        1
  firmware version:              7.53
  hardware version:              1
Serial number:                   xx:xx:xx:xx:xx:xx
Device ID:                       JTAGICE mkII
```

この`UPDI4AVR_FW753B`は、`AVRDUDE 7.2`以前に適応して作成されたものだが、`7.3`以降での運用には適さず、もはや非標準である。`AVRDUDE 7.3`以降は [UPDI4AVR_FW634B](../UPDI4AVR_FW634B/)での運用が推奨される。

This `UPDI4AVR_FW753B` was created to adapt to `AVRDUDE 7.2` or earlier, but it is not suitable for operation with `7.3` or later and is no longer standard. For `AVRDUDE 7.3` or later, it is recommended to operate with [UPDI4AVR_FW634B](../UPDI4AVR_FW634B/).

## Copyright and Contact

Twitter(X): [@askn37](https://twitter.com/askn37) \
BlueSky Social: [@multix.jp](https://bsky.app/profile/multix.jp) \
GitHub: [https://github.com/askn37/](https://github.com/askn37/) \
Product: [https://askn37.github.io/](https://askn37.github.io/)

Copyright (c) 2023 askn (K.Sato) multix.jp \
Released under the MIT license \
[https://opensource.org/licenses/mit-license.php](https://opensource.org/licenses/mit-license.php) \
[https://www.oshwa.org/](https://www.oshwa.org/)
