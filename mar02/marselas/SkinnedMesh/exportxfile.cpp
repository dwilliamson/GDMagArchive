//-----------------------------------------------------------------------------
// File: ExportXFile.cpp
//
// Desc: Functions used to export max data to an X File
//
// Copyright (C) 1998-2000 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------

#include "pch.h"
#include "XSkinExp.h"
#include "MeshData.h"

#include <initguid.h>

#include <modstack.h>

#include "XSkinExpTemplates.h"
#include "dxfile.h"
#include "rmxfguid.h"
//#include "rmxftmpl.h"

// had to put this into a single centralized helper fn 
// cause whomever came up with this scheme was crezzy
bool registerTemplates(LPDIRECTXFILE &pxofapi);

extern HINSTANCE ghInstance;

#define POWER_DEFAULT   0.0

struct SDialogOptions
{
    BOOL                        m_bProceedWithExport;
    DXFILEFORMAT                m_xFormat;
    DWORD                       m_cMaxBonesPerVertex;
    DWORD                       m_cMaxBonesPerFace;

    BOOL                        m_bSavePatchData;
    BOOL                        m_bSaveAnimationData;
    DWORD                       m_iAnimSamplingRate;
};

struct SBoneInfo
{
    INode                       *m_pBoneNode;
    DWORD                       m_cVertices;
};

// structure used to map an mesh to a bone info structure
struct SSkinMap
{
    SSkinMap()
        :m_pMeshNode(NULL), m_rgbiBones(NULL), m_cbiBones(0), m_cbiBonesMax(0) {}
    ~SSkinMap()
        { delete []m_rgbiBones; }

    INode                       *m_pMeshNode;

    SBoneInfo                   *m_rgbiBones;
    DWORD                       m_cbiBones;
    DWORD                       m_cbiBonesMax;

    SBoneInfo *FindBone(INode *pBoneNode)
    {
        SBoneInfo *pbi = NULL;
        DWORD iBone;

        for (iBone = 0; iBone < m_cbiBones; iBone++)
        {
            if (pBoneNode == m_rgbiBones[iBone].m_pBoneNode)
            {
                pbi = &m_rgbiBones[iBone];
                break;
            }
        }

        return pbi;
    }

    HRESULT AddBone(INode *pBoneNode, SBoneInfo **ppbiBoneInfo)
    {
        HRESULT hr = S_OK;
        SBoneInfo *rgbiTemp;

        // reallocate if neccessary
        if (m_cbiBones == m_cbiBonesMax)
        {
            m_cbiBonesMax = max(1, m_cbiBonesMax);
            m_cbiBonesMax *= 2;

            rgbiTemp = m_rgbiBones;
            m_rgbiBones = new SBoneInfo[m_cbiBonesMax];
            if (m_rgbiBones == NULL)
            {
                m_rgbiBones = rgbiTemp;
                hr = E_OUTOFMEMORY;
                goto e_Exit;
            }

            if (m_cbiBones > 0)
            {
                memcpy(m_rgbiBones, rgbiTemp, m_cbiBones * sizeof(SBoneInfo));
            }

            delete []rgbiTemp;
        }

        // not initialize the next bone in the array and return a pointer to it

        m_rgbiBones[m_cbiBones].m_cVertices = 0;
        m_rgbiBones[m_cbiBones].m_pBoneNode = pBoneNode;

        *ppbiBoneInfo = &m_rgbiBones[m_cbiBones];

        m_cbiBones += 1;

e_Exit:
        return hr;
    }
};

struct SPreprocessContext
{
    SPreprocessContext()
        :m_rgpsmSkinMaps(NULL), 
		 m_cpsmSkinMaps(NULL), 
		 m_cpsmSkinMapsMax(0), 
		 m_cMaxWeightsPerVertex(0), 
		 m_cMaxWeightsPerFace(0),
         m_cNodes(0) {}

    ~SPreprocessContext()
        { delete []m_rgpsmSkinMaps; }

    BOOL                        m_bSaveSelection;

    SSkinMap                    **m_rgpsmSkinMaps;
    DWORD                       m_cpsmSkinMaps;
    DWORD                       m_cpsmSkinMapsMax;

    DWORD                       m_cMaxWeightsPerVertex;
    DWORD                       m_cMaxWeightsPerFace;

    DWORD                       m_cNodes;
};

const int x_cbStringBufferMax = 4088;

struct SStringBlock
{
	SStringBlock()
		:m_psbNext(NULL), m_cbData(0) {}
	~SStringBlock()
	{
		delete m_psbNext;
	}

	SStringBlock				*m_psbNext;
	DWORD						m_cbData;

	TCHAR						szData[x_cbStringBufferMax];
};

class CStringTable
{
public:
	CStringTable()
		:m_psbHead(NULL) {}

	~CStringTable()
	{
		delete m_psbHead;
	}

	// allocate a string out of the data blocks to be free'd later, and make it a valid
	//   x-file name at the same time
	TCHAR *CreateNiceString(TCHAR *szString)
    {
        TCHAR* szNewString = NULL;
		BOOL bFirstCharIsDigit;
		DWORD cbLength;
		SStringBlock *psbNew;

        if (szString == NULL)
            return NULL;

        cbLength = _tcslen(szString) + 1;

        bFirstCharIsDigit = _istdigit(*szString);
        if (bFirstCharIsDigit)
        {
            cbLength += 1;
        }

		// if no string blocks or the current doesn't have enough space, then allocate one
		if ((m_psbHead == NULL) || ((x_cbStringBufferMax - m_psbHead->m_cbData) < cbLength))
		{
			psbNew = new SStringBlock();
			if (psbNew == NULL)
				return NULL;

			psbNew->m_psbNext = m_psbHead;
			m_psbHead = psbNew;
		}

		// allocate a string out of the data block
        szNewString = m_psbHead->szData + m_psbHead->m_cbData;
		m_psbHead->m_cbData += cbLength;

		// deal with the fact that the string can't start with digits
        *szNewString = _T('\0');
        if( bFirstCharIsDigit ) 
        {
            _tcscat(szNewString, _T("_"));
        }

        _tcscat(szNewString, szString);

        TCHAR* pchCur = szNewString;
        while( NULL != *pchCur )
        {
            if( *pchCur != _T('_') && !_istalnum(*pchCur) )
            {
                *pchCur = _T('_');
            }
            pchCur++;
        }
        return szNewString;
    }

	// Allocate a new string with '\\' in place of '\' characters
	TCHAR* CreateNiceFilename(TCHAR *szString)
	{
		TCHAR* szNewString = NULL;
		DWORD cbNameLength;
		DWORD cbLength;
		TCHAR* pchCur;
		TCHAR* pchOrig;
		SStringBlock *psbNew;

		if( NULL == szString )
		{
			return NULL;
		}

		cbNameLength = _tcslen(szString);
		cbLength = cbNameLength*2 + 1;


		// if no string blocks or the current doesn't have enough space, then allocate one
		if ((m_psbHead == NULL) || ((x_cbStringBufferMax - m_psbHead->m_cbData) < cbLength))
		{
			psbNew = new SStringBlock();
			if (psbNew == NULL)
				return NULL;

			psbNew->m_psbNext = m_psbHead;
			m_psbHead = psbNew;
		}

		// allocate a string out of the data block
        szNewString = m_psbHead->szData + m_psbHead->m_cbData;
		m_psbHead->m_cbData += cbLength;

		pchCur = szNewString;
		pchOrig = szString;
		while (NULL != *pchOrig)
		{
			if( _T('\\') == *pchOrig )
			{
				*(pchCur++) = _T('\\');
				*(pchCur++) = _T('\\');
			}
			else
			{
				*(pchCur++) = *pchOrig;
			}
			pchOrig++;
		}
		*pchCur = _T('\0');

		return szNewString;
	}
private:
	SStringBlock *m_psbHead;
};

struct SSaveContext
{
    SSaveContext()
        :m_rgpsmSkinMaps(NULL), m_pAnimationSet(NULL) {}

    ~SSaveContext()
        { delete []m_rgpsmSkinMaps; }

    LPDIRECTXFILESAVEOBJECT     m_pxofsave;
    DXFILEFORMAT                m_xFormat;
    DWORD                       m_iTime;
    BOOL                        m_bSaveSelection;
    BOOL                        m_bSavePatchData;
    BOOL                        m_bSaveAnimationData;
    DWORD                       m_iAnimSamplingRate;
    DWORD                       m_cMaxWeightsPerVertex;
    DWORD                       m_cMaxWeightsPerFace;

    SSkinMap                    **m_rgpsmSkinMaps;
    DWORD                       m_cpsmSkinMaps;

    LPDIRECTXFILEDATA           m_pAnimationSet;
    Interface                   *m_pInterface;

    DWORD                       m_cNodes;
    DWORD                       m_cNodesCur;
    INode                       **m_rgpnNodes;

	CStringTable				m_stStrings;

    SSkinMap *GetSkinMap(INode *pMeshNode)
    {
        SSkinMap *psm = NULL;
        DWORD iMesh;

        for (iMesh = 0; iMesh < m_cpsmSkinMaps; iMesh++)
        {
            if (pMeshNode == m_rgpsmSkinMaps[iMesh]->m_pMeshNode)
            {
                psm = m_rgpsmSkinMaps[iMesh];
                break;
            }
        }

        return psm;
    }

};


// Macros for saving data to memory at DWORD* pbCur (this pointer is incremented)
#define WRITE_PTCHAR(pbCur, ptchar) {TCHAR** __pptchar = (TCHAR**)pbCur; *(__pptchar++) = ptchar;\
                             pbCur = (PBYTE)__pptchar;}

#define WRITE_STRING(pbCur, pstring) {TCHAR* __pCurrDestChar = (TCHAR*)pbCur; TCHAR* __pCurrOrgChar = pstring;\
                                while(NULL != *__pCurrOrgChar) { *(__pCurrDestChar++) = *(__pCurrOrgChar++); }\
                                *(__pCurrDestChar++) = _T('\0'); pbCur = __pCurrDestChar;}\

#define WRITE_WORD(pbCur, word) {WORD* __pword = (WORD*)pbCur; *(__pword++) = word;\
                             pbCur = (PBYTE)__pword;}

#define WRITE_DWORD(pbCur, dword) {DWORD* __pdword = (DWORD*)pbCur; *(__pdword++) = dword;\
                             pbCur = (PBYTE)__pdword;}

#define WRITE_FLOAT(pbCur, _float) {float* __pfloat = (float*)pbCur; *(__pfloat++) = _float;\
                             pbCur = (PBYTE)__pfloat;}

#define WRITE_POINT3(pbCur, _point3) {Point3 _temp = (Point3)_point3; float __tempVal;\
                               __tempVal = _temp[0]; WRITE_FLOAT(pbCur, __tempVal);\
                               __tempVal = _temp[1]; WRITE_FLOAT(pbCur, __tempVal);\
                               __tempVal = _temp[2]; WRITE_FLOAT(pbCur, __tempVal);}

#define WRITE_COLOR(pbCur, _color) {D3DXCOLOR _temp = (D3DXCOLOR)_color; float __tempVal;\
                               __tempVal = _temp.r; WRITE_FLOAT(pbCur, __tempVal);\
                               __tempVal = _temp.g; WRITE_FLOAT(pbCur, __tempVal);\
                               __tempVal = _temp.b; WRITE_FLOAT(pbCur, __tempVal);\
                               __tempVal = _temp.a; WRITE_FLOAT(pbCur, __tempVal);}

#define WRITE_MATRIX4_FROM_MATRIX3(pbCur, _matrix3) {Point3 __tempRow = ((Matrix3)_matrix3).GetRow(0);\
                                WRITE_POINT3(pbCur, __tempRow); WRITE_FLOAT(pbCur, 0);\
                                __tempRow = _matrix3.GetRow(1); WRITE_POINT3(pbCur, __tempRow); WRITE_FLOAT(pbCur, 0);\
                                __tempRow = _matrix3.GetRow(2); WRITE_POINT3(pbCur, __tempRow); WRITE_FLOAT(pbCur, 0);\
                                __tempRow = _matrix3.GetRow(3); WRITE_POINT3(pbCur, __tempRow); WRITE_FLOAT(pbCur, 1);}

#define WRITE_MATRIX(pbCur, mat) { *(D3DXMATRIX*)pbCur = mat;\
                               pbCur = (PBYTE)pbCur + sizeof(D3DXMATRIX);}

const GUID* aIds[] = {&DXFILEOBJ_XSkinMeshHeader,
                &DXFILEOBJ_VertexDuplicationIndices,
                &DXFILEOBJ_SkinWeights};




