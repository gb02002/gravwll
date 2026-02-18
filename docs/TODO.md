# TODO

## Getting back on track

### Version 0.0.1

#### Goals

- [x] rewrite context class
- [~] get project under naming convention
- [x] get all deps
- [x] start with testing approaches to physics layout

#### Internal steps

##### Context(ctx)

> god object for now. Def tear apart, maybe leave new structs under it, maybe  
just pass under Simulation class

New structs/classes:

- ConfigClass(readFile)
- smth related to values initialization
- state(time, SimState enum)
- tree initialization(mb better just pass values to physics, idk)

Maybe layout is not that important as it is a subject to change a lot in the future.

##### Memory arenas  

> During ctx tearing, we came to creating arenas. I have concluded we need 3  
different arenas for different types of objects

Different object:

1. Particle Blocks. It is a center of computation. It must be 2 layered:
simple stab-like arena, and a manager service, that tries to put logical  
locality(tree's p2p integration) with it immanent desire to run away.
2. Multipole arena. This one has much simpler lifetime: we start FMM step -> take
needed amount of memory(`a` leaves, `b` nodes). Initial memory block can be
allocated after first tree build.
3. Other structs are either not memory intense, initialize once or go under graphics.

The \#1 should be started now.

##### Manager

- [ ] Decision is not final yet. It's complicated, and for now it seems that it
won't be the bottleneck and what I read is useless. Decide between giving ptr  
to a Node or an int from arena, with which we can march array and make a deref  
if needed. Best decision would be to leave it for now and implement it with  
perf, knowing data layout.

##### Generators

I think the simplest is to give interface and then do everything. For now we  
need 3 generators and 2 data sources:

1. PlummerGenerator
2. RandomGenerator
3. NoGenerator
4. FromFile
5. FetchURL

##### Get back with render and n^2

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
- [~] add AROctree.get_particles_for_render(params_for_culling) method

Add stage buffering only when needed. This is complex, especially with LOD

### Version 0.0.3

This is big physics version.

- [x] turn back internal forces
- [~] p2p

#### Internal forces

We used to have the implementation

#### Within block

We must change p2p to work on storage...
