# TODO

## Getting back on track

### Version 0.0.2

#### Milestones

##### Render

- [x] Read pipeline tutorial
- [x] Get any of our points to render
- [x] Introduce scene concept
- [x] Get camera and glm
- [x] Put everything together

10.12:
> Now we have render using static scene and camera. We must bring
  semi-final shaders(difference must be left only in terms of camera),
  and then try to figure out why each time introducing mvp-camera breaks
  everything.

- [x] bring semi-final shaders
- [x] port static camera to mvp-camera keeping same position
- [x] make camera movable

- [x] shaders-path to root-folder

##### Connect tree and vertex buffer

- [x] start simple
- [x] add AROctree.get_particles_for_render(params_for_culling) method

Add stage buffering only when needed. This is complex, especially with LOD

### Version 0.0.3

Think... This is big physics version.

- [x] turn back internal forces
- [ ] p2p

#### Internal forces

We used to have the implementation

#### Within block

We must change p2p to work on storage...
I can't figure out yet how to implement linear storage traverse with spatial-aware offset, so lets start with simple recursion with deref