BOOL CALLBACK XSkinExpOptionsDlgProc
    (
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    ) 
{
    static SDialogOptions *pDialogOptions = NULL;
    TCHAR buff[8];

    switch(message) 
    {
        case WM_INITDIALOG:
            pDialogOptions = (SDialogOptions *)lParam;

            CenterWindow(hWnd,GetParent(hWnd));

            CheckDlgButton(hWnd, IDC_ANIMATION, 
                    pDialogOptions->m_bSaveAnimationData ? BST_CHECKED : BST_UNCHECKED);

            CheckDlgButton(hWnd, IDC_PATCHDATA, 
                    pDialogOptions->m_bSavePatchData ? BST_CHECKED : BST_UNCHECKED);

            if (pDialogOptions == NULL)
                return FALSE;

            switch (pDialogOptions->m_xFormat)
            {
                case DXFILEFORMAT_BINARY:
                    CheckRadioButton(hWnd,IDC_TEXT,IDC_BINARYCOMPRESSED,IDC_BINARY);
                    break;

                case DXFILEFORMAT_TEXT:
                    CheckRadioButton(hWnd,IDC_TEXT,IDC_BINARYCOMPRESSED,IDC_TEXT);
                    break;

                case DXFILEFORMAT_BINARY | DXFILEFORMAT_COMPRESSED:
                    CheckRadioButton(hWnd,IDC_TEXT,IDC_BINARYCOMPRESSED,IDC_BINARYCOMPRESSED);
                    break;
            }

            _stprintf(buff,_T("%d"),pDialogOptions->m_cMaxBonesPerVertex);
            SetDlgItemText(hWnd,IDC_MAX_BONES_PER_VERTEX,buff);

            _stprintf(buff,_T("%d"),pDialogOptions->m_cMaxBonesPerFace);
            SetDlgItemText(hWnd,IDC_MAX_BONES_PER_FACE,buff);

            _stprintf(buff,_T("%d"),pDialogOptions->m_iAnimSamplingRate);
            SetDlgItemText(hWnd,IDC_SAMPLERATE,buff);

            return TRUE;

        case WM_CLOSE:
            pDialogOptions->m_bProceedWithExport = FALSE;

            EndDialog(hWnd, 0);
            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
            case IDC_GO:
                pDialogOptions->m_bProceedWithExport = TRUE;

                EndDialog(hWnd,0);
                break;

            case IDC_CANCEL:
                pDialogOptions->m_bProceedWithExport = FALSE;

                EndDialog(hWnd,0);
                break;

            case IDC_TEXT:
                pDialogOptions->m_xFormat = DXFILEFORMAT_TEXT;
                break;

            case IDC_BINARY:
                pDialogOptions->m_xFormat = DXFILEFORMAT_BINARY;
                break;

            case IDC_BINARYCOMPRESSED:
                pDialogOptions->m_xFormat = DXFILEFORMAT_BINARY | DXFILEFORMAT_COMPRESSED;
                break;

            case IDC_ANIMATION:
                pDialogOptions->m_bSaveAnimationData = !pDialogOptions->m_bSaveAnimationData;
                break;

            case IDC_PATCHDATA:
                pDialogOptions->m_bSavePatchData = !pDialogOptions->m_bSavePatchData;
                break;

            case IDC_SAMPLERATE:
                GetDlgItemText(hWnd,IDC_SAMPLERATE,buff,sizeof(buff) / sizeof(TCHAR));

                pDialogOptions->m_iAnimSamplingRate = _ttoi(buff);
                break;

            default:
                break;
            }
    }
    return FALSE;
}

// ================================================== FindPhysiqueModifier()
// Find if a given node contains a Physique Modifier
// DerivedObjectPtr requires you include "modstack.h" from the MAX SDK
Modifier* FindPhysiqueModifier (INode* nodePtr)
{
    // Get object from node. Abort if no object.
    Object* ObjectPtr = nodePtr->GetObjectRef();
    

    if ( NULL == ObjectPtr)
    {
        return NULL;
    }

    // Is derived object ?
    if (ObjectPtr->SuperClassID() == GEN_DERIVOB_CLASS_ID)
    {
        // Yes -> Cast.
        IDerivedObject* DerivedObjectPtr = static_cast<IDerivedObject*>(ObjectPtr);

        // Iterate over all entries of the modifier stack.
        int ModStackIndex = 0;
        while (ModStackIndex < DerivedObjectPtr->NumModifiers())
        {
            // Get current modifier.
            Modifier* ModifierPtr = DerivedObjectPtr->GetModifier(ModStackIndex);
            Class_ID clsid = ModifierPtr->ClassID();
            // Is this Physique ?
            if (ModifierPtr->ClassID() == Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B))
            {
                // Yes -> Exit.
                return ModifierPtr;
            }

            // Next modifier stack entry.
            ModStackIndex++;
        }
    }

    // Not found.
    return NULL;
}

// ================================================== GetTriObjectFromObjRef
// Return a pointer to a TriObject given an object reference or return NULL
// if the node cannot be converted to a TriObject
TriObject *GetTriObjectFromObjRef
	(
	Object* pObj, 
	BOOL *pbDeleteIt
	) 
{
	TriObject *pTri;

	assert(pObj != NULL);
	assert(pbDeleteIt != NULL);

    *pbDeleteIt = FALSE;

    if (pObj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) 
	{ 
        pTri = (TriObject *) pObj->ConvertToType(0, Class_ID(TRIOBJ_CLASS_ID, 0));

        // Note that the TriObject should only be deleted
        // if the pointer to it is not equal to the object
        // pointer that called ConvertToType()
        if (pObj != pTri) 
			*pbDeleteIt = TRUE;

        return pTri;
    } 
	else 
	{
        return NULL;
    }
}

// ================================================== GetTriObjectFromObjRef
// Return a pointer to a TriObject given an object reference or return NULL
// if the node cannot be converted to a TriObject
PatchObject *GetPatchObjectFromObjRef
	(
	Object* pObj, 
	BOOL *pbDeleteIt
	) 
{
	PatchObject *pPatchObject;

	assert(pObj != NULL);
	assert(pbDeleteIt != NULL);

    *pbDeleteIt = FALSE;

    if (pObj->CanConvertToType(Class_ID(PATCHOBJ_CLASS_ID, 0))) 
	{ 
        pPatchObject = (PatchObject *) pObj->ConvertToType(0, Class_ID(PATCHOBJ_CLASS_ID, 0));

        // Note that the PatchObject should only be deleted
        // if the pointer to it is not equal to the object
        // pointer that called ConvertToType()
        if (pObj != pPatchObject) 
			*pbDeleteIt = TRUE;

        return pPatchObject;
    } 
	else 
	{
        return NULL;
    }
}

// ================================================== IsExportableMesh()
BOOL IsExportableMesh(INode* pNode, Object* &pObj)
{
    ULONG superClassID;

    assert(pNode);
    pObj = pNode->GetObjectRef();

    if (pObj == NULL)
    {
        return FALSE;
    }

    superClassID = pObj->SuperClassID();
    //find out if mesh is renderable

    if( !pNode->Renderable() || pNode->IsNodeHidden())
    {
        return FALSE;
    }

    BOOL bFoundGeomObject = FALSE;
    //find out if mesh is renderable (more)
    switch(superClassID)
    {
    case GEN_DERIVOB_CLASS_ID:

        do
        {
            pObj = ((IDerivedObject*)pObj)->GetObjRef();
            superClassID = pObj->SuperClassID();
        }
        while( superClassID == GEN_DERIVOB_CLASS_ID );

        switch(superClassID)
        {
        case GEOMOBJECT_CLASS_ID:
            bFoundGeomObject = TRUE;
            break;
        }
        break;

    case GEOMOBJECT_CLASS_ID:
        bFoundGeomObject = TRUE;

    default:
        break;
    }

    return bFoundGeomObject;
}

// ================================================== IsExportablePatchMesh()
BOOL IsExportablePatchMesh(INode* pNode, Object* &pObj)
{
    ULONG superClassID;
	Object *pObjBase;

    assert(pNode != NULL);

    pObj = pNode->GetObjectRef();
    if (pObj == NULL)
        return FALSE;

    if( !pNode->Renderable() || pNode->IsNodeHidden())
        return FALSE;


	pObjBase = pObj;

    superClassID = pObjBase->SuperClassID();
    switch(superClassID)
    {
    case GEN_DERIVOB_CLASS_ID:

        do
        {
            pObjBase = ((IDerivedObject*)pObjBase)->GetObjRef();
            superClassID = pObjBase->SuperClassID();
        }
        while( superClassID == GEN_DERIVOB_CLASS_ID );

        switch(superClassID)
        {
        case GEOMOBJECT_CLASS_ID:
            break;
        }
        break;

    case GEOMOBJECT_CLASS_ID:
		break;

    default:
        break;
    }

	pObj = pObjBase;

	// if the base object is a patch based object
	if ((pObjBase->ClassID() == Class_ID(PATCHOBJ_CLASS_ID, 0))
		|| (pObjBase->ClassID() == Class_ID(PATCHGRID_CLASS_ID, 0)))
	{
		// then check to make sure that the intervening steps don't disallow
		//   a patch object to be converted to
		return pObjBase->CanConvertToType(Class_ID(PATCHOBJ_CLASS_ID, 0));
	}
	else
		return FALSE;
}

// ================================================== dummyFn()
// used by 3DS progress bar
DWORD WINAPI dummyFn(LPVOID arg)
{
    return(0);
}


HRESULT AddNormals
    (
    SSaveContext *psc,
    SMeshData *pMeshData, 
    BOOL bSwapTriOrder,
    LPDIRECTXFILEDATA pParent
    )
{
    HRESULT hr = S_OK;
    LPDIRECTXFILEDATA pDataObject = NULL;
    PBYTE         pbData = NULL;
    PBYTE         pbCur;        
    DWORD          cbSize;
    DWORD cNormals;
    DWORD cFaces;
    DWORD iFace;
    DWORD iVertex;

    assert(psc != NULL);
    assert(pParent != NULL);
    assert(pMeshData != NULL);

    cNormals = pMeshData->m_cVertices;
    cFaces = pMeshData->m_cFaces;

    cbSize = sizeof(DWORD) // nNormals
             + 3*sizeof(float)*cNormals // normals
             + sizeof(DWORD) // nFaces
             + cFaces* // MeshFace array
                (sizeof(DWORD) //nFaceVertexIndices (number of normal indices)
                 + 3*sizeof(DWORD)); // faceVertexIndices (normal indices)

    pbCur = pbData = new BYTE[cbSize];
    if (pbData == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    // nNormals
    WRITE_DWORD(pbCur, cNormals);

    // normals
    for (iVertex = 0; iVertex < pMeshData->m_cVertices; iVertex++)
    {
        WRITE_POINT3(pbCur, pMeshData->m_rgVertices[iVertex].vNormal);
    }

    // nFaces
    WRITE_DWORD(pbCur, cFaces);


    // MeshFace array
    for( iFace = 0; iFace < cFaces; iFace++ )
    {
        WRITE_DWORD(pbCur, 3); // nFaceVertexIndices (number of normal indices)

        // faceVertexIndices (indices to normals - same as indices to vertices for us)
        if( bSwapTriOrder )
        {
            WRITE_DWORD(pbCur, pMeshData->m_rgFaces[iFace].index[0]);
            WRITE_DWORD(pbCur, pMeshData->m_rgFaces[iFace].index[2]);
            WRITE_DWORD(pbCur, pMeshData->m_rgFaces[iFace].index[1]);
        }
        else
        {
            WRITE_DWORD(pbCur, pMeshData->m_rgFaces[iFace].index[0]);
            WRITE_DWORD(pbCur, pMeshData->m_rgFaces[iFace].index[1]);
            WRITE_DWORD(pbCur, pMeshData->m_rgFaces[iFace].index[2]);
        }
    }

    hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMMeshNormals,
                                    NULL,
                                    NULL,
                                    cbSize,
                                    pbData,
                                    &pDataObject
                                    );
    if (FAILED(hr))
    {
        OutputDebugString("Failed to create x file data object!");
        goto e_Exit;
    }

    hr = pParent->AddDataObject(pDataObject);
    if (FAILED(hr))
    {
        OutputDebugString("Failed to add x file data object!");
        goto e_Exit;
    }

    // falling through
e_Exit:
    RELEASE(pDataObject);
    delete []pbData;

    return hr;
}

HRESULT AddTextureCoordinates
    (
    SSaveContext *psc,
    SMeshData* pMeshData, 
    Mesh* pMesh,
    LPDIRECTXFILEDATA pParent
    )
{
    LPDIRECTXFILEDATA pDataObject = NULL;
    PBYTE         pbData = NULL;
    PBYTE         pbCur = NULL;
    DWORD         cbSize;
    DWORD         cTexCoords;
    DWORD         iVertex;
    DWORD         iTextureIndex;
    HRESULT    hr = S_OK;

    assert( psc );
    assert( pParent );
    assert( pMesh );
    assert( pMeshData );

    // if no tex coords, then don't add them
    if( !pMeshData->m_bTexCoordsPresent )
        goto e_Exit;

    cTexCoords = pMeshData->m_cVertices;

    cbSize = sizeof(DWORD) //nTextureCoords
             + cTexCoords*2*sizeof(float); //texture coords

    pbCur = pbData = new BYTE[cbSize];
    if (pbData == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    WRITE_DWORD(pbCur, cTexCoords); //nTextureCoords

    for (iVertex = 0; iVertex < pMeshData->m_cVertices; iVertex++)
    {
        iTextureIndex  = pMeshData->m_rgVertices[iVertex].iTextureIndex;
        if( iTextureIndex == 0xFFFFFFFF ) // none present, or bad data
        {
            WRITE_FLOAT(pbCur, 0); //u
            WRITE_FLOAT(pbCur, 0); //v
        }
        else
        {
            WRITE_FLOAT(pbCur, pMesh->tVerts[iTextureIndex].x); //u
            WRITE_FLOAT(pbCur, pMesh->tVerts[iTextureIndex].y * -1); //v
        }
    }

    hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMMeshTextureCoords,
                                    NULL,
                                    NULL,
                                    cbSize,
                                    pbData,
                                    &pDataObject
                                    );
    if (FAILED(hr))
    {
        OutputDebugString("Failed to create x file data object!");
        goto e_Exit;
    }

    hr = pParent->AddDataObject(pDataObject);
    if (FAILED(hr))
    {
        OutputDebugString("Failed to add x file data object!");
        goto e_Exit;
    }

    // falling through
e_Exit:
    RELEASE(pDataObject);

    delete []pbData;
    return hr;
}

