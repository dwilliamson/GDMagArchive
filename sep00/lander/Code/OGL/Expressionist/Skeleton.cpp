///////////////////////////////////////////////////////////////////////////////
//
// Skeleton.cpp : Animation System Skeleton supprt file
//
// Purpose:	Structure Supprt routines for Hierarchical Animation System
//
// I DIDN'T PUT THESE IN A C++ CLASS FOR CROSS PLATFORM COMPATIBILITY
// SINCE THE ENGINE MAY BE IMPLEMENTED ON CONSOLES AND OTHER SYSTEMS
//
// Created:
//		JL 9/1/97		
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1997 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "skeleton.h"

///////////////////////////////////////////////////////////////////////////////
// Function:	DestroySkeleton
// Purpose:		Clear memory for a skeletal system
// Arguments:	Pointer to bone system
///////////////////////////////////////////////////////////////////////////////
void DestroySkeleton(t_Bone *root)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop;
	t_Bone *child;
///////////////////////////////////////////////////////////////////////////////
	// NEED TO RECURSIVELY GO THROUGH THE CHILDREN
	if (root->childCnt > 0)
	{
		child = root->children;
		for (loop = 0; loop < root->childCnt; loop++,child++)
		{
			if (child->childCnt > 0)
				DestroySkeleton(child);
			if (child->primChannel > NULL)
			{
				free(child->primChannel);
				child->primChannel = NULL;
			}
			if (child->curMatrix)
				free(child->curMatrix);
		}
		free(root->children);
		if (root->curMatrix)
			free(root->curMatrix);
	}

	root->primChanType = CHANNEL_TYPE_NONE;
	root->secChanType = CHANNEL_TYPE_NONE;
	root->primFrameCount = 0;
	root->secFrameCount = 0;
	root->primCurFrame = 0;
	root->secCurFrame = 0;
	root->primChannel = NULL;
	root->secChannel = NULL;

	root->CV_select = NULL;					// POINTER TO WEIGHTS
	root->CV_weight = NULL;					// POINTER TO VISUALS
	root->visualCnt = 0;					// COUNT OF ATTACHED VISUAL ELEMENTS
	root->visuals = NULL;					// POINTER TO VISUALS
	root->childCnt = 0;						// COUNT OF ATTACHED BONE ELEMENTS
	root->children = NULL;					// POINTER TO CHILDREN
}
//// DestroySkeleton //////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	ResetSkeleton
// Purpose:		Reset a skeletal system
// Arguments:	Pointer to bone system
///////////////////////////////////////////////////////////////////////////////
void ResetSkeleton(t_Bone *bone)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop;
	t_Bone *child;
///////////////////////////////////////////////////////////////////////////////
	bone->rot.x = bone->b_rot.x;
	bone->rot.y = bone->b_rot.y;
	bone->rot.z = bone->b_rot.z;

	bone->trans.x = bone->b_trans.x;
	bone->trans.y = bone->b_trans.y;
	bone->trans.z = bone->b_trans.z;

	// NEED TO RECURSIVELY GO THROUGH THE CHILDREN
	if (bone->childCnt > 0)
	{
		child = bone->children;
		for (loop = 0; loop < bone->childCnt; loop++,child++)
		{
			ResetSkeleton(child);
		}
	}
}
//// ResetSkeleton //////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	FreezeSkeleton
// Purpose:		Freeze a skeletal system
// Arguments:	Pointer to bone system
///////////////////////////////////////////////////////////////////////////////
void FreezeSkeleton(t_Bone *bone)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop;
	t_Bone *child;
