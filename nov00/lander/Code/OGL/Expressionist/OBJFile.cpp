///////////////////////////////////////////////////////////////////////////////
//
// OBJFile.cpp : implementation file
//
// Purpose:	Implementation of OpenGL Window of OBJ Loader/Saver
//
// Created:
//		JL 9/23/98	
// Modified:
//		JL 9/10/00	- Added a OBJ Save routine	
//
// Notes: This handles models with 3 vertices per triangle.
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 2000 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "OBJFile.h"

char *gMatName;
// Needs some stuff saved so I can save out the file
CString g_ObjectName, g_MaterialFileName;

///////////////////////////////////////////////////////////////////////////////
// Function:	ParseOBJString
// Purpose:		Actually breaks a string of words into individual pieces
// Arguments:	Source string in, array to put the words and the count
///////////////////////////////////////////////////////////////////////////////
void ParseOBJString(char *buffer,CStringArray *words,int *cnt)
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
//// ParseOBJString //////////////////////////////////////////////////////////////

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
	int matCnt = 0,curMat = -1;
///////////////////////////////////////////////////////////////////////////////
	strcpy(visual->map,"");
	fp = fopen((LPCTSTR)name,"r");
	if (fp != NULL)
	{
		// FIRST PASS SETS UP THE NUMBER OF OBJECTS IN THE FILE
		while (!feof(fp))
		{
			fgets(buffer,MAX_STRINGLENGTH,fp);	// GET A STRING FROM THE FILE
			ParseOBJString(buffer,&words,&cnt);	// BREAK THE STRING INTO cnt WORDS
			if (cnt > 0)						// MAKE SURE SOME WORDS ARE THERE
			{
				temp = words.GetAt(0);			// CHECK THE FIRST WORK
				if (temp.GetLength() > 0)
				{
					if (temp == "newmtl")			// AMBIENT
					{
						matCnt++;
					}
				}
			}
			words.RemoveAll();		// CLEAR WORD BUFFER
		}

		gMatName = (char *)malloc(20 * matCnt);
		visual->matColor = (tVector *)malloc(sizeof(tVector) * matCnt);
		fseek(fp,0,SEEK_SET);
		// Get Data
		while (!feof(fp))
		{
			fgets(buffer,MAX_STRINGLENGTH,fp);	// GET A STRING FROM THE FILE
			ParseOBJString(buffer,&words,&cnt);	// BREAK THE STRING INTO cnt WORDS
			if (cnt > 0)						// MAKE SURE SOME WORDS ARE THERE
			{
				temp = words.GetAt(0);			// CHECK THE FIRST WORK
				if (temp.GetLength() > 0)
				{
					if (temp == "newmtl")			// AMBIENT
					{
						curMat++;
						strncpy(&gMatName[20 * curMat],words.GetAt(1),20);
					}
					else if (temp == "Ka")			// AMBIENT
					{
						visual->Ka.r = (float)atof(words.GetAt(1));
						visual->Ka.g = (float)atof(words.GetAt(2));
						visual->Ka.b = (float)atof(words.GetAt(3));
					}
					else if (temp == "Kd")		// DIFFUSE COLOR
					{
						visual->Kd.r = (float)atof(words.GetAt(1));
						visual->Kd.g = (float)atof(words.GetAt(2));
						visual->Kd.b = (float)atof(words.GetAt(3));
						visual->matColor[curMat].r = visual->Kd.r;
						visual->matColor[curMat].g = visual->Kd.g;
						visual->matColor[curMat].b = visual->Kd.b;
					}
					else if (temp == "Ks")		// SPECULAR COLOR
					{
						visual->Ks.r = (float)atof(words.GetAt(1));
						visual->Ks.g = (float)atof(words.GetAt(2));
						visual->Ks.b = (float)atof(words.GetAt(3));
					}
					else if (temp == "Ns")		// SPECULAR COEFFICIENT
					{
						visual->Ns = (float)atof(words.GetAt(1));
					}
					else if (temp == "map_Kd")	// TEXTURE MAP NAME
					{
						strcpy(&visual->map[curMat * 80],(LPCTSTR)words.GetAt(1));
					}
				}
			}
			words.RemoveAll();		// CLEAR WORD BUFFER
		}
		fclose(fp);
	}

	visual->matCnt = matCnt;
}

///////////////////////////////////////////////////////////////////////////////
// Procedure:	HandleFace
// Purpose:		Handles the Face Line in an OBJ file.  Extracts index info to 
//				a face Structure
// Arguments:	Array of words from the face line, place to put the data
// Notes:		Not an Official OBJ loader as it doesn't handle anything other than
//				3-4 vertex polygons.
///////////////////////////////////////////////////////////////////////////////		
void HandleFace(CStringArray *words,t_faceIndex *face)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop,loopcnt;
	CString temp;
	CString vStr,nStr,tStr;		// HOLD POINTERS TO ELEMENT POINTERS
	int nPos,tPos;
