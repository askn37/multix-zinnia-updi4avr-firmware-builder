# MultiX Zinnia UPDI4AVR Firmware Builder

このブランチは`UPDI4AVR Programmer`ファームウェアサポートを含んでいる。
同時に任意のスケッチをファームウェアと置き換えて書き込むビルド環境も提供する。

`UPDI4AVR`プロダクトには以下の三種類があるが、このブランチはその最後のものである。

- UPDI対応Arduino互換機で動作する汎用ソフトウェアとしての __UPDI4AVR Software__
- 専用HV制御回路を含むオープンソースハードウェアとして設計された __UPDI4AVR Programmer__
- その専用ハードウェア用に作成された __UPDI4AVR Firmware__

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
Programmer Type : JTAGMKII_PDI
Description     : JTAGv2 to UPDI bridge
M_MCU HW version: 1
M_MCU FW version: 2.01
S_MCU HW version: 1
S_MCU FW version: 7.53
Serial number   : 4a:34:0f:a6:28:3f
Vtarget         : 5.0 V
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
- __UPDI4AVR Firmware__ は Macro/Micro_API環境で記述され、専用設計のハードウェア向けに書かれている。
  - USB-UARTのRTS信号のレベル検出ができるため状態遷移が高速。
  - 各種信号の制御にほぼ GPIOを使わず、CCL/LUT を活用している。
  - JTAG2プロトコルサブバージョンは __7系__ で、*avrdude* から対象デバイスの追加情報を取得できる。
  よってHV制御電圧の自動切換えと、USERROW特殊書込は __UPDI4AVR Firmware__ にしか実装されていない。

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

## 更新履歴

- v0.2.4 (23/09/09)
  - `7.3.0-avr8-gnu-toolchain-230831`に更新。
  - `programmers.txt`を改正。
    - `SerialUPDI`の`-xrtsdtr=High`オプションを有効化。

- v0.2.3 (23/07/09)
  - `7.3.0-avr8-gnu-toolchain-230628`に更新。

- v0.2.2 (23/05/23)
  - `7.1-arduino.1`に更新。

- v0.2.1 (23/04/29)
  - AVR_DA初期ロット(RIVID=A6/A7)対応

- v0.2.0 (23/04/10)
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
GitHub: [https://github.com/askn37/](https://github.com/askn37/) \
Product: [https://askn37.github.io/](https://askn37.github.io/)

Copyright (c) askn (K.Sato) multix.jp \
Released under the MIT license \
[https://opensource.org/licenses/mit-license.php](https://opensource.org/licenses/mit-license.php) \
[https://www.oshwa.org/](https://www.oshwa.org/)
