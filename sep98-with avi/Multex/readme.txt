
(c) Copyright ATI Research, Inc. 1998

=========================================
 Special Game Developer Magazine Release
=========================================


Send bugs and feedback to MulTex@atitech.com


MulTex - An interactive illustration for learning and testing
         Multiple Texture support in Direct3D under DirectX 6


     MulTex is designed to help Direct3D application and driver
writers understand the multiple texture support provided by MicroSoft's
Direct3D in DirectX 6.  Due to its extreme flexibility, the model
used to control texture mapping operations--particularly when texture
mapping from multiple textures--in DirectX 6 can initially appear
complex and potentially confusing.  MulTex lays out Direct3D's
abstracted texture blending units graphically in order to allow
developers to experiment with the units' functionality and see
rendered results interactively.  The user can also "probe" the
texture blending units at various stages to see intermediate results.

     The MulTex interface consists of three windows: A window containing
an interactive illustration of the Direct3D abstracted texture blending
units, a window showing the Direct3D code necessary to put the blending
units in the corresponding states, and a window which shows pixels as they
progress through the pipeline.  This last window, the Probe Window, shows
pixels in the RGB and A channels side by side for both a user selectable
probe point and the final output pixels.  The render states shown in the
Interactive Illustration Window can be modified by context-sensitive
pop-ups and the Code and Probe windows will be updated accordingly.

  How to use:  Clicking on any of the Args or Ops in the Texture
    boxes (blue) will bring up a pop-up menu of appropriate settings.
    Selecting these will update the states of the boxes, show the
    associated code in the Direct3D Code Window, update the probe window,
    and draw abstract "connections" between relevant sources and
    inputs.  The user may "probe" the abstraction at various stages
    of the pixel pipeline.  The user can also open TGA files to use as
    textures in the app.  Several sample TGAs are included.  When an
    interesting set of render states is found, the code used to select
    those render states can be copied from the Code Window and pasted
    into a developer's own application.



Bug Reports and Feedback
------------------------

Send bugs and feedback to MulTex@atitech.com



