//////////////////////////////////////////////////////////////////////////////////////////
//	BSP.h
//	class declaration for quake 3 .bsp file
//	You may use this code as you see fit, provided this header is kept intact.
//	Downloaded from: users.ox.ac.uk/~univ1234
//	Created:	8th August 2002
//////////////////////////////////////////////////////////////////////////////////////////	

//Directory entry in header
struct BSP_DIRECTORY_ENTRY
{
public:
	int offset;
	int length;
};

//Types of directory entry
enum BSP_DIRECTORY_ENTRY_TYPE
{
	bspEntities=0,
	bspTextures,
	bspPlanes,
	bspNodes,
	bspLeaves,
	bspLeafFaces,
	bspLeafBrushes,
	bspModels,
	bspBrushes,
	bspBrushSides,
	bspVertices,
	bspMeshIndices,
	bspEffect,
	bspFaces,
	bspLightmaps,
	bspLightVols,
	bspVisData
};

//BSP file header
struct BSP_HEADER
{
public:
	char string[4];
	int version;
	BSP_DIRECTORY_ENTRY directoryEntries[17];
};



//vertex as found in file
struct BSP_LOAD_VERTEX
{
public:
	Vector3 position;
	float decalS, decalT;
	float lightmapS, lightmapT;
	Vector3 normal;
	unsigned char color[4];
};
	
//vertex as used for drawing
struct BSP_VERTEX
{
public:
	Vector3 position;
	float decalS, decalT;
	float lightmapS, lightmapT;

	BSP_VERTEX operator+(const BSP_VERTEX & rhs) const
	{
		BSP_VERTEX result;
		result.position=position+rhs.position;
		result.decalS=decalS+rhs.decalS;
		result.decalT=decalT+rhs.decalT;
		result.lightmapS=lightmapS+rhs.lightmapS;
		result.lightmapT=lightmapT+rhs.lightmapT;

		return result;
	}

	BSP_VERTEX operator*(const float rhs) const
	{
		BSP_VERTEX result;
		result.position=position*rhs;
		result.decalS=decalS*rhs;
		result.decalT=decalT*rhs;
		result.lightmapS=lightmapS*rhs;
		result.lightmapT=lightmapT*rhs;

		return result;
	}
};


//types of faces
enum BSP_FACE_TYPE
{
	bspPolygonFace=1,
	bspPatch,
	bspMeshFace,
	bspBillboard
};

//Stores which type each face is.
//for example, stores face 8 is a polygon face. It is polygonFace[3]
struct BSP_FACE_DIRECTORY_ENTRY
{
public:
	BSP_FACE_TYPE faceType;
	int typeFaceNumber;		//face number in the list of faces of this type
};

//face as found in the file
struct BSP_LOAD_FACE
{
public:
	int texture;
	int effect;
	int type;
	int firstVertexIndex;
	int numVertices;
	int firstMeshIndex;
	int numMeshIndices;
	int lightmapIndex;
	int lightmapStart[2];
	int lightmapSize[2];
	Vector3 lightmapOrigin;
	Vector3 sTangent, tTangent;
	Vector3 normal;
	int patchSize[2];
};

//polygon face for drawing
struct BSP_POLYGON_FACE
{
public:
	int firstVertexIndex;
	int numVertices;
	int textureIndex;
	int lightmapIndex;
};

//mesh face for drawing
struct BSP_MESH_FACE
{
public:
	int firstVertexIndex;
	int numVertices;
	int textureIndex;
	int lightmapIndex;
	int firstMeshIndex;
	int numMeshIndices;
};

//every patch (curved surface) is split into biquadratic (3x3) patches
struct BSP_BIQUADRATIC_PATCH
{
public:
	bool Tesselate(int newTesselation);
	void Draw();
	
	BSP_VERTEX controlPoints[9];
		
	int tesselation;
	BSP_VERTEX * vertices;
	int *indices;

	//arrays for multi_draw_arrays
	int * trianglesPerRow;
	unsigned int ** rowIndexPointers;

