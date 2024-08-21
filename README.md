# MultiX Zinnia UPDI4AVR Firmware Builder

このブランチは`UPDI4AVR Programmer`ファームウェアサポートを含んでいる。
同時に任意のスケッチをファームウェアと置き換えて書き込むビルド環境も提供する。

`UPDI4AVR`プロダクトには以下の三種類があるが、このブランチはその最後のものである。

- UPDI対応Arduino互換機で動作する汎用ソフトウェアとしての __UPDI4AVR Software__
- 専用HV制御回路を含むオープンソースハードウェアとして設計された __UPDI4AVR Programmer__
- その専用ハードウェア用に作成された __UPDI4AVR Firmware__

(0.2.11以降) __SERIAL/UPDI-MANAGER(SUM)__ のファームウェアもこのSDKで同時に提供される。

## 対象AVR

### [megaAVR] tinyAVR-0/1/2

|系統|pin|2KiB|4KiB|8KiB|16KiB|32KiB
|-|-|-|-|-|-|-|
|tinyAVR-0
||8 |__ATtiny202__|ATtiny402
||14|ATtiny204|ATtiny404|ATtiny804|ATtiny1604
||20|         |ATtiny406|ATtiny806|ATtiny1606
||24|         |         |ATtiny807|ATtiny1607
|tinyAVR-1
||8 |ATtiny212|__ATtiny412__
||14|ATtiny214|ATtiny414|__ATtiny814__|__ATtiny1614__
||20|         |__ATtiny416__|ATtiny816|__ATtiny1616__|__ATtiny3216__
||24|         |ATtiny417|ATtiny817|ATtiny1617|ATtiny3217
|tinyAVR-2
||14|         |ATtiny424|__ATtiny824__|ATtiny1624|ATtiny3224
||20|         |ATtiny426|ATtiny826|__ATtiny1626__|ATtiny3226
||24|         |ATtiny427|ATtiny827|__ATtiny1627__|ATtiny3227

> __太字__ は実物確認済

### [megaAVR] megaAVR-0

|系統|pin|8KiB|16KiB|32KiB|48KiB
|-|-|-|-|-|-|
|megaAVR-0
||28|ATmega808|ATmega1608|ATmega3208|__ATmega4808__
||32|ATmega808|ATmega1608|ATmega3208|__ATmega4808__
||40|         |          |          |__ATmega4809__
||48|ATmega809|ATmega1609|ATmega3209|__ATmega4809__

> __太字__ は実物確認済

### [modernAVR] AVR DA/DB/DD/DU/EA/EB

|系統|pin|8KiB|16KiB|32KiB|64KiB|128KiB
|-|-|-|-|-|-|-|
|AVR_DA
||28|        |         |AVR32DA28|AVR64DA28|__AVR128DA28__
||32|        |         |__AVR32DA32__|AVR64DA32|AVR128DA32
||48|        |         |AVR32DA48|AVR64DA48|AVR128DA48
||64|        |         |         |AVR64DA64|AVR128DA64
|AVR_DB
||28|        |         |AVR32DB28|AVR64DB28|__AVR128DB28__
||32|        |         |AVR32DB32|AVR64DB32|__AVR128DB32__
||48|        |         |AVR32DB48|AVR64DB48|__AVR128DB48__
||64|        |         |         |AVR64DB64|AVR128DB64
|AVR_DD
||14|        |AVR16DD14|__AVR32DD14__|AVR64DD14
||20|        |AVR16DD20|AVR32DD20|AVR64DD20
||28|        |AVR16DD28|AVR32DD28|__AVR64DD28__
||32|        |AVR16DD32|AVR32DD32|__AVR64DD32__
|AVR_DU
||14||AVR16DU14|AVR32DU14
||20||AVR16DU20|AVR32DU20
||28||AVR16DU28|AVR32DU28|__AVR64DU28__
||32||AVR16DU32|AVR32DU32|AVR64DU32
|AVR_EA
||28||AVR16EA28|AVR32EA28|AVR64EA28
||32||AVR16EA32|AVR32EA32|__AVR64EA32__
||48||AVR16EA48|AVR32EA48|AVR64EA48
|AVR_EB
||14||AVR16EB14|AVR32EB14
||20||AVR16EB20|AVR32EB20
||28||AVR16EB28|AVR32EB28
||32||__AVR16EB32__|AVR32EB32

