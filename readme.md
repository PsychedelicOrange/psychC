# pyschC
_psychspiration's alter ego written in c_

psychC strives to be a collection of well written (reusable, general, sane) functions required for making a game.
Functions which do not fit these are written separetely and loosy goosy way in the main file most often.
We can think of this as a library interface, but i currently dont have any plans to separate code and make libs, as functions are not mature yet.

## Summary of features planned

* simple rendering ( pbr (strict), skeletal system, opengl )
* audio rendering (openal)
* multiplayer ( client/server, peer to peer)
* physics system ( bullet )

## Motivations
- i only know opengl, opengl is the simplest of all, and is sufficient as of now, hence it is picked over vulkan
- functions should be such that they can be reused to put together a game before a game jam
- functions should run on both linux and windows ( cross-platform )

## Detailed feature plan

### animations
- [ ] model.play_animation()
- [ ] load model with multiple animations
- [ ] morph targets for facial animation
- [ ] Animation blending
- [ ] drive animation through code
  
### input and camera
- [ ] expose proper functions for camera management styles tps,fps, etc.
- [ ] a way to handle input that is exhaustive to all input styles (time held, last pressed, modifier keys)
- [ ] add collision to 3rd person camera
- [ ] controller support

### audio
- [ ] play_sound("walk.wav", transform)
- [ ] streaming audio from disk
- [ ] mixing multiple audio??

### pbr
- [ ] general purpose material system
- [ ] cloth sheen
- [ ] focus on authoring a engine specific asset pipeline/format using gltf exporter of blender
- [ ] texture splatting
- [ ] decals

### lighting and _**gaphics**_
- [ ] bake lights for static objects via blender or in engine
- [ ] shadows ( cascade )
- [ ] skybox
- [ ] hdr
- [ ] global illumination

### physics
- [ ] collision detection and callback
- [ ] ray casting and intersection
- [ ] load convex hull from disk
- [ ] use existing physics library like bullet,etc.

### netcode
- [ ] lobby system
- [ ] udp reliable and unreliable communication
- [ ] p2p implementation ( one computer will act as server) ( research needed )
- [ ] maybe using webrtc ( research needed )
- [ ] maybe voip ?

### UI (in game)
- [ ] Helper functions for 2d textured plane rendering
- [ ] play video
- [ ] text rendering ( if possible via imgui )
- [ ] imgui

### UI (dev)
- [ ] (imgui) pattern to use quickly to debug/iterate on values.

## Version history
### v0.1
( currently being worked on )

