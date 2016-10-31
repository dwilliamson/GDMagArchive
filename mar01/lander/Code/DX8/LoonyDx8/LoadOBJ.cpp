///////////////////////////////////////////////////////////////////////////////
//
// LoadOBJ.cpp : implementation file
//
// Purpose:	Implementation of OBJ Loader
//
// Created:
//		JL 9/23/98		
//
///////////////////////////////////////////////////////////////////////////////
#include <windows.h>	// Normal Windows stuff
#include <stdio.h>
#include "vshader.h"
#include "loadOBJ.h"

#define MAX_WORDS	20

///////////////////////////////////////////////////////////////////////////////
// Function:	ParseOBJString
// Purpose:		Actually breaks a string of words into individual pieces
// Arguments:	Source string in, array to put the words and the count
///////////////////////////////////////////////////////////////////////////////
void ParseOBJString(char *buffer,char **words,int *cnt)
{
/// Local Variables ///////////////////////////////////////////////////////////
	char *in = buffer, *temp,*walk;
///////////////////////////////////////////////////////////////////////////////
	while (*in == ' ' || *in == '\t') in++;

	*cnt = 0;
	do 
	{
		temp = in;
		while (*temp != ' ' && *temp != '\t' && *temp != 0) temp++;
		if (*temp == 0)
		{
			words[*cnt] = in;
			walk = words[*cnt];
			// Strip hard returns and line feeds
			int j = strlen(walk);
			for (int i = 0; i <= j; i++,walk++)
			{
				if (*walk == 10 || *walk == 12) *walk = 0;
			}
			*cnt = *cnt + 1;
			break;
		}
		else
		{
			*temp = 0;
			words[*cnt] = in;
			walk = words[*cnt];
			// Strip hard returns and line feeds
			int j = strlen(walk);
			for (int i = 0; i <= j; i++,walk++)
				if (*walk == 10 || *walk == 12) *walk = 0;
			*cnt = *cnt + 1;
			temp++;
			while (*temp == ' ') temp++;	// skip extra spaces
		}
		in = temp;
	} while (1);
}
//// ParseOBJString //////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Procedure:	LoadMaterialLib
// Purpose:		Handles the Loading of a Material library
// Arguments:	Name of the Material Library
///////////////////////////////////////////////////////////////////////////////		
void LoadMaterialLib(char *name,t_Visual *visual)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int cnt;
	char buffer[MAX_STRINGLENGTH];
	char *words[MAX_WORDS],*temp;
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
			ParseOBJString(buffer,words,&cnt);	// BREAK THE STRING INTO cnt WORDS
			if (cnt > 0)						// MAKE SURE SOME WORDS ARE THERE
			{
				if (strlen(words[0]) > 0)
				{
					if (strcmp(words[0],"newmtl") == 0)			// AMBIENT
					{
						matCnt++;
					}
				}
			}
			cnt = 0;
		}

		fseek(fp,0,SEEK_SET);
		// Get Data
		while (!feof(fp))
		{
			fgets(buffer,MAX_STRINGLENGTH,fp);	// GET A STRING FROM THE FILE
			ParseOBJString(buffer,words,&cnt);	// BREAK THE STRING INTO cnt WORDS
			if (cnt > 0)						// MAKE SURE SOME WORDS ARE THERE
			{
				temp = words[0];			// CHECK THE FIRST WORK
				if (strlen(temp) > 0)
				{
					if (strcmp(temp,"newmtl") == 0)			// AMBIENT
					{
//						curMat++;
//						strncpy(&gMatName[20 * curMat],words.GetAt(1),20);
					}
					else if (strcmp(temp,"Ka") == 0)			// AMBIENT
					{
						visual->Ka.x = (float)atof(words[1]);
						visual->Ka.y = (float)atof(words[2]);
						visual->Ka.z = (float)atof(words[3]);
					}
					else if (strcmp(temp,"Kd") == 0)		// DIFFUSE COLOR
					{
						visual->Kd.x = (float)atof(words[1]);
						visual->Kd.y = (float)atof(words[2]);
						visual->Kd.z = (float)atof(words[3]);
					}
					else if (strcmp(temp,"Ks") == 0)		// SPECULAR COLOR
					{
						visual->Ks.x = (float)atof(words[1]);
						visual->Ks.y = (float)atof(words[2]);
						visual->Ks.z = (float)atof(words[3]);
					}
					else if (strcmp(temp,"Ns") == 0)		// SPECULAR COEFFICIENT
					{
						visual->Ns = (float)atof(words[1]);
					}
					else if (strcmp(temp,"map_Kd") == 0)	// TEXTURE MAP NAME
					{
						strcpy(visual->map,words[1]);
					}
				}
			}
			cnt = 0;
		}
		fclose(fp);
	}

}

