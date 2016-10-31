
Finally, I have posted source for Skeletal deformation in
OpenGL.  It is a pretty complex sample as you need to have an
interface to select vertices, manipulate bones, set weights, and
lots of other stuff.  It was much easier with the simple arm
two bone system from last year.

Realize, when I create characters for my work (and most others
doing this) use animation packages and exporters so they don't
need all this tool stuff.  But if you don't have access to a nice
package, this is a start.  It will take some work to study though
as there is a lot going on here.

Here is how it works.

Loading an Object, Skeleton, and Weight system
---------------------------------------------------------------------
I have included one skeletal system with a model in three resolutions
(so it will perform on most systems) with weight files.

The process is:

1. Load the Skeletal system with "Open Skeleton" and select Femme.dar
2. Load the Mesh with "Open Mesh File" Select one of the three models.

The mesh is now loaded and associated with the skeleton.  If you select
"View/Draw Deformed" it will vanish.  This is because the weights are 
not set.  You could set the weights manually (instructions later) or 
load a weight file.

3. "Load Weight File" to get the weighting for the model.  You need to
select the correct file for the model you selected or it will give an
error.

This automatically selects "Draw Deformed" so you can start manipulating
the character.


Manipulating the View
---------------------------------------------------------------------
You can change the view by using the Right Mouse Button
Click and drag to orbit view.
Hold CTRL and drag RMB to change Z view distance
Hold SHIFT and drag RMB to Move Camera in XY


Manipulating the Object
---------------------------------------------------------------------
Select a Bone from the Hierarchy window.
SHIFT + LMB drag to rotate the bone in XY
CTRL + LMB drag to rotate in Z

Note: There is no way to translate the bone without double clicking
in the hierarchy window and editing the box.  A better interface would
make that easier.  But, it is really only need when building the skeleton
originally.

If "Draw Deformed" is selected and the weights are set, rotating the
bones will deform the model.


Setting the Weights
---------------------------------------------------------------------
Ones you have loaded a model and skeleton you need to set the
rest frame.  In most cases it will match in the import.  If not,
manipulate the skeleton until it matches the mesh.  Then select
"Skeleton/Set Rest Pose"  This sets the rest postion that the
deformation is performed from.

To set the weights:

1. Select the Bone that you want to work with by selecting it in
the heirarchy.  
2. Double Click on the Bone name to bring up the editing window. 
Set the weight value that will influence that bone. Value of 1 is
full influence, .5 is 50%, etc.
3. Select the Vertices to influence by LMB drag a selection box on
the mesh.
4. Select "Skeleton/Set Bone Weights" to lock in those vertices into
the selected bone.
5. Do this for each bone and each vertex making sure that the sum
for each vertex is 100% (1.0).

You need to make sure that if you weight a vertex .5 to one bone, it
must be .5 to another bone or group of bones.  This is a place where
the interface needs lots of work.  If I was better at windows controls,
I would put a percentage slider after each bone and make it so they
must add up to 100 %.

If you screw up, select the vertex and hit "Skeleton/Clear Selected Weights"
this will reset it to 0.

Once you like it, "File/Save Weight File" to store.



Todo (ideas and things needed)
---------------------------------------------------------------------
Fix Screwy interface.  Controls are wacky and need work.
Put in Paint on weighting system like Maya and Sumatra.
Add Bone creation and deletion functions.
Allow Save Skeleton
Allow Save "Object" which includes Mesh+Skeleton+Weights
Allow save and load of Poses and/or animation
Add texture loading "uv coords" are already supported in OBJ loader
Possibly add automatic weighting based on bone layout.
Support for other load formats (other then OBJ)
Integrate the 3D IK techniques from last year (pretty easy really)
Lighting is not correct for deformation (know the fix? it is easy but requires some CPU)



Notes
---------------------------------------------------------------------

For the sample models, I roughly set up the weights.  Some are 50-50 but
most are 100% to a single bone.  It could be refined a lot more to fix
problems.  This software method is totally flexible. This is not true
of the DX7 and GL-HW T&L version.  The restrictions mean I need to add
some filters.

DX7 is still in Beta and completely broken on all 3D cards
as of last week.  I have new drivers I need to install and 
test but for now, my Direct X sample will no longer work.
I will post the DX7 version as soon as it stablizes.

For people trying to develop DX apps, this must be a pretty
frustrating time.

I also want to get a version working with the NVidia OpenGL
Extension.  Watch for word of that one.  I need to get a card
first to try it out....

The OpenGL code is a complete software solution and as such
works on both Windows 95/98 as well as NT4.0/W2K.


Email with questions or issues. I am sure there will be plenty.


Jeff					 jeffl@darwin3d.com