> __太字__ は実物確認済、*斜体* は暫定対応

## 導入方法

- Arduino IDE の「環境設定」「追加のボードマネージャーのURL」に以下のリンクを追加
  - [`https://askn37.github.io/package_multix_zinnia_index.json`](https://askn37.github.io/package_multix_zinnia_index.json)
- メニューバーから「ツール」「ボード選択」「ボードマネージャー」ダイアログパネルを開き、検索欄に "multix" と入力
- `MultiX Zinnia UPDI4AVR Firmware Builder`を選択して「インストール」

## ボードマネージャー設定

- メニューバーから「ツール」「ボード選択」から`MultiX Zinnia UPDI4AVR Firmware Builder`を選択
- メニューバーから「ツール」「シリアルポート」で`UPDI4AVR Programmer`に接続されたポートを選択

## ファームウェアアップデート（推奨される手順）

- `UPDI4AVR Programmer`の`SW3`を`FW`側にセットして作業PCにUSBケーブルで接続
  - `UPDI4AVR Programmer`のLEDがハートビート明滅を始める
- メニューバーから「ツール」「Variant」で望むファームウェアを選択
- メニューバーから「ツール」「書込装置」で`SerialUPDI over USB`を選択
- メニューバーから「ツール」「__ブートローダーを書き込む__」を選択
  - ファームウェア書換中は`UPDI4AVR Programmer`のLEDは消灯する
  - ファームウェア書換が完了すると`UPDI4AVR Programmer`のLEDはハートビート明滅を始める
- `UPDI4AVR Programmer`を作業PCから取り外し`SW3`を`RUN`側に戻す

この手順はビルド済のファームウェアを`UPDI4AVR Programmer`本体に書き込む。
「アップロード」ボタンをクリックすると現在IDEで表示中の任意のスケッチが書き込まれるが、これは誤った手順になるので注意。

## UPDI4AVRのファームウェアバージョン確認方法

- Arduino IDEの場合：対象デバイスを繋がず、__書込装置に`UPDI4AVR over UART`を選択して「ブートローダーを書き込む」__
- CLIの場合：*avrdude*にパスが通っているなら、対象デバイスを繋がずに次のようなコマンドを実行する。

```plain
avrdude -p avr64ea32 -c updi4avr -P <実際のCOMポート> -v
```

何れの場合も（対象デバイスがないため）エラーで終了するが、それに先立って`UPDI4AVR`の個体情報をコンソールに表示する。

```plain
Programmer Type : JTAGMKII_UPDI
Description     : JTAGv2 to UPDI bridge
M_MCU HW version: 2
M_MCU FW version: 7.53
S_MCU HW version: 2
S_MCU FW version: 6.34
Serial number   : xx:xx:xx:xx:xx:xx
Vtarget         : 5.2 V
```

`S_MCU FW version`が現在実行中の`UPDI4AVR`FWバージョン、`Serial number`が`UPDI4AVR`ハードウェア固有識別子を示す。
`Vtarget`は実際に`VDD/VTG`端子で観測される電源電圧を示す。

> `Serial number`はFWを変更しても（消去しても）失われることなく不変。

## UPDI4AVR Software との相違

- __UPDI4AVR Software__ は汎用の Arduino 互換機向けに書かれている。
  - USB-UARTのRTS信号は、互換機の制約により端検出のみしか行えない。
  - 各種信号の制御は、多様なチップで共通する機能のみで行う。
  - JTAG2プロトコルサブバージョンは __6系__ で、JTAG2UPDIと同じ。
  - シリアル番号、制御電圧情報はダミー。
  - ~~近日、__UPDI4AVR Firmware__ からのバックポートを予定している。~~完了。
  - ~~JTAG2UPDI(clone)へもバックポートされたクローンの公開も予定している。~~完了。
- __UPDI4AVR Firmware__ は Macro/Micro_API環境で記述され、専用設計のハードウェア向けに書かれている。
  - USB-UARTのRTS信号のレベル検出ができるため状態遷移が高速。
  - 各種信号の制御にほぼ GPIOを使わず、CCL/LUT を活用している。
  - ~~FW753Bでは JTAG2プロトコルサブバージョンは __7系__ で、*avrdude* から対象デバイスの追加情報を取得できる。
  よってHV制御電圧の自動切換えと、USERROW特殊書込は __UPDI4AVR Firmware__ にしか実装されていない。~~
  - FW634Bでは JTAG2プロトコルサブバージョンは __6系__ で、JTAG2UPDIと同じに戻った。HV制御電圧の自動切換えは *avrdude.conf* に制御情報を追記することで対応する。つまり標準の構成ファイルでは、HV制御は有効化されない。USERROW特殊書込はより汎用性の高い実装に変更された。

## ファームウェアビルド＆アップロード

- メニューバーから「ファイル」「スケッチ例」から「__UPDI4AVR firmware__」を探す
- サブメニューで望むファームウェアを選択
- メニューバーから「スケッチ」「マイコンボードに書き込む」あるいは「アップロード」ボタンをクリック

この手順は選択したファームウェアをソースからビルドして`UPDI4AVR Programmer`本体に書き込む。

## カスタムファームウェア

`UPDI4AVR Programmer`本体はArduino互換機としても動作するので、このブランチでも任意のスケッチをビルドして書き込むことができる。

しかしこのブランチでのスケッチビルドでは必要最小限の機能だけ
（だがAVR-LIBCは備えているのでMicrochip純正環境とほぼ等価）
を含みFUSE等は推奨設定に固定されている。
以下のビルド環境に切り替えればこれらを任意に変更して機能を拡張することができる。

以下のボードマネージャーサポートをインストールする。

- __MultiX Zinnia Product SDK [megaAVR]__ [->HERE](https://github.com/askn37/multix-zinnia-sdk-megaAVR)

このブランチと同等の設定にするには、以下のようにボードマネージャーを変更する。

-  ボードマネージャーで`Multix Zinnia Product SDK [megaAVR]`中の`tinyAVR-0/1/2 w/o Bootloader`を選択（必須）
- `Variant`中の`tinyAVR-2 20pin ATtiny1626 (16KiB+2KiB)`を選択（必須）
- `Clock`中の`Internal 10 MHz`を選択（任意）
- `BOD Mode`中の`BOD Enabled`を選択（任意）
- `BOD Level`中の`Level 1.8V (default)`を選択（任意）
- `FUSE PF6`中の`UPDI and PB4 pin=RESET (tinyAVR-2 20/24pin only)`を選択（任意）
- `Console and LED`中の`UART1 TX:PC2 RX:PC1 LED=PA7 (tinyAVR-2 alt)`を選択（USBサイド）
  - あるいは`UART0 TX:PB2 RX:PB3 LED=PA7`（TP1サイド）

ブードローダーを組み込む場合は`TP1`サイドをPC接続に使用し`SW1`の#3と#4はオフにしなければならない。
`CN3:USB`サイドはブートローダーでのスケッチアップロードに対応していない。

## Arduino API対応環境

このブランチおよび前述のビルド環境は Arduino API をサポートしていない。
従って一般的な多くのArduinoスケッチはサポートされていない。
フルサポートを欲する場合は以下のビルド環境を用意すると良い。
ただし設定可能項目は増大するので取り扱いには注意が必要。
またArduino API対応と引き換えにコードがとても大きく遅くなる。

- __megaTinyCore - Arduino support for all tinyAVR 0/1/2-Series__ [->HERE](https://github.com/SpenceKonde/megaTinyCore)

## AVR_DU/EA/EB 系統への対応

- __AVR_EA/EB__ 系統の正式サポートには *avrdude 7.3* 以降のリリースが必要
  - ただし __AVR_EB__ の BOOTROW 書き込みには書込器側ファームウェア対応の制約がある。
- __AVR_DU__ 系統の正式サポートには *avrdude 7.4* 以降のリリースが必要（計画）

23/12現在、*avrdude 7.2* 内蔵の [__SerialUPDI__](https://avrdudes.github.io/avrdude/7.2/avrdude_19.html#index-SerialUPDI/) は、AVR_DU/EA/EB 系統を正しく操作することができない。暫定的に AVR_DU/EB/EA 系統の不揮発メモリを読み書きできるプログラムライターは、[__UPDI4AVR__](https://askn37.github.io/product/UPDI4AVR/)、[__JTAG2UPDI(Clone)__](https://github.com/askn37/jtag2updi) と __PICkit4/5__ （要 Firmware アップグレード）だけである。
本SDKに付属の *avrdude.conf.UPDI4AVR* は [__UPDI4AVR__](https://askn37.github.io/product/UPDI4AVR/) および [__JTAG2UPDI(Clone)__](https://github.com/askn37/jtag2updi) でのみ正しく動作する記述であることに注意されたい。

### AVR_DU/EB系統のメモリマップ変更

AVR_DU/EB系統では新たな不揮発メモリとして`BOOTROW`が設定された。
これは 64byteの`Flash`メモリにして`FUSE`に追加された保護ビットにより
フラッシュ消去と連動消去するか保護されるか選べるものである。
そしてその配置アドレスは従来の`SIGNATURE`領域を指している。

|Symbol|tinyAVR / megaAVR|AVR_DA/DB/DD/EA|DU/EB|
|-|-|-|-|
|SIGNATURES_START / SIGNATURE_0|0x1100|0x1100|0x1080|
|USER_SIGNATURES_START (USERROW)|0x1300|0x1080|0x1200|
|BOOTROW_START (BOOTROW)|-|-|0x1100|

このため`BOOTROW`に本来の`SIGNATURE`の内容が転写されており、
かつ`FUSE`で消去保護がされているならば、
AVR_EA以前とは互換性があるように振る舞う。
2023/10時点での最新の *avrdude.conf*
[(GitHUB)](https://github.com/avrdudes/avrdude/blob/main/src/avrdude.conf.in)
は、この互換性を期待した記述であることに注意されたい。

> `BOOTROW`と`SIGNATURE`を共に0x1100とし、
`-U bootrow`でアクセスできるようにしている。
一方で`USERROW`は0x1080のままであるため正常に処理できない。
`PRODSIG`も`BOOTROW`から読み出される。

一方で本リポジトリに付属の *avrdude.conf.UPDI4AVR* での配置アドレス指定は、
AVR_DU/EB ネイティブである。
これは`BOOTROW`を特殊ではないメモリアクセスと、
`USERROW`への正しいメモリアクセスを保証する。

> 前述の表の通りに記述されており、
`BOOTROW`と`SIGNATURE`は別の領域である。
`PRODSIG`も`SIGNATURE`の次アドレスから正しく読み出される。

### AVR_EA 系統の制約

- FUSE_SYSCFG0.CRCSRC を既定値の NOCRC 値以外に変更してはならない。初期生産ロット(B1)は回路の不具合により正常な動作をしない。この不具合は二次生産ロット(B2)以降で解消されている。

### AVR_EB 系統の制約

- 初期生産ロット(A0)は、LOCK.KEY または FUSE.PDICFG を既定値以外に変更すると、以後の UPDI NVMPROG 制御が（HV制御と無関係に）全面的に困難または不可能となる。これは公開データシートの記述と異なる挙動である。
- FUSE_SYSCFG0.CRCSRC を既定値の NOCRC 以外に変更してはならない。初期生産ロット(A0)は回路の不具合により正常な動作をしない。
- HV制御の推奨投入電圧が 7.5V に変更（低下）した。RESET/PF6 パッドの絶対定格は 8.5V なので、AVR_DD 用に設計された HV制御回路では電圧が高すぎる恐れがある。

## SERIAL/UPDI-MANAGER(SUM)

SERIAL/UPDI-MANAGER(SUM)は、SerialUPDIプログラム機能を混載した UARTパススルー対応 USBシリアル変換アダプタだ。高度な機能は持たないが、UPDI4AVR に近い操作性を低コストで提供する。

- UARTパススルーと tinyAVR リセット機能（URP）を専用FWで実現している。
- 対応する AVRDUDE は、v7.3以降を推奨（v7.1以降も一部制約付きで使用可能）
- pymcuprg も使用可能（UPDI固定モード使用時）
- ハードウェアの制約により、HV機能には非対応。
- AVRDUDE の実装上の都合により `-D`による部分領域書き換えは非対応。

24/04/21 現在のFWバージョンは、FW2401Aである。

## 更新履歴

- 0.2.14 (24/06/17)
  - 各ファイルの MITライセンスリンク対応
  - UPDI4AVR: AVRDUDE>=8.0対応の微修正（24/08/22）

- 0.2.13 (24/05/12)
  - `7.3.0-avr8-gnu-toolchain-240510`に更新。

- 0.2.12 (24/04/29)
  - 動作確認済に __AVR64DU28__ を追加。

- 0.2.11 (24/04/21)
  - 動作確認済に __AVR128DB28__、__AVR64DD28__ を追加。
  - __SERIAL/UPDI-MANAGER(SUM)__ のファームウェア対応を同梱。

- 0.2.10 (24/01/16)
  - `7.3.0-avr8-gnu-toolchain-231214`に更新。
    - __AVR64DU28/32__ に暫定対応。
    - 動作確認済に __AVR64EA32__、 __AVR64EA48__、 __AVR16EB32__、__ATtiny1627__, __ATtiny416__ を追加。

- 0.2.9 (23/12/08)
  - 制御困難デバイスに対するリセット処理の調整。

- 0.2.8 (23/11/24)
  - `7.3.0-avr8-gnu-toolchain-231113`に更新。
  - UPDI4AVR Firmware を`UPDI4AVR_FW634B`に変更。
    - `AVRDUDE 7.3`仕様に準拠。
    - HV制御は添付の`avrdude.conf.UPDI4AVR`を使用した場合に有効。

- 0.2.7 (23/10/18)
  - `dryrun`を書込器選択に追加。
  - `avrdude.conf`参照ルールの変更。
    - `arduino`/`UPDI4VAR`/`TPI4AVR`/`dryrun`を書込器に指定した場合のみ、ローカルの特別な設定ファイルを参照する。それ以外は規定の（tools/avrdude/etc内の）`avrdude.conf`を参照する。
    - この変更により、AVR_EA系統のようにまだ他の書込器で未対応／未検証のパーツ設定が分離された。

- 0.2.6 (23/10/16)
  - `7.2-arduino.1`に更新。
  - `7.3.0-avr8-gnu-toolchain-231004`に更新。

- 0.2.5 (23/10/09)
  - AVR_EB系統の対応準備
  - *avrdude.conf.updi* 記述を avrdude 7.1 準拠に改正
  - 前述に対応したNVM制御の修正
  - 規定の動作主クロックを10Mhz→20MHzに変更
    - 4.5V未満でのNVM書換は未だ可能であるが非推奨とする

- 0.2.4 (23/09/09)
  - `7.3.0-avr8-gnu-toolchain-230831`に更新。
  - `programmers.txt`を改正。
    - `SerialUPDI`の`-xrtsdtr=High`オプションを有効化。

- 0.2.3 (23/07/09)
  - `7.3.0-avr8-gnu-toolchain-230628`に更新。

- 0.2.2 (23/05/23)
  - `7.1-arduino.1`に更新。

- 0.2.1 (23/04/29)
  - AVR_DA初期ロット(RIVID=A6/A7)対応

- 0.2.0 (23/04/10)
  - 新規公開

## 許諾

各構成要素はそれぞれ異なる配布ライセンスに属する。条件はそれぞれの規約に従う。

- BSD License
  - avr-libc
- GNU General Public License v2.0
  - avr-gcc
  - avrdude
- MIT License
  - other original document and code

## 著作表示

Twitter: [@askn37](https://twitter.com/askn37) \
BlueSky Social: [@multix.jp](https://bsky.app/profile/multix.jp) \
GitHub: [https://github.com/askn37/](https://github.com/askn37/) \
Product: [https://askn37.github.io/](https://askn37.github.io/)

Copyright (c) askn (K.Sato) multix.jp \
Released under the MIT license \
[https://opensource.org/licenses/mit-license.php](https://opensource.org/licenses/mit-license.php) \
[https://www.oshwa.org/](https://www.oshwa.org/)
