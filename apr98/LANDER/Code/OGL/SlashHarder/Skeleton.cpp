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

void DestroySkeleton(t_Bone *root)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop;
	t_Bone *child;
///////////////////////////////////////////////////////////////////////////////
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
		}
		free(root->children);
	}

	root->primChanType = CHANNEL_TYPE_NONE;
	root->secChanType = CHANNEL_TYPE_NONE;
	root->primFrameCount = 0;
	root->secFrameCount = 0;
	root->primCurFrame = 0;
	root->secCurFrame = 0;
	root->primChannel = NULL;
	root->secChannel = NULL;

	root->visualCnt = 0;					// COUNT OF ATTACHED VISUAL ELEMENTS
	root->visuals = NULL;					// POINTER TO VISUALS
	root->childCnt = 0;						// COUNT OF ATTACHED BONE ELEMENTS
	root->children = NULL;					// POINTER TO CHILDREN
}

void ResetBone(t_Bone *bone,t_Bone *parent)
{
	bone->p_scale.x =
	bone->p_scale.y =
	bone->p_scale.z = 1.0;
	bone->s_scale.x =
	bone->s_scale.y =
	bone->s_scale.z = 1.0;
	bone->scale.x =
	bone->scale.y =
	bone->scale.z = 1.0;

	bone->p_rot.x =
	bone->p_rot.y =
	bone->p_rot.z = 0.0;
	bone->s_rot.x =
	bone->s_rot.y =
	bone->s_rot.z = 0.0;
	bone->rot.x =
	bone->rot.y =
	bone->rot.z = 0.0;

	bone->p_trans.x =
	bone->p_trans.y =
	bone->p_trans.z = 0.0;
	bone->s_trans.x =
	bone->s_trans.y =
	bone->s_trans.z = 0.0;
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
	bone->animBlend = 0.0;

	bone->visualCnt = 0;					// COUNT OF ATTACHED VISUAL ELEMENTS
	bone->visuals = NULL;					// POINTER TO VISUALS
	bone->childCnt = 0;						// COUNT OF ATTACHED BONE ELEMENTS
	bone->children = NULL;					// POINTER TO CHILDREN
	bone->parent = parent;
}

void BoneSetFrame(t_Bone *bone,int frame)
{
/// Local Variables ///////////////////////////////////////////////////////////
	float *offset;
///////////////////////////////////////////////////////////////////////////////

	if (bone->primChannel != NULL)
	{
		offset = (float *)(bone->primChannel + (s_Channel_Type_Size[bone->primChanType] * frame));

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

			bone->scale.x = offset[6];
			bone->scale.y = offset[7];
			bone->scale.z = offset[8];
			break;

		}
	}
}

void BoneAdvanceFrame(t_Bone *bone,int direction,BOOL doChildren)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop;
	t_Bone *child;
///////////////////////////////////////////////////////////////////////////////
	if (bone->childCnt > 0)
	{
		child = bone->children;
		for (loop = 0; loop < bone->childCnt; loop++,child++)
		{
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