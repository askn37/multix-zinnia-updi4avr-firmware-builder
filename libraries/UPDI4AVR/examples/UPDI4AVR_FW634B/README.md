# UPDI4AVR_FW634B

Switch Language [(en_US)](README_en.md) [(ja_JP)](README.md)

この UPDI4AVR ファームウェアは、以下のバージョンを示す。

```sh
# avrdude -vv

JTAG ICE mkII sign-on message:
Communications protocol version: 1
M_MCU:
  boot-loader FW version:        2
  firmware version:              7.53
  hardware version:              2
S_MCU:
  boot-loader FW version:        2
  firmware version:              6.34
  hardware version:              2
Serial number:                   xx:xx:xx:xx:xx:xx
Device ID:                       UPDI4AVR
```

`UPDI4AVR_FW753B`よりも示される`S_MCU FW`バージョン値は小さいが、
`UPDI4AVR_FW634B`の方がより新しく改めて設計された。

## バージョン表記が低くなった理由

`AVRDUDE 7.3`では10数年ぶりに`JTAGICE mkII`プロトコル実装が一部変更され`7.2`以前との完全な互換性が保証されなくなった。元々公式には`XMEGA`系統デバイス以前に対応するプロトコルであり、`UPDI`系統デバイス対応は全くの非公式であるのが主たる理由である。`UPDI4AVR_FW753B`は該当する一部の仕様変更が問題になると判明したことから`AVRDUDE 7.3`に対応することを目的に`UPDI4AVR_FW634B`が改めて作成された。

- `UPDI4AVR_FW753B` はプロトコル変種`FWV=7`（`XMEGA`ベース）に基づいている。
- `UPDI4AVR_FW634B` はプロトコル変種`FWV=6`（`PP/PDI/SPI`ベース）に基づいている。

> 端的に言えば`FWV=7`ベースの応用が稀であることから継続的メンテナンスが困難であり、新規応用開発は非推奨になった。`FWV=7`は`SIB`やデバイス署名を読める以前に未知の対象デバイスの素性を推測できる情報が揃っていたが、`FWV=6`ではそれらが全く含まれない。これは`UPDI`対応デバイスが`XMEGA`技術の発展型であるためだ。しかし`FWV=7`は`XMEGA`デバイスのみを対象とすべきという合意により`UPDI`への発展的応用は困難となった。

### ユーザー定義メモリ設定の非対応

`AVRDUDE 7.2`以前はユーザーが独自のメモリ設定を追加して、`-U`でのメモリタイプ指定や、インタラクティブモードの`dump`、`read`、`write`コマンドで使うことができたが、`7.3`ではこれが使用できないか大幅に応用が制限された。例えば`memory "sib"`は構成ファイルに記述されているにも関わらず、使用できない。

### IOメモリ指定の非互換性

従来の`memory "data"`設定は新設された`memory "io"`メモリ設定の基準オフセットに転用されることになったため、これに依存していた過去の応用実装は構成ファイルを変更するとアドレス計算が異常値を示す。一部の用途は`7.3`で新設された`"sram"`メモリ設定で代替できるが、全てではない。

### メモリ参照順序の逆転

デバイス署名を表す`"signature"`メモリ読み取りより先に、一部の`"io"`メモリ読み取り（具体的にはシリコンリビジョン値取得）が強制的に発生する。これは現状でこそ実用上の問題になっていないが、明らかに`JTAGICE mkII`プロトコル規約に基づいておらず継続的互換性維持が懸念される。`UPDI4AVR`では高電圧制御でこの順序逆転の影響が表面化するため、内部的な対応を要した。

## 非標準拡張

将来的に既存の`JTAGICE mkII`プロトコル実装に代わる制御ドライバーを新規実装する計画を前提に、いくつかの拡張実装が準備されている。

### 拡張パラメータでのSIB読み取り

`SIB`は既存のメモリタイプでは表現できないため、`JTAGICE mkII`プロトコルの一般規定に沿って拡張パラメータでの取得を許した。これはプログラミングモードを開始した後から使用できる。

### 拡張パラメータでのUPDIデバイスディスクリプタ有効化

従来から使用されていない（あるいは無視されている）エミュレーションモードパラメータ取得を用いると、旧式かつ過大な`PP/PDI/SPI`デバイスディスクリプタに代えて、`UPDI`デバイスに最適化したデバイスディスクリプタの利用に変更することができる。これには各種NVMオフセットアドレスやNVMCTRLのバージョン番号を含めることが可能で、`SIB`と照合してダブルチェックをするのに役立つ。

### リセットコマンドでの強制HV制御対応

プログラミングモードに入る前のリセットコマンドの追加パラメータで、HV制御を強制することができる。そういう状態のデバイスではこの段階以前で`SIB`を取得する方法が全くない。

