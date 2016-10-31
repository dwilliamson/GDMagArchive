//-----------------------------------------------------------------------------
// File: MeshData.cpp
//
// Desc: Functions used to massage mesh data into a format useable in X Files
//
// Copyright (C) 1998-2000 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------

#include "pch.h"
#include "MeshData.h"

BOOL FindIdenticalVertex
    (
    SMeshData *pMeshData, 
    DWORD iVertexIndex, 
    DWORD iSmoothingGroup, 
    DWORD iTextureIndex,
    DWORD *piVertex
    )
{
    DWORD iHeadVertex;
    DWORD iCurVertex;

    // walk the wedge list to see if any of the other vertice have the same smoothing and texture index
    iHeadVertex = iVertexIndex;
    iCurVertex = iHeadVertex;
    do
    {
        // if identical requirements, then return true with the vertex index
        if ((pMeshData->m_rgVertices[iCurVertex].iSmoothingGroupIndex == iSmoothingGroup)
            && (pMeshData->m_rgVertices[iCurVertex].iTextureIndex == iTextureIndex) )
        {
            *piVertex = iCurVertex;
            return TRUE;
        }

        // move to next element of wedge list and check to see if we wrapped (circular list)
        iCurVertex = pMeshData->m_rgVertices[iCurVertex].iWedgeList;
    }
    while (iHeadVertex != iCurVertex);

    return FALSE;
}

void AddNormalContribution
    (
    SMeshData *pMeshData,
    DWORD iVertexIndex, 
    DWORD iSmoothingGroup,
    Point3 vNormal 
    )
{
    DWORD iHeadVertex;
    DWORD iCurVertex;

    // walk the wedge list to find other split vertices with the same smoothing group
    iHeadVertex = iVertexIndex;
    iCurVertex = iHeadVertex;
    do
    {
        // if same smoothing group add the normal in
        if (pMeshData->m_rgVertices[iCurVertex].iSmoothingGroupIndex == iSmoothingGroup)    
        {
            pMeshData->m_rgVertices[iCurVertex].vNormal += vNormal;
        }

        // move to next element of wedge list and check to see if we wrapped (circular list)
        iCurVertex = pMeshData->m_rgVertices[iCurVertex].iWedgeList;
    }
    while (iHeadVertex != iCurVertex);

}