///////////////////////////////////////////////////////////////////////////////
	bone->b_rot.x = bone->rot.x;
	bone->b_rot.y = bone->rot.y;
	bone->b_rot.z = bone->rot.z;

	bone->b_trans.x = bone->trans.x;
	bone->b_trans.y = bone->trans.y;
	bone->b_trans.z = bone->trans.z;

	// NEED TO RECURSIVELY GO THROUGH THE CHILDREN
	if (bone->childCnt > 0)
	{
		child = bone->children;
		for (loop = 0; loop < bone->childCnt; loop++,child++)
		{
			FreezeSkeleton(child);
		}
	}
}
//// FreezeSkeleton //////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	ResetBone
// Purpose:		Reset the bone system and set the parent bone
// Arguments:	Pointer to bone system, and parent bone (could be null)
///////////////////////////////////////////////////////////////////////////////
void ResetBone(t_Bone *bone,t_Bone *parent)
{
	bone->b_scale.x =
	bone->b_scale.y =
	bone->b_scale.z = 1.0;
	bone->scale.x =
	bone->scale.y =
	bone->scale.z = 1.0;

	bone->b_rot.x =
	bone->b_rot.y =
	bone->b_rot.z = 0.0;
	bone->rot.x =
	bone->rot.y =
	bone->rot.z = 0.0;

	bone->b_trans.x =
	bone->b_trans.y =
	bone->b_trans.z = 0.0;
	bone->trans.x =
	bone->trans.y =
	bone->trans.z = 0.0;

	bone->primChanType = CHANNEL_TYPE_NONE;
	bone->secChanType = CHANNEL_TYPE_NONE;
	bone->primFrameCount = 0;
	bone->secFrameCount = 0;
	bone->primCurFrame = 0;
	bone->secCurFrame = 0;
	bone->primChannel = NULL;
	bone->secChannel = NULL;
	bone->animBlend = 0.0f;

	bone->bsphere = 1.0f;
	bone->visualCnt = 0;					// COUNT OF ATTACHED VISUAL ELEMENTS
	bone->visuals = NULL;					// POINTER TO VISUALS
	bone->childCnt = 0;						// COUNT OF ATTACHED BONE ELEMENTS
	bone->children = NULL;					// POINTER TO CHILDREN
	bone->flags = CHANNEL_TYPE_RXYZ;	// ROTATION (RX RY RZ) ORDER
	bone->parent = parent;
	bone->CV_select = NULL;					// POINTER TO WEIGHTS
	bone->CV_weight = NULL;					// POINTER TO VISUALS
	bone->curMatrix = NULL;
}
//// ResetBone ////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	BoneSetFrame
// Purpose:		Set the animation stream for a bone
// Arguments:	Pointer to bone system, frame to set to
///////////////////////////////////////////////////////////////////////////////
void BoneSetFrame(t_Bone *bone,int frame)
{
/// Local Variables ///////////////////////////////////////////////////////////
	float *offset;
///////////////////////////////////////////////////////////////////////////////

	if (bone->primChannel != NULL)
	{
		offset = (float *)(bone->primChannel + (s_Channel_Type_Size[bone->primChanType] * frame));

		// THIS HANDLES THE INDIVIDUAL STREAM TYPES.  ONLY ONE NOW.
		switch (bone->primChanType)
		{
		// TYPE_SRT HAS 9 FLOATS IN TXYZ,RXYZ,SXYZ ORDER
		case CHANNEL_TYPE_SRT:
			bone->trans.x = offset[0];
			bone->trans.y = offset[1];
			bone->trans.z = offset[2];

			bone->rot.x = offset[3];
			bone->rot.y = offset[4];
			bone->rot.z = offset[5];

// I DON'T REALLY WANT MY ANIMATION TO DEAL WITH SCALE RIGHT NOW 
// EVEN THOUGH IT IS IN THE BVA FILE
//			bone->scale.x = offset[6];
//			bone->scale.y = offset[7];
//			bone->scale.z = offset[8];
			break;

		}
	}
}
//// BoneAdvanceFrame /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	BoneAdvanceFrame
// Purpose:		Increment the animation stream for a bone and possible the
//				children attached to that bone1
// Arguments:	Pointer to bone system, Delta frame value to move, if it is recursive
///////////////////////////////////////////////////////////////////////////////
void BoneAdvanceFrame(t_Bone *bone,int direction,BOOL doChildren)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop;
	t_Bone *child;
///////////////////////////////////////////////////////////////////////////////
	// THERE MUST BE SOME THINGS TO ADVANCE 
	if (bone->childCnt > 0)
	{
		child = bone->children;
		for (loop = 0; loop < bone->childCnt; loop++,child++)
		{
			// ADVANCE THE STREAM
			child->primCurFrame += direction;
			if (child->primCurFrame >= child->primFrameCount)
				child->primCurFrame = 0;
			if (child->primCurFrame < 0)
				child->primCurFrame += child->primFrameCount;
			BoneSetFrame(child,(int)child->primCurFrame);
			if (doChildren && child->childCnt > 0)				// IF THIS CHILD HAS CHILDREN
				BoneAdvanceFrame(child,direction,doChildren);	// RECURSE DOWN HIER
		}
	}
}
//// BoneAdvanceFrame /////////////////////////////////////////////////////////////////
