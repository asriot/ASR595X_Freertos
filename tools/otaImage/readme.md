# image_gen_header 

### 示例：image_gen_header.exe bootloader.bin -d SONATA -b COMPRESS
### 目前支持以下命令：
1. -d (不可缺省，参数: SONATA, LEGA_A0V1, LEGA_A0V2, COMBO, ALTO, BASS)，用于设置设备类型用于生成相应的image_token。
2. -b (不可缺省，参数: COPY, COMPRESS, REMAPPING)，用于设置boot中，ota image引导方式。
3. -r (可缺省，无参)，用于使能image roll back功能，缺省时默认关闭roll back功能。
4. -i (可缺省，无参)，用于使能加签image的ota info生成逻辑，缺省时默认采用非加签固件ota infa生成逻辑。
5. -t (可缺省，参数: APP, ROM)，用于设置image为app升级固件或是ROM升级固件的ota info生成逻辑，缺省时默认采用app升级固件ota infa生成逻辑（使能ROM方式生成固件时，ota info强制设置为remapping方式）。