### HV制御の有効化

既存の`AVRDUDE`で`UPDI4AVR`のHV制御機能を有効化するには、以下の後世ファイル設定を使うように変更された。該当する`part`セクションに、本来の用途ではない`idr`記述子が必要とされる。

```sh
part # Any 'UPDI' device parts

  # Shared UPDI pin, HV on _RESET
  hvupdi_variant         = 2;
  idr                    = 0x32;  # substitute hvupdi_variant

  # Valid value : 0x30=tinyAVR、0x31=No HV、0x32=AVR_DD/EA/EB
```

その上で正しいパーツ選択をして`-eF`オプションを併用すると、必要な場面で HV制御が活性化される。これは現行の`PP/PDI/SPI`デバイスディスクリプタに`hvupdi_variant`または`hvupdi_variant`値を含める手段がないことへの代替処置だ。当然ながら非`UPDI`パーツにこれを記述することは許されない。

> `avrdude.conf.UPDI4AVR`には、もれなくこの設定が含まれる。将来的に、`UPDI4AVR`用の公式ドライバが統合されると、この代替処置は不要になる。

## カスタムビルドオプション

`Configuration.h`で以下のビルドオプションを有効化することができる。既定のビルドでは`ENABLE_ADDFEATS_LOCK_SIG`だけが有効。

### ENABLE_ADDFEATS_LOCK_SIG

デバイスが施錠されている場合のデバイス署名は、通常は単に`0xff 0xff 0xff`が返されるだけだが、これを`SIB`に基づく特別な署名に置き換えて返す。これは`avrdude.conf`構成ファイルに次の特別なパーツ登録を加えることで、ヒューマンリーダブルに可視化できることを意味する。現状では以下の8種類が存在する。

```sh
#------------------------------------------------------------
# Locked Device
#------------------------------------------------------------

part
    id                     = "tinyAVR-0/1/2 locked device";
    signature              = 0x1e 0x74 0x30;
;

part
    id                     = "megaAVR-0 locked device";
    signature              = 0x1e 0x6d 0x30;
;

part
    id                     = "AVR-DA/DB/DD locked device";
    signature              = 0x1e 0x41 0x32;
;

part
    id                     = "AVR-EA locked device";
    signature              = 0x1e 0x41 0x33;
;

part
    id                     = "AVR-DU locked device";
    signature              = 0x1e 0x41 0x34;
;

part
    id                     = "AVR-EB locked device";
    signature              = 0x1e 0x41 0x35;
;

part
    id                     = "Offline or UPDI disabled device";
    signature              = 0xff 0xff 0xff;
;

part
    id                     = "Devices that use UPDI for GPIO?";
    signature              = 0x00 0x00 0x00;
;
```

### ENABLE_ADDFEATS_LOCK_SIG_DEBUGOUT

`ENABLE_ADDFEATS_LOCK_SIG`が有効な時、`RSP_MEMORY`の追加情報に`UPDI4AVR`の内部情報構造体を返す。

### ENABLE_ADDFEATS_DUMP_SIB

`AVRDUDE 7.2`固有の`memory "sib"`または`memory "io"`読み出しに対応し、IOメモリ領域の先頭32バイトを読む時、これを`UPDI`で読み出された`SIB`に置き換える。ホスト側からはROMとして見えるので、このメモリアドレス範囲に対する書き換えと検証は許されなくなる。現行のバージョンで単に`SIB`を取得したいだけなら、拡張パラメータ取得を使うのが正しい。

### ENABLE_DEBUG_UPDI_SENDER

メモリ書き込みとメモリ消去操作に対する`UPDI`低レベル通信ログを、`RSP_OK`出力の後に追加出力する。最初の2バイトは該当処理が完了した後の`UPDI4AVR`内部状態を示すフラグレジスタである。その後に最大512バイトの通信ログが現れる。送信データは「送信バイト値＋ループバックデータ値」の2バイトで表現される。両者が異なる場合は`UPDI`バスに信号妨害があったことを示す。受信データは読み出し命令に続く通常のバイト列で表現される。これらを外部プログラムで正しく解釈してヒューマンリーダブルに可視化すると、`serialupdi`実装のデバッグログと比較することが可能になる。

## Copyright and Contact

Twitter(X): [@askn37](https://twitter.com/askn37) \
BlueSky Social: [@multix.jp](https://bsky.app/profile/multix.jp) \
GitHub: [https://github.com/askn37/](https://github.com/askn37/) \
Product: [https://askn37.github.io/](https://askn37.github.io/)

Copyright (c) 2023 askn (K.Sato) multix.jp \
Released under the MIT license \
[https://opensource.org/licenses/mit-license.php](https://opensource.org/licenses/mit-license.php) \
[https://www.oshwa.org/](https://www.oshwa.org/)
