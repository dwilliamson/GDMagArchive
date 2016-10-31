//////////////////////////////////////////////////////////////////////////////////////////
//	BSP.cpp
//	Functions for bsp file
//	You may use this code as you see fit, provided this header is kept intact.
//	Downloaded from: users.ox.ac.uk/~univ1234
//	Created:	8th August 2002
//////////////////////////////////////////////////////////////////////////////////////////	

/* 
   Decimated, shredded, and otherwise modified by Jonathan Blow
   (jon@number-none.com).
*/

#include "framework.h"
#include "quake3_bsp.h"

bool BSP::load(char * filename, int curveTesselation)
{
	FILE * file;

	file=fopen(filename, "rb");
	if(!file)
	{
		printf("Unable to open %s", filename);
		return false;
	}

	//read in header
	fread(&header, sizeof(BSP_HEADER), 1, file);

	//check header data is correct
	if(	header.string[0]!='I' || header.string[1]!='B' ||
		header.string[2]!='S' || header.string[3]!='P' ||
		header.version  !=0x2E )
	{
		printf("%s is not a version 0x2E .bsp map file", filename);
		return false;
	}


	//Load in vertices
	if(!LoadVertices(file))
		return false;

    if (!LoadTextures(file)) return false;


	//Load in mesh indices
	//Calculate number of indices
	int numMeshIndices=header.directoryEntries[bspMeshIndices].length/sizeof(int);

	//Create space
	meshIndices=new int[numMeshIndices];
	if(!meshIndices)
	{
		printf("Unable to allocate memory for %d mesh indices", numMeshIndices);
		return false;
	}

	//read in the mesh indices
	fseek(file, header.directoryEntries[bspMeshIndices].offset, SEEK_SET);
	fread(meshIndices, header.directoryEntries[bspMeshIndices].length, 1, file);

	

	//Load in faces
	if(!LoadFaces(file, curveTesselation))
		return false;


	//Load BSP Data
	if(!LoadBSPData(file))
		return false;


	//Load in entity string
	entityString=new char[header.directoryEntries[bspEntities].length];
	if(!entityString)
	{
		printf(	"Unable to allocate memory for %d length entity string",
								header.directoryEntries[bspEntities].length);
		return false;
	}

	//Go to entity string in file
	fseek(file, header.directoryEntries[bspEntities].offset, SEEK_SET);
	fread(entityString, 1, header.directoryEntries[bspEntities].length, file);

	//Output the entity string
	//errorLog.OutputSuccess("Entity String: %s", entityString);


	fclose(file);

	printf("%s Loaded successfully", filename);

	return true;
}



///////////////////BSP::LoadVertices////////
////////////////////////////////////////////
bool BSP::LoadVertices(FILE * file)
{
	//calculate number of vertices
	numVertices=header.directoryEntries[bspVertices].length/sizeof(BSP_LOAD_VERTEX);

	//Create space for this many BSP_LOAD_VERTICES
	BSP_LOAD_VERTEX * loadVertices=new BSP_LOAD_VERTEX[numVertices];
	if(!loadVertices)
	{
		printf("Unable to allocate memory for %d BSP_LOAD_VERTEXes", numVertices);
		return false;
	}

	//go to vertices in file
	fseek(file, header.directoryEntries[bspVertices].offset, SEEK_SET);

	//read in the vertices
	fread(loadVertices, header.directoryEntries[bspVertices].length, 1, file);

	//Convert to BSP_VERTEXes
	vertices=new BSP_VERTEX[numVertices];
	if(!vertices)
	{
		printf("Unable to allocate memory for vertices");
		return false;
	}

	for(int i=0; i<numVertices; ++i)
	{
		vertices[i].position =loadVertices[i].position;

		vertices[i].decalS=loadVertices[i].decalS;
		vertices[i].decalT=loadVertices[i].decalT;

		vertices[i].lightmapS=loadVertices[i].lightmapS;
		vertices[i].lightmapT=loadVertices[i].lightmapT;
	}

	if(loadVertices)
		delete [] loadVertices;
	loadVertices=NULL;

	return true;
}

