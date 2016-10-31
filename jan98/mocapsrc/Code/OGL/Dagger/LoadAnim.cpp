///////////////////////////////////////////////////////////////////////////////
//
// LoadAnim.cpp 
//
// Purpose: implementation of the Biovision BVA Loaded
//
// Created:
//		JL 9/5/97		
//
// Todo:
//		I WILL PROBABLY PULL OUT THE CHANNELS INTO A NEW STRUCT
//		ADD SPEED OF PLAYBACK VARIABLE TO CHANNEL STRUCT
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
#include "LoadAnim.h"

///////////////////////////////////////////////////////////////////////////////
// Function:	ParseString
// Purpose:		Actually breaks a string of words into individual pieces
// Arguments:	Source string in, array to put the words and the count
///////////////////////////////////////////////////////////////////////////////
void ParseString(char *buffer,CStringArray *words,int *cnt)
{
/// Local Variables ///////////////////////////////////////////////////////////
	CString in = buffer, temp;
///////////////////////////////////////////////////////////////////////////////
	
	in.TrimLeft();
	in.TrimRight();
	*cnt = 0;
	do 
	{
		temp = in.SpanExcluding(" \t");		// GET UP TO THE NEXT SPACE OR TAB
		words->Add(temp);
		if (temp == in) break;
		in = in.Right(in.GetLength() - temp.GetLength());
		in.TrimLeft();
		*cnt = *cnt + 1;			
	} while (1);
	*cnt = *cnt + 1;
}
//// ParseString //////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	LoadBVA
// Purpose:		Actually load a BVA file into the system
// Arguments:	Name of the file to open and root skeleton to put it in
///////////////////////////////////////////////////////////////////////////////
BOOL LoadBVA(CString name,t_Bone *root)
{
/// Local Variables ///////////////////////////////////////////////////////////
	FILE *fp;		// I PREFER THIS STYLE OF FILE ACCESS
	int loop,loop2,cnt;
	char buffer[MAX_STRINGLENGTH];
	CStringArray words;
	CString temp;
	int frameCnt;
	float frameTime;
	t_Bone *tempBones,*curBone;
	float *tempChannel,*fptr;
///////////////////////////////////////////////////////////////////////////////
	// OPEN THE BVA FILE
	if (fp = fopen((LPCTSTR)name,"r")) {
		while (!feof(fp))
		{
			fgets(buffer,MAX_STRINGLENGTH,fp);
			ParseString(buffer,&words,&cnt);
			// SEE IF WE CAN FIND THE SEGMENT KEYWORD
			if (words.GetAt(0) == "Segment:")
			{
				// IF SO, WE FOUND A BONE SO ALLOC ROOM FOR IT
				tempBones = (t_Bone *)malloc((root->childCnt + 1) * sizeof(t_Bone));
				if (root->childCnt > 0 && root->children != NULL)
				{
					// COPY OVER THE ONES THAT WERE ALREADY THERE
					memcpy(tempBones,root->children,root->childCnt * sizeof(t_Bone));
					free(root->children);
				}
				curBone = &tempBones[root->childCnt++];
				root->children = tempBones;
				// COPY THE NAME INTO THE BONE
				strcpy(curBone->name,words.GetAt(1));
				// SET THE ID TO THE BONE NUMBER
				curBone->id = root->childCnt;
				ResetBone(curBone, root);		// SETUP INITIAL BONE SETTINGS
				words.RemoveAll();		// CLEAR WORD BUFFER
				// NEXT GET THE FRAMECOUNT
				fgets(buffer,MAX_STRINGLENGTH,fp);
				ParseString(buffer,&words,&cnt);
				// NEXT LINE SHOULD BE FRAMES THEN A COUNT
				if (words.GetAt(0) == "Frames:" && cnt == 2)
				{
					frameCnt = atoi(words.GetAt(1));
					words.RemoveAll();		// CLEAR WORD BUFFER
					// NEXT GET THE FRAMETIME
					fgets(buffer,MAX_STRINGLENGTH,fp);
					ParseString(buffer,&words,&cnt);
					if (words.GetAt(0) == "Frame" &&
						words.GetAt(1) == "Time:" &&
						cnt == 3)
					{
						frameTime = (float)atof(words.GetAt(2));
						words.RemoveAll();		// CLEAR WORD BUFFER
						// TWO JUNK LINES FOR THE UNITS.  I DON'T CARE
						fgets(buffer,MAX_STRINGLENGTH,fp);
						fgets(buffer,MAX_STRINGLENGTH,fp);
						// NOW READY TO LOAD ALL THE ANIMATION
						// ALLOC 9 FLOATS PER FRAME SINCE BVA HAS 9 ELEMENTS TO THE CHANNEL
						tempChannel = (float *)malloc(sizeof(float) * frameCnt * 
							s_Channel_Type_Size[CHANNEL_TYPE_SRT]);
						fptr = tempChannel;
						// LOOP THROUGH THE FRAMES OF ANIMATION DATA
						for (loop = 0; loop < frameCnt; loop++)
						{
							fgets(buffer,MAX_STRINGLENGTH,fp);
							ParseString(buffer,&words,&cnt);
							if (cnt == s_Channel_Type_Size[CHANNEL_TYPE_SRT])
							{
								for (loop2 = 0; loop2 < s_Channel_Type_Size[CHANNEL_TYPE_SRT]; loop2++)
								{
									*fptr = (float)atof(words.GetAt(loop2));
									// CONVERT INCHES TO FEET
									if (loop2 < 3) *fptr = *fptr / 12;
									fptr++;
								}
							}
							else
							{
								sprintf(buffer,"Not Enough Entries in channel %s frame %d\nExpected %d Got %d",curBone->name,loop,s_Channel_Type_Size[CHANNEL_TYPE_SRT],cnt);
								::MessageBox(NULL,buffer,"BVA Load ERROR!!",MB_OK);
								free(tempChannel);
								return FALSE;
							}
							words.RemoveAll();		// CLEAR WORD BUFFER
						}
	
						// SET THE CHANNEL INFO INTO BONE
						curBone->primChannel = tempChannel;
						curBone->primChanType = CHANNEL_TYPE_SRT;
						curBone->primFrameCount = (float)frameCnt;
						curBone->primSpeed = (float)(30.0 * frameTime);	// CONVERT TO FRAME STEP SIZE AT 30FPS
						BoneSetFrame(curBone,0);
					}
					
				}
				else
				{
					sprintf(buffer,"Couldn't Find FrameCnt Area for Bone %s",curBone->name);
					::MessageBox(NULL,buffer,"BVA Load ERROR!!",MB_OK);
					free(tempBones);
					root->childCnt = 0;
					root->children = NULL;
					return FALSE;
				}
			}
		}
		fclose(fp);
		return TRUE;
	}	
	return FALSE;
}
//// LoadBVA //////////////////////////////////////////////////////////////
