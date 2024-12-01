# How animation data is stored in gltf file.
[gltf spec](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html)
```
{
    "nodes": [
        {
            "name": "node0",
            "children": [ 1 ],
            "translation": [ 0.0, 1.0, 0.0 ]
        },
        {
            "name": "node1",
            "children": [ 2 ],
            "scale": [ 0.5, 0.5, 0.5 ]
        },
        {
            "name": "node2",
            "translation": [ 1.0, 0.0, 0.0 ]
        },
        {
            "name": "node4",
            "mesh": 0,
            "rotation": [ 0.0, 1.0, 0.0, 0.0 ],
            "skin": 0
        }
    ],
    "skins": [
        {
            "inverseBindMatrices": 0,
            "joints": [ 1, 2 ],
            "skeleton": 1
        }
    ]
    "animations": [
            {
            "name": "Animate all properties of one node with different samplers",
            "channels": [
                {
                    "sampler": 0,
                    "target": {
                        "node": 1,
                        "path": "rotation"
                    }
                },
                {
                    "sampler": 1,
                    "target": {
                        "node": 1,
                        "path": "scale"
                    }
                },
                {
                    "sampler": 2,
                    "target": {
                        "node": 1,
                        "path": "translation"
                    }
                }
            ],
            "samplers": [
                {
                    "input": 4,
                    "interpolation": "LINEAR",
                    "output": 5
                },
                {
                    "input": 4,
                    "interpolation": "LINEAR",
                    "output": 6
                },
                {
                    "input": 4,
                    "interpolation": "LINEAR",
                    "output": 7
                }
            ]
        },
    ]
}
```

## Some points to note: 
- we don't have a "root" joint concept in gltf. We don't have `joints` as well, they are just nodes in the nodes array of the model. Hierarchy of joints is defined in nodes array itself, using `children` attribute of each node, which lists it's children nodes. This implies we can have multiple "root" nodes. Note that leaf nodes don't have / have empty `children` array.
- a joint can be shared by multiple skins. so skin specific data such as inverse bind matrix is stored in skin