bool BSP::LoadFaces(FILE * file, int curveTesselation)
{
	//calculate number of load faces
	numTotalFaces=header.directoryEntries[bspFaces].length/sizeof(BSP_LOAD_FACE);

	//Create space for this many BSP_LOAD_FACES
	BSP_LOAD_FACE * loadFaces=new BSP_LOAD_FACE[numTotalFaces];
	if(!loadFaces)
	{
		printf("Unable to allocate memory for %d BSP_LOAD_FACEs", numTotalFaces);
		return false;
	}

	//go to faces in file
	fseek(file, header.directoryEntries[bspFaces].offset, SEEK_SET);

	//read in the faces
	fread(loadFaces, header.directoryEntries[bspFaces].length, 1, file);


	//Create space for face directory
	faceDirectory=new BSP_FACE_DIRECTORY_ENTRY[numTotalFaces];
	if(!faceDirectory)
	{
		printf(	"Unable to allocate space for face directory with %d entries",
								numTotalFaces);
		return false;
	}
	
	//Clear the face directory
	memset(faceDirectory, 0, numTotalFaces*sizeof(BSP_FACE_DIRECTORY_ENTRY));


	//Calculate how many of each face type there is
	for(int i=0; i<numTotalFaces; ++i)
	{
		if(loadFaces[i].type==bspPolygonFace)
			++numPolygonFaces;
		if(loadFaces[i].type==bspPatch)
			++numPatches;
		if(loadFaces[i].type==bspMeshFace)
			++numMeshFaces;
	}



	//Create space for BSP_POLYGON_FACEs
	polygonFaces=new BSP_POLYGON_FACE[numPolygonFaces];
	if(!polygonFaces)
	{
		printf("Unable To Allocate memory for BSP_POLYGON_FACEs");
		return false;
	}

	int currentFace=0;
	//convert loadFaces to polygonFaces
	for(i=0; i<numTotalFaces; ++i)
	{
		if(loadFaces[i].type!=bspPolygonFace)		//skip this loadFace if it is not a polygon face
			continue;

		polygonFaces[currentFace].firstVertexIndex=loadFaces[i].firstVertexIndex;
		polygonFaces[currentFace].numVertices=loadFaces[i].numVertices;
		polygonFaces[currentFace].textureIndex=loadFaces[i].texture;
		polygonFaces[currentFace].lightmapIndex=loadFaces[i].lightmapIndex;

		//fill in this entry on the face directory
		faceDirectory[i].faceType=bspPolygonFace;
		faceDirectory[i].typeFaceNumber=currentFace;

		++currentFace;
	}



	//Create space for BSP_MESH_FACEs
	meshFaces=new BSP_MESH_FACE[numMeshFaces];
	if(!meshFaces)
	{
		printf("Unable To Allocate memory for BSP_MESH_FACEs");
		return false;
	}

	int currentMeshFace=0;
	//convert loadFaces to faces
	for(i=0; i<numTotalFaces; ++i)
	{
		if(loadFaces[i].type!=bspMeshFace)		//skip this loadFace if it is not a mesh face
			continue;

		meshFaces[currentMeshFace].firstVertexIndex=loadFaces[i].firstVertexIndex;
		meshFaces[currentMeshFace].numVertices=loadFaces[i].numVertices;
		meshFaces[currentMeshFace].textureIndex=loadFaces[i].texture;
		meshFaces[currentMeshFace].lightmapIndex=loadFaces[i].lightmapIndex;
		meshFaces[currentMeshFace].firstMeshIndex=loadFaces[i].firstMeshIndex;
		meshFaces[currentMeshFace].numMeshIndices=loadFaces[i].numMeshIndices;

		//fill in this entry on the face directory
		faceDirectory[i].faceType=bspMeshFace;
		faceDirectory[i].typeFaceNumber=currentMeshFace;

		++currentMeshFace;
	}
	


	//Create space for BSP_PATCHes
	patches=new BSP_PATCH[numPatches];
	if(!patches)
	{
		printf("Unable To Allocate memory for BSP_PATCHes");
		return false;
	}

	int currentPatch=0;
	//convert loadFaces to patches
	for(i=0; i<numTotalFaces; ++i)
	{
		if(loadFaces[i].type!=bspPatch)		//skip this loadFace if it is not a patch
			continue;

		patches[currentPatch].textureIndex=loadFaces[i].texture;
		patches[currentPatch].lightmapIndex=loadFaces[i].lightmapIndex;
		patches[currentPatch].width=loadFaces[i].patchSize[0];
		patches[currentPatch].height=loadFaces[i].patchSize[1];
		
		//fill in this entry on the face directory
		faceDirectory[i].faceType=bspPatch;
		faceDirectory[i].typeFaceNumber=currentPatch;

		//Create space to hold quadratic patches
		int numPatchesWide=(patches[currentPatch].width-1)/2;
		int numPatchesHigh=(patches[currentPatch].height-1)/2;

		patches[currentPatch].numQuadraticPatches=	numPatchesWide*numPatchesHigh;
		patches[currentPatch].quadraticPatches=new BSP_BIQUADRATIC_PATCH
													[patches[currentPatch].numQuadraticPatches];
		if(!patches[currentPatch].quadraticPatches)
		{
			printf(	"Unable to allocate memory for %d quadratic patches", 
									patches[currentPatch].numQuadraticPatches);
			return false;
		}

		//fill in the quadratic patches
		for(int y=0; y<numPatchesHigh; ++y)
		{
			for(int x=0; x<numPatchesWide; ++x)
			{
				for(int row=0; row<3; ++row)
				{
					for(int point=0; point<3; ++point)
					{
						patches[currentPatch].quadraticPatches[y*numPatchesWide+x].
							controlPoints[row*3+point]=vertices[loadFaces[i].firstVertexIndex+
								(y*2*patches[currentPatch].width+x*2)+
									row*patches[currentPatch].width+point];
					}
				}

				//tesselate the patch
				patches[currentPatch].quadraticPatches[y*numPatchesWide+x].Tesselate(curveTesselation);
			}
		}


		++currentPatch;
	}

	if(loadFaces)
		delete [] loadFaces;
	loadFaces=NULL;

	return true;
}


