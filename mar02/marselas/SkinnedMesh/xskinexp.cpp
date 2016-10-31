//-----------------------------------------------------------------------------
// File: XSkinExp.cpp
//
// Desc: Export interface for max
//
// Copyright (C) 1998-2000 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------

#include "pch.h"
#include "pch.h"
#include "XSkinExp.h"

HRESULT ExportXFile
    (
    const TCHAR *szFilename,
    Interface *pInterface, 
    BOOL bSuppressPrompts,
    BOOL bSaveSelection,
    HWND hwndParent,
    INode *pRootNode
    );


static XSkinExpClassDesc XSkinExpDesc;
ClassDesc* GetXSkinExpDesc() {return &XSkinExpDesc;}

void XSkinExpClassDesc::ResetClassParams (BOOL fileReset) 
{
    // nothing to reset when Max is reset.
}

//--- XSkinExp -------------------------------------------------------
XSkinExp::XSkinExp()
{
}

XSkinExp::~XSkinExp() 
{

}

int XSkinExp::ExtCount()
{
    //TODO: Returns the number of file name extensions supported by the plug-in.
    return 1;
}

const TCHAR *XSkinExp::Ext(int n)
{        
    //TODO: Return the 'i-th' file name extension (i.e. "3DS").
    return _T("x");
}

const TCHAR *XSkinExp::LongDesc()
{
    //TODO: Return long ASCII description (i.e. "Targa 2.0 Image File")
    return _T("DirectX X-File");
}
    
const TCHAR *XSkinExp::ShortDesc() 
{            
    //TODO: Return short ASCII description (i.e. "Targa")
    return _T("X-File");
}

const TCHAR *XSkinExp::AuthorName()
{            
    //TODO: Return ASCII Author name
    return _T("Craig Peeper");
}

const TCHAR *XSkinExp::CopyrightMessage() 
{    
    // Return ASCII Copyright message
    return _T("Copyright 2000, Microsoft Corporation");
}

const TCHAR *XSkinExp::OtherMessage1() 
{        
    //TODO: Return Other message #1 if any
    return _T("");
}

const TCHAR *XSkinExp::OtherMessage2() 
{        
    //TODO: Return other message #2 in any
    return _T("");
}

unsigned int XSkinExp::Version()
{                
    //TODO: Return Version number * 100 (i.e. v3.01 = 301)
    return 100;
}

void XSkinExp::ShowAbout(HWND hWnd)
{            
    // Optional
}

BOOL XSkinExp::SupportsOptions(int ext, DWORD options)
{
    return ( options == SCENE_EXPORT_SELECTED );
}

// -------------------------------------------------------------------------------
//   class      CFindRootNode
//
//   devnote    Helper class for FindRootNode, used to implement callback function
//                  for scene traversal.  Finds the root node by aborting the search
//                  after the first node is returned.  It takes the first node and
//                  walks to the root from there.
//
class CFindRootNode : public ITreeEnumProc
{
public:
    CFindRootNode()
        :m_pNodeRoot(NULL) {}

    virtual int callback(INode *pNode)
    {
        INode *pNodeCur = pNode;

        while (!pNodeCur->IsRootNode())
        {
            pNodeCur = pNodeCur->GetParentNode();
        }
        m_pNodeRoot = pNodeCur;

        return TREE_ABORT;
    }

    INode *m_pNodeRoot;
};

// -------------------------------------------------------------------------------
//  function    FindRootNode
//
//   devnote    Finds the root node of the scene by aborting a tree walk after the first node
//
//   returns    S_OK if a node is found, E_FAIL if not
//
HRESULT FindRootNode
    (
    IScene *pScene, 
    INode **ppNode
    )
{
    CFindRootNode RootNode;

    // grab the first node of the scene graph
    pScene->EnumTree(&RootNode);

    *ppNode = RootNode.m_pNodeRoot;

    return RootNode.m_pNodeRoot != NULL ? S_OK : E_FAIL;
}


int XSkinExp::DoExport(const TCHAR *szFilename,ExpInterface *ei,
                        Interface *i, BOOL suppressPrompts, DWORD options) 
{
   // the ExpInterface is useless
   ei;

    HRESULT hr;
    BOOL bSaveSelection = (options & SCENE_EXPORT_SELECTED);

    // actually export the file
    hr = ExportXFile(szFilename, i, suppressPrompts, bSaveSelection, GetActiveWindow(), i->GetRootNode());

    if (FAILED(hr))
        return FALSE;
    else
        return TRUE;
}
    