HRESULT GenerateMeshData
    (
    Mesh *pMesh,
    SMeshData *pMeshData 
    )
{
    HRESULT hr = S_OK;
    BOOL *rgbVertexReferencedArray = NULL;
    DWORD cVerticesMax;
    DWORD iRawVertexIndex;
    DWORD iTextureIndex;
    DWORD iSmoothingGroupIndex;
    DWORD iVertex;
    DWORD iFace;
    DWORD iPoint;
    DWORD iNewVertex;
    BOOL bFound;
    SVertexData *rgVerticesNew;

    assert(pMesh != NULL);
    assert(pMeshData != NULL);

    pMeshData->m_bTexCoordsPresent = FALSE;
    pMeshData->m_cFaces = pMesh->numFaces;
    pMeshData->m_cVertices = pMesh->numVerts;
    pMeshData->m_cVerticesBeforeDuplication = pMesh->numVerts;
    cVerticesMax = pMesh->numVerts;

    pMeshData->m_rgVertices = new SVertexData[cVerticesMax];
    pMeshData->m_rgFaces = new SFaceData[pMeshData->m_cFaces];
    rgbVertexReferencedArray = new BOOL[pMeshData->m_cVerticesBeforeDuplication];

    if ((pMeshData ->m_rgVertices == NULL) 
            || (pMeshData ->m_rgFaces == NULL)
            || (rgbVertexReferencedArray == NULL))
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    if( !(pMesh->normalsBuilt) )
    {
        pMesh->checkNormals(TRUE);
    }

    // Initialize vertex node list so that first batch are the same as the
    // vertex array in the mesh.  Duplicated vertices will be appended to the list.
    // The first time a vertex comes up in face enumeration below, the initial vertex
    // node added here will be modified to reflect the smoothing and texture info.
    // When the vertex comes up again with different smoothing or texture info, the
    // vertex will be duplicated and appended to the list.
    for (iVertex = 0; iVertex < pMeshData->m_cVertices; iVertex++ )
    {
        rgbVertexReferencedArray[iVertex] = FALSE;

        pMeshData->m_rgVertices[iVertex].iPointRep = iVertex;
        pMeshData->m_rgVertices[iVertex].iWedgeList = iVertex;

        // default values that should be reset if the vertex is actually used by a face...
        pMeshData->m_rgVertices[iVertex].vNormal.x = 0;
        pMeshData->m_rgVertices[iVertex].vNormal.y = 0;
        pMeshData->m_rgVertices[iVertex].vNormal.z = 0;
        pMeshData->m_rgVertices[iVertex].iSmoothingGroupIndex = 0;
        pMeshData->m_rgVertices[iVertex].iTextureIndex = 0;
    }
    
    // for each face, add the face normal for each corner to the vertex node list array.
    // The index into the vertex node list array is just the vertex index, and a list of CVertexNodes
    // is built for the unique smoothing groups for that vertex.  Each CVertexNode holds the normal
    // at that vertex based on a particular smoothing group.  All CVertexNodes get an index, and
    // the list of CFaceIndices is a face list with the corners updated with the new expanded vertex
    // indices (CVertexNode index).
    //
    // Added: Now CVertexNodes each represent a unique smoothing group + texture coordinate set
    // for a vertex.
    for( iFace = 0; iFace < pMeshData->m_cFaces; iFace++ )
    {
        for( iPoint = 0; iPoint < 3; iPoint++ ) // vertex indices
        {
            iRawVertexIndex = pMesh->faces[iFace].v[iPoint];
            iTextureIndex = 0xFFFFFFFF;
            iSmoothingGroupIndex = pMesh->faces[iFace].smGroup;

            if ((pMesh->faces[iFace].flags & HAS_TVERTS) 
                    && (NULL != pMesh->tvFace)
                    && ((int)pMesh->tvFace[iFace].t[iPoint] < pMesh->numTVerts) )
            {
                pMeshData->m_bTexCoordsPresent = TRUE;

                iTextureIndex = pMesh->tvFace[iFace].t[iPoint];
            }
            
            if (FALSE == rgbVertexReferencedArray[iRawVertexIndex])
            {
                // first reference to this vertex.
                rgbVertexReferencedArray[iRawVertexIndex] = TRUE;

                pMeshData->m_rgVertices[iRawVertexIndex].iSmoothingGroupIndex = iSmoothingGroupIndex;
                pMeshData->m_rgVertices[iRawVertexIndex].iTextureIndex = iTextureIndex;

                pMeshData->m_rgFaces[iFace].index[iPoint] = iRawVertexIndex;
            }
            else
            {
                // need to remember the index
                bFound = FindIdenticalVertex(pMeshData, iRawVertexIndex, iSmoothingGroupIndex, iTextureIndex, &iNewVertex);

                // if not found, then split out another vertex
                if (!bFound)
                {
                    // realloc if array too small
                    if (pMeshData->m_cVertices == cVerticesMax)
                    {
                        cVerticesMax = cVerticesMax * 2;
                        rgVerticesNew = new SVertexData[cVerticesMax];
                        if (rgVerticesNew == NULL)
                        {
                            hr = E_OUTOFMEMORY;
                            goto e_Exit;
                        }

                        memcpy(rgVerticesNew, pMeshData->m_rgVertices, sizeof(SVertexData) * pMeshData->m_cVertices);

                        delete []pMeshData->m_rgVertices;
                        pMeshData->m_rgVertices = rgVerticesNew;
                    }

                    // grab the next spot in the array
                    iNewVertex = pMeshData->m_cVertices;
                    pMeshData->m_cVertices += 1;

                    // setup point rep and wedge list
                    pMeshData->m_rgVertices[iNewVertex].iPointRep = iRawVertexIndex;

                    // link into wedge list of point rep
                    pMeshData->m_rgVertices[iNewVertex].iWedgeList = pMeshData->m_rgVertices[iRawVertexIndex].iWedgeList;
                    pMeshData->m_rgVertices[iRawVertexIndex].iWedgeList = iNewVertex;

                    // setup vertex info
                    pMeshData->m_rgVertices[iNewVertex].vNormal.x = 0;
                    pMeshData->m_rgVertices[iNewVertex].vNormal.y = 0;
                    pMeshData->m_rgVertices[iNewVertex].vNormal.z = 0;
                    pMeshData->m_rgVertices[iNewVertex].iSmoothingGroupIndex = iSmoothingGroupIndex;
                    pMeshData->m_rgVertices[iNewVertex].iTextureIndex = iTextureIndex;
                }

                pMeshData->m_rgFaces[iFace].index[iPoint] = iNewVertex;
            }

            // add normal contribution to every vertex with this rawVertexIndex and
            // this smoothinggroup
            AddNormalContribution(pMeshData, iRawVertexIndex, iSmoothingGroupIndex, pMesh->getFaceNormal(iFace));
        }
    }

    for (iVertex = 0; iVertex < pMeshData->m_cVertices; iVertex++)
    {
        pMeshData->m_rgVertices[iVertex].vNormal.Normalize();
    }

e_Exit:
    delete[] rgbVertexReferencedArray;

    return hr;
}



