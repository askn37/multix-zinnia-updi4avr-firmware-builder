# UPDI4AVR_FW634B

Switch Language [(en_US)](README_en.md) [(ja_JP)](README.md)

This UPDI4AVR firmware shows the following versions:

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

The `S_MCU FW` version value shown is smaller than `UPDI4AVR_FW753B`, but
`UPDI4AVR_FW634B` is newer and has been redesigned.

## Reason for lower version notation

In `AVRDUDE 7.3`, the `JTAGICE mkII` protocol implementation has been partially changed for the first time in over 10 years, and full compatibility with `7.2` and earlier is no longer guaranteed. The main reason is that it was originally a protocol that officially supported devices before the `XMEGA` family, and support for `UPDI` family devices was completely unofficial. `UPDI4AVR_FW753B` has been re-created as `UPDI4AVR_FW634B` for the purpose of being compatible with `AVRDUDE 7.3` since it was found that some of the relevant specification changes were problematic.

- `UPDI4AVR_FW753B` is based on protocol variant `FWV=7` (based on `XMEGA`).
- `UPDI4AVR_FW634B` is based on protocol variant `FWV=6` (`PP/PDI/SPI` based).

> In short, since applications based on FWV=7 are rare, continuous maintenance is difficult, and new application development is no longer recommended. `FWV=7` had enough information to infer the identity of an unknown target device before being able to read the `SIB` and device signature, but `FWV=6` does not include any of this information. This is because `UPDI` compatible devices are an advanced version of `XMEGA` technology. However, the agreement that `FWV=7` should be applied only to `XMEGA` devices made it difficult to further apply it to `UPDI`.

### User-defined memory settings not supported

Prior to `AVRDUDE 7.2`, users could add their own memory settings and use them by specifying the memory type with `-U` or with the `dump`, `read`, and `write` commands in interactive mode, in `7.3`, this cannot be used or its application is greatly limited. For example, `memory "sib"` cannot be used even though it is written in the configuration file.

### IO memory specification incompatibility

The old `memory "data"` setting will be reused as the base offset for the newly created `memory "io"` memory setting, so past application implementations that relied on this will change the address when the configuration file is changed. Calculations show abnormal values. Some uses can be replaced with the new ``sram'' memory setting in 7.3, but not all.

### Reversing memory reference order

Some `io` memory reads (specifically, getting the silicon revision value) are forced to occur before the `signature` memory read representing the device signature. Although this is not a practical problem at present, it is clearly not based on the `JTAGICE mkII` protocol specifications and there are concerns about maintaining compatibility. In `UPDI4AVR`, the effect of this order reversal becomes apparent during high voltage control, so internal countermeasures were required.

## Non-standard extensions

Several extension implementations are being prepared based on the plan to implement a new control driver to replace the existing `JTAGICE mkII` protocol implementation in the future.

### SIB reading with extended parameters

Since `SIB` cannot be expressed using existing memory types, it is allowed to be obtained using extended parameters in accordance with the general provisions of the `JTAGICE mkII` protocol. This can be used after entering programming mode.

### Enabling UPDI device descriptor in extended parameters

Traditionally unused (or ignored) emulation mode parameter acquisition allows the use of device descriptors optimized for `UPDI` devices instead of outdated and oversized `PP/PDI/SPI` device descriptors. can be changed to . This can include various NVM offset addresses and NVMCTRL version numbers, which can be useful for double-checking against the `SIB`.

### Supports forced HV control using reset command

HV control can be forced with an additional parameter in the reset command before entering programming mode. For devices in such a state, there is no way to obtain `SIB` before this stage.

### Enabling HV control

To enable the HV control function of `UPDI4AVR` in an existing `AVRDUDE`, it has been changed to use the following configuration file settings. An `idr` descriptor is required in the corresponding `part` section, which is not its intended purpose.

```sh
part # Any 'UPDI' device parts

  # Shared UPDI pin, HV on _RESET
  hvupdi_variant         = 2;
  idr                    = 0x32;  # substitute hvupdi_variant

  # Valid value : 0x30=tinyAVR、0x31=No HV、0x32=AVR_DD/EA/EB
```