HRESULT AddPatchTextureCoordinates
    (
    SSaveContext *psc,
    SPatchMeshData* pPatchMeshData, 
    PatchMesh* pPatchMesh,
    LPDIRECTXFILEDATA pParent
    )
{
    LPDIRECTXFILEDATA pDataObject = NULL;
    PBYTE         pbData = NULL;
    PBYTE         pbCur = NULL;
    DWORD         cbSize;
    DWORD         cTexCoords;
    DWORD         iVertex;
    DWORD         iTextureIndex;
    HRESULT    hr = S_OK;

    assert( psc );
    assert( pParent );
    assert( pPatchMesh );
    assert( pPatchMeshData );

    // if no tex coords, then don't add them
    if( !pPatchMeshData->m_bTexCoordsPresent )
        goto e_Exit;

    cTexCoords = pPatchMeshData->m_cVertices;

    cbSize = sizeof(DWORD) //nTextureCoords
             + cTexCoords*2*sizeof(float); //texture coords

    pbCur = pbData = new BYTE[cbSize];
    if (pbData == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    WRITE_DWORD(pbCur, cTexCoords); //nTextureCoords

    for (iVertex = 0; iVertex < pPatchMeshData->m_cVertices; iVertex++)
    {
        iTextureIndex  = pPatchMeshData->m_rgVertices[iVertex].iTextureIndex;
        if( iTextureIndex == 0xFFFFFFFF ) // none present, or bad data
        {
            WRITE_FLOAT(pbCur, 0); //u
            WRITE_FLOAT(pbCur, 0); //v
        }
        else
        {
            WRITE_FLOAT(pbCur, pPatchMesh->tVerts[1][iTextureIndex].p.x); //u
            WRITE_FLOAT(pbCur, pPatchMesh->tVerts[1][iTextureIndex].p.y * -1); //v
        }
    }

    hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMMeshTextureCoords,
                                    NULL,
                                    NULL,
                                    cbSize,
                                    pbData,
                                    &pDataObject
                                    );
    if (FAILED(hr))
    {
        OutputDebugString("Failed to create x file data object!");
        goto e_Exit;
    }

    hr = pParent->AddDataObject(pDataObject);
    if (FAILED(hr))
    {
        OutputDebugString("Failed to add x file data object!");
        goto e_Exit;
    }

    // falling through
e_Exit:
    RELEASE(pDataObject);

    delete []pbData;
    return hr;
}

HRESULT AddVertexDuplicationIndices
    (
    SSaveContext *psc,
    SMeshData *pMeshData, 
    LPDIRECTXFILEDATA pParent
    )
{
    HRESULT    hr = S_OK;
    LPDIRECTXFILEDATA pDataObject = NULL;
    PBYTE         pbData = NULL;
    PBYTE         pbCur = NULL;
    DWORD         cbSize;
    DWORD         cIndices;
    DWORD         cVerticesBeforeDuplication;
    DWORD         iVertex;

    assert(psc != NULL);
    assert(pParent != NULL);
    assert(pMeshData != NULL);

    cIndices = pMeshData->m_cVertices;
    cVerticesBeforeDuplication = pMeshData->m_cVerticesBeforeDuplication;

	// if no new vertices, then don't add a record to the file
	if (pMeshData->m_cVerticesBeforeDuplication == pMeshData->m_cVertices)
		goto e_Exit;

    cbSize = sizeof(DWORD) //nIndices
             + sizeof(DWORD) //nVerticesBeforeDuplication
             + cIndices*sizeof(DWORD); // array of indices

    pbCur = pbData = new BYTE[cbSize];
    if (pbData == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    WRITE_DWORD(pbCur, cIndices) // nIndices;
    WRITE_DWORD(pbCur, cVerticesBeforeDuplication) // nVerticesBeforeDuplication
    
    for (iVertex = 0; iVertex < cIndices; iVertex++)
    {
        WRITE_DWORD(pbCur, pMeshData->m_rgVertices[iVertex].iPointRep); // index to original vertex without duplication.
    }

    hr = psc->m_pxofsave->CreateDataObject(DXFILEOBJ_VertexDuplicationIndices,
                                    NULL,
                                    NULL,
                                    cbSize,
                                    pbData,
                                    &pDataObject
                                    );
    if (FAILED(hr))
    {
        OutputDebugString("Failed to create x file data object!");
        goto e_Exit;
    }

    hr = pParent->AddDataObject(pDataObject);
    if (FAILED(hr))
    {
        OutputDebugString("Failed to add x file data object!");
        goto e_Exit;
    }

    // falling through
e_Exit:
    RELEASE(pDataObject);

    delete []pbData;
    return hr;
}

HRESULT AddPatchVertexDuplicationIndices
    (
    SSaveContext *psc,
    SPatchMeshData *pPatchMeshData, 
    LPDIRECTXFILEDATA pParent
    )
{
    HRESULT    hr = S_OK;
    LPDIRECTXFILEDATA pDataObject = NULL;
    PBYTE         pbData = NULL;
    PBYTE         pbCur = NULL;
    DWORD         cbSize;
    DWORD         cIndices;
    DWORD         cVerticesBeforeDuplication;
    DWORD         iVertex;

    assert(psc != NULL);
    assert(pParent != NULL);
    assert(pPatchMeshData != NULL);

    cIndices = pPatchMeshData->m_cVertices;
    cVerticesBeforeDuplication = pPatchMeshData->m_cVerticesBeforeDuplication;

	// if no new vertices, then don't add a record to the file
	if (pPatchMeshData->m_cVerticesBeforeDuplication == pPatchMeshData->m_cVertices)
		goto e_Exit;

    cbSize = sizeof(DWORD) //nIndices
             + sizeof(DWORD) //nVerticesBeforeDuplication
             + cIndices*sizeof(DWORD); // array of indices

    pbCur = pbData = new BYTE[cbSize];
    if (pbData == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    WRITE_DWORD(pbCur, cIndices) // nIndices;
    WRITE_DWORD(pbCur, cVerticesBeforeDuplication) // nVerticesBeforeDuplication
    
    for (iVertex = 0; iVertex < cIndices; iVertex++)
    {
        WRITE_DWORD(pbCur, pPatchMeshData->m_rgVertices[iVertex].iPointRep); // index to original vertex without duplication.
    }

    hr = psc->m_pxofsave->CreateDataObject(DXFILEOBJ_VertexDuplicationIndices,
                                    NULL,
                                    NULL,
                                    cbSize,
                                    pbData,
                                    &pDataObject
                                    );
    if (FAILED(hr))
    {
        OutputDebugString("Failed to create x file data object!");
        goto e_Exit;
    }

    hr = pParent->AddDataObject(pDataObject);
    if (FAILED(hr))
    {
        OutputDebugString("Failed to add x file data object!");
        goto e_Exit;
    }

    // falling through
e_Exit:
    RELEASE(pDataObject);

    delete []pbData;
    return hr;
}

HRESULT
AddWireframeMaterial(
    SSaveContext *psc,
    INode *pNode,
    LPDIRECTXFILEDATA pParent
    )
{
    HRESULT hr = S_OK;
    DWORD cbSize;
    LPDIRECTXFILEDATA pDataObject = NULL;
    PBYTE pbData = NULL;
    PBYTE pbCur;
    DWORD dwWFColor;
    D3DXCOLOR colorWF;

    cbSize = 4*sizeof(float) // colorRGBA
             + sizeof(float) //power
             + 3*sizeof(float) //specularColor
             + 3*sizeof(float); //emissiveColor

    pbData = pbCur = new BYTE[cbSize];
    if (pbCur == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }
    
    // get the wireframe color
    dwWFColor = pNode->GetWireColor();
    dwWFColor |= 0xff000000;  // set alpha to fully opaque

    // convert to floating point
    colorWF = D3DXCOLOR(dwWFColor);

    //RGBA
    WRITE_COLOR(pbCur, colorWF);

    // Wireframe doesn't have an explicit specular power, so output our default.
    WRITE_FLOAT(pbCur, POWER_DEFAULT);

    // Set the specular color identical to diffuse, as recommended in 3DSMax docs.
    WRITE_FLOAT(pbCur, colorWF.r);
    WRITE_FLOAT(pbCur, colorWF.g);
    WRITE_FLOAT(pbCur, colorWF.b);

    // Set the luminence to 0: the material isn't glowing.
    WRITE_FLOAT(pbCur, 0.0f);
    WRITE_FLOAT(pbCur, 0.0f);
    WRITE_FLOAT(pbCur, 0.0f);

    hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMMaterial,
                                    NULL,
                                    NULL,
                                    cbSize,
                                    pbData,
                                    &pDataObject
                                    );

    hr = pParent->AddDataObject(pDataObject);
    if (FAILED(hr))
        goto e_Exit;

e_Exit:
    delete []pbData;
    RELEASE(pDataObject);

    return hr;
}

HRESULT
AddTextureFilename(
    SSaveContext *psc,
    TCHAR *szName,
    LPDIRECTXFILEDATA pParent
    )
{
    HRESULT hr = S_OK;
    LPDIRECTXFILEDATA pDataObject = NULL;
    DWORD cbSize;

    cbSize = sizeof(TCHAR**);

    hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMTextureFilename,
                                    NULL,
                                    NULL,
                                    cbSize,
                                    &szName,
                                    &pDataObject
                                    );
    if (FAILED(hr))
        goto e_Exit;

    hr = pParent->AddDataObject(pDataObject);
    if (FAILED(hr))
        goto e_Exit;

e_Exit:
    RELEASE(pDataObject);

    return hr;
}

// ================================================== FindTextureFileName
// callback called by 3DSMAX
class FindTextureFilename : public NameEnumCallback
{
public:
    FindTextureFilename()
        :m_szTextureName(NULL) {}

    virtual void RecordName(TCHAR *szName)
    {
        m_szTextureName = szName;
    }

    TCHAR *m_szTextureName;    
};

HRESULT
AddMaterial(
    SSaveContext *psc,
    Mtl *pMtl,
    LPDIRECTXFILEDATA pParent
    )
{
    HRESULT hr = S_OK;
    LPDIRECTXFILEDATA pDataObject = NULL;
    PBYTE pbData = NULL;
    PBYTE pbCur;
    DWORD cbSize;
    TCHAR *szNiceFilename;
    Texmap *pTexMap;
    FindTextureFilename findTextureFilename;
    BOOL bDetailMap = FALSE;

    cbSize = 4*sizeof(float) // colorRGBA
             + sizeof(float) //power
             + 3*sizeof(float) //specularColor
             + 3*sizeof(float); //emissiveColor

    pbData = pbCur = new BYTE[cbSize];
    if (pbCur == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    pTexMap = (Texmap *) pMtl->GetActiveTexmap();
    if ((pTexMap != NULL) && (pTexMap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0x00)))
    {
        bDetailMap = TRUE;
    }

    
    // 'Ambient color' is specified in Max docs as "the color of the object in shadows," and 
    //                 is usually a darker version of the diffuse color.
    // 'Diffuse color' is specified in the Max docs as "the color of the object in 
    //                  good lighting," usually referred to as the objects' color.
    // 'Specular color' is specified as the color of reflection highlights in direct lighting,
    //                  and according to Max docs is usually the same as diffuse.

    if (!bDetailMap)
    {
        // We're going to use the 'Diffuse' color as the object color for DirectX
        WRITE_POINT3(pbCur, pMtl->GetDiffuse());
        //Alpha
        WRITE_FLOAT(pbCur, 1 - pMtl->GetXParency());

        // 3DSMax specular power is comprised of shininess, and shininess strength.
        // TODO - figure out a mapping from shininess to power
        WRITE_FLOAT(pbCur, POWER_DEFAULT);

        // Specular
        WRITE_POINT3(pbCur, pMtl->GetSpecular());

        // Emmissive
        WRITE_POINT3(pbCur, pMtl->GetSelfIllumColor());

    }
    else  // if a detail map, then don't write the color
    {
        // diffuse - write white RGBA
        WRITE_FLOAT(pbCur, 1.0f);
        WRITE_FLOAT(pbCur, 1.0f);
        WRITE_FLOAT(pbCur, 1.0f);
        WRITE_FLOAT(pbCur, 1.0f);

        // specular power
        WRITE_FLOAT(pbCur, POWER_DEFAULT);

        // specular - write white RGBA
        WRITE_FLOAT(pbCur, 1.0f);
        WRITE_FLOAT(pbCur, 1.0f);
        WRITE_FLOAT(pbCur, 1.0f);

        // emmissive - write white RGBA
        WRITE_FLOAT(pbCur, 0.0f);
        WRITE_FLOAT(pbCur, 0.0f);
        WRITE_FLOAT(pbCur, 0.0f);
    }

    hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMMaterial,
                                    NULL,
                                    NULL,
                                    cbSize,
                                    pbData,
                                    &pDataObject
                                    );
    if (FAILED(hr))
        goto e_Exit;

    // See if there is a valid bitmap texture
    if (bDetailMap)
    {
        pTexMap->EnumAuxFiles(findTextureFilename, FILE_ENUM_ALL);

        if (findTextureFilename.m_szTextureName == NULL)
        {
            OutputDebugString("AddMaterial: Bitmap texture found, but no texture name\n");
            hr = E_FAIL;
            goto e_Exit;
        }

        if (psc->m_xFormat == DXFILEFORMAT_TEXT)
        {
            szNiceFilename = psc->m_stStrings.CreateNiceFilename(findTextureFilename.m_szTextureName); /*expand \ char to \\ */
			if (szNiceFilename == NULL)
			{
				hr = E_OUTOFMEMORY;
				goto e_Exit;
			}


            hr = AddTextureFilename(psc, szNiceFilename, pDataObject);
            if (FAILED(hr))
                goto e_Exit;
        }
        else
        {
            hr = AddTextureFilename(psc, findTextureFilename.m_szTextureName, pDataObject);
            if (FAILED(hr))
                goto e_Exit;
        }
    }

    hr = pParent->AddDataObject(pDataObject);
    if (FAILED(hr))
        goto e_Exit;

e_Exit:
    delete []pbData;
    RELEASE(pDataObject);

    return hr;
}

