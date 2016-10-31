Readme File for PathDemo
------------------------

PathDemo was developed by Bryan Stout as a means of visually illustrating how 
different pathfinding algorithms worked.  It was developed for his lecture at 
the 1996 Computer Game Developers' Conference, and has since been enhanced.  It was used as the source of the illustrations for his Game 
Developer Magazine article "Smart Moves: Intelligent Path-Finding," and is 
available here to download from GDM's website.  This file explains the basics 
of how PathDemo and its components work, there having been no help file 
developed for it yet.  

PathDemo was developed in Delphi 2.0, and so needs Win95 or Windows NT to run.

This version of PathDemo may be distributed freely, as long as this unaltered 
ReadMe file accompanies it.


Overview
--------

In PathDemo, one can edit a square grid map, select a search algorithm, and 
observe how the search functions in the map.  Functions available include:
- Map Edit: lay out terrain of varying cost, assign start/goal nodes.
- Search Control: start, pause, continue, clear search; change display speed; 
   zoom in or out.
- Algorithm Selection: choose search algorithm, assign appropriate parameters.
- Heuristic Selection: assign heuristic weight, choose distance approximation 
   function.


Map Editor Panel and Map Display
---------------------------------

The Map Display shows a square grid which is the map used for searching paths.  
The map begins at the largest magnification, and all the squares begin with 
clear terrain.  The grid's full size is 40 columns by 32 rows.

The Map Editor panel contains a set of radio buttons showing the different 
terrains that can be assigned to the map tiles -- with their relative costs -- 
including blocked terrain, and the start and goal nodes.

- To show different parts of the map grid, use the scroll bars beside the grid.
- To zoom the map grid out or in, use the "Map Size" track bar in Search 
Control panel in the upper right corner.
- To place a start node in the map, click the Start radio button, and then 
click on any square in the map.  Clicking in other squares will move the start 
to that location -- there can be only one start square.  The Goal node 
placement works in exactly the same way.  The location of the start and goal 
nodes has no effect on the terrain cost, and vice versa.
- To change the terrain cost of map squares, click on the terrain you want to
paint on the map.  Every individual square you click on will change to that 
terrain.  Clicking and dragging will paint every square in the rectangle which 
has the the click and current squares as corners.  To correct mistakes, you 
will have to paint them back to the way you want them.   


Search Algorithm Panel
----------------------

In this panel, you can select the search algorithm, and alter parameters 
affecting them.  These are the algorithm choices available:

First, there are unplanned searches, which simply head towards the goal, and
deal with obstacles as they come up (for details on the algorithms, see the 
article "Smart Moves"):

- Random Bounce. If an obstacle is detected, head in a random direction for 
one square, and try again.
- Simple Trace. Trace around obstacles until one is heading in the same 
direction again.
- Robust Trace. Compute the line segment joining the blocking spot and the 
goal.  Trace until that line is crossed again.

All the other searches plan the whole route ahead.  Some of them use a 
recursive stack to keep track of the search's progress:

- Depth-First Search (DFS).  This simply tries all paths out to a given depth.
- Iterative-Deepening Depth-First Search (IDDF).  This does the DFS search 
at increasing depths, until a path is found. 
- Iterative-Deepening A-Star Search (IDA*).  This is like IDDF, but it keeps 
track of the cost of the path from home, and estimates the distance to the 
goal.  If their sum is too great, the search cuts off (but this cutoff depth
keeps increasing until the path is found).

The remaining searches keep a couple of lists of the squares visited -- Open, 
for those not examined yet, and Closed, for those which have been.  When each 
node is examined, its neighbors are put on Open: 

- Breadth-First Search.  Open is just a FIFO queue, so all the nodes are 
expanded in the order they are place on Open.
- Dijkstra's Algorithm.  Similar to breadth-first, but it keeps track of the 
distance from the goal of all the squares.  Open is kept sorted in order by 
this distance, and nodes can be re-ordered in the lists (and even moved from 
Closed back to Open) if a shorter path to the square is found.
- Best-First Search.  This sorts nodes in Open according to the shortest 
estimated distance to the goal.  This makes the search very fast, but it 
ignores the cost of terrain.
- A-Star Search (A*).  The nodes in Open are sorted by the sum (f) of the 
distance from start (g) and the estimate to the goal (h).  All around, the 
most robust of these search algorithms.


