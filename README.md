       <img width="400" height="400" alt="filtered_bridge_logo_small" src="https://github.com/user-attachments/assets/59ee6b99-a924-47f8-af90-9255d6da59e1" />  # MeshCore / ESP-NOW Crossband Bridge Filter

<img width="1536" height="1024" alt="filtered_bridge_hero" src="https://github.com/user-attachments/assets/0dba7791-e350-4121-87a0-9099fee88bc0" />


We are an Organization Dedicated to Promoting, Healthy, Federated, and Intelligently-Scalable MeshCore Networks
## Mission
To enable intentional, low‑noise communication between independent MeshCore regions operating on different frequency settings—while filtering out common non‑essential flood traffic and preserving the long‑term sanity of the mesh ecosystem.

# Overview
The Crossband Filtered Bridge is a dual‑radio ESP‑NOW system that links two MeshCore regions without forcing them to share the same RF environment.
It selectively forwards only meaningful traffic, preventing cross‑region noise, advert storms, and public‑channel chatter from overwhelming neighboring meshes.

For instance, while its nice to receive a "Good Morning Mesh" message in the main Public channel from someone nearby, the cacophony of these messages from far-off locales tends to dull the experience over time.

This project promotes regional autonomy, mesh hygiene, and cooperative inter‑mesh communication.

## Key Features
Customizable Packet Filtering

CLI‑controlled filtering of:    
    
    - Adverts
    - Main Public channel
    - Up to 8 hashtag channels

Supports granular control of what crosses the bridge
(e.g., #sacsfbridge for inter‑mesh threaded communication corridors)

Crossband Bridge Filter will still allow, and message types that persist, across bridge:
    
    - Private messages
    - Private groups
    - Repeater admin/guest control

## Why This Matters
Modern MeshCore deployments face challenges as they grow:
    
    - Advert storms
    - Public‑channel super saturation
    - Region bleed‑through
    - Frequency mismatches
    - Mesh threshold collapse due to excessive node count

The Crossband Filtered Bridge helps to address these challenges by acting as a selective gateway, not a simple common repeater.
It forwards only what should cross, thereby minimizing a large percentage of cascading traffic and helping to preserve the integrity and performance of each region.