///////////////////////////////////////////////////////////////////////////////
// Procedure:	HandleFace
// Purpose:		Handles the Face Line in an OBJ file.  Extracts index info to 
//				a face Structure
// Arguments:	Array of words from the face line, place to put the data
// Notes:		Not an Official OBJ loader as it doesn't handle anything other than
//				3-4 vertex polygons.
///////////////////////////////////////////////////////////////////////////////		
void HandleFace(char *words[MAX_WORDS],t_triIndex *tri, int wordCnt)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop,loopcnt;
	char *temp;
	char *vStr,*nStr,*tStr;		// HOLD POINTERS TO ELEMENT POINTERS
///////////////////////////////////////////////////////////////////////////////
	loopcnt = wordCnt;
	if (loopcnt > 4) loopcnt = 4;
	// LOOP THROUGH THE 3 - 4 WORDS OF THE FACELIST LINE, WORD 0 HAS 'f'
	for (loop = 1; loop < loopcnt; loop++)
	{
		temp = words[loop];			// GRAB THE NEXT WORD
		// FACE DATA IS IN THE FORMAT vertex/texture/normal
		tStr = strstr(temp,"/");				// FIND THE '/' SEPARATING VERTEX AND TEXTURE
		*tStr = 0;
		tStr++;
		vStr = temp;				// GET THE VERTEX NUMBER
		nStr = strstr(tStr,"/");				// FIND THE '/' SEPARATING TEXTURE AND NORMAL
		*nStr = 0;
		nStr++;
		tri->v[loop - 1] = atoi(vStr) - 1;		// STORE OFF THE INDEX FOR THE VERTEX
		tri->t[loop - 1] = atoi(tStr) - 1;		// STORE OFF THE INDEX FOR THE TEXTURE
		tri->n[loop - 1] = atoi(nStr) - 1;		// STORE OFF THE INDEX FOR THE NORMAL
	}
	
}
///// HandleFace //////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// Procedure:	LoadOBJ
// Purpose:		Load an OBJ file into the current bone visual
// Arguments:	Name of OBJ file and pointer to bone
// Notes:		Not an Official OBJ loader as it doesn't handle more then
//				3 vertex polygons or multiple objects per file.
///////////////////////////////////////////////////////////////////////////////		
int LoadOBJ(const char *filename,t_Visual *visual)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int cnt;
	char buffer[MAX_STRINGLENGTH];
	char *words[MAX_WORDS],*temp;
	FILE *fp;
	long vCnt = 0, nCnt = 0, tCnt = 0, fCnt = 0;
	long vPos = 0, nPos = 0, tPos = 0, fPos = 0;
	t_triIndex *tri = NULL;
	int	   curMat = 0;
	float vertexScale = 1.0;
