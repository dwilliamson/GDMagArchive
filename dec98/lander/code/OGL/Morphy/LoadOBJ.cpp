///////////////////////////////////////////////////////////////////////////////
//
// LoadOBJ.cpp : implementation file
//
// Purpose:	Implementation of OpenGL Window of OBJ Loader
//
// Created:
//		JL 9/23/98		
//
// Notes: This version doesn't used shared vertices in a vertex array.  That
//		  would be a faster way of doing things.  This creates 3 vertices per
//        triangle.
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1998 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "loadOBJ.h"

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
// Procedure:	LoadMaterialLib
// Purpose:		Handles the Loading of a Material library
// Arguments:	Name of the Material Library
///////////////////////////////////////////////////////////////////////////////		
void LoadMaterialLib(CString name,t_Visual *visual)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int cnt;
	char buffer[MAX_STRINGLENGTH];
	CStringArray words;
	CString temp;
	FILE *fp;
///////////////////////////////////////////////////////////////////////////////
	strcpy(visual->map,"");
	fp = fopen((LPCTSTR)name,"r");
	if (fp != NULL)
	{
		// FIRST PASS SETS UP THE NUMBER OF OBJECTS IN THE FILE
		while (!feof(fp))
		{
			fgets(buffer,MAX_STRINGLENGTH,fp);	// GET A STRING FROM THE FILE
			ParseString(buffer,&words,&cnt);	// BREAK THE STRING INTO cnt WORDS
			if (cnt > 0)						// MAKE SURE SOME WORDS ARE THERE
			{
				temp = words.GetAt(0);			// CHECK THE FIRST WORK
				if (temp.GetLength() > 0)
				{
					if (temp == "Ka")			// AMBIENT
					{
						visual->Ka.r = atof(words.GetAt(1));
						visual->Ka.g = atof(words.GetAt(2));
						visual->Ka.b = atof(words.GetAt(3));
					}
					else if (temp == "Kd")		// DIFFUSE COLOR
					{
						visual->Kd.r = atof(words.GetAt(1));
						visual->Kd.g = atof(words.GetAt(2));
						visual->Kd.b = atof(words.GetAt(3));
					}
					else if (temp == "Ks")		// SPECULAR COLOR
					{
						visual->Ks.r = atof(words.GetAt(1));
						visual->Ks.g = atof(words.GetAt(2));
						visual->Ks.b = atof(words.GetAt(3));
					}
					else if (temp == "Ns")		// SPECULAR COEFFICIENT
					{
						visual->Ns = atof(words.GetAt(1));
					}
					else if (temp == "map_Kd")	// TEXTURE MAP NAME
					{
						strcpy(visual->map,(LPCTSTR)words.GetAt(1));
					}
				}
			}
			words.RemoveAll();		// CLEAR WORD BUFFER
		}
		fclose(fp);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Procedure:	HandleFace
// Purpose:		Handles the Face Line in an OBJ file.  Extracts index info to 
//				a face Structure
// Arguments:	Array of words from the face line, place to put the data
// Notes:		Not an Official OBJ loader as it doesn't handle more then
//				3 vertex polygons.  This only handles Triangles
///////////////////////////////////////////////////////////////////////////////		
void HandleFace(CStringArray *words,t_faceIndex *face)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop;
	CString temp;
	CString vStr,nStr,tStr;		// HOLD POINTERS TO ELEMENT POINTERS
	int nPos,tPos;
///////////////////////////////////////////////////////////////////////////////
	// LOOP THROUGH THE 3 WORDS OF THE FACELIST LINE, WORD 0 HAS 'f'
	for (loop = 1; loop < 4; loop++)
	{
		temp = words->GetAt(loop);			// GRAB THE NEXT WORD
		// FACE DATA IS IN THE FORMAT vertex/texture/normal
		tPos = temp.Find('/');				// FIND THE '/' SEPARATING VERTEX AND TEXTURE
		vStr = temp.Left(tPos);				// GET THE VERTEX NUMBER
		temp.SetAt(tPos,' ');				// CHANGE THE '/' TO A SPACE SO I CAN TRY AGAIN
		nPos = temp.Find('/');				// FIND THE '/' SEPARATING TEXTURE AND NORMAL
		tStr = temp.Mid(tPos + 1, nPos - tPos - 1);		// GET THE TEXTURE NUMBER
		nStr = temp.Right(temp.GetLength() - nPos - 1);	// GET THE NORMAL NUMBER
		face->v[loop - 1] = atoi(vStr);		// STORE OFF THE INDEX FOR THE VERTEX
		face->t[loop - 1] = atoi(tStr);		// STORE OFF THE INDEX FOR THE TEXTURE
		face->n[loop - 1] = atoi(nStr);		// STORE OFF THE INDEX FOR THE NORMAL
	}
}
///// HandleFace //////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// Procedure:	LoadOBJ
// Purpose:		Load an OBJ file into the current bone visual
// Arguments:	Name of 0BJ file and pointer to bone
// Notes:		Not an Official OBJ loader as it doesn't handle more then
//				3 vertex polygons or multiple objects per file.
///////////////////////////////////////////////////////////////////////////////		
BOOL LoadOBJ(char *filename,t_Visual *visual)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop,loop2,cnt;
	char buffer[MAX_STRINGLENGTH];
	CStringArray words;
	CString temp;
	FILE *fp;
	long vCnt = 0, nCnt = 0, tCnt = 0, fCnt = 0;
	long vPos = 0, nPos = 0, tPos = 0, fPos = 0;
	tVector *vertex = NULL,*normal = NULL,*texture = NULL;
	t_faceIndex *face = NULL;
	float *data;
