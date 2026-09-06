# Bridge Filter
export FIRMWARE_VERSION="BridgeFilter-3.0"
sh build.sh build-firmware \
Heltec_v3_repeater_bridge_espnow \
Heltec_WSL3_repeater_bridge_espnow \
heltec_v4_repeater_bridge_espnow \
heltec_v4_expansionkit_repeater_bridge_espnow \
heltec_v4_tft_repeater_bridge_espnow \
heltec_v4_r8_repeater_bridge_espnow \
heltec_v4_r8_tft_repeater_bridge_espnow

# Changed files for Bridge Filter
\examples\simple_repeater\MyMesh.cpp
\examples\simple_repeater\MyMesh.h
\src\MeshCore.h
\src\Packet.h
\src\helpers\CommonCLI.cpp
\src\helpers\CommonCLI.h
\src\helpers\ConfigSerializer.h
\src\helpers\bridges\BridgeBase.cpp
\src\helpers\bridges\BridgeBase.h
\src\helpers\bridges\BridgeFilter.cpp
\src\helpers\bridges\BridgeFilter.h
\variants\heltec_v3\platformio.ini
\variants\heltec_v4\platformio.ini
\variants\heltec_v4_r8\platformio.ini

# Usage for bridge
set bridge.enabled on
set bridge.channel 8
set bridge.secret <password up to 15 chars>
set repeat on # This is default setting

# Usage for bridge filter
get bridge.filter
set bridge.filter clear
set bridge.filter add/del advert
set bridge.filter add/del public
set bridge.filter add/del <1-byte>: Up to 8 first-bytes.
set bridge.filter add/del #hashtag: Up to 16 hashtags.
get bridge.filter stats

