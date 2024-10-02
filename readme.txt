# psychspiration written in c

* goal: make simple multiplayer game to play with frnds

- simple rendering ( pbr (strict), skeletal system, opengl )
- audio rendering (openal)
- multiplayer ( client/server, peer to peer)
- physics system ( bullet )
- in-engine ui and editor (compared to blender shenanigans)

Features for v1
-----------------
[ ] animations
- model.play_animation()
[ ] pbr 
- general purpose material system
- focus on authoring a engine specific asset pipeline/format using gltf exporter of blender
[ ] audio
- spatial audio
- play_sound("walk.wav", transform)
[ ] physics
- collision detection and callback
- use existing physics library like bullet,etc.
- honestly gonna be hardest part
[ ] netcode
- lobby system
- udp reliable and unreliable communication
- p2p implementation ( one computer will act as server)
- maybe using webrtc ( research needed )
- maybe voip ?
[ ] UI(in game)
- Helper functions for 2d textured plane rendering
- play video
- text rendering
- imgui

* not implemented yet
- morph targets (could be done later)