///////////////////////////////////////////////////////////////////////////////
	fp = fopen(filename,"r");
	visual->texID = 0;
	if (fp != NULL)
	{
		// FIRST PASS SETS UP THE NUMBER OF OBJECTS IN THE FILE
		while (!feof(fp))
		{
			if (fgets(buffer,MAX_STRINGLENGTH,fp))	// GET A STRING FROM THE FILE
			{
				ParseOBJString(buffer,words,&cnt);	// BREAK THE STRING INTO cnt WORDS
				if (cnt > 0)						// MAKE SURE SOME WORDS ARE THERE
				{
					temp = words[0];			// CHECK THE FIRST WORK
					if (strlen(temp) > 0)
					{
						if (temp[0] == 'v')			// ONLY LOOK AT WORDS THAT START WITH v
						{
							if (strlen(temp) > 1 && temp[1] == 'n')			// vn IS A NORMAL
								nCnt++;
							else if (strlen(temp) > 1 && temp[1] == 't')	// vt IS A TEXTURE 
								tCnt++;
							else
								vCnt++;											// v IS A VERTEX
						}
						else if (temp[0] == 'f')
							fCnt++;												// f IS A FACE
						else if (strcmp(temp,"mtllib") == 0)  // HANDLE THE MATERIAL LIBRARY
						{
							LoadMaterialLib(words[1],visual);
						}
					}
				}
				cnt = 0;		// CLEAR WORD BUFFER
			}
		}

		// NOW THAT I KNOW HOW MANY, ALLOCATE ROOM FOR IT
		if (vCnt > 0)
		{
			visual->vertex = (D3DXVECTOR3 *)malloc(vCnt * sizeof(D3DXVECTOR3));
			if (nCnt > 0)
			{
				visual->normal = (D3DXVECTOR3 *)malloc(nCnt * sizeof(D3DXVECTOR3));
			}
			if (tCnt > 0)
			{
				visual->uv = (D3DXVECTOR3 *)malloc(tCnt * sizeof(D3DXVECTOR3));
			}
			if (fCnt > 0)
			{
				visual->index = (t_triIndex *)malloc(fCnt * sizeof(t_triIndex));
			}
			fseek(fp,0,SEEK_SET);

			// NOW THAT IT IS ALL ALLOC'ED.  GRAB THE REAL DATA
			while (!feof(fp))
			{
				if (fgets(buffer,MAX_STRINGLENGTH,fp))
				{
					ParseOBJString(buffer,words,&cnt);
					if (cnt > 0)
					{
						temp = words[0];			// CHECK THE FIRST WORK
						if (strlen(temp) > 0)
						{
							if (temp[0] == 'v')		// WORDS STARTING WITH v
							{
								if (strlen(temp) > 1 && temp[1] == 'n')	// vn NORMALS
								{
									visual->normal[nPos].x = (float)atof(words[1]);
									visual->normal[nPos].y = (float)atof(words[2]);
									visual->normal[nPos].z = (float)atof(words[3]);
									nPos++;
								}
								else if (strlen(temp) > 1 && temp[1] == 't')	// vt TEXTURES
								{
									visual->uv[tPos].x = (float)atof(words[1]);
									visual->uv[tPos].y = (float)atof(words[2]);
									tPos++;
								}
								else											// VERTICES
								{
									visual->vertex[vPos].x = (float)atof(words[1]) * vertexScale;
									visual->vertex[vPos].y = (float)atof(words[2]) * vertexScale;
									visual->vertex[vPos].z = (float)atof(words[3]) * vertexScale;
									vPos++;
								}
							}
							else if (temp[0] == 'f')			// f v/t/n v/t/n v/t/n	TRI LINE
							{
								if (strlen(temp) > 4)
								{
									sprintf(buffer,"Face %d has more than 3 vertices",fPos);
									MessageBox(NULL,buffer,"ERROR",MB_OK);
								}
								HandleFace(words,&visual->index[fPos],cnt);
								visual->index[fPos].mat = curMat;
								fPos++;
							}
							else if (temp == "usemtl")			// f v/t/n v/t/n v/t/n	FACE LINE
							{
							}
							else if (temp == "scale")			// f v/t/n v/t/n v/t/n	FACE LINE
							{
								 vertexScale = (float)atof(words[1]);
							}
						}
					}
					cnt = 0;		// CLEAR WORD BUFFER
				}
			}

			visual->vertexCnt = vPos;	// Set the vertex count
			visual->triCnt = fCnt;	// Set the vertex count
			visual->uvCnt = tCnt;	// Set the vertex count
			visual->normalCnt = nCnt;	// Set the vertex count
		}
		fclose(fp);
	}
	else
		return FALSE;
	if (visual->triCnt > 0) return TRUE;
	else return FALSE;
}