- To view the algorithm choices, click on the up or down arrows at the right 
end of the list box at the top of the panel.
- To select an algorithm, click on the choise that is visible; the selected 
choice is colored blue.  The seach won't run if an algorithm is not chosen.
- To specify a bi-directional search for Open list-based searches, click the
"Bidirectional Search" check box.  Note: this does not yet work perfectly, as 
it takes the first solution found which joins both ends, rather than waiting 
for the best one.
The remaining controls affect stack-based searches (DFS, IDDF, IDA*):
- To save, at each square, the shortest path discovered to that square, click 
the "Save Min Path Length" check box.  With this unchecked, the stack-based 
searches are horribly inefficient, retreading old ground frequently (try it!).
- To have the search always aim first towards the goal, check the "Aim to Goal
First" check box.  Also a big help.
- To set the depth for DFS, click on the arrows of the "Search Depth" spin 
control, or enter a value into the box directly.


Heuristic Estimate Panel
------------------------

The controls here control how the Best-First and A* searches make estimates
of the distance remaining to the goal.  This estimate can vary depending on 
one's application; PathDemo always computes it by multiplying a constant weight 
times the estimated true distance.  Both of these factors can be altered.

- To change the constant weight, move the slider in the Heuristic Weight track
bar.  Note that if the terrain is all clear, '1' is the correct value.  If 
there is much terrain of higher cost, other values may give a better 
approximation.  A '0' yields the same as Dijkstra's search; a value higher than 
all the terrain cost yields the same as Best-first search.
- To choose a different distance function, view the different entries in the 
Distance Function list box, and click the one desired.  

The different distance functions are:
- The maximum of the differences in the X and Y coordinates (max(dx,dy)).
- The Euclidean distance (square root of sum of squares of dx, dy).
- The diagonal shortcut.  A diagonal route is followed until one is directly 
up, down, or across from the goal, and an orthogonal distance is used for the 
remainder of the way.  PathDemo uses 1.4 as an estimate of the diagonal 
distance.
- The Manhattan distance (dx + dy).
If the start were at [1,1], and the goal at [5,4], then dx=4, dy=3, and the 
different distance functions would give us 4, 5, 6.2, and 7 respectively.  In 
PathDemo's map (squares, with diagonal moves allowed), the diagonal shortcut 
gives the true distance between two squares as the crow flies; the first 
two give underestimates, the last one an overestimate.


Search Control Panel
--------------------

This panel has controls over starting, stopping, and clearing the search, and 
over the search speed and map size.

When the search runs, its visual display depends on the type of search being
done:
- Unplanned searches show a blue diamond moving across the map, leaving a blue
trail behind it showing its past moves.
- Stack-based searches show a red line for the path being considered, which 
snakes in and out as different routes are considered.
- List-based searches outline the squares in different colors according to 
their situation: 
   - green for nodes on the Open list 
   - red for the current node (the one just popped from Open)
   - blue for nodes on the Closed list
A dark red line connects each square with its parent node.
- For both list-based and stack-based searches, when a final path is found, its 
squares are outlined in bright green, and its path is a bright red line of 
double thickness.

- To start the search, click on the "Go" button.  This will also disable most 
of the other controls, and change the button's label to "Pause".
- To pause the search temporarily, press the "Pause" button.  The changes the 
label to "Continue", and  enables most of the controls, so one can alter the 
map terrain and heuristic estimate.  The start and goal nodes, and the search 
algorithm, cannot be altered mid-stream.
- To continue the search after pausing it, click the "Continue" button, which
has the same effect as clicking "Go". 
- To clear all the search, click "Clear".  This is only possible when the 
search is paused, or after the search has finished.  Clearing clears out all 
the search information and enables all the controls, but leaves the map and 
the control values unaltered.
- To change the speed of the search, move the slider in the Search Speed track 
bar.  The track bar adjusts the speed exponentially, so the speed can range 
from excruciatingly slow to almost instantaneous.


Afterword
---------

There are several more features I'd like to add to PathDemo.  I am interested 
in your feedback about it, to know whether it has helped you, and what 
improvements you'd like to see.  

Comments can be directed to gdmag@mfi.com, who will forward them to me.


Regards,
Bryan Stout

----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----|