Revision History
----------------


 [10 Aug 98] - Special Release to Game Developer Magazine

   * Interactive Illustration Window
     - Graphically presents the texture blending units of the
       DirectX 6.0 spec and allows the user to set specific
       states of the multiple texture pipeline.  Currently allows
       single and dual texture modes (most common in hardware).
       All render state boxes can be modified with context sensitive
       popup menus.  Probe points (indicated with magnifying glass
       icons) can be set by clicking on connection lines at the
       outputs of Arguments and operations.

   * Code Window
     - Shows code necessary to set Direct3D render states corresponding
       to Interactive Illustration Window.  Code can be selected and
       copied to the clipboard, to allow developers to paste right into
       their applications.

   * Probe Window
     - Shows RGB and A quads for current probe point and final
       output.  The Alpha channels are drawn in grayscale in the right
       column of the window.

   * Possible improvements:
     - Write color data and filenames into Registry to make them persist
       between runs of app
     - Add Increment/Decrement slider to alpha channel in color dialog box
     - Better color dialogs in general
     - Add support for currently greyed-out OPs (bumpy ones probably need a
       third texture blending unit)


 [13 Feb 98] - Most likely the final multi-texture programming model?

   * Interactive Illustration Window
     - Graphically presents the texture blending units of the
       DirectX 6.0 spec and allows the user to set specific
       states of the multiple texture pipeline.  Currently allows
       single and dual texture modes (most common in hardware).
       All render state boxes can be modified with context sensitive
       popup menus.  Probe points (indicated with magnifying glass
       icons) can be set by clicking on connection lines at the
       outputs of Arguments and operations.

   * Code Window
     - Shows code necessary to set Direct3D render states corresponding
       to Interactive Illustration Window.  Code can be selected and
       copied to the clipboard, to allow developers to paste right into
       their applications.

   * Probe Window
     - Shows RGB and A quads for current probe point and final
       output.  The Alpha channels are drawn in grayscale in the right
       column of the window.

   * Yet To Do:
     - Make checkmarks update properly in Windows menu
     - Write color data and filenames into Registry to make them persist
       between runs of app
     - Add Increment/Decrement slider to alpha channel in color dialog box
     - Better color dialogs in general
     - Add support for currently greyed-out OPs (bumpy ones require a third texture
       blending unit!)
     - Put shortcut menu back in?  It's been killed in the spec, so not likely.


 [13 Feb 98] - First Release to SDK Group at Microsoft

   * Interactive Illustration Window
     - Graphically presents the texture blending units of the
       DirectX 6.0 spec and allows the user to set specific
       states of the multiple texture pipeline.  Currently allows
       single and dual texture modes (most common in hardware).
       All render state boxes can be modified with context sensitive
       popup menus.  Probe points (indicated with magnifying glass
       icons) can be set by clicking on connection lines at the
       outputs of Arguments and operations.

   * Code Window
     - Shows code necessary to set Direct3D render states corresponding
       to Interactive Illustration Window.  Code can be selected and
       copied to the clipboard, to allow developers to paste right into
       their applications.

   * Probe Window
     - Shows RGB and A quads for current probe point and final
       output.  The Alpha channels are drawn in grayscale in the right
       column of the window.

   * Yet To Do:
     - Make checkmarks update properly in Windows menu
     - Write color data and filenames into Registry to make them persist
       between runs of app
     - Add Increment/Decrement slider to alpha channel in color dialog box
     - Better color dialogs in general
     - Add support for currently greyed-out OPs (some require a third texture
       blending unit!)
     - Put shortcut menu back in?


 [30 Jan 98]

   * Interactive Illustration Window
     - Graphically presents the texture blending units of the
       DirectX 6.0 spec and allows the user to set specific
       states of the multiple texture pipeline.  Currently allows
       single and dual texture modes (most common in hardware).
       All render state boxes can be modified with context sensitive
       popup menus.  Probe points (indicated with magnifying glass
       icons) can be set by clicking on connection lines at the
       outputs of Arguments and operations.

   * Code Window
     - Shows code necessary to set Direct3D render states corresponding
       to Interactive Illustration Window.  Code can be selected and
       copied to the clipboard, to allow developers to paste right into
       their applications.

   * Probe Window
     - Shows RGB and A quads for current probe point and final
       output.  The Alpha channels are drawn in grayscale in the right
       column of the window.

   * Notes:

     - Shortcuts have been left out for this release
     - This release enables the blending operations (D3DTOP_BLENDDIFFUSEALPHA,
       D3DTOP_BLENDTEXTUREALPHA and D3DTOP_BLENDFACTORALPHA)

   * Yet To Do:
     - Make checkmarks update properly in Windows menu
     - Write color data and filenames into Registry to make them persist
       between runs of app
     - Add Increment/Decrement slider to alpha channel in color dialog box
     - Better color dialogs in general
     - Add support for currently greyed-out OPs (some require a third texture
       blending unit!)



 [23 Jan 98]

   * Interactive Illustration Window
     - Graphically presents the texture blending units of the
       DirectX 6.0 spec and allows the user to set specific
       states of the multiple texture pipeline.  Currently allows
       single and dual texture modes (most common in hardware).
       All render state boxes can be modified with context sensitive
       popup menus.  Probe points (indicated with magnifying glass
       icons) can be set by clicking on connection lines at the
       outputs of Arguments and operations.

   * Code Window
     - Shows code necessary to set Direct3D render states corresponding
       to Interactive Illustration Window.  When appropriate, shortcut
       notation will also appear in the window, properly commented.

   * Probe Window
     - Shows RGB and A quads for current probe point and final
       output.  The Alpha channels are drawn in grayscale in the right
       column of the window.

   * Yet To Do:
     - Make text in Code Window select-able and copy-able
     - Make checkmarks update properly in View menu
     - Write color data and filenames into Registry to make them persist
       between runs of app
     - Add Increment/Decrement arrows to alpha channel in color dialog box
     - Shortcut dialog
     - Specular
     - Better color dialogs in general
     - Alpha and Inv indicators for arguments in Interactive Illustration Window
     - Color comments in code window
     - Fix artifacts in code window
     - Show current settings, if any, in shortcut dialog
     - Add ability to load more than just 128x128 TGA files



 [31 Dec 97]

   * Interactive Illustration Window
     - Graphically presents the texture blending units of the
       DirectX 6.0 spec and allows the user to set specific
       states of the multiple texture pipeline.  Currently allows
       single and dual texture modes (most common in hardware).
       All render state boxes can be modified with context sensitive
       popup menus.

   * Code Window
     - Shows code necessary to set Direct3D render states corresponding
       to Interactive Illustration Window

   * Probe Window (new in this release)
     - Shows RGB and A quads for current probe point and final
       output.  The Alpha channels are drawn in grayscale in the right
       column of the window.

   * Output Window
     - Not currently implemented

   * Yet To Do:
     - Implement Polygon Window
     - Gray out meaningless options in pop-ups? (make more context-sensitive)
     - Make text in Code Window select-able and copy-able
     - Finish layout of dual texture mode in Interactive Illustration Window.
     - Complete connections between Args and Ops, and the Device
       box and Texture boxes.
     - Make checkmarks update properly in View menu
     - Pickable probe points
     - Make probed connections drawn with dotted lines
     - Show alpha channel (in grayscale) in Probe Window
     - Write color data into Registry to make it persist between runs of app
     - Add Increment/Decrement arrows to alpha channel in color dialog box

 [4 Dec 97]

   * Interactive Illustration Window
     - Graphically presents the texture blending units of the
	 DirectX 6.0 spec and allows the user to set specific
 	 states of the multiple texture pipeline.  Currently allows
	 single and dual texture modes (most common in hardware).

   * Code Window
     - Shows code necessary to set Direct3D render states corresponding
       to Interactive Illustration Window

   * Polygon Window
     - Currently shows GDI-drawn placeholder triangle

   * Yet To Do:
     - Integrate Direct3D rendering into application (Polygon Window)
     - Gray out meaningless options in pop-ups (make context-sensitive)
     - Make text in Code Window select-able and copy-able
     - Finish layout of dual texture mode in Interactive Illustration Window.
     - Complete abstract connections between Args and Ops, and the Device
       box and Texture boxes.
     - Make the "Constant" box pop a dialog for inputing a scalar.
     - Make the "Cdiff" box pop a dialog for inputing the colors of the
       polygon vertices.
     - Make closing of Polygon Window or Code Window not close app
     - Checkmark and separator line which separates INV flags from rest
     - Generate proper code if INV flag is set (OR'ed in)
     - Make checkmarks update properly in View menu.