///////////////////////////////////////////////////////////////////////////////
	loopcnt = words->GetSize();
	if (loopcnt > 4) loopcnt = 4;
	
	// LOOP THROUGH THE 3 - 4 WORDS OF THE FACELIST LINE, WORD 0 HAS 'f'
	for (loop = 1; loop < loopcnt; loop++)
	{
		temp = words->GetAt(loop);			// GRAB THE NEXT WORD
		// FACE DATA IS IN THE FORMAT vertex/texture/normal
		tPos = temp.Find('/');				// FIND THE '/' SEPARATING VERTEX AND TEXTURE
		vStr = temp.Left(tPos);				// GET THE VERTEX NUMBER
		temp.SetAt(tPos,' ');				// CHANGE THE '/' TO A SPACE SO I CAN TRY AGAIN
		nPos = temp.Find('/');				// FIND THE '/' SEPARATING TEXTURE AND NORMAL
		tStr = temp.Mid(tPos + 1, nPos - tPos - 1);		// GET THE TEXTURE NUMBER
		nStr = temp.Right(temp.GetLength() - nPos - 1);	// GET THE NORMAL NUMBER
		face->v[loop - 1] = atoi(vStr) - 1;		// STORE OFF THE INDEX FOR THE VERTEX
		face->t[loop - 1] = atoi(tStr) - 1;		// STORE OFF THE INDEX FOR THE TEXTURE
		face->n[loop - 1] = atoi(nStr) - 1;		// STORE OFF THE INDEX FOR THE NORMAL
	}
}
///// HandleFace //////////////////////////////////////////////////////////////

int GetCurMat(CString name,t_Visual *visual)
{
	for (int loop = 0; loop < visual->matCnt; loop++)
		if (name == &gMatName[20 * loop])
			return loop;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Procedure:	LoadOBJ
// Purpose:		Load an OBJ file into the current bone visual
// Arguments:	Name of 0BJ file and pointer to bone
// Notes:		Not an Official OBJ loader as it doesn't handle more then
//				3 vertex polygons or multiple objects per file.
///////////////////////////////////////////////////////////////////////////////		
BOOL LoadOBJ(const char *filename,t_Visual *visual)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int cnt;
	char buffer[MAX_STRINGLENGTH];
	CStringArray words;
	CString temp;
	FILE *fp;
	long vCnt = 0, nCnt = 0, tCnt = 0, fCnt = 0;
	long vPos = 0, nPos = 0, tPos = 0, fPos = 0;
	t_faceIndex *face = NULL;
	int	   curMat = 0;
	float vertexScale = 1.0;
///////////////////////////////////////////////////////////////////////////////
	fp = fopen(filename,"r");
	visual->glTex = 0;
	if (fp != NULL)
	{
		// FIRST PASS SETS UP THE NUMBER OF OBJECTS IN THE FILE
		while (!feof(fp))
		{
			fgets(buffer,MAX_STRINGLENGTH,fp);	// GET A STRING FROM THE FILE
			ParseOBJString(buffer,&words,&cnt);	// BREAK THE STRING INTO cnt WORDS
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
					else if (temp == "mtllib")  // HANDLE THE MATERIAL LIBRARY
					{
						g_MaterialFileName = words.GetAt(1);

						LoadMaterialLib(words.GetAt(1),visual);
					}
				}
			}
			words.RemoveAll();		// CLEAR WORD BUFFER
		}

		// NOW THAT I KNOW HOW MANY, ALLOCATE ROOM FOR IT
		if (vCnt > 0)
		{
			visual->vertex = (tVector *)malloc(vCnt * sizeof(tVector));
			visual->deformData = (tVector *)malloc(sizeof(tVector) * vCnt);
			if (nCnt > 0)
			{
				visual->normal = (tVector *)malloc(nCnt * sizeof(tVector));
			}
			// Since I will be changing UV coords, allocate 3 per triangle
//			if (tCnt > 0)
//			{
				visual->texture = (tVector *)malloc(fCnt * 3 * sizeof(tVector));
//			}
			if (fCnt > 0)
			{
				visual->index = (t_faceIndex *)malloc(fCnt * sizeof(t_faceIndex));
			}
			fseek(fp,0,SEEK_SET);

			// NOW THAT IT IS ALL ALLOC'ED.  GRAB THE REAL DATA
			while (!feof(fp))
			{
				fgets(buffer,MAX_STRINGLENGTH,fp);
				ParseOBJString(buffer,&words,&cnt);
				if (cnt > 0)
				{
					temp = words.GetAt(0);
					if (temp.GetLength() > 0)
					{
						if (temp[0] == 'v')		// WORDS STARTING WITH v
						{
							if (temp.GetLength() > 1 && temp[1] == 'n')	// vn NORMALS
							{
								visual->normal[nPos].x = (float)atof(words.GetAt(1));
								visual->normal[nPos].y = (float)atof(words.GetAt(2));
								visual->normal[nPos].z = (float)atof(words.GetAt(3));
								nPos++;
							}
							else if (temp.GetLength() > 1 && temp[1] == 't')	// vt TEXTURES
							{
								visual->texture[tPos].u = (float)atof(words.GetAt(1));
								visual->texture[tPos].v = (float)atof(words.GetAt(2));
								tPos++;
							}
							else											// VERTICES
							{
								visual->vertex[vPos].x = (float)atof(words.GetAt(1)) * vertexScale;
								visual->vertex[vPos].y = (float)atof(words.GetAt(2)) * vertexScale;
								visual->vertex[vPos].z = (float)atof(words.GetAt(3)) * vertexScale;
								vPos++;
							}
						}
						else if (temp[0] == 'f')			// f v/t/n v/t/n v/t/n	FACE LINE
						{
							if (words.GetSize() > 4)
							{
								sprintf(buffer,"Face %d has more than 3 vertices",fPos);
								MessageBox(NULL,buffer,"ERROR",MB_OK);
							}
							HandleFace(&words,&visual->index[fPos]);
							visual->index[fPos].mat = curMat;
							fPos++;
						}
						else if (temp == "usemtl")			// f v/t/n v/t/n v/t/n	FACE LINE
						{
							curMat = GetCurMat(words.GetAt(1),visual);
						}
						else if (temp == "scale")			// f v/t/n v/t/n v/t/n	FACE LINE
						{
							 vertexScale = (float)atof(words.GetAt(1));
						}
						else if (temp == "o")			// f v/t/n v/t/n v/t/n	FACE LINE
						{
							 g_ObjectName = words.GetAt(1);
						}
					
					}
				}
				words.RemoveAll();		// CLEAR WORD BUFFER
			}

			visual->vertexCnt = vPos;	// Set the vertex count
			visual->faceCnt = fCnt;	// Set the vertex count
//			visual->uvCnt = tCnt;	// Set the vertex count
			// Manually set to 3 * facecount
			visual->uvCnt = fCnt * 3;
			visual->normalCnt = nCnt;	// Set the vertex count
		}

		fclose(fp);

		if (gMatName)
			free(gMatName);
	}
	else
		return FALSE;
	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// Procedure:	SaveOBJ
