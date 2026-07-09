  <table>
  <tr>
    <td>
      <p align="center">
      <img width="250" height="250" alt="MeshCore Filtered Bridge Logo" src="https://github.com/user-attachments/assets/59ee6b99-a924-47f8-af90-9255d6da59e1" />
      </td>
    <td>
      <h1 align="center">MESHCORE / ESP-NOW<br>:: F I L T E R E D&ensp;B R I D G E ::</h1>
  <p align="center"><strong>An Organization Dedicated to Producing Tools to Promote Healthy, Federated, and Intelligently-Scalable MeshCore Networks</strong></p>
      <p align="center"><strong> Contact: mcregbridge@gmail.com </strong></p>
  <p align="center">
  </p>
</p>
   </td>
  </tr>
</table>

<img width="1536" height="1024" alt="filtered_bridge_hero" src="https://github.com/user-attachments/assets/0dba7791-e350-4121-87a0-9099fee88bc0" />

# Mission
To enable intentional, lower‑noise communication between independent MeshCore regions operating on different frequency settings—while filtering out common non‑essential flood traffic and preserving the long‑term sanity of the mesh ecosystem.

## Overview
The Filtered Bridge is a two (or more) repeater MeshCore / ESP‑NOW system that links two (or more) MeshCore regions without forcing them to share the same RF settings.

