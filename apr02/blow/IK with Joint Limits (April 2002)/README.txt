There are two main modes in the program, the IK solver mode
(which you are dropped into by default) and the little closest-point-
within-polygon graphing mode, which you can toggle by pressing 'G'.

Controls for polygon graphing mode:

    'W', 'S', 'A', 'D': Move the target point around the window plane.
    'G': Switch to IK solver mode.

Controls for IK solver mode:

    'W', 'S', 'A', 'D': Move the camera around.
    'E', 'R': Move the IK target forward or backward along the current
              control axis.
    'X', 'Y', 'Z': Change the IK target control axis.  So basically the idea
                   here is you can move the IK target to anywhere in 3D
                   space by choosing one of the 3 axes, using 'E' and 'R'
                   to move it around on that axis; then pick a different
                   axis, do it again.  etc.
    '0', '1', '2', '3', '4': Change the starting pose for the arm.  There
                   are some hardcoded initial states in the program; some
                   of these states converge more readily to some areas
                   of the target space.  If the IK does not converge, try
                   changing to a different starting pose.  Of course in
                   a shipping app you would automate this process.  Also,
                   due to the joint limits I have set up, there are areas
                   where the arm just can't reach.
    'J': Toggle drawing joint limits.
    'L': Toggle enforcement of joint limits.
    'G': Switch to graphing mode.



Notes
-----
Sometimes it looks like the elbow is missing the elbow reach window
(blue).  It's not missing; what's happening is that the elbow gets
tilted so far that the distance from the bone root at which I am
drawing the reach window (20% of the bone length) is beyond the
projection of the bone onto the X axis, so the visualization is no
longer helpful.  I could have fixed this by pulling the reach window
visualization closer to the bone in such a case, but I figured that
that would be more confusing than helpful.

Because I put such overly-restrictive joint limits on the arm,
it's kind of gimpy and can't reach very much.  But hey, the point
of this app was to demonstrate joint limiting.  Also because
the arm is kind of gimpy it tends to look unstable sometimes
as you move the IK target around.  A more free arm would not
have this problem.

To some degree, though, CCD will always exhibit instability like
this... there are invisible thresholds where if you cross them
CCD makes a different decision about how to get to the target.
This is not a big deal though... if you are going to have a game
character reach an object, you will generally not animate that
by doing IK every frame.  You would do the IK once, and then
animate him such that he enacts the IK solution.  Thus the
jitteriness of CCD doesn't matter much, because you just pick
one solution and go that way.

One reason why the arm's joint limits are overly restrictive
is that I didn't have enough time to implement multiple reach
windows per joint.  These windows would overlap and be embedded
in different planes.  It would not be very much work to implement
that in this program, but I just reached the point where I had
to stop and go on to my other work.  If you are interested in
how to implement this, email me.

This app exhibits the typical CCD problem of the end-joints (here,
the wrist) bending too much to reach the target.  You can
overcome this by some hacks (damping motions if they are out near
the leaf of the CCD tree) or by more sophisticated methods (having
a general body knowledge scheme that tells your dude how he
prefers to move).  I didn't put any such thing in, because I 
wanted to keep this app focused on the quaternion math.


Anyway; I hope this app can be of some use to people studying IK.

    -Jonathan  (jon@bolt-action.com)

UPDATE 11 August 2003: Fixed a numerical instability due to
floating-point error [ensured that the parameter is clamped
to [-1, 1] before calling acos()].
