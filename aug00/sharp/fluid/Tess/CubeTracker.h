#ifndef CUBETRACKER_H
#define CUBETRACKER_H

// NOTE: CubeTracker is not currently used (or written) as it's only useful for a continuous tessellator.
// Right now I'm working on a recursive tessellator, so I don't really need to track any cubelets.

//
// CubeTracker
//
// CubeTracker provides a way to mark cubelets "considered" and to check whether a cubelet
// has been considered or not.  The only way to mark a cubelet "unconsidered" is to clear
// the whole tracker, so it's easily implemented as, say, a hashtable or something like that.
// This is used by SurfaceWalker since the graph formed by the cubelets covering the surface
// is a directed cyclic graph, and so CubeTracker provides a way to break cycles by making
// sure no cubelet is considered twice during the surface walk.
//
// CubeTracker is depended upon by SurfaceWalker.
//

class CubeTracker
{
public:

protected:

};

#endif //CUBETRACKER_H