	BSP_BIQUADRATIC_PATCH() : vertices(NULL)
	{}
	~BSP_BIQUADRATIC_PATCH()
	{
		if(vertices)
			delete [] vertices; 
		vertices=NULL;

		if(indices)
			delete [] indices;
		indices=NULL;
	}
};

//curved surface
struct BSP_PATCH
{
public:
	int textureIndex;
	int lightmapIndex;
	int width, height;

	int numQuadraticPatches;
	BSP_BIQUADRATIC_PATCH * quadraticPatches;
};

//texture as found in file
struct BSP_LOAD_TEXTURE
{
public:
	char name[64];
	int flags, contents;	//unknown
};

//lightmap as found in file
struct BSP_LOAD_LIGHTMAP
{
	unsigned char lightmapData[128*128*3];
};


//leaf of bsp tree as found in file
struct BSP_LOAD_LEAF
{
	int cluster;	//cluster index for visdata
	int area;		//areaportal area
	int mins[3];	//min x,y,z (bounding box)
	int maxs[3];
	int firstLeafFace;	//first index in leafFaces array
	int numFaces;
	int firstLeafBrush;	//first index into leaf brushes array
	int numBrushes;
};

//leaf of bsp tree as stored
struct BSP_LEAF
{
	int cluster;	//cluster index for visdata
	Vector3 boundingBoxVertices[8];
	int firstLeafFace;	//first index in leafFaces array
	int numFaces;
};

//node of BSP tree
struct BSP_NODE
{
	int planeIndex;
	int front, back;	//child nodes
	int mins[3];	//min x,y,z (bounding box)
	int maxs[3];
};

//VIS data table
struct BSP_VISIBILITY_DATA
{
	int numClusters;
	int bytesPerCluster;
	unsigned char * bitset;

	BSP_VISIBILITY_DATA()
	{}
	~BSP_VISIBILITY_DATA()
	{
	}
};

//main bsp struct
struct BSP {
	bool load(char * filename, int curveTesselation);

	//header
	BSP_HEADER header;


	//vertices
	int numVertices;
	BSP_VERTEX * vertices;

	bool LoadVertices(FILE * file);



	//faces
	int numTotalFaces;

	BSP_FACE_DIRECTORY_ENTRY * faceDirectory;

	//polygon faces
	int numPolygonFaces;
	BSP_POLYGON_FACE * polygonFaces;

	//mesh faces
	int * meshIndices;
	int numMeshFaces;
	BSP_MESH_FACE * meshFaces;

	//patches
	int numPatches;
	BSP_PATCH * patches;

	bool LoadFaces(FILE * file, int curveTesselation);



	//entities
	char * entityString;



	//Leaves
	int numLeaves;
	BSP_LEAF * leaves;
	
	//leaf faces array
	int * leafFaces;

	//nodes for BSP tree
	int numNodes;
	BSP_NODE * nodes;

    BSP_LOAD_TEXTURE *loadTextures;
    int numTextures;

	bool LoadBSPData(FILE * file);
    bool LoadTextures(FILE * file);


	BSP() : numVertices(0), vertices(NULL),
			numPolygonFaces(0), polygonFaces(NULL),
            numMeshFaces(0), meshFaces(NULL),
			numPatches(0), patches(NULL),
			entityString(NULL)
	{}
	~BSP()
	{
		if(vertices)
			delete [] vertices;
		vertices=NULL;

		if(faceDirectory)
			delete [] faceDirectory;
		faceDirectory=NULL;

		if(polygonFaces)
			delete [] polygonFaces;
		polygonFaces=NULL;

		if(meshIndices)
			delete [] meshIndices;
		meshIndices=NULL;

		if(meshFaces)
			delete [] meshFaces;
		meshFaces=NULL;

		if(patches)
			delete [] patches;
		patches=NULL;

		if(entityString)
			delete [] entityString;
		entityString=NULL;
	}
};
