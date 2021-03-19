# pstv_ds4_enabler
This plugin was based on **Graphene**'s [post](https://forum.devchroma.nl/index.php?topic=311.0).

### What this plugin does?
It will enable the touchpad native mode for the DS4. This way the pointer won't appear in the screen and there will be no need of pressing the touchpad as the input will be recognized by only touch the device.

### Dependencies
*pstv_ds4_enabler* will create a patched **param.sfo** file in **rePatch** folder while launching a game. So for this plugin to work [rePatch](https://github.com/dots-tb/rePatch-reDux0/releases) needs to be used.

### Instalation
Add *pstv_ds4_enabler.skprx* to your **confg.txt** file in the **\*KERNEL** segment. But it is important that *pstv_ds4_enabler.skprx* be placed before *repatch.skprx*.

Example:
```
*KERNEL
pstv_ds4_enabler.skprx
repatch.skprx
```