HRESULT
AddMeshMaterials(
    SSaveContext *psc,
    INode *pNode,
    Mesh *pMesh,
    LPDIRECTXFILEDATA pParent
    )
{
    HRESULT hr = S_OK;
    LPDIRECTXFILEDATA pDataObject = NULL;
    Mtl *pMtlMain;
    Mtl *pMtlCur;
    DWORD cSubMaterials;
    DWORD iFace;
    DWORD cFaces;
    BOOL bNoSubMaterials;
    BOOL bWireframeColor;
    PBYTE pbCur;
    PBYTE pbData = NULL;
    DWORD cbSize;
    DWORD iCurMatID;
    DWORD iCurMaterial;

    pMtlMain = pNode->GetMtl();
    cFaces = pMesh->getNumFaces();
    cSubMaterials = 0;
    bNoSubMaterials = FALSE;
    bWireframeColor = FALSE;

    // The color of a given mesh can be provided by three different sources:
    //   1) applied texture maps, as part of a material
    //   2) explicitly defined & applied materials without texture maps
    //   3) a 'wireframe' color.
    
    // For our purposes, we will output these in this order of preference, ignoring
    // processing past the first one we find.

    cbSize = sizeof(DWORD) // nMaterials
                + sizeof(DWORD) // nFaceIndexes
                + cFaces*sizeof(DWORD); // face indexes

    pbData = pbCur = new BYTE[cbSize];
    if (pbCur == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }
    
    if (pMtlMain != NULL)
    {
        // There is at least one material. We're in case 1) or 2)

        cSubMaterials = pMtlMain->NumSubMtls();
        if (cSubMaterials < 1)
        {
            // Count the material itself as a submaterial.
            cSubMaterials = 1;
            bNoSubMaterials = TRUE;
        }
    }
    else  // no material, then we'll create a material corresponding to the default wire color.
    {
        bWireframeColor = TRUE;
        cSubMaterials = 1;
    }

    WRITE_DWORD(pbCur, cSubMaterials);
    WRITE_DWORD(pbCur, cFaces);


    // For each face, output the index of the material which applies to it, 
    // starting from  0

    for (iFace=0; iFace < cFaces; iFace++)
    {

        if (bWireframeColor || bNoSubMaterials) 
        {
            // If we're using wireframe color, it's our only material
            WRITE_DWORD(pbCur, 0);
        }
        else
        {
            // Otherwise we have at least one material to process.

            iCurMatID = pMesh->faces[iFace].getMatID();

            if (cSubMaterials == 1)
            {
                iCurMatID = 0;
            }
            else
            {
                // SDK recommends mod'ing the material ID by the valid # of materials, 
                // as sometimes a material number that's too high is returned.
                iCurMatID %= cSubMaterials;
            }

            // output the appropriate material color

            WRITE_DWORD(pbCur, iCurMatID);

        } 

    } 

    // now finally create the mesh material list
    hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMMeshMaterialList,
                                    NULL,
                                    NULL,
                                    cbSize,
                                    pbData,
                                    &pDataObject
                                    );
    if (FAILED(hr))
        goto e_Exit;

    hr = pParent->AddDataObject(pDataObject);
    if (FAILED(hr))
        goto e_Exit;

    // 3DSMax uses wireframe color as its default material for a mesh.
    // Output the wireframe color as the material if there are no explicit materials.

    if (bWireframeColor)
    {
        AddWireframeMaterial(psc, pNode, pDataObject);
    } 
    else
    {
        // 3DSMax allows multiple materials to be used on a single mesh via
        // 'submaterials'. When the first submaterial is defined, the main material
        // is copied into submaterial #1, and the new submaterial because submaterial #2.
        // 
        // We have to catch the case where there's a material, but no submaterials. For this
        // case, set NumSubMaterials to 1 which would never happen otherwise. It's imperative
        // that the first material be set to MtlMain, rather than trying to GetSubMtl() to 
        // allow this logic to work.

        // Loop through the materials (if any) and output them.

        for (iCurMaterial = 0; (iCurMaterial < cSubMaterials); iCurMaterial++)
        {
            if (bNoSubMaterials)
            {
                // We're on the parent material, possibly the ONLY material.
                // We won't be able to get it with GetSubMtl() if it's the only one, and
                // the data in the first submaterial is identical to the main material,
                // so just use the main material.

                pMtlCur = pMtlMain;
            }
            else
            {
                // We're into the true submaterials. Safe to get with 'GetSubMtl'

                pMtlCur = pMtlMain->GetSubMtl(iCurMaterial);
            }

            hr = AddMaterial(psc, pMtlCur, pDataObject);
            if (FAILED(hr))
                goto e_Exit;
        } 
    }

e_Exit:
    delete []pbData;
    RELEASE(pDataObject);


    return hr;
} 

// structe used by AddSkinData to convert skin data from per vertex to per bone
struct SBoneState
{
    SBoneState()
        :m_pbData(NULL), m_szBoneName(NULL) {}
    ~SBoneState()
        { 
            delete []m_pbData; 

			// Bone name is allocated out of a string pool because it has to be deleted after
			//   the data has been saved to disk
            //delete []m_szBoneName; 
        }

    // info computed up front (note m_rgdwIndices, m_rgfWeights and m_pmatOFfset point into pbData)
    INode *m_pBoneNode;
    DWORD m_cVertices;
    DWORD m_cbData;
    PBYTE m_pbData;
    char *m_szBoneName;
    DWORD *m_rgdwIndices;
    float *m_rgfWeights;
    D3DXMATRIX *m_pmatOffset;

    // current index of last added vertex index
    DWORD m_iCurIndex;
};

SBoneState *FindBoneData
    (
    INode *pBoneNode, 
    SBoneState *rgbsBoneData, 
    DWORD cBones
    )
{
    DWORD iBone;
    SBoneState *pbs = NULL;

    for (iBone = 0; iBone < cBones; iBone++)
    {
        if (rgbsBoneData[iBone].m_pBoneNode == pBoneNode)
        {
            pbs = &rgbsBoneData[iBone];
            break;
        }
    }

    return pbs;
}

HRESULT AddSkinData
    (
    SSaveContext *psc,
    INode *pNode,
    SMeshData *pMeshData,
    LPDIRECTXFILEDATA pParent
    )
{
    HRESULT hr = S_OK;
    IPhyVertexExport *pVertexExport;
    IPhyRigidVertex* pRigidVertexExport;
    IPhyBlendedRigidVertex *pBlendedRigidVertexExport;
    INode* pBoneNode;
    SSkinMap *psmSkinMap = NULL;
    Modifier* pPhyMod = NULL;
    IPhysiqueExport* pPhyExport = NULL;
    IPhyContextExport* pPhyContextExport = NULL;
    SBoneState *rgbsBoneData = NULL;
    SBoneState *rgbsBoneDataDups = NULL;
    SBoneState *pbsCurData = NULL;
    SBoneState *pbsOldData = NULL;
    DWORD iVertex;
    DWORD cVertices;
    DWORD iVertexType;
    DWORD cTotalBones;
    DWORD iBone;
    PBYTE pbCur;
    float fWeight;
    LPDIRECTXFILEDATA pDataObject = NULL;
    DWORD iIndex;
    DWORD iNextWedge;
    Matrix3 matMesh;
    Matrix3 matOffset;
    Matrix3 matZero;
	PBYTE pbHeaderData = NULL;
	DWORD cbHeaderData;
    static bool shownError = false;

    matZero.Zero();
    pPhyMod = FindPhysiqueModifier(pNode);
    if (pPhyMod != NULL)
    {
        // Get a Physique Export interface
        pPhyExport = (IPhysiqueExport *)pPhyMod->GetInterface(I_PHYINTERFACE);
        if (pPhyExport == NULL)
        {   // not all interfaces present, so ignore
            goto e_NoPhysique;
        }

        // get the init matrix for the mesh (used to mult by the bone matrices)
        int rval = pPhyExport->GetInitNodeTM(pNode, matMesh);
        if (rval || matMesh.Equals(matZero, 0.0))
        {
            if (!shownError)
            {
                shownError = true;
                MessageBox(NULL, "This mesh was skinned using an older version of Character Studio. Mesh may not be exported correctly",
                           "X File export Warning", MB_OK);
            }
        }

        // For a given Object's INode get a
        // ModContext Interface from the Physique Export Interface:
        pPhyContextExport = (IPhyContextExport *)pPhyExport->GetContextInterface(pNode);
        if(pPhyContextExport == NULL)
        {
            // not all interfaces present, so ignore
            goto e_NoPhysique;
        }

        // should already have been converted to rigid with blending by Preprocess
            // convert to rigid with blending
            pPhyContextExport->ConvertToRigid(TRUE);
            pPhyContextExport->AllowBlending(TRUE);

        psmSkinMap = psc->GetSkinMap(pNode);
        if (psmSkinMap == NULL)
        {
            OutputDebugString("AddSkinData: Found physique info at save time, but not preprocess\n");
            hr = E_FAIL;
            goto e_Exit;
        }

        // now setup state used to fill arrays for output 
        rgbsBoneData = new SBoneState[psmSkinMap->m_cbiBones];
        if (rgbsBoneData == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto e_Exit;
        }

        // intialize each of the bone structures 
        for (iBone = 0; iBone < psmSkinMap->m_cbiBones; iBone++)
        {
            pbsCurData = &rgbsBoneData[iBone];

            pbsCurData->m_iCurIndex = 0;
            pbsCurData->m_cVertices = psmSkinMap->m_rgbiBones[iBone].m_cVertices;
            pbsCurData->m_pBoneNode = psmSkinMap->m_rgbiBones[iBone].m_pBoneNode;

            // allocate memory to pass to D3DXOF lib
            pbsCurData->m_cbData = sizeof(TCHAR*)
                                    + sizeof(DWORD) // numWeights
                                    + sizeof(DWORD) * pbsCurData->m_cVertices // array of vertex indices
                                    + sizeof(float) * pbsCurData->m_cVertices // array of weights
                                    + sizeof(float) * 16; // offset matrix

            pbCur = pbsCurData->m_pbData = new BYTE[pbsCurData->m_cbData];
            if (pbsCurData->m_pbData == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto e_Exit;
            }

            // fill first few entries, and remember pointers to the other arrays/matrix
            WRITE_PTCHAR(pbCur, pbsCurData->m_szBoneName);
            WRITE_DWORD(pbCur, pbsCurData->m_cVertices);

            pbsCurData->m_rgdwIndices = (DWORD*)pbCur;
            pbCur += sizeof(DWORD) * pbsCurData->m_cVertices;

            pbsCurData->m_rgfWeights = (float*)pbCur;
            pbCur += sizeof(float) * pbsCurData->m_cVertices;

            pbsCurData->m_pmatOffset = (D3DXMATRIX*)pbCur;

            // compute the mat offset
            int rval = pPhyExport->GetInitNodeTM(pbsCurData->m_pBoneNode, matOffset);
            if (rval)
            {
                MessageBox(NULL, "This mesh was skinned using an older version of Character Studio. Mesh may not be exported correctly",
                           "X File export Warning", MB_OK);
            }
            matOffset.Invert();
            matOffset = matMesh * matOffset;

            WRITE_MATRIX4_FROM_MATRIX3(pbCur, matOffset);
        }

        cVertices = pPhyContextExport->GetNumberVertices();
        for (iVertex = 0; iVertex < cVertices; iVertex++ )
        {
            pVertexExport = (IPhyVertexExport *)pPhyContextExport->GetVertexInterface(iVertex);    
            if (pVertexExport == NULL)
            {
                OutputDebugString("Couldn't get export interface!");
                hr = E_FAIL;
                goto e_Exit;
            }
        
            // What kind of vertices are these?
            iVertexType = pVertexExport->GetVertexType();

            pPhyContextExport->ReleaseVertexInterface( pVertexExport );    

            if( iVertexType == RIGID_TYPE )
            {
                pRigidVertexExport = (IPhyRigidVertex *)pPhyContextExport->GetVertexInterface(iVertex);
                if (pRigidVertexExport == NULL)
                {
                    OutputDebugString("Couldn't get rigid vertex export interface!");
                    hr = E_FAIL;
                    goto e_Exit;
                }
                // Get the vertex info!
            
                pBoneNode = pRigidVertexExport->GetNode();

                pbsCurData = FindBoneData(pBoneNode, rgbsBoneData, psmSkinMap->m_cbiBones);
                if (pbsCurData == NULL)
                {
                    assert(0);
                    OutputDebugString("AddSkinData: Bone node not found on second pass\n");
                    hr = E_FAIL;
                    goto e_Exit;
                }

                pbsCurData->m_rgdwIndices[pbsCurData->m_iCurIndex] = iVertex;
                pbsCurData->m_rgfWeights[pbsCurData->m_iCurIndex] = 1.0f;
                pbsCurData->m_iCurIndex += 1;

                pPhyContextExport->ReleaseVertexInterface( pRigidVertexExport);
            }
            else
            {
                assert( iVertexType == RIGID_BLENDED_TYPE );

                pBlendedRigidVertexExport = (IPhyBlendedRigidVertex *)pPhyContextExport->GetVertexInterface(iVertex);
                if (pBlendedRigidVertexExport == NULL)
                {
                    OutputDebugString("Couldn't get blended rigid vertex export interface!");
                    hr = E_FAIL;
                    goto e_Exit;
                }

                // How many nodes affect his vertex?
                cTotalBones = pBlendedRigidVertexExport->GetNumberNodes();
                for (iBone = 0; iBone < cTotalBones; iBone++ )
                {
                    pBoneNode = pBlendedRigidVertexExport->GetNode(iBone);

                    pbsCurData = FindBoneData(pBoneNode, rgbsBoneData, psmSkinMap->m_cbiBones);
                    if (pbsCurData == NULL)
                    {
                        assert(0);
                        OutputDebugString("AddSkinData: Bone node not found on second pass\n");
                        hr = E_FAIL;
                        goto e_Exit;
                    }

                    fWeight = pBlendedRigidVertexExport->GetWeight(iBone);

                    // first see if it is a repeat weight, is so just add to previous
                    if ((pbsCurData->m_iCurIndex > 0) && (pbsCurData->m_rgdwIndices[pbsCurData->m_iCurIndex-1] == iVertex))
                    {
                        pbsCurData->m_rgfWeights[pbsCurData->m_iCurIndex-1] += fWeight;
                    }
                    else
                    {
                        pbsCurData->m_rgdwIndices[pbsCurData->m_iCurIndex] = iVertex;
                        pbsCurData->m_rgfWeights[pbsCurData->m_iCurIndex] = fWeight;
                        pbsCurData->m_iCurIndex += 1;
                    }                    
                }

                pPhyContextExport->ReleaseVertexInterface( pBlendedRigidVertexExport);
            }
        }
    }
e_NoPhysique:



    if (rgbsBoneData != NULL)
    {
        assert(psmSkinMap != NULL);

// now deal with the wonderful world of duplicated indices
        rgbsBoneDataDups = new SBoneState[psmSkinMap->m_cbiBones];
        if (rgbsBoneDataDups == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto e_Exit;
        }

        // first calculate new lengths for the bone weight arrays
        for (iBone = 0; iBone < psmSkinMap->m_cbiBones; iBone++)
        {
            pbsOldData = &rgbsBoneData[iBone];
            pbsCurData = &rgbsBoneDataDups[iBone];

            pbsCurData->m_cVertices = pbsOldData->m_cVertices;
            for (iIndex = 0; iIndex < pbsOldData->m_cVertices; iIndex++)
            {
                iVertex = pbsOldData->m_rgdwIndices[iIndex];

                iNextWedge = pMeshData->m_rgVertices[iVertex].iWedgeList;
                while (iVertex != iNextWedge)
                {
                    pbsCurData->m_cVertices += 1;

                    iNextWedge = pMeshData->m_rgVertices[iNextWedge].iWedgeList;
                }
            }
        }

        // next build 
        for (iBone = 0; iBone < psmSkinMap->m_cbiBones; iBone++)
        {
            pbsOldData = &rgbsBoneData[iBone];
            pbsCurData = &rgbsBoneDataDups[iBone];

            pbsCurData->m_pBoneNode = pbsOldData->m_pBoneNode;
            pbsCurData->m_iCurIndex = 0;

            // allocate memory to pass to D3DXOF lib
            pbsCurData->m_cbData = sizeof(TCHAR*)
                                    + sizeof(DWORD) // numWeights
                                    + sizeof(DWORD) * pbsCurData->m_cVertices // array of vertex indices
                                    + sizeof(float) * pbsCurData->m_cVertices // array of weights
                                    + sizeof(float) * 16; // offset matrix

            pbCur = pbsCurData->m_pbData = new BYTE[pbsCurData->m_cbData];
            pbsCurData->m_szBoneName = psc->m_stStrings.CreateNiceString(pbsCurData->m_pBoneNode->GetName());
            if ((pbsCurData->m_pbData == NULL) || (pbsCurData->m_szBoneName == NULL))
            {
                hr = E_OUTOFMEMORY;
                goto e_Exit;
            }

            // fill first few entries, and remember pointers to the other arrays/matrix
            WRITE_PTCHAR(pbCur, pbsCurData->m_szBoneName);
            WRITE_DWORD(pbCur, pbsCurData->m_cVertices);

            pbsCurData->m_rgdwIndices = (DWORD*)pbCur;
            pbCur += sizeof(DWORD) * pbsCurData->m_cVertices;

            pbsCurData->m_rgfWeights = (float*)pbCur;
            pbCur += sizeof(float) * pbsCurData->m_cVertices;

            pbsCurData->m_pmatOffset = (D3DXMATRIX*)pbCur;

            // already loaded above, copy from the original state data
            *pbsCurData->m_pmatOffset = *pbsOldData->m_pmatOffset;


            // now that we have the new states set up, copy the data from the old states
            for (iIndex = 0; iIndex < pbsOldData->m_cVertices; iIndex++)
            {
                iVertex = pbsOldData->m_rgdwIndices[iIndex];

                pbsCurData->m_rgdwIndices[pbsCurData->m_iCurIndex] = iVertex;
                pbsCurData->m_rgfWeights[pbsCurData->m_iCurIndex] = pbsOldData->m_rgfWeights[iIndex];
                pbsCurData->m_iCurIndex += 1;

                iNextWedge = pMeshData->m_rgVertices[iVertex].iWedgeList;
                while (iVertex != iNextWedge)
                {
                    pbsCurData->m_rgdwIndices[pbsCurData->m_iCurIndex] = iNextWedge;
                    pbsCurData->m_rgfWeights[pbsCurData->m_iCurIndex] = pbsOldData->m_rgfWeights[iIndex];
                    pbsCurData->m_iCurIndex += 1;

                    iNextWedge = pMeshData->m_rgVertices[iNextWedge].iWedgeList;
                }
            }
        }

		// now that we do have skin data to put in the file, add a skinning header record
		cbHeaderData = sizeof(DWORD) * 3;
		pbCur = pbHeaderData = new BYTE[cbHeaderData];
		if (pbHeaderData == NULL)
		{
			hr = E_OUTOFMEMORY;
			goto e_Exit;
		}

		WRITE_WORD(pbCur, psc->m_cMaxWeightsPerVertex);
		WRITE_WORD(pbCur, psc->m_cMaxWeightsPerFace);
		WRITE_WORD(pbCur, psmSkinMap->m_cbiBones);

        hr = psc->m_pxofsave->CreateDataObject(DXFILEOBJ_XSkinMeshHeader,
                                        NULL,
                                        NULL,
                                        cbHeaderData,
                                        pbHeaderData,
                                        &pDataObject
                                        );
        hr = pParent->AddDataObject(pDataObject);
        if (FAILED(hr))
            goto e_Exit;

        RELEASE(pDataObject);

// now actually output the prepared buffers
        for (iBone = 0; iBone < psmSkinMap->m_cbiBones; iBone++)
        {
            assert(rgbsBoneData[iBone].m_iCurIndex == rgbsBoneData[iBone].m_cVertices);
            assert(rgbsBoneDataDups[iBone].m_iCurIndex == rgbsBoneDataDups[iBone].m_cVertices);

            hr = psc->m_pxofsave->CreateDataObject(DXFILEOBJ_SkinWeights,
                                            NULL,
                                            NULL,
                                            rgbsBoneDataDups[iBone].m_cbData,
                                            rgbsBoneDataDups[iBone].m_pbData,
                                            &pDataObject
                                            );
            if (FAILED(hr))
                goto e_Exit;

            hr = pParent->AddDataObject(pDataObject);
            if (FAILED(hr))
                goto e_Exit;

            RELEASE(pDataObject);
        }
    }

e_Exit:
	delete []pbHeaderData;
    delete []rgbsBoneData;
    delete []rgbsBoneDataDups;
    return hr;
}