BOOL FindIdenticalPatchVertex
    (
    SPatchMeshData *pPatchMeshData, 
    DWORD iVertexIndex, 
    DWORD iTextureIndex,
    DWORD *piVertex
    )
{
    DWORD iHeadVertex;
    DWORD iCurVertex;

    // walk the wedge list to see if any of the other vertice have the same smoothing and texture index
    iHeadVertex = iVertexIndex;
    iCurVertex = iHeadVertex;
    do
    {
        // if identical requirements, then return true with the vertex index
        if (pPatchMeshData->m_rgVertices[iCurVertex].iTextureIndex == iTextureIndex)
        {
            *piVertex = iCurVertex;
            return TRUE;
        }

        // move to next element of wedge list and check to see if we wrapped (circular list)
        iCurVertex = pPatchMeshData->m_rgVertices[iCurVertex].iWedgeList;
    }
    while (iHeadVertex != iCurVertex);

    return FALSE;
}

HRESULT GeneratePatchMeshData
    (
    PatchMesh *pPatchMesh,
    SPatchMeshData *pPatchMeshData 
    )
{
    HRESULT hr = S_OK;
    BYTE *rgbVertexReferencedArray = NULL;
    DWORD cVerticesMax;
    DWORD iRawVertexIndex;
    DWORD iTextureIndex;
    DWORD iVertex;
    DWORD iPoint;
    DWORD iNewVertex;
    BOOL bFound;
    SPatchVertexData *rgVerticesNew;
	DWORD iCurVertex;
	DWORD iPatch;
	DWORD cPoints;
	PatchVec *pvPatchVecs;
    Patch *pPatch;
	DWORD *rgdwControl;

    assert(pPatchMesh != NULL);
    assert(pPatchMeshData != NULL);

    pPatchMeshData->m_bTexCoordsPresent = FALSE;
    pPatchMeshData->m_cPatches = pPatchMesh->numPatches;

	// UNDONE - need to refine
    cVerticesMax = pPatchMeshData->m_cPatches * 16;

    pPatchMeshData->m_rgVertices = new SPatchVertexData[cVerticesMax];
    pPatchMeshData->m_rgPatches = new SPatchData[pPatchMesh->numPatches];

    if ((pPatchMeshData->m_rgVertices == NULL) 
            || (pPatchMeshData->m_rgPatches == NULL))
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

	// initialize the verts being copied from the pPatchMesh->verts array
	for (iVertex = 0; iVertex < pPatchMesh->numVerts; iVertex++)
	{
        pPatchMeshData->m_rgVertices[iVertex].vPosition = pPatchMesh->verts[iVertex].p;

		pPatchMeshData->m_rgVertices[iVertex].iPointRep = iVertex;
		pPatchMeshData->m_rgVertices[iVertex].iWedgeList = iVertex;

        pPatchMeshData->m_rgVertices[iVertex].iTextureIndex = 0;
	}

	// for the rest of vers, init the non position fields, position filled in when
	//   reading patches
	iCurVertex = pPatchMesh->numVerts;
	for (iVertex = iCurVertex; iVertex < cVerticesMax; iVertex++)
	{
		pPatchMeshData->m_rgVertices[iVertex].iPointRep = iVertex;
		pPatchMeshData->m_rgVertices[iVertex].iWedgeList = iVertex;

        pPatchMeshData->m_rgVertices[iVertex].iTextureIndex = 0;
	}


    pvPatchVecs = pPatchMesh->vecs;
    for (iPatch = 0; iPatch < pPatchMesh->numPatches; iPatch++)
    {
        pPatch = &pPatchMesh->patches[iPatch];

        if (pPatch->type == PATCH_TRI)
        {
            // nControlIndices
            pPatchMeshData->m_rgPatches[iPatch].m_cControl = 10;

            rgdwControl = pPatchMeshData->m_rgPatches[iPatch].m_rgdwControl;
            rgdwControl[0] = pPatch->v[0];
            rgdwControl[3] = pPatch->v[1];
            rgdwControl[6] = pPatch->v[2];

            rgdwControl[1] = iCurVertex;
            pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->vec[0]].p;
            iCurVertex += 1;
        
            rgdwControl[2] = iCurVertex;
            pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->vec[1]].p;
            iCurVertex += 1;
        
            rgdwControl[4] = iCurVertex;
            pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->vec[2]].p;
            iCurVertex += 1;
        
            rgdwControl[5] = iCurVertex;
            pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->vec[3]].p;
            iCurVertex += 1;
        
            rgdwControl[7] = iCurVertex;
            pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->vec[4]].p;
            iCurVertex += 1;
        
            rgdwControl[8] = iCurVertex;
            pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->vec[5]].p;
            iCurVertex += 1;
        
            // UNDONE - is the correct way to get a single interior control
            //  point from 3ds max
            rgdwControl[9] = iCurVertex;
            pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->interior[0]].p;
            pPatchMeshData->m_rgVertices[iCurVertex].vPosition += pvPatchVecs[pPatch->interior[1]].p;
            pPatchMeshData->m_rgVertices[iCurVertex].vPosition += pvPatchVecs[pPatch->interior[2]].p;
            pPatchMeshData->m_rgVertices[iCurVertex].vPosition /= 3;
            iCurVertex += 1;
        }
        else if (pPatch->type == PATCH_QUAD)
        {
            // nControlIndices
            pPatchMeshData->m_rgPatches[iPatch].m_cControl = 16;

            rgdwControl = pPatchMeshData->m_rgPatches[iPatch].m_rgdwControl;
            rgdwControl[0] = pPatch->v[0];
            rgdwControl[3] = pPatch->v[1];
            rgdwControl[6] = pPatch->v[2];
            rgdwControl[9] = pPatch->v[3];

            rgdwControl[1] = iCurVertex;
            pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->vec[0]].p;
            iCurVertex += 1;
        
            rgdwControl[2] = iCurVertex;
            pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->vec[1]].p;
            iCurVertex += 1;
        
            rgdwControl[4] = iCurVertex;
            pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->vec[2]].p;
            iCurVertex += 1;
        
            rgdwControl[5] = iCurVertex;
            pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->vec[3]].p;
            iCurVertex += 1;
        
            rgdwControl[7] = iCurVertex;
            pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->vec[4]].p;
            iCurVertex += 1;
        
            rgdwControl[8] = iCurVertex;
            pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->vec[5]].p;
            iCurVertex += 1;
        
            rgdwControl[10] = iCurVertex;
            pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->vec[6]].p;
            iCurVertex += 1;

            rgdwControl[11] = iCurVertex;
            pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->vec[7]].p;
            iCurVertex += 1;

            rgdwControl[12] = iCurVertex;
            pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->interior[0]].p;
            iCurVertex += 1;

            rgdwControl[13] = iCurVertex;
            pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->interior[1]].p;
            iCurVertex += 1;

            rgdwControl[14] = iCurVertex;
            pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->interior[2]].p;
            iCurVertex += 1;

            rgdwControl[15] = iCurVertex;
            pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->interior[3]].p;
            iCurVertex += 1;
        }
        else // undefined patch type
        {
            hr = E_INVALIDARG;
            goto e_Exit;
        }
    }

	// now record the initial number of vertices
	pPatchMeshData->m_cVerticesBeforeDuplication = iCurVertex;
	pPatchMeshData->m_cVertices = iCurVertex;

    rgbVertexReferencedArray = new BYTE[pPatchMeshData->m_cVertices];
	if (rgbVertexReferencedArray == NULL)
	{
		hr = E_OUTOFMEMORY;
		goto e_Exit;
	}
	memset(rgbVertexReferencedArray, 0, sizeof(BYTE) * pPatchMeshData->m_cVertices);

    for( iPatch = 0; iPatch < pPatchMeshData->m_cPatches; iPatch++ )
    {
		cPoints = (pPatchMeshData->m_rgPatches[iPatch].m_cControl == 10) ? 3 : 4;
        for( iPoint = 0; iPoint < cPoints; iPoint++ ) 
		{
            iTextureIndex = 0xFFFFFFFF;
			iRawVertexIndex = pPatchMeshData->m_rgPatches[iPatch].m_rgdwControl[iPoint * 3];

			// try to get a texture index
            if ((pPatchMesh->tvPatches.Count() >= 2) && (pPatchMesh->tvPatches[1] != NULL))
			{
				iTextureIndex = pPatchMesh->tvPatches[1][iPatch].tv[iPoint];
				if (iTextureIndex < pPatchMesh->numTVerts[1])
				{
	                pPatchMeshData->m_bTexCoordsPresent = TRUE;
				}
				else
				{
					iTextureIndex = 0xFFFFFFFF;
				}
			}

            if (rgbVertexReferencedArray[iRawVertexIndex] == FALSE)
            {
                // first reference to this vertex.
                rgbVertexReferencedArray[iRawVertexIndex] = TRUE;

                pPatchMeshData->m_rgVertices[iRawVertexIndex].iTextureIndex = iTextureIndex;
            }
            else
            {
                // need to remember the index
                bFound = FindIdenticalPatchVertex(pPatchMeshData, iRawVertexIndex, iTextureIndex, &iNewVertex);

                // if not found, then split out another vertex
                if (!bFound)
                {
                    // realloc if array too small
                    if (pPatchMeshData->m_cVertices == cVerticesMax)
                    {
                        cVerticesMax = cVerticesMax * 2;
                        rgVerticesNew = new SPatchVertexData[cVerticesMax];
                        if (rgVerticesNew == NULL)
                        {
                            hr = E_OUTOFMEMORY;
                            goto e_Exit;
                        }

                        memcpy(rgVerticesNew, pPatchMeshData->m_rgVertices, sizeof(SPatchVertexData) * pPatchMeshData->m_cVertices);

                        delete []pPatchMeshData->m_rgVertices;
                        pPatchMeshData->m_rgVertices = rgVerticesNew;
                    }

                    // grab the next spot in the array
                    iNewVertex = pPatchMeshData->m_cVertices;
                    pPatchMeshData->m_cVertices += 1;

                    // setup point rep and wedge list
                    pPatchMeshData->m_rgVertices[iNewVertex].iPointRep = iRawVertexIndex;

                    // link into wedge list of point rep
                    pPatchMeshData->m_rgVertices[iNewVertex].iWedgeList = pPatchMeshData->m_rgVertices[iRawVertexIndex].iWedgeList;
                    pPatchMeshData->m_rgVertices[iRawVertexIndex].iWedgeList = iNewVertex;

                    // setup vertex info
                    pPatchMeshData->m_rgVertices[iNewVertex].vPosition = pPatchMeshData->m_rgVertices[iRawVertexIndex].vPosition;
                    pPatchMeshData->m_rgVertices[iNewVertex].iTextureIndex = iTextureIndex;
                }

				// update the control point to the new vertex position
                pPatchMeshData->m_rgPatches[iPatch].m_rgdwControl[iPoint * 3] = iNewVertex;
            }
		}

	}

e_Exit:
	delete []rgbVertexReferencedArray;

	return hr;
}