///////////////////////////////////////////////////////////////////////////////
	fp = fopen(filename,"r");
	if (fp != NULL)
	{
		// FIRST PASS SETS UP THE NUMBER OF OBJECTS IN THE FILE
		while (!feof(fp))
		{
			fgets(buffer,MAX_STRINGLENGTH,fp);	// GET A STRING FROM THE FILE
			ParseString(buffer,&words,&cnt);	// BREAK THE STRING INTO cnt WORDS
			if (cnt > 0)						// MAKE SURE SOME WORDS ARE THERE
			{
				temp = words.GetAt(0);			// CHECK THE FIRST WORK
				if (temp.GetLength() > 0)
				{
					if (temp[0] == 'v')			// ONLY LOOK AT WORDS THAT START WITH v
					{
						if (temp.GetLength() > 1 && temp[1] == 'n')			// vn IS A NORMAL
							nCnt++;
						else if (temp.GetLength() > 1 && temp[1] == 't')	// vt IS A TEXTURE 
							tCnt++;
						else
							vCnt++;											// v IS A VERTEX
					}
					else if (temp[0] == 'f')
						fCnt++;												// f IS A FACE
				}
			}
			words.RemoveAll();		// CLEAR WORD BUFFER
		}

		// NOW THAT I KNOW HOW MANY, ALLOCATE ROOM FOR IT
		if (vCnt > 0)
		{
			vertex = (tVector *)malloc(vCnt * sizeof(tVector));
			if (nCnt > 0)
				normal = (tVector *)malloc(nCnt * sizeof(tVector));
			if (tCnt > 0)
				texture = (tVector *)malloc(tCnt * sizeof(tVector));
			if (fCnt > 0)
				face = (t_faceIndex *)malloc(fCnt * sizeof(t_faceIndex));

			fseek(fp,0,SEEK_SET);

			// NOW THAT IT IS ALL ALLOC'ED.  GRAB THE REAL DATA
			while (!feof(fp))
			{
				fgets(buffer,MAX_STRINGLENGTH,fp);
				ParseString(buffer,&words,&cnt);
				if (cnt > 0)
				{
					temp = words.GetAt(0);
					if (temp.GetLength() > 0)
					{
						if (temp[0] == 'v')		// WORDS STARTING WITH v
						{
							if (temp.GetLength() > 1 && temp[1] == 'n')	// vn NORMALS
							{
								normal[nPos].x = atof(words.GetAt(1));
								normal[nPos].y = atof(words.GetAt(2));
								normal[nPos].z = atof(words.GetAt(3));
								nPos++;
							}
							else if (temp.GetLength() > 1 && temp[1] == 't')	// vt TEXTURES
							{
								texture[tPos].u = atof(words.GetAt(1));
								texture[tPos].v = atof(words.GetAt(2));
								tPos++;
							}
							else											// VERTICES
							{
								vertex[vPos].x = atof(words.GetAt(1));
								vertex[vPos].y = atof(words.GetAt(2));
								vertex[vPos].z = atof(words.GetAt(3));
								vPos++;
							}
						}
						else if (temp[0] == 'f')			// f v/t/n v/t/n v/t/n	FACE LINE
						{
							if (words.GetSize() > 4)
							{
								sprintf(buffer,"Face %d has more then 3 vertices",fPos);
								MessageBox(NULL,buffer,"ERROR",MB_OK);
							}
							HandleFace(&words,&face[fPos]);
							fPos++;
						}
						else if (temp == "mtllib")  // HANDLE THE MATERIAL LIBRARY
						{
							LoadMaterialLib(words.GetAt(1),visual);
						}
					}
				}
				words.RemoveAll();		// CLEAR WORD BUFFER
			}

			if (nCnt > 0)
			{
				if (tCnt > 0)
				{
					visual->dataFormat = GL_T2F_N3F_V3F;
					visual->vSize = 8;					// 2 texture, 3 normal, 3 vertex
					visual->vertexData = (float *)malloc(sizeof(float) * visual->vSize * fPos * 3);
					visual->triCnt = fPos;
				}
				else
				{
					visual->dataFormat = GL_N3F_V3F;
					visual->vSize = 6;					// 3 floats for normal, 3 for vertex
					visual->vertexData = (float *)malloc(sizeof(float) * visual->vSize * fPos * 3);
					visual->triCnt = fPos;
				}
			}
			else
			{
				visual->dataFormat = GL_V3F;
				visual->vSize = 3;					// 3 floats for vertex
				visual->vertexData = (float *)malloc(sizeof(float) * visual->vSize * fPos * 3);
				visual->triCnt = fPos;
			}

			data = visual->vertexData;
			for (loop = 0; loop < fPos; loop++)
			{
				for (loop2 = 0; loop2 < 3; loop2++)
				{
					// ALL FACE INDICES ARE 1 BASED INSTEAD OF 0
					if (tCnt > 0)	// IF TEXTURE COORDS WRITE OUT THOSE
					{
						*data++ = texture[face[loop].t[loop2] - 1].u;
						*data++ = texture[face[loop].t[loop2] - 1].v;
					}
					if (nCnt > 0)	// IF THERE ARE NORMALS WRITE THOSE OUT
					{
						*data++ = normal[face[loop].n[loop2] - 1].x;
						*data++ = normal[face[loop].n[loop2] - 1].y;
						*data++ = normal[face[loop].n[loop2] - 1].z;
					}
					*data++ = vertex[face[loop].v[loop2] - 1].x;	// SAVE OUT VERTICES
					*data++ = vertex[face[loop].v[loop2] - 1].y;
					*data++ = vertex[face[loop].v[loop2] - 1].z;
				}
			}

			if (vertex) free(vertex);
			if (normal) free(normal);
			if (texture) free(texture);
			if (face) free(face);
		}

		fclose(fp);
	}
	else
		return FALSE;
	return TRUE;
}