HRESULT AddMesh
    (
    SSaveContext *psc,
    INode *pNode, 
    Object* pObj,
    LPDIRECTXFILEDATA pParent
    )
{
    HRESULT hr = S_OK;
    BOOL bDeleteTriObject = false;
    TriObject *pTriObject = NULL;
    Mesh *pMesh;
    BOOL bSwapTriOrder;
    PBYTE pbData = NULL;
    PBYTE pbCur;
    DWORD cbSize;
    DWORD cVertices;
    DWORD cFaces;
    DWORD iFace;
    Matrix3 matNodeTM;
    SMeshData MeshData;
    LPDIRECTXFILEDATA pDataObject = NULL;
    DWORD iVertex;

    // Retrieve the TriObject from the node

    pTriObject = GetTriObjectFromObjRef(pObj, &bDeleteTriObject);

    // If no TriObject then skip
    if (pTriObject == NULL) 
        goto e_Exit;

    pMesh = &(pTriObject->mesh);
    pMesh->checkNormals(TRUE); // TODO: Is this necessary?
    matNodeTM = pNode->GetNodeTM(psc->m_iTime);
    bSwapTriOrder = matNodeTM.Parity();

    hr = GenerateMeshData(pMesh, &MeshData);
    if (FAILED(hr))
        goto e_Exit;

    cVertices = MeshData.m_cVertices;
    cFaces = MeshData.m_cFaces;
    cbSize = sizeof(DWORD) // nVertices
             + cVertices*sizeof(float)*3 // vertices
             + sizeof(DWORD) // nFaces
             + cFaces*(sizeof(DWORD) /*nFaceVertexIndices*/ 
                            + sizeof(DWORD)*3 /*faceVertexIndices*/); // faces

    pbCur = pbData = new BYTE[cbSize];
    if (pbData == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    // write nVertices
    WRITE_DWORD(pbCur, cVertices);

    // write vertices
    for (iVertex = 0; iVertex < MeshData.m_cVertices; iVertex++)
    {
        WRITE_POINT3(pbCur, pMesh->verts[MeshData.m_rgVertices[iVertex].iPointRep]);
    }

    // write nFaces
    WRITE_DWORD(pbCur, cFaces);
    
    // write faces
    for( iFace = 0; iFace < cFaces; iFace++ )
    {
        WRITE_DWORD(pbCur, 3); //nFaceVertexIndices

        // face vertex indices
        if( bSwapTriOrder )
        {
            WRITE_DWORD(pbCur, MeshData.m_rgFaces[iFace].index[0]);
            WRITE_DWORD(pbCur, MeshData.m_rgFaces[iFace].index[2]);
            WRITE_DWORD(pbCur, MeshData.m_rgFaces[iFace].index[1]);
        }
        else
        {
            WRITE_DWORD(pbCur, MeshData.m_rgFaces[iFace].index[0]);
            WRITE_DWORD(pbCur, MeshData.m_rgFaces[iFace].index[1]);
            WRITE_DWORD(pbCur, MeshData.m_rgFaces[iFace].index[2]);
        }
    }

    hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMMesh,
                                    NULL,
                                    NULL,
                                    cbSize,
                                    pbData,
                                    &pDataObject
                                    );
    if (FAILED(hr))
        goto e_Exit;

    // add the normals to the file
    hr = AddNormals(psc, &MeshData, bSwapTriOrder, pDataObject);
    if (FAILED(hr))
        goto e_Exit;

    // write texture coordinates
    hr = AddTextureCoordinates(psc, &MeshData, pMesh, pDataObject);
    if (FAILED(hr))
        goto e_Exit;

    hr = AddVertexDuplicationIndices(psc, &MeshData, pDataObject);
    if (FAILED(hr))
        goto e_Exit;

    hr = AddMeshMaterials(psc, pNode, pMesh, pDataObject);
    if (FAILED(hr))
        goto e_Exit;

    hr = AddSkinData(psc, pNode, &MeshData, pDataObject);
    if (FAILED(hr))
        goto e_Exit;

    hr = pParent->AddDataObject(pDataObject);
    if (FAILED(hr))
        goto e_Exit;

e_Exit:
    if (bDeleteTriObject)
    {
        delete pTriObject;
    }

    RELEASE(pDataObject);
    return hr;
}

HRESULT
AddPatchMeshMaterials(
    SSaveContext *psc,
    INode *pNode,
    PatchMesh *pPatchMesh,
    LPDIRECTXFILEDATA pParent
    )
{
    HRESULT hr = S_OK;
    LPDIRECTXFILEDATA pDataObject = NULL;
    Mtl *pMtlMain;
    Mtl *pMtlCur;
    DWORD cSubMaterials;
    DWORD iPatch;
    DWORD cPatches;
    BOOL bNoSubMaterials;
    BOOL bWireframeColor;
    PBYTE pbCur;
    PBYTE pbData = NULL;
    DWORD cbSize;
    DWORD iCurMatID;
    DWORD iCurMaterial;

    pMtlMain = pNode->GetMtl();
    cPatches = pPatchMesh->getNumPatches();
    cSubMaterials = 0;
    bNoSubMaterials = FALSE;
    bWireframeColor = FALSE;

    // The color of a given mesh can be provided by three different sources:
    //   1) applied texture maps, as part of a material
    //   2) explicitly defined & applied materials without texture maps
    //   3) a 'wireframe' color.
    
    // For our purposes, we will output these in this order of preference, ignoring
    // processing past the first one we find.

    cbSize = sizeof(DWORD) // nMaterials
                + sizeof(DWORD) // nFaceIndexes
                + cPatches*sizeof(DWORD); // face indexes

    pbData = pbCur = new BYTE[cbSize];
    if (pbCur == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }
    
    if (pMtlMain != NULL)
    {
        // There is at least one material. We're in case 1) or 2)

        cSubMaterials = pMtlMain->NumSubMtls();
        if (cSubMaterials < 1)
        {
            // Count the material itself as a submaterial.
            cSubMaterials = 1;
            bNoSubMaterials = TRUE;
        }
    }
    else  // no material, then we'll create a material corresponding to the default wire color.
    {
        bWireframeColor = TRUE;
        cSubMaterials = 1;
    }

    WRITE_DWORD(pbCur, cSubMaterials);
    WRITE_DWORD(pbCur, cPatches);


    // For each face, output the index of the material which applies to it, 
    // starting from  0

    for (iPatch=0; iPatch < cPatches; iPatch++)
    {

        if (bWireframeColor || bNoSubMaterials) 
        {
            // If we're using wireframe color, it's our only material
            WRITE_DWORD(pbCur, 0);
        }
        else
        {
            // Otherwise we have at least one material to process.

            iCurMatID = pPatchMesh->getPatchMtlIndex(iPatch);

            if (cSubMaterials == 1)
            {
                iCurMatID = 0;
            }
            else
            {
                // SDK recommends mod'ing the material ID by the valid # of materials, 
                // as sometimes a material number that's too high is returned.
                iCurMatID %= cSubMaterials;
            }

            // output the appropriate material color

            WRITE_DWORD(pbCur, iCurMatID);

        } 

    } 

    // now finally create the mesh material list
    hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMMeshMaterialList,
                                    NULL,
                                    NULL,
                                    cbSize,
                                    pbData,
                                    &pDataObject
                                    );
    if (FAILED(hr))
        goto e_Exit;

    hr = pParent->AddDataObject(pDataObject);
    if (FAILED(hr))
        goto e_Exit;

    // 3DSMax uses wireframe color as its default material for a mesh.
    // Output the wireframe color as the material if there are no explicit materials.

    if (bWireframeColor)
    {
        AddWireframeMaterial(psc, pNode, pDataObject);
    } 
    else
    {
        // 3DSMax allows multiple materials to be used on a single mesh via
        // 'submaterials'. When the first submaterial is defined, the main material
        // is copied into submaterial #1, and the new submaterial because submaterial #2.
        // 
        // We have to catch the case where there's a material, but no submaterials. For this
        // case, set NumSubMaterials to 1 which would never happen otherwise. It's imperative
        // that the first material be set to MtlMain, rather than trying to GetSubMtl() to 
        // allow this logic to work.

        // Loop through the materials (if any) and output them.

        for (iCurMaterial = 0; (iCurMaterial < cSubMaterials); iCurMaterial++)
        {
            if (bNoSubMaterials)
            {
                // We're on the parent material, possibly the ONLY material.
                // We won't be able to get it with GetSubMtl() if it's the only one, and
                // the data in the first submaterial is identical to the main material,
                // so just use the main material.

                pMtlCur = pMtlMain;
            }
            else
            {
                // We're into the true submaterials. Safe to get with 'GetSubMtl'

                pMtlCur = pMtlMain->GetSubMtl(iCurMaterial);
            }

            hr = AddMaterial(psc, pMtlCur, pDataObject);
            if (FAILED(hr))
                goto e_Exit;
        } 
    }

