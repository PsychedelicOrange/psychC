#Mesh

## Environment: non-destructable static motionless objects. it will have built-in lightmaps.
* All mesh data of all objects will be packed in one vbo (or multiple in case limit is exceeded)
* Following attributes are allowed : 
- Position, normal, tangent, texcoord1, texcoord2.
* Following textures are allowed :
- Albedo, normal,Metalic/Roughness, lightmap

## moving mesh: moving static mesh objects. These will have fixed mesh data, but the 3d transform might change.
* Here too mesh data is packed together.
* Following attributes are allowed : 
- Position, normal, tangent, texcoord1.
* Following textures are allowed :
- Albedo, normal,Metalic/Roughness

## Actors: Actors moving meshes which need to be animated. They will have joints and weights associated with them.

## Other: entities such as water, smoke, grass need to be handled separately with instancing, culling etc.