// Purpose:		Save an OBJ file into the current bone visual
// Arguments:	Name of 0BJ file and pointer to bone
// Notes:		Not an Official OBJ loader as it doesn't handle more then
//				3 vertex polygons or multiple objects per file.
///////////////////////////////////////////////////////////////////////////////		
BOOL SaveOBJ(const char *filename,t_Visual *visual)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop;
	FILE *fp;
	tVector *vertex;
	t_faceIndex *face = NULL;
///////////////////////////////////////////////////////////////////////////////
	if (!visual->vertex) return FALSE;

	fp = fopen(filename,"w");
	if (fp != NULL)
	{
		fprintf(fp,"o %s\n\n",(LPCSTR)g_ObjectName);
		fprintf(fp,"mtllib %s\n\n",(LPCSTR)g_MaterialFileName);
		vertex = visual->vertex;
		for (loop = 0; loop < visual->vertexCnt; loop++, vertex++)
		{
			fprintf(fp,"v %.6f %.6f %.6f\n",vertex->x, vertex->y, vertex->z);
		}

		vertex = visual->texture;
		for (loop = 0; loop < visual->uvCnt; loop++, vertex++)
		{
			fprintf(fp,"vt %.6f %.6f\n",vertex->u, vertex->v);
		}

		vertex = visual->normal;
		for (loop = 0; loop < visual->normalCnt; loop++, vertex++)
		{
			fprintf(fp,"vn %.6f %.6f %.6f\n",vertex->x, vertex->y, vertex->z);
		}
		
		face = visual->index;
		for (loop = 0; loop < visual->faceCnt; loop++, face++)
		{
			fprintf(fp,"f %d/%d/%d %d/%d/%d %d/%d/%d\n",
				face->v[0] + 1,face->t[0] + 1,face->n[0] + 1,
				face->v[1] + 1,face->t[1] + 1,face->n[1] + 1,
				face->v[2] + 1,face->t[2] + 1,face->n[2] + 1);
		}
		fclose(fp);
	}
	return TRUE;
}