e_Exit:
    delete []pbData;
    RELEASE(pDataObject);


    return hr;
} 

HRESULT AddPatchMesh
    (
    SSaveContext *psc,
    INode *pNode, 
    Object* pObj,
    LPDIRECTXFILEDATA pParent
    )
{
    HRESULT    hr = S_OK;
    LPDIRECTXFILEDATA pDataObject = NULL;
    PBYTE         pbData = NULL;
    PBYTE		  pbCur = NULL;        
    DWORD          cbSize;
    DWORD iVertex;
    DWORD iPatch;
    Point3 *pvVertices;
    PatchMesh *pPatchMesh;
	PatchObject *pPatchObject = NULL;
	BOOL bDeletePatchObject;
	DWORD iControl;
    DWORD cPatchIndices;
	SPatchMeshData PatchMeshData;

	assert(psc != NULL);
	assert(pNode != NULL);
	assert(pObj != NULL);
	assert(pParent != NULL);

    pPatchObject = GetPatchObjectFromObjRef(pObj, &bDeletePatchObject);

    // If no PatchObject then skip
    if (pPatchObject == NULL) 
        goto e_Exit;

    pPatchMesh = &pPatchObject->patch;

	// massage the patch data into an outputable format (including texture coord handling)
	hr = GeneratePatchMeshData(pPatchMesh, &PatchMeshData);
	if (FAILED(hr))
		goto e_Exit;

	// figure out the total number of control indices
	cPatchIndices = 0;
	for (iPatch = 0; iPatch < PatchMeshData.m_cPatches; iPatch++)
	{
		cPatchIndices += PatchMeshData.m_rgPatches[iPatch].m_cControl;
	}

    cbSize = sizeof(DWORD) // nVertices
             + PatchMeshData.m_cVertices * sizeof(float)*3 // vertices
             + sizeof(DWORD) // nPatches
             + PatchMeshData.m_cPatches * sizeof(DWORD) /*nControlIndices*/ 
             + cPatchIndices * sizeof(DWORD) /*controlIndices*/; // patches

    pbCur = pbData = new BYTE[cbSize];
    if (pbData == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

	// write the number of vertices
	WRITE_DWORD(pbCur, PatchMeshData.m_cVertices);

    // first write all the vertices
    pvVertices = (Point3*)pbCur;
    for (iVertex = 0; iVertex < PatchMeshData.m_cVertices; iVertex++)
    {
        pvVertices[iVertex] = PatchMeshData.m_rgVertices[iVertex].vPosition;
    }

    // skip over the vertices
    pbCur = pbData + sizeof(DWORD) + sizeof(float)*3*PatchMeshData.m_cVertices;

	// write the number of patches
	WRITE_DWORD(pbCur, PatchMeshData.m_cPatches);

	// now write all the patch data
	for (iPatch = 0; iPatch < PatchMeshData.m_cPatches; iPatch++)
	{
		WRITE_DWORD(pbCur, PatchMeshData.m_rgPatches[iPatch].m_cControl);
		for (iControl = 0; iControl < PatchMeshData.m_rgPatches[iPatch].m_cControl; iControl++)
		{
			WRITE_DWORD(pbCur, PatchMeshData.m_rgPatches[iPatch].m_rgdwControl[iControl]);
		}
	}

    hr = psc->m_pxofsave->CreateDataObject(DXFILEOBJ_PatchMesh,
                                    NULL,
                                    NULL,
                                    cbSize,
                                    pbData,
                                    &pDataObject
                                    );
    if (FAILED(hr))
        goto e_Exit;

    hr = AddPatchTextureCoordinates(psc, &PatchMeshData, pPatchMesh, pDataObject);
    if (FAILED(hr))
        goto e_Exit;

    hr = AddPatchVertexDuplicationIndices(psc, &PatchMeshData, pDataObject);
    if (FAILED(hr))
        goto e_Exit;

    hr = AddPatchMeshMaterials(psc, pNode, pPatchMesh, pDataObject);
    if (FAILED(hr))
        goto e_Exit;

    hr = pParent->AddDataObject(pDataObject);
    if (FAILED(hr))
        goto e_Exit;

e_Exit:
    delete []pbData;
    RELEASE(pDataObject);

    if (bDeletePatchObject)
    {
        delete pPatchObject;
    }

    return hr;
}

HRESULT AddTransform
    (
    SSaveContext *psc,
    INode *pNode, 
    LPDIRECTXFILEDATA pParent
    )
{
    HRESULT hr = S_OK;
    PBYTE pbData = NULL;
    PBYTE pbCur;
    DWORD cbSize;
    Matrix3 matNodeTM;
    Matrix3 matRelativeTM;
    LPDIRECTXFILEDATA pDataObject = NULL;

    cbSize = 16*sizeof(float);

    pbCur = pbData = new BYTE[cbSize];
    if (pbData == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    // Node transform
    matNodeTM = pNode->GetNodeTM(psc->m_iTime);
    if( pNode->IsRootNode() )
    {
        // scene root
        matRelativeTM = matNodeTM;
    }
    else
    {
        Matrix3 matParentTM = pNode->GetParentTM(psc->m_iTime);
        if( memcmp(&matNodeTM,&matParentTM,12*sizeof(float)) == 0 )
        {
            matRelativeTM.IdentityMatrix();
        }
        else
        {
            matRelativeTM = matNodeTM * Inverse(matParentTM);
        }
    }

    WRITE_MATRIX4_FROM_MATRIX3(pbCur, matRelativeTM);

    hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMFrameTransformMatrix,
                                    NULL,
                                    NULL,
                                    cbSize,
                                    pbData,
                                    &pDataObject
                                    );
    if (FAILED(hr))
        goto e_Exit;

    hr = pParent->AddDataObject(pDataObject);
    if (FAILED(hr))
        goto e_Exit;

e_Exit:
    delete []pbData;
    RELEASE(pDataObject);

    return hr;
}

HRESULT AddObjectOffsetTransform
    (
    SSaveContext *psc,
	INode *pNode,
    LPDIRECTXFILEDATA pParent,
    LPDIRECTXFILEDATA *ppMeshParent
    )
{
    HRESULT hr = S_OK;
    PBYTE pbData = NULL;
    PBYTE pbCur;
    DWORD cbSize;
    LPDIRECTXFILEDATA pDataObject = NULL;
	Matrix3 matNodeTM;
	Matrix3 matObjTMAfterWSM;
	Matrix3 matObjectOffset;
    LPDIRECTXFILEDATA pObjectOffset = NULL;

	// check to see if the node has an object offset matrix
	matNodeTM = pNode->GetNodeTM(psc->m_iTime);
	matObjTMAfterWSM = pNode->GetObjTMAfterWSM(psc->m_iTime);
	if (memcmp(&matObjTMAfterWSM, &matNodeTM,12*sizeof(Matrix3)) != 0)
	{
		// the mesh is positioned offset from the node, so add another
		//   frame (unnamed) to offset the mesh without affecting the node's children
		//   and/or animation attached to the node
		hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMFrame,
										NULL,
										NULL,
										0,
										NULL,
										&pObjectOffset
										);

		matObjectOffset = matObjTMAfterWSM * Inverse(matNodeTM);


		cbSize = 16*sizeof(float);

		pbCur = pbData = new BYTE[cbSize];
		if (pbData == NULL)
		{
			hr = E_OUTOFMEMORY;
			goto e_Exit;
		}

		WRITE_MATRIX4_FROM_MATRIX3(pbCur, matObjectOffset);

		hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMFrameTransformMatrix,
										NULL,
										NULL,
										cbSize,
										pbData,
										&pDataObject
										);
		if (FAILED(hr))
			goto e_Exit;

		hr = pObjectOffset->AddDataObject(pDataObject);
		if (FAILED(hr))
			goto e_Exit;

		hr = pParent->AddDataObject(pObjectOffset);
		if (FAILED(hr))
			goto e_Exit;


		*ppMeshParent = pObjectOffset;
	}
	else  // identity object offset, mesh should use node as parent
	{
		*ppMeshParent = pParent;
	}

e_Exit:
    delete []pbData;
    RELEASE(pDataObject);
	RELEASE(pObjectOffset);

    return hr;
}

HRESULT EnumTree
    (
    SSaveContext *psc,
    INode *pNode, 
    LPDIRECTXFILEDATA *ppDataObjectNew
    )
{
    HRESULT hr = S_OK;
    DWORD cChildren;
    DWORD iChild;
    LPDIRECTXFILEDATA pChildDataObject;
    LPDIRECTXFILEDATA pDataObject = NULL;
    LPDIRECTXFILEDATA pMeshParent;
    Object* pObj;
    char *szName = NULL;
    
    szName = psc->m_stStrings.CreateNiceString(pNode->GetName());
    if (szName == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    OutputDebugString(szName);
    OutputDebugString("\n");

    // add the node to the array for anim purposes
    assert( psc->m_cNodesCur < psc->m_cNodes );
    psc->m_rgpnNodes[psc->m_cNodesCur] = pNode;
    psc->m_cNodesCur += 1;

    hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMFrame,
                                    szName,
                                    NULL,
                                    0,
                                    NULL,
                                    &pDataObject
                                    );

    hr = AddTransform(psc, pNode, pDataObject);
    if (FAILED(hr))
        goto e_Exit;

	if (psc->m_bSavePatchData && IsExportablePatchMesh(pNode, pObj) && !(psc->m_bSaveSelection && !pNode->Selected()))
	{
		hr = AddObjectOffsetTransform(psc, pNode, pDataObject, &pMeshParent);
		if (FAILED(hr))
			goto e_Exit;

        hr = AddPatchMesh(psc, pNode, pObj, pMeshParent);
        if (FAILED(hr))
            goto e_Exit;
	}
    else if (IsExportableMesh(pNode, pObj) && !(psc->m_bSaveSelection && !pNode->Selected()))
    {
		hr = AddObjectOffsetTransform(psc, pNode, pDataObject, &pMeshParent);
		if (FAILED(hr))
			goto e_Exit;
		
        hr = AddMesh(psc, pNode, pObj, pMeshParent);
        if (FAILED(hr))
            goto e_Exit;
    }

    cChildren = pNode->NumberOfChildren();
    for (iChild = 0; iChild < cChildren; iChild++)
    {
        // enumerate the child
        hr = EnumTree(psc, pNode->GetChildNode(iChild), &pChildDataObject);
        if (FAILED(hr))
            goto e_Exit;
        
        hr = pDataObject->AddDataObject(pChildDataObject);
        if (FAILED(hr))
            goto e_Exit;

        RELEASE(pChildDataObject);
    }

    *ppDataObjectNew = pDataObject;
e_Exit:
    return hr;
}