bool BSP::LoadBSPData(FILE * file) {
	//Load leaves
	//Calculate number of leaves
	numLeaves=header.directoryEntries[bspLeaves].length/sizeof(BSP_LOAD_LEAF);

	//Create space for this many BSP_LOAD_LEAFS
	BSP_LOAD_LEAF * loadLeaves=new BSP_LOAD_LEAF[numLeaves];
	if(!loadLeaves)
	{
		printf("Unable to allocate space for %d BSP_LOAD_LEAFs", numLeaves);
		return false;
	}

	//Create space for this many BSP_LEAFs
	leaves=new BSP_LEAF[numLeaves];
	if(!leaves)
	{
		printf("Unable to allocate space for %d BSP_LEAFs", numLeaves);
		return false;
	}

	//Load leaves
	fseek(file, header.directoryEntries[bspLeaves].offset, SEEK_SET);
	fread(loadLeaves, 1, header.directoryEntries[bspLeaves].length, file);

	//Convert the load leaves to leaves
	for(int i=0; i<numLeaves; ++i)
	{
		leaves[i].cluster=loadLeaves[i].cluster;
		leaves[i].firstLeafFace=loadLeaves[i].firstLeafFace;
		leaves[i].numFaces=loadLeaves[i].numFaces;

		//Create the bounding box
		leaves[i].boundingBoxVertices[0].set((float)loadLeaves[i].mins[0], (float)loadLeaves[i].mins[2],-(float)loadLeaves[i].mins[1]);
		leaves[i].boundingBoxVertices[1].set((float)loadLeaves[i].mins[0], (float)loadLeaves[i].mins[2],-(float)loadLeaves[i].maxs[1]);
		leaves[i].boundingBoxVertices[2].set((float)loadLeaves[i].mins[0], (float)loadLeaves[i].maxs[2],-(float)loadLeaves[i].mins[1]);
		leaves[i].boundingBoxVertices[3].set((float)loadLeaves[i].mins[0], (float)loadLeaves[i].maxs[2],-(float)loadLeaves[i].maxs[1]);
		leaves[i].boundingBoxVertices[4].set((float)loadLeaves[i].maxs[0], (float)loadLeaves[i].mins[2],-(float)loadLeaves[i].mins[1]);
		leaves[i].boundingBoxVertices[5].set((float)loadLeaves[i].maxs[0], (float)loadLeaves[i].mins[2],-(float)loadLeaves[i].maxs[1]);
		leaves[i].boundingBoxVertices[6].set((float)loadLeaves[i].maxs[0], (float)loadLeaves[i].maxs[2],-(float)loadLeaves[i].mins[1]);
		leaves[i].boundingBoxVertices[7].set((float)loadLeaves[i].maxs[0], (float)loadLeaves[i].maxs[2],-(float)loadLeaves[i].maxs[1]);

	}

	
	
	//Load leaf faces array
	int numLeafFaces=header.directoryEntries[bspLeafFaces].length/sizeof(int);

	//Create space for this many leaf faces
	leafFaces=new int[numLeafFaces];
	if(!leafFaces)
	{
		printf("Unable to allocate space for %d leaf faces", numLeafFaces);
		return false;
	}

	//Load leaf faces
	fseek(file, header.directoryEntries[bspLeafFaces].offset, SEEK_SET);
	fread(leafFaces, 1, header.directoryEntries[bspLeafFaces].length, file);

	if(loadLeaves)
		delete [] loadLeaves;
	loadLeaves=NULL;

	return true;
}



