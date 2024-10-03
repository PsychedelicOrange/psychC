# psychspiration's alter ego written in c

* goal: make simple multiplayer game to play with frnds

* simple rendering ( pbr (strict), skeletal system, opengl )
* audio rendering (openal)
* multiplayer ( client/server, peer to peer)
* physics system ( bullet )

Features and roadmap plan for v0.5
*****************
* Planned release v0.5 January 2025 ( 3 months remaining while writing this )

v0.1 (current)

[ ] animations | 15 oct 24
* model.play_animation()
* load model with multiple animations
[ ] input and camera | 17th oct 24
* expose proper apis for camera management styles tps,fps, etc.
* a way to handle input that is exhaustive to all input styles (time held, last pressed, modifier keys)

v0.2 *> 15 oct 24 ( write a game )

[ ] audio | 20 oct 24
* spatial audio
* play_sound("walk.wav", transform)
* multithreading ? (explore)
[ ] pbr | 1 nov 24
* general purpose material system
* focus on authoring a engine specific asset pipeline/format using gltf exporter of blender

v0.3 *> 1st week november do another game jam

[ ] physics | 17 nov 24
* collision detection and callback
* ray casting and intersection
* load convex hull from disk
* use existing physics library like bullet,etc.
* honestly gonna be hardest part
* add collision to camera ( third person style )
[ ] netcode | 1 dec 24
* lobby system
* udp reliable and unreliable communication
* p2p implementation ( one computer will act as server)
* maybe using webrtc ( research needed )
* maybe voip ?

v0.4 *> 1st week of december do a multiplayer physics based game 

[ ] UI(in game) | 15 dec 24
* Helper functions for 2d textured plane rendering
* play video
* text rendering
* imgui

[ ] polish graphics | 1 jan 25
* lights ( point, directional ) 
* shadows
* some way to bake lighting on env models
* skybox

v0.5 *> built one of your game ideas (single player narrative driven)

[ ? ]
* morph targets for facial animation
* ? 
* ?
* ?