HRESULT Preprocess
    (
    SPreprocessContext *ppc,
    INode *pNode
    )
{
    HRESULT hr = S_OK;
    DWORD cChildren;
    DWORD iChild;
    Object* pObj;
    SSkinMap **rgpsmTemp = NULL;
    IPhyVertexExport *pVertexExport;
    IPhyRigidVertex* pRigidVertexExport;
    IPhyBlendedRigidVertex *pBlendedRigidVertexExport;
    INode* pBoneNode;
    SSkinMap *psmSkinMap;
    Modifier* pPhyMod = NULL;
    IPhysiqueExport* pPhyExport = NULL;
    IPhyContextExport* pPhyContextExport = NULL;
    DWORD iVertex;
    DWORD cVertices;
    DWORD iVertexType;
    SBoneInfo *pbi;
    DWORD cTotalBones;
    DWORD iBone;
    DWORD cpnBonesSeen;
    DWORD cpnBonesSeenMax;
    INode **rgpnBonesSeen = NULL;
    INode **rgpnTemp;
    BOOL bBoneSeen;
    DWORD iBoneSeen;
    BOOL bDeleteTriObject = false;
    TriObject *pTriObject = NULL;
    Mesh *pMesh;
	DWORD iFace;
	DWORD iPoint;

    ppc->m_cNodes += 1;

    if (IsExportableMesh(pNode,pObj) && !(ppc->m_bSaveSelection && !pNode->Selected()))
    {
        // first see if physique is present
        pPhyMod = FindPhysiqueModifier(pNode);
        if (pPhyMod != NULL)
        {
            // Get a Physique Export interface
            pPhyExport = (IPhysiqueExport *)pPhyMod->GetInterface(I_PHYINTERFACE);
            if (pPhyExport == NULL)
            {   // not all interfaces present, so ignore
                goto e_NoPhysique;
            }
            // For a given Object's INode get a
            // ModContext Interface from the Physique Export Interface:
            pPhyContextExport = (IPhyContextExport *)pPhyExport->GetContextInterface(pNode);
            if(pPhyContextExport == NULL)
            {
                // not all interfaces present, so ignore
                goto e_NoPhysique;
            }

            // convert to rigid with blending
            pPhyContextExport->ConvertToRigid(TRUE);
            pPhyContextExport->AllowBlending(TRUE);

            psmSkinMap = new SSkinMap;
            if (psmSkinMap == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto e_Exit;
            }

            // realloc if necessary
            if (ppc->m_cpsmSkinMaps == ppc->m_cpsmSkinMapsMax)
            {
                rgpsmTemp = ppc->m_rgpsmSkinMaps;

                ppc->m_cpsmSkinMapsMax = max(1, ppc->m_cpsmSkinMapsMax);
                ppc->m_cpsmSkinMapsMax = ppc->m_cpsmSkinMapsMax * 2;
                ppc->m_rgpsmSkinMaps = new SSkinMap*[ppc->m_cpsmSkinMapsMax];
                if (ppc->m_rgpsmSkinMaps == NULL)
                {
                    ppc->m_rgpsmSkinMaps = rgpsmTemp;
                    hr = E_OUTOFMEMORY;
                    goto e_Exit;
                }

                if (ppc->m_cpsmSkinMaps > 0)
                {
                    memcpy(ppc->m_rgpsmSkinMaps, rgpsmTemp, sizeof(SSkinMap*) * ppc->m_cpsmSkinMaps);
                }

                delete []rgpsmTemp;
            }
            ppc->m_rgpsmSkinMaps[ppc->m_cpsmSkinMaps] = psmSkinMap;
            ppc->m_cpsmSkinMaps += 1;

            // init the map
            psmSkinMap->m_pMeshNode = pNode;
            psmSkinMap->m_cbiBonesMax = 30;
            psmSkinMap->m_rgbiBones = new SBoneInfo[psmSkinMap->m_cbiBonesMax];
            if (psmSkinMap->m_rgbiBones == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto e_Exit;
            }

            // init duplication removal for redundant weights
            cpnBonesSeenMax = 30;
            cpnBonesSeen = 0;
            rgpnBonesSeen = new INode*[cpnBonesSeenMax];
            if (rgpnBonesSeen == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto e_Exit;
            }

            cVertices = pPhyContextExport->GetNumberVertices();
            for (iVertex = 0; iVertex < cVertices; iVertex++ )
            {
                pVertexExport = (IPhyVertexExport *)pPhyContextExport->GetVertexInterface(iVertex);    
                if (pVertexExport == NULL)
                {
                    OutputDebugString("Couldn't get export interface!");
                    hr = E_FAIL;
                    goto e_Exit;
                }
            
                // What kind of vertices are these?
                iVertexType = pVertexExport->GetVertexType();

                pPhyContextExport->ReleaseVertexInterface( pVertexExport );    

                if( iVertexType == RIGID_TYPE )
                {
                    pRigidVertexExport = (IPhyRigidVertex *)pPhyContextExport->GetVertexInterface(iVertex);
                    if (pRigidVertexExport == NULL)
                    {
                        OutputDebugString("Couldn't get rigid vertex export interface!");
                        hr = E_FAIL;
                        goto e_Exit;
                    }
                    // Get the vertex info!
                
                    pBoneNode = pRigidVertexExport->GetNode();

                    pbi = psmSkinMap->FindBone(pBoneNode);
                    if (pbi == NULL)
                    {
                        hr = psmSkinMap->AddBone(pBoneNode, &pbi);
                        if (FAILED(hr))
                            goto e_Exit;
                    }

                    pbi->m_cVertices += 1;

                    ppc->m_cMaxWeightsPerVertex = max(1, ppc->m_cMaxWeightsPerVertex);

                    pPhyContextExport->ReleaseVertexInterface( pRigidVertexExport);
                }
                else
                {
                    assert( iVertexType == RIGID_BLENDED_TYPE );

                    pBlendedRigidVertexExport = (IPhyBlendedRigidVertex *)pPhyContextExport->GetVertexInterface(iVertex);
                    if (pBlendedRigidVertexExport == NULL)
                    {
                        OutputDebugString("Couldn't get blended rigid vertex export interface!");
                        hr = E_FAIL;
                        goto e_Exit;
                    }

                    // How many nodes affect his vertex?
                    cTotalBones = pBlendedRigidVertexExport->GetNumberNodes();
                    cpnBonesSeen = 0;
                    for (iBone = 0; iBone < cTotalBones; iBone++ )
                    {
                        pBoneNode = pBlendedRigidVertexExport->GetNode(iBone);

                        // first see if the bone has already been seen
                        bBoneSeen = FALSE;
                        for (iBoneSeen = 0; iBoneSeen < cpnBonesSeen; iBoneSeen++)
                        {
                            if (rgpnBonesSeen[iBoneSeen] == pBoneNode)
                            {
                                bBoneSeen = TRUE;
                                break;
                            }
                        }
                        
                        // if not seen, collect stats and add to already seen
                        if (!bBoneSeen)
                        {
                            pbi = psmSkinMap->FindBone(pBoneNode);
                            if (pbi == NULL)
                            {
                                hr = psmSkinMap->AddBone(pBoneNode, &pbi);
                                if (FAILED(hr))
                                    goto e_Exit;
                            }
                            pbi->m_cVertices += 1;

                            if (cpnBonesSeen >= cpnBonesSeenMax)
                            {
                                rgpnTemp = rgpnBonesSeen;
                                cpnBonesSeenMax *= 2;

                                rgpnBonesSeen = new INode*[cpnBonesSeenMax];
                                if (rgpnBonesSeen == NULL)
                                {
                                    rgpnBonesSeen = rgpnTemp;
                                    hr = E_OUTOFMEMORY;
                                    goto e_Exit;
                                }

                                memcpy(rgpnBonesSeen, rgpnTemp, cpnBonesSeen * sizeof(INode*));
                                delete []rgpnTemp;
                            }

                            rgpnBonesSeen[cpnBonesSeen] = pBoneNode;
                            cpnBonesSeen += 1;
                        }
                    }

                    // actualNum... accounts for duplicated weights to same transform node
                    // that are combined automatically above
                    ppc->m_cMaxWeightsPerVertex = max(cpnBonesSeen, ppc->m_cMaxWeightsPerVertex);
            
                    pPhyContextExport->ReleaseVertexInterface( pBlendedRigidVertexExport);
                }
            }


		// now figure out the max number of weights per face

            pTriObject = GetTriObjectFromObjRef(pObj, &bDeleteTriObject);
            if (pTriObject == NULL) 
            {
                OutputDebugString("Physique info, but no mesh");
                hr = E_FAIL;
                goto e_Exit;
            }

            pMesh = &(pTriObject->mesh);

            for (iFace = 0; iFace < pMesh->numFaces; iFace++)
            {
				cpnBonesSeen = 0;
				for (iPoint = 0; iPoint < 3; iPoint++ )
				{
					iVertex = pMesh->faces[iFace].v[iPoint];

					pVertexExport = (IPhyVertexExport *)pPhyContextExport->GetVertexInterface(iVertex);    
					if (pVertexExport == NULL)
					{
						OutputDebugString("Couldn't get export interface!");
						hr = E_FAIL;
						goto e_Exit;
					}
            
					// What kind of vertices are these?
					iVertexType = pVertexExport->GetVertexType();

					pPhyContextExport->ReleaseVertexInterface( pVertexExport );    

					if( iVertexType == RIGID_TYPE )
					{
						pRigidVertexExport = (IPhyRigidVertex *)pPhyContextExport->GetVertexInterface(iVertex);
						if (pRigidVertexExport == NULL)
						{
							OutputDebugString("Couldn't get rigid vertex export interface!");
							hr = E_FAIL;
							goto e_Exit;
						}
						// Get the vertex info!
                
						pBoneNode = pRigidVertexExport->GetNode();

						pbi = psmSkinMap->FindBone(pBoneNode);
						if (pbi == NULL)
						{
							hr = psmSkinMap->AddBone(pBoneNode, &pbi);
							if (FAILED(hr))
								goto e_Exit;
						}

						// first see if the bone has already been seen
						bBoneSeen = FALSE;
						for (iBoneSeen = 0; iBoneSeen < cpnBonesSeen; iBoneSeen++)
						{
							if (rgpnBonesSeen[iBoneSeen] == pBoneNode)
							{
								bBoneSeen = TRUE;
								break;
							}
						}
                    
						// if not seen, collect stats and add to already seen
						if (!bBoneSeen)
						{
							if (cpnBonesSeen >= cpnBonesSeenMax)
							{
								rgpnTemp = rgpnBonesSeen;
								cpnBonesSeenMax *= 2;

								rgpnBonesSeen = new INode*[cpnBonesSeenMax];
								if (rgpnBonesSeen == NULL)
								{
									rgpnBonesSeen = rgpnTemp;
									hr = E_OUTOFMEMORY;
									goto e_Exit;
								}

								memcpy(rgpnBonesSeen, rgpnTemp, cpnBonesSeen * sizeof(INode*));
								delete []rgpnTemp;
							}

							rgpnBonesSeen[cpnBonesSeen] = pBoneNode;
							cpnBonesSeen += 1;
						}

						pPhyContextExport->ReleaseVertexInterface( pRigidVertexExport);
					}
					else
					{
						assert( iVertexType == RIGID_BLENDED_TYPE );

						pBlendedRigidVertexExport = (IPhyBlendedRigidVertex *)pPhyContextExport->GetVertexInterface(iVertex);
						if (pBlendedRigidVertexExport == NULL)
						{
							OutputDebugString("Couldn't get blended rigid vertex export interface!");
							hr = E_FAIL;
							goto e_Exit;
						}

						// How many nodes affect his vertex?
						cTotalBones = pBlendedRigidVertexExport->GetNumberNodes();
						for (iBone = 0; iBone < cTotalBones; iBone++ )
						{
							pBoneNode = pBlendedRigidVertexExport->GetNode(iBone);

							// first see if the bone has already been seen
							bBoneSeen = FALSE;
							for (iBoneSeen = 0; iBoneSeen < cpnBonesSeen; iBoneSeen++)
							{
								if (rgpnBonesSeen[iBoneSeen] == pBoneNode)
								{
									bBoneSeen = TRUE;
									break;
								}
							}
                        
							// if not seen, collect stats and add to already seen
							if (!bBoneSeen)
							{
								if (cpnBonesSeen >= cpnBonesSeenMax)
								{
									rgpnTemp = rgpnBonesSeen;
									cpnBonesSeenMax *= 2;

									rgpnBonesSeen = new INode*[cpnBonesSeenMax];
									if (rgpnBonesSeen == NULL)
									{
										rgpnBonesSeen = rgpnTemp;
										hr = E_OUTOFMEMORY;
										goto e_Exit;
									}

									memcpy(rgpnBonesSeen, rgpnTemp, cpnBonesSeen * sizeof(INode*));
									delete []rgpnTemp;
								}

								rgpnBonesSeen[cpnBonesSeen] = pBoneNode;
								cpnBonesSeen += 1;
							}
						}

						pPhyContextExport->ReleaseVertexInterface( pBlendedRigidVertexExport);
					}
				}

				ppc->m_cMaxWeightsPerFace = max(cpnBonesSeen, ppc->m_cMaxWeightsPerFace);
            }
        }

e_NoPhysique:;
    }

    cChildren = pNode->NumberOfChildren();
    for (iChild = 0; iChild < cChildren; iChild++)
    {
        // enumerate the child
        hr = Preprocess(ppc, pNode->GetChildNode(iChild));
        if (FAILED(hr))
            goto e_Exit;        
    }

e_Exit:
    if (bDeleteTriObject)
    {
        delete pTriObject;
    }

    delete []rgpnBonesSeen;
    return hr;
}


struct SAnimInfo
{
    DWORD dwTime;
    DWORD dwNumValues;
    float rgfValues[16];
};

BOOL BFloatsEqual
    (
    float f1,
    float f2
    )
{
    // first do a bitwise compare
    if ((*(DWORD*)&f1) == (*(DWORD*)&f2))
        return TRUE;

    // next do an epsilon compare
    float fDiff = (f1 - f2);
    return (fDiff <= 1e-6f) && (fDiff >= -1e-6f);
}

BOOL BEqualMatrices(float *rgfMat1, float *rgfMat2)
{
    DWORD iFloat;

    for (iFloat = 0; iFloat < 16; iFloat++)
    {
        if (!BFloatsEqual(rgfMat1[iFloat], rgfMat2[iFloat]))
            return FALSE;
    }

    return TRUE;
}

