  <table>
  <tr>
    <td>
      <p align="center">
      <img width="250" height="250" alt="MeshCore Filtered Bridge Logo" src="https://github.com/user-attachments/assets/59ee6b99-a924-47f8-af90-9255d6da59e1" />
      </td>
    <td>
      <h1 align="center">MESHCORE / ESP-NOW<br>:: F I L T E R E D&ensp;B R I D G E ::</h1>
  <p align="center"><strong>Part of An Organization Dedicated to Producing Tools to Promote Healthy, Federated, and Intelligently-Scalable MeshCore Networks</strong></p>
  <p align="center">
  </p>
</p>
   </td>
  </tr>
</table>

<img width="1536" height="1024" alt="filtered_bridge_hero" src="https://github.com/user-attachments/assets/0dba7791-e350-4121-87a0-9099fee88bc0" />

## Mission
To enable intentional, lower‑noise communication between independent MeshCore regions operating on different frequency settings—while filtering out common non‑essential flood traffic and preserving the long‑term sanity of the mesh ecosystem.

# Overview
The Filtered Bridge is a two (or more) repeater MeshCore / ESP‑NOW system that links two (or more) MeshCore regions without forcing them to share the same RF settings.

It empowers bridge admins to selectively forward traffic by blocking Adverts, Public channel which mitigates cross‑region noise, advert storms, and public‑channel chatter from overwhelming neighboring meshes. In addition, admins can specify up to 32 hashtag channels for blocking messages from crossing the bridge (eg; #test, #bot, #wardriving etc;) thereby keeping the responses region-specific. This project promotes regional autonomy, mesh hygiene, cooperative inter‑mesh communication, and regional community.

For example, while its nice to receive a "Good Morning Mesh" message in the main Public channel from someone nearby, the cacophony of these messages from far-off locales tends to dull the experience over time.

## Key Features
(strong)Customizable CLI‑controlled Packet Filtering of:(/strong)

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

## Why This Matters
We believe that a good vector to scaling up MeshCore meshes moving forward would be to encourage established or growing MeshCore regions to adopt a segmented, regional custom-tailored settings approach. A flat mesh does not scale. Expecting everyone to adopt the 910.525/62.5/7/5 as a standard ignores the regional needs (both known and unknown) that would justify modifying those settings.  E.G.; Southern California, The Netherlands, and Sacramento Foothills are prime case studies that bolster this theory. We understand that this presents hurdles to onboarding new users but we also believe that these hurdles can be overcome with technology and disseminating info during onboarding. Also, this crossband filtered bridge dovetails with the region and scoping effort being rolled out by MeshCore.

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
***Filtered Bridge is power hungry. Take your power solution to the bridge seriously. Powersaving features of 1.16 can *not* be applied to the ESP-NOW bridge. Unfortunate, but we are actively searching for an elegant solution.

## How Can I Create One Of These Bridges?
Our Firmware is currently limited to two ESP32 boards - Heltec V3 and v4.  You only need the filtered bridge firmware to be flashed to a minimum of 1 of the two repeaters for the bridge to work - it filters theflags set on both tx and rx. However, you will be limited to blocking 16 hashtag channels. To block 17 to 32 hashtag channels you must flash both repeaters with the appropriate firmware on the bridge. A filtered bridge always requires two (or more) ESP32 based repeaters to function. Yes, it has been tested with up to 4 repeaters on different frequency settings and it works well.