//Tesselate a biquadratic patch
bool BSP_BIQUADRATIC_PATCH::Tesselate(int newTesselation)
{
	tesselation=newTesselation;

	float px, py;
	BSP_VERTEX temp[3];
	vertices=new BSP_VERTEX[(tesselation+1)*(tesselation+1)];

	for(int v=0; v<=tesselation; ++v)
	{
		px=(float)v/tesselation;

		vertices[v]=controlPoints[0]*((1.0f-px)*(1.0f-px))+
					controlPoints[3]*((1.0f-px)*px*2)+
					controlPoints[6]*(px*px);
	}

	for(int u=1; u<=tesselation; ++u)
	{
		py=(float)u/tesselation;

		temp[0]=controlPoints[0]*((1.0f-py)*(1.0f-py))+
				controlPoints[1]*((1.0f-py)*py*2)+
				controlPoints[2]*(py*py);

		temp[1]=controlPoints[3]*((1.0f-py)*(1.0f-py))+
				controlPoints[4]*((1.0f-py)*py*2)+
				controlPoints[5]*(py*py);

		temp[2]=controlPoints[6]*((1.0f-py)*(1.0f-py))+
				controlPoints[7]*((1.0f-py)*py*2)+
				controlPoints[8]*(py*py);

		for(int v=0; v<=tesselation; ++v)
		{
			px=(float)v/tesselation;

			vertices[u*(tesselation+1)+v]=	temp[0]*((1.0f-px)*(1.0f-px))+
											temp[1]*((1.0f-px)*px*2)+
											temp[2]*(px*px);
		}
	}

	//Create indices
	indices=new int[tesselation*(tesselation+1)*2];
	if(!indices)
	{
		printf("Unable to allocate memory for surface indices");
		return false;
	}

	for(int row=0; row<tesselation; ++row)
	{
		for(int point=0; point<=tesselation; ++point)
		{
			//calculate indices
			//reverse them to reverse winding
			indices[(row*(tesselation+1)+point)*2+1]=row*(tesselation+1)+point;
			indices[(row*(tesselation+1)+point)*2]=(row+1)*(tesselation+1)+point;
		}
	}


	//Fill in the arrays for multi_draw_arrays
	trianglesPerRow=new int[tesselation];
	rowIndexPointers=new unsigned int *[tesselation];
	if(!trianglesPerRow || !rowIndexPointers)
	{
		printf("Unable to allocate memory for indices for multi_draw_arrays");
		return false;
	}

	for(row=0; row<tesselation; ++row)
	{
		trianglesPerRow[row]=2*(tesselation+1);
		rowIndexPointers[row]=(unsigned int *)&indices[row*2*(tesselation+1)];
	}

	return true;
}



bool BSP::LoadTextures(FILE * file)
{
	//Calculate number of textures
	numTextures=header.directoryEntries[bspTextures].length/sizeof(BSP_LOAD_TEXTURE);

	//Create space for this many BSP_LOAD_TEXTUREs
	loadTextures=new BSP_LOAD_TEXTURE[numTextures];
	if(!loadTextures)
	{
		printf("Unable to allocate space for %d BSP_LOAD_TEXTUREs", numTextures);
		return false;
	}

	//Load textures
	fseek(file, header.directoryEntries[bspTextures].offset, SEEK_SET);
	fread(loadTextures, 1, header.directoryEntries[bspTextures].length, file);

	return true;
}

