///////////////////////////////////////////////////////////////////////////////
//
// LoadSkel.h 
//
// Purpose: implementation of the Custom Mesh file Loader
//
// Created:
//		JL 9/12/99		
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1999 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////

#if !defined(LoadSkel_H__INCLUDED_)
#define LoadSkel_H__INCLUDED_

#define MAX_STRINGLENGTH	255

#include "Skeleton.h"

BOOL LoadSkeleton(CString name,t_Bone *root);

#endif // !defined(LoadSkel_H__INCLUDED_)