HRESULT GenerateAnimationSet
    (
    SSaveContext *psc
    )
{
    HRESULT hr = S_OK;
    DWORD cKeys;
    PBYTE pbData = NULL;
    PBYTE pbCur;
    DWORD cTicksPerFrame;
    DWORD iCurTime;
    DWORD iCurKey;
    Matrix3 matFirstTM;
    Matrix3 matTM;
    Matrix3 matRelativeTM;
    Matrix3 matParentTM;
	Interval interval;
    DWORD iNode;
    INode *pNode;
    DWORD cbSize;
    DWORD iKey;
    DWORD cCurrentKeys;
    SAnimInfo *rgaiAnimData = NULL;
    SAnimInfo *rgaiAnimDataCur;
    LPDIRECTXFILEDATA pAnimation = NULL;
    LPDIRECTXFILEDATA pAnimationKeys = NULL;
    char *szName;
    BOOL bAddEndKey = FALSE;
    DWORD iInterval;
    DWORD iStartTime;
    DWORD iEndTime;
    DWORD cFrameRate;

    // find the animation info from the interface
    interval = psc->m_pInterface->GetAnimRange();
    cTicksPerFrame = GetTicksPerFrame();
    cFrameRate = GetFrameRate();
    iStartTime = interval.Start();
    iEndTime = interval.End();

    iInterval = (cTicksPerFrame * cFrameRate) / psc->m_iAnimSamplingRate;

    cKeys = (iEndTime - iStartTime) / iInterval;

    // if the sampling frequency doesn't end directly on 
    //   on the end time, then add a partial end key
    if (((iEndTime - iStartTime) % iInterval) != 0)
    {
        bAddEndKey = TRUE;
        cKeys += 1;
    }

    // add one more key for looping
    cKeys += 1;

    rgaiAnimData = new SAnimInfo[psc->m_cNodes*cKeys];
    if (rgaiAnimData == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

	for (iCurKey = 0, iCurTime = iStartTime; iCurTime <= iEndTime; iCurTime += iInterval, iCurKey++ )
	{
        for (iNode = 0; iNode < psc->m_cNodes; iNode++)
        {
            pNode = psc->m_rgpnNodes[iNode];

		    //iCurTime = iFrame * (cTicksPerFrame / 2);
		    matTM = pNode->GetNodeTM(iCurTime);

            if (pNode->GetParentNode() == NULL)
            {
                matRelativeTM = matTM;
            }
            else
            {
                matParentTM = pNode->GetParentNode()->GetNodeTM(iCurTime);
                if( memcmp(&matTM,&matParentTM,12*sizeof(float)) == 0 )
                {
                    matRelativeTM.IdentityMatrix();
                }
                else
                {
                    matRelativeTM = matTM * Inverse(matParentTM);
                }
            }

            // set the current pointer to the correct buffer position
            pbCur = (PBYTE)&rgaiAnimData[iNode*cKeys + iCurKey];

            // first write the time and dword count            
            WRITE_DWORD(pbCur, iCurTime);
            WRITE_DWORD(pbCur, 16);

            // next write the matrix
            WRITE_MATRIX4_FROM_MATRIX3(pbCur, matRelativeTM);

        }
    }

    // if the sampling rate doesn't evenly end on the last frame, add a partial key frame
    if (bAddEndKey)
    {
        assert(((iEndTime - iStartTime) % iInterval) != 0);

        // just add the end time as a key frame
        for (iNode = 0; iNode < psc->m_cNodes; iNode++)
        {
            // set the current pointer to the correct buffer position
            pbCur = (PBYTE)&rgaiAnimData[iCurKey];

		    matTM = pNode->GetNodeTM(iEndTime);
            if (pNode->GetParentNode() == NULL)
            {
                matRelativeTM = matTM;
            }
            else
            {
                matParentTM = pNode->GetParentNode()->GetNodeTM(iEndTime);
                if( memcmp(&matTM,&matParentTM,12*sizeof(float)) == 0 )
                {
                    matRelativeTM.IdentityMatrix();
                }
                else
                {
                    matRelativeTM = matTM * Inverse(matParentTM);
                }
            }

            WRITE_DWORD(pbCur, iEndTime);
            WRITE_DWORD(pbCur, 16);

            // next write the matrix
            WRITE_MATRIX4_FROM_MATRIX3(pbCur, matRelativeTM);
        }
    }

    // loop the anim
    for (iNode = 0; iNode < psc->m_cNodes; iNode++)
    {
        // set the current pointer to the correct buffer position
        pbCur = (PBYTE)&rgaiAnimData[iNode*cKeys + (cKeys-1)];

        WRITE_DWORD(pbCur, iEndTime + iInterval);
        WRITE_DWORD(pbCur, 16);

        // next write the matrix
        memcpy(pbCur, rgaiAnimData[iNode*cKeys].rgfValues, sizeof(float)*16);
    }

    // allocate memory to send to D3DXOF lib
    cbSize = sizeof(DWORD) + sizeof(DWORD) +
            (sizeof(DWORD) //time
                + sizeof(DWORD) //nValues
                + sizeof(FLOAT)*16) // x, y, z
               * cKeys; // number of keys

    pbData = new BYTE[cbSize];
    if (pbData == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMAnimationSet,
                                    NULL,
                                    NULL,
                                    0,
                                    NULL,
                                    &psc->m_pAnimationSet
                                    );
    if (FAILED(hr))
        goto e_Exit;

	for (iNode = 0; iNode < psc->m_cNodes; iNode++)
	{
		pbCur = pbData;

        szName = psc->m_stStrings.CreateNiceString(psc->m_rgpnNodes[iNode]->GetName());
        if (szName == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto e_Exit;
        }

		// write the key type
		WRITE_DWORD(pbCur, 4);

		// write the number of keys
		WRITE_DWORD(pbCur, cKeys);

        rgaiAnimDataCur = &rgaiAnimData[iNode*cKeys];
        cCurrentKeys = 0;
        for (iKey = 0; iKey < cKeys; iKey++)
        {
		    memcpy(pbCur, &rgaiAnimDataCur[iKey], sizeof(SAnimInfo));
            pbCur += sizeof(SAnimInfo);
            cCurrentKeys += 1;

            // if not last key, then check for start of a run of identical keys
            if (iKey < (cKeys-1))
            {
                // if equal to next, check for a run of equal matrices
                if (BEqualMatrices(rgaiAnimDataCur[iKey].rgfValues, rgaiAnimDataCur[iKey+1].rgfValues))
                {
                    // move to the next key, and skip all equal matrices
                    iKey += 1;
                    while ((iKey < (cKeys-1)) && BEqualMatrices(rgaiAnimDataCur[iKey].rgfValues, rgaiAnimDataCur[iKey+1].rgfValues))
                    {
                        iKey += 1;
                    }

		            memcpy(pbCur, &rgaiAnimDataCur[iKey], sizeof(SAnimInfo));
                    pbCur += sizeof(SAnimInfo);
                    cCurrentKeys += 1;
                }
            }
        }

        // update to current count of keys
        ((DWORD*)pbData)[1] = cCurrentKeys;

		hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMAnimation,
										NULL,
										NULL,
										0,
										NULL,
										&pAnimation
										);

	// add the data to the file
		hr = psc->m_pxofsave->CreateDataObject(TID_D3DRMAnimationKey,
										NULL,
										NULL,
										cbSize,
										pbData,
										&pAnimationKeys
										);
		if (FAILED(hr))
			goto e_Exit;

		// add to the animation set
		hr = pAnimation->AddDataObject(pAnimationKeys);
		if (FAILED(hr))
			goto e_Exit;

		hr = pAnimation->AddDataReference(szName, NULL);
		if (FAILED(hr))
			goto e_Exit;

		hr = psc->m_pAnimationSet->AddDataObject(pAnimation);
		if (FAILED(hr))
			goto e_Exit;

        RELEASE(pAnimation);
        RELEASE(pAnimationKeys);
	}

e_Exit:
    delete []rgaiAnimData;
	delete []pbData;
    RELEASE(pAnimation);
    RELEASE(pAnimationKeys);

    return hr;
}


// ================================================== CDataSave::CDataSave()
HRESULT ExportXFile
    (
    const TCHAR *szFilename,
//    ExpInterface *pExportInterface,
    Interface *pInterface, 
    BOOL bSuppressPrompts,
    BOOL bSaveSelection,
    HWND hwndParent,
    INode *pRootNode
    ) 
{
    LPDIRECTXFILE pxofapi = NULL;
    LPDIRECTXFILEDATA pRootData = NULL;
    LPDIRECTXFILESAVEOBJECT pxofsave = NULL; 
    HRESULT hr = S_OK;
//    INode *pRootNode;
    SSaveContext sc;
    SPreprocessContext pc;
    SDialogOptions DlgOptions;

    assert(szFilename && pInterface);

    // Extract scene information
    pInterface->ProgressStart(_T("Extracting skinning data"),TRUE,dummyFn,NULL);
    pInterface->ProgressUpdate(0);
    //pInterface->ProgressUpdate(100);

    // first find the root node
    // dumbest. code. ever.
//    hr = FindRootNode(pExportInterface->theScene, &pRootNode);
//    if (FAILED(hr))
//        goto e_Exit;

    // setup options for Preprocess stage
    pc.m_bSaveSelection = bSaveSelection;

    // figure out bone counts, etc.
    hr = Preprocess(&pc, pRootNode);
    if (FAILED(hr))
        goto e_Exit;

    // move the skin map data from the preprocess context to the save context
    sc.m_cpsmSkinMaps = pc.m_cpsmSkinMaps;
    sc.m_rgpsmSkinMaps = pc.m_rgpsmSkinMaps;
    pc.m_rgpsmSkinMaps = NULL;

    pInterface->ProgressUpdate(25);

    // setup dialog options
    DlgOptions.m_xFormat = DXFILEFORMAT_BINARY;
    DlgOptions.m_bSavePatchData = FALSE;
    DlgOptions.m_bSaveAnimationData = TRUE;
    DlgOptions.m_iAnimSamplingRate = 30;
    DlgOptions.m_cMaxBonesPerVertex = pc.m_cMaxWeightsPerVertex;
    DlgOptions.m_cMaxBonesPerFace = pc.m_cMaxWeightsPerFace;

    // if prompts not suppressed, then check with the user on options
    if (!bSuppressPrompts)
    {
        DialogBoxParam(ghInstance, 
                       MAKEINTRESOURCE(IDD_PANEL), 
                        hwndParent, 
                        XSkinExpOptionsDlgProc, 
                        (LPARAM)&DlgOptions);

        if (!DlgOptions.m_bProceedWithExport)
            goto e_Exit;
    }

    pInterface->ProgressStart(_T("Exporting data"),TRUE,dummyFn,NULL);
    pInterface->ProgressUpdate(25);

    // Create xofapi object.
    hr = DirectXFileCreate(&pxofapi);
    if (FAILED(hr))
        goto e_Exit;

    // Register templates for d3drm.

   // had to put this into a single centralized helper fn 
   // cause whomever came up with this scheme was crezzy

    if (!registerTemplates(pxofapi))
       goto e_Exit;

//    hr = pxofapi->RegisterTemplates((LPVOID)D3DRM_XTEMPLATES,
//                                    D3DRM_XTEMPLATE_BYTES);
//    if (FAILED(hr))
//        goto e_Exit;

    hr = pxofapi->RegisterTemplates((LPVOID)XSKINEXP_TEMPLATES,
                                    strlen(XSKINEXP_TEMPLATES));
    if (FAILED(hr))
        goto e_Exit;

    // Create save object.
    hr = pxofapi->CreateSaveObject(szFilename,    // filename
                                   DlgOptions.m_xFormat,  // binary or text
                                   &pxofsave);
    if (FAILED(hr))
        goto e_Exit;

    hr = pxofsave->SaveTemplates(3, aIds);
    if (FAILED(hr))
        goto e_Exit;

    sc.m_pxofsave = pxofsave;
    sc.m_xFormat = DlgOptions.m_xFormat;
    sc.m_bSaveAnimationData = DlgOptions.m_bSaveAnimationData;
    sc.m_iAnimSamplingRate = DlgOptions.m_iAnimSamplingRate;
	sc.m_bSavePatchData = DlgOptions.m_bSavePatchData;
    sc.m_iTime = pInterface->GetTime();
    sc.m_pInterface = pInterface;
    sc.m_bSaveSelection = bSaveSelection;
    sc.m_cMaxWeightsPerVertex = pc.m_cMaxWeightsPerVertex;
    sc.m_cMaxWeightsPerFace = pc.m_cMaxWeightsPerFace;

    sc.m_cNodes = pc.m_cNodes;
    sc.m_cNodesCur = 0;
    sc.m_rgpnNodes = new INode*[sc.m_cNodes];
    if (sc.m_rgpnNodes == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    // then write the whole tree out into file data's
    hr = EnumTree(&sc, pRootNode, &pRootData);
    if (FAILED(hr))
        goto e_Exit;

    pInterface->ProgressStart(_T("Saving data to file"),TRUE,dummyFn,NULL);
    pInterface->ProgressUpdate(50);

    // now save that file data to the file
    hr = pxofsave->SaveData(pRootData);
    if (FAILED(hr))
        goto e_Exit;

    pInterface->ProgressUpdate(75);

    if (DlgOptions.m_bSaveAnimationData)
    {
        pInterface->ProgressStart(_T("Saving animation data to file"),TRUE,dummyFn,NULL);
        pInterface->ProgressUpdate(75);

        hr = GenerateAnimationSet(&sc);
        if (FAILED(hr))
            goto e_Exit;

        hr = pxofsave->SaveData(sc.m_pAnimationSet);
        if (FAILED(hr))
        {
            OutputDebugString("Failed to add animation set to X File\n");
            goto e_Exit;
        }            
    }

e_Exit:
    if (FAILED(hr))
    {
        OutputDebugString("File was not successfully exported.");
        {
            TCHAR errstr[500 + _MAX_PATH];
            _stprintf(errstr,"Could not write to file: %s.\n"\
                "Try checking the file's permissions, or if it is currently open.",szFilename);
            MessageBox(hwndParent,errstr,_T("Error!"),MB_OK);
        }
    }

    // falling through
    // Free up outstanding interfaces
    RELEASE(pxofsave);
    RELEASE(pRootData);
    RELEASE(pxofapi);

    pInterface->ProgressUpdate(100);
    pInterface->ProgressEnd();

    return hr;
}