Then, if you select the correct parts and use the `-eF` option, HV control will be activated when necessary. This is an alternative to the lack of a way to include an `hvupdi_variant` or `hvupdi_variant` value in the current `PP/PDI/SPI` device descriptor. Of course, it is not allowed to write this in non-`UPDI` parts.

> `avrdude.conf.UPDI4AVR` includes this setting without exception. In the future, when the official driver for `UPDI4AVR` is integrated, this alternative will not be necessary.

When writing FUSE assuming HV control to the target device, the following should always be avoided.

- PA0 for tinyAVR and PF7 for others should not be used for GPIO output. Either always for input or only for RESET function.
- The SUT item of `FUSE.SYSCFG1` should not be anything other than the default value `0b111`. The smaller the value, the lower the success rate of external control.

### Retry control using packet sequence number

A sequence number is placed at the beginning of a `JTAGICE mkII` packet, but only implementations that utilize this correctly will be able to efficiently retry NVM writes.

Two-wire serial communications become relatively unreliable and packet error rates increase as the speed increases (a few Mbps or more) or the distance increases (a few thousand meters or more, or via a wireless bridge). At this time, the sequence number can be used to distinguish in which direction the error occurred. Simply put, the host side requests again a packet with the same sequence number as when the reception error was detected. The end node saves the sequence number used last time and can make the next decision when the same value is sent again.

- Sequence number was retransmitted: The response packet going to the host side was lost.
   - If it is for a normal response, do not repeat any work and return the same normal response as before.
   - If it is for an abnormal response, execute the process according to the instruction and respond.

The key to this implementation is whether the host side can distinguish between reception error timeouts and packet loss. Many simple implementations ignore this, so they don't have the same packet retransmission feature, but simply throw it away and regenerate a new request.

## Custom build options

You can enable the following build options in `Configuration.h`. In the default build, only `ENABLE_ADDFEATS_LOCK_SIG` is enabled.

### ENABLE_ADDFEATS_LOCK_SIG

Normally, the device signature when the device is locked is simply `0xff 0xff 0xff`, but this is replaced with a special signature based on `SIB` and returned. This means that it can be visualized in a human-readable manner by adding the following special part registration to the `avrdude.conf` configuration file. Currently, there are eight types:

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

> If `0xff 0xff 0xff` is returned, if it is not caused by a wiring error, it may be possible to recover with HV control. However, if `0x00 0x00 0x00` is returned repeatedly, there is a high possibility that HV control for that target device will also fail.

### ENABLE_ADDFEATS_DUMP_SIB

Corresponds to `memory "sib"` or `memory "io"` reading specific to `AVRDUDE 7.2`. When reading the first 32 bytes of the IO memory area, replace it with `SIB` read by `UPDI`. Since it appears as ROM from the host side, rewriting and verification of this memory address range is no longer allowed. If you just want to get the `SIB` in the current version, it is correct to use extended parameter acquisition.

### ENABLE_DEBUG_UPDI_SENDER

Additional output of the `UPDI` low-level communication log for memory write and memory erase operations after the `RSP_OK` output. The first 2 bytes are flag registers that indicate the internal status of `UPDI4AVR` after the corresponding processing is completed. After that, a communication log of up to 512 bytes appears. Transmission data is expressed in 2 bytes: "transmission byte value + loopback data value". If the two are different, it indicates that there was signal interference on the `UPDI` bus. The received data is expressed as a normal byte sequence following the read command. If these are interpreted correctly by an external program and visualized in a human readable manner, it will be possible to compare them with the debug log of the `serialupdi` implementation.

## Copyright and Contact

Twitter(X): [@askn37](https://twitter.com/askn37) \
BlueSky Social: [@multix.jp](https://bsky.app/profile/multix.jp) \
GitHub: [https://github.com/askn37/](https://github.com/askn37/) \
Product: [https://askn37.github.io/](https://askn37.github.io/)

Copyright (c) 2023 askn (K.Sato) multix.jp \
Released under the MIT license \
[https://opensource.org/licenses/mit-license.php](https://opensource.org/licenses/mit-license.php) \
[https://www.oshwa.org/](https://www.oshwa.org/)