It empowers bridge admins to selectively forward traffic by blocking Adverts, Public channel which mitigates cross‑region noise, advert storms, and public‑channel chatter from overwhelming neighboring meshes. In addition, admins can specify up to 32 hashtag channels for blocking messages from crossing the bridge (eg; #test, #bot, #wardriving etc;) thereby keeping the responses region-specific. This project promotes regional autonomy, mesh hygiene, cooperative inter‑mesh communication, and regional community.

For example, while its nice to receive a "Good Morning Mesh" message in the main Public channel from someone nearby, the cacophony of these messages from far-off locales tends to dull the experience over time.

## Key Features
### Customizable CLI‑controlled Packet Filtering of:

    - Adverts (flood and zero-hop)
    - Main Public channel
    - Up to 32 hashtag channels (16 hashtag channels per repeater)

Supports granular control of what crosses the bridge
(e.g., #sacsfbridge for inter‑mesh communication corridors)

The Crossband Filtered Bridge will still allow (and message types that persist) across the bridge:
    
    - Private messages
    - Private groups
    - Repeater admin/guest access
    - Can still retrieve path prefix data (1, 2, or 3 byte) of messages,from origin to destination,
      that transit across the bridge

## How Does the Filtered Bridge Work?

<img width="1635" height="1227" alt="Filtered_Bridge_Message_Flow_Diagram" src="https://github.com/user-attachments/assets/aeac0161-2161-4762-81da-a7e7e6a87c51" />


## Why This Matters
We believe that a good strategy to scaling up MeshCore meshes moving forward would be to encourage established or growing MeshCore regions to adopt a segmented, regional custom-tailored settings approach. A flat mesh does not scale. Expecting everyone to adopt the 910.525/62.5/7/5 as a standard ignores the regional needs (both known and emergent) that would justify modifying those settings.  For example, public utility meters in Southern California crowded the standard US frequency range so they adopted 927.875 frequency setting. The Netherlands reached a repeater density that led to severe airtime congestion and they opted to change their spreading factor. The Sacramento Valley Foothills Mesh utilizes a higher spreading factor because of its varying topography. These are prime case studies that, we believe, bolster this theory. We understand that this presents hurdles to onboarding new users but we also believe that these hurdles can be overcome with technology and disseminating info during onboarding. Also, this crossband filtered bridge dovetails with the region and scoping effort being rolled out by MeshCore.

If, moving forwared, Regional meshes can adopt settings that are optimal for their particular environment, then they can focus on growing their region and avoid threshold collapse. With the addition of filtered bridges deployed on the edges of these regions,  crossband communication corridors can facilitate messaging between the regions and extend message reach.

## Modern MeshCore meshes face challenges as they grow:
    
    - Maximizing and perserving link budget
    - Advert storms
    - Public channel super-saturation
    - Region bleed‑through
    - Frequency mismatches for repeater density and topography
    - Eventual Mesh threshold collapse due to excessive node count

The Crossband Filtered Bridge helps to address these challenges by acting as a selective gateway, not a simple common repeater.
It forwards only what should cross, thereby minimizing a large percentage of cascading traffic and helps to preserve the integrity and performance of each region.

## Current Limitations
***blocking more than 4-5 hashtag channels affects the CLI response to get bridge.filter because of the limitations of the 150 byte response on CLI commands.  Recommended workaround is to keep track of blocked hashtag channels in notepad/obsidian/spreadsheet in order to facilitate unblocking.<br>
***Filtered Bridge is power hungry. Take your power solution for the bridge deployment seriously. Powersaving features of 1.16 can *not* be applied to the ESP-NOW bridge. Unfortunate, but we are actively searching for an elegant solution.

## How Can I Create One Of These Bridges?
Our Firmware is currently limited to two ESP32 boards - Heltec V3 and v4.  You only need the filtered bridge firmware to be flashed to a minimum of 1 of the two repeaters for the bridge to work - it filters the flags set on both tx and rx. However, you will be limited to blocking 16 hashtag channels. To block 17 to 32 hashtag channels you must flash both repeaters with the appropriate firmware and set the blocking flags on each of the bridges independently. A filtered bridge always requires two (or more) ESP32 based repeaters to function. Yes, it has been tested with up to 4 repeaters on different frequency settings and it works well. Theoretically, it will work with many more repeaters.

## Filtered Bridge Setup Instructions
[] Make sure you have at least 2 Heltec v3 or v4 boards.
[] Download either the fresh install or upgrade bin files below for flashing:
## HELTEC V3 Filtered Bridge Firmware Files:
[Heltec_v3_repeater_bridge_espnow-BridgeFilter-2.1-freshInstall-merged.bin](https://github.com/Department-of-Mesh-Regional-Bridging/meshcore-esp-now-bridge-filter/releases/download/v2.1/Heltec_v3_repeater_bridge_espnow-BridgeFilter-2.1-freshInstall-merged.bin)

[Heltec_v3_repeater_bridge_espnow-BridgeFilter-2.1-upgrade.bin](https://github.com/Department-of-Mesh-Regional-Bridging/meshcore-esp-now-bridge-filter/releases/download/v2.1/Heltec_v3_repeater_bridge_espnow-BridgeFilter-2.1-upgrade.bin)
## HELTEC V4 Filtered Bridge Firmware Files:
[heltec_v4_repeater_bridge_espnow-BridgeFilter-2.1-freshInstall-merged.bin](https://github.com/Department-of-Mesh-Regional-Bridging/meshcore-esp-now-bridge-filter/releases/download/v2.1/heltec_v4_repeater_bridge_espnow-BridgeFilter-2.1-freshInstall-merged.bin)

[heltec_v4_repeater_bridge_espnow-BridgeFilter-2.1-upgrade.bin](https://github.com/Department-of-Mesh-Regional-Bridging/meshcore-esp-now-bridge-filter/releases/download/v2.1/heltec_v4_repeater_bridge_espnow-BridgeFilter-2.1-upgrade.bin)

[] Proceed to the official [Meshcore.io Flasher](https://meshcore.io/flasher) and flash your boards.

[] Configure each of the repeaters to your desired settings. For a crossband bridge define the different frequency settings on each repeater.

[] You will need to run the following CLI commands on each repeater via the console on the repeater configuration screen
  - `set bridge.enabled on`
  - `set bridge.channel X` (X = a number of your choosing between 1 and 14 - both repeaters *must* be set to the same channel number)
  - `set bridge.secret xxxxxx` (xxxxxx = a word of your choosing that contains numbers/letters/symbols and is case sensitive. Max 15 chars. both repeaters *must* be set to the same secret word.

Note: For each of the above commands, the console response should be 'OK' if done correctly. To verify any of the above settings, use the CLI command `get` to retrieve the saved value. (e.g.; `get bridge.enabled`, `get bridge.secret` etc;)

Link to MeshCore.io docs for all Bridge (non-filtered) CLI commands: [Bridge CLI Commands](https://docs.meshcore.io/cli_commands/#bridge-when-bridge-support-is-compiled-in)

## Filtered Bridge CLI Commands

## The following set of CLI commands are unique to this filtered bridge implementation and facilitate the blocking of various message types.

### Block or unblock all Adverts

Usage:
  - `set bridge.filter add adverts` (to block)
  - `set bridge.filter del adverts` (to unblock)

Response:  
  - adverts blocked
  - adverts unblocked

### Block or unblock Public channel (this is the default common Public channel that is added to all MeshCore app installs)<br>

Usage:  

  - `set bridge.filter add Public` (to block)
  - `set bridge.filter del Public` (to unblock)

Response:

  - `public blocked`
  - `public unblocked`

### Block or unblock hashtag channels (Must add # to name of hashtag channel. up to 16 channels blockable per filtered bridge enabled repeater)

Usage:  
  
  - `set bridge.filter add #hashtagchannel` (to block)
  - `set bridge.filter del #hashtagchannel` (to unblock)

Response: 
  - `hashtag #hashtagchannel blocked`
  - `hashtag #hashtagchannel unblocked`

### Retrieve list of blocked items

Usage:  
  - `get bridge.filter`

Response: 
  - `blocked: adverts public #hashtagchannel`

### Retrieve stats of blocked items since last reboot of repeater

Usage:
  - `get bridge.filter stats`

Response: 
  - `TX:3322 Sent, 929 Blocked. RX: 886 received, 269 Blocked. #hashtagchannel: 1142 blocked`






