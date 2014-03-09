#include <Windows.h>
#include <mmsystem.h>
#include <d3dx9.h>
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 )

#define MESH_COUNT 2


//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
LPDIRECT3D9         g_pD3D = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE9   g_pd3dDevice = NULL; // Our rendering device
LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL; // Buffer to hold vertices
LPDIRECT3DTEXTURE9      g_pTexture[2] = { NULL, NULL }; // Our texture

LPD3DXMESH          g_pMesh[MESH_COUNT] = { NULL, NULL }; // Our mesh object in sysmem
D3DMATERIAL9*       g_pMeshMaterials[MESH_COUNT] = { NULL, NULL }; // Materials for our mesh
LPDIRECT3DTEXTURE9* g_pMeshTextures[MESH_COUNT] = { NULL, NULL }; // Textures for our mesh
DWORD               g_dwNumMaterials[MESH_COUNT] = { 0L, 0L };   // Number of mesh materials


struct CUSTOMVERTEX
{
	D3DXVECTOR3 position; // The position
	D3DXVECTOR3 normal;   // The surface normal for the vertex
	D3DCOLOR color;    // The color
#ifndef SHOW_HOW_TO_USE_TCI
	FLOAT tu, tv;   // The texture coordinates
#endif
};

// Our custom FVF, which describes our custom vertex structure
#ifdef SHOW_HOW_TO_USE_TCI
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_NORMAL)
#else
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1|D3DFVF_NORMAL)
#endif

//-----------------------------------------------------------------------------
// Name: InitD3D()
// Desc: Initializes Direct3D
//-----------------------------------------------------------------------------
HRESULT InitD3D( HWND hWnd )
{
    // Create the D3D object.
    if( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
        return E_FAIL;

    // Set up the structure used to create the D3DDevice. Since we are now
    // using more complex geometry, we will create a device with a zbuffer.
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof( d3dpp ) );
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

    // Create the D3DDevice
    if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
                                      D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                      &d3dpp, &g_pd3dDevice ) ) )
    {
        return E_FAIL;
    }

	// Turn off culling
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    // Turn on the zbuffer
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitGeometry()
// Desc: Load the mesh and build the material and texture arrays
//-----------------------------------------------------------------------------
HRESULT InitGeometry()
{
	// Use D3DX to create a texture from a file based image
	if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice, L"env2.bmp", &g_pTexture[0])))
	{
		MessageBox(NULL, L"Could not find banana.bmp", L"Final.exe", MB_OK);
		return E_FAIL;
	}
	if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice, L"banana.bmp", &g_pTexture[1])))
	{
		MessageBox(NULL, L"Could not find env2.bmp", L"Final.exe", MB_OK);
		return E_FAIL;
	}

	// Create the vertex buffer.
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(50 * 2 * sizeof(CUSTOMVERTEX),
		0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_DEFAULT, &g_pVB, NULL)))
	{
		return E_FAIL;
	}

	// Fill the vertex buffer. We are setting the tu and tv texture
	// coordinates, which range from 0.0 to 1.0
	CUSTOMVERTEX* pVertices;
	if (FAILED(g_pVB->Lock(0, 0, (void**)&pVertices, 0)))
		return E_FAIL;
	for (DWORD i = 0; i < 50; i++)
	{
		FLOAT theta = (2 * D3DX_PI * i) / (50 - 1);

		pVertices[2 * i + 0].position = D3DXVECTOR3(sinf(theta), -1.0f, cosf(theta));
		pVertices[2 * i + 0].normal = D3DXVECTOR3(sinf(theta), 0.0f, cosf(theta));
		pVertices[2 * i + 0].color = 0xffffffff;
#ifndef SHOW_HOW_TO_USE_TCI
		pVertices[2 * i + 0].tu = ((FLOAT)i) / (50 - 1);
		pVertices[2 * i + 0].tv = 1.0f;
#endif

		pVertices[2 * i + 1].position = D3DXVECTOR3(sinf(theta), 1.0f, cosf(theta));
		pVertices[2 * i + 1].normal = D3DXVECTOR3(sinf(theta), 0.0f, cosf(theta));
		pVertices[2 * i + 1].color = 0xff808080;
#ifndef SHOW_HOW_TO_USE_TCI
		pVertices[2 * i + 1].tu = ((FLOAT)i) / (50 - 1);
		pVertices[2 * i + 1].tv = 0.0f;
#endif
	}
	g_pVB->Unlock();

    LPD3DXBUFFER pD3DXMtrlBuffer[2];

	// Load the mesh from the specified file
	if (FAILED(D3DXLoadMeshFromX(L"Tiger0.x", D3DXMESH_SYSTEMMEM,
		g_pd3dDevice, NULL,
		&pD3DXMtrlBuffer[0], NULL, &g_dwNumMaterials[0],
		&g_pMesh[0])))
	{
		MessageBox(NULL, L"Could not find tiger0.x", L"Final.exe", MB_OK);
		return E_FAIL;
	}

	// Load the mesh from the specified file
	if (FAILED(D3DXLoadMeshFromX(L"Tiger1.x", D3DXMESH_SYSTEMMEM,
		g_pd3dDevice, NULL,
		&pD3DXMtrlBuffer[1], NULL, &g_dwNumMaterials[1],
		&g_pMesh[1])))
	{
		MessageBox(NULL, L"Could not find tiger1.x", L"Final.exe", MB_OK);
		return E_FAIL;
	}


	// We need to extract the material properties and texture names from the 
	// pD3DXMtrlBuffer
	for (int loop = 0; loop < MESH_COUNT; ++loop)
	{
		D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer[loop]->GetBufferPointer();

		g_pMeshMaterials[loop] = new D3DMATERIAL9[g_dwNumMaterials[loop]];
		if (g_pMeshMaterials[loop] == NULL)
			return E_OUTOFMEMORY;
		g_pMeshTextures[loop] = new LPDIRECT3DTEXTURE9[g_dwNumMaterials[loop]];
		if (g_pMeshTextures[loop] == NULL)
			return E_OUTOFMEMORY;

		for (DWORD i = 0; i < g_dwNumMaterials[loop]; i++)
		{
			// Copy the material
			g_pMeshMaterials[loop][i] = d3dxMaterials[i].MatD3D;

			// Set the ambient color for the material (D3DX does not do this)
			g_pMeshMaterials[loop][i].Ambient = g_pMeshMaterials[loop][i].Diffuse;

			g_pMeshTextures[loop][i] = NULL;
			if (d3dxMaterials[i].pTextureFilename != NULL &&
				lstrlenA(d3dxMaterials[i].pTextureFilename) > 0)
			{
				// Create the texture
				if (FAILED(D3DXCreateTextureFromFileA(g_pd3dDevice,
					d3dxMaterials[i].pTextureFilename,
					&g_pMeshTextures[loop][i])))
				{
						MessageBox(NULL, L"Could not find texture map", L"Final.exe", MB_OK);
				}
			}
		}
		pD3DXMtrlBuffer[loop]->Release();
	}
    
    

    // Done with the material buffer

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Cleanup()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
VOID Cleanup()
{
	if (g_pTexture[0] != NULL)
		g_pTexture[0]->Release();

	if (g_pTexture[1] != NULL)
		g_pTexture[1]->Release();

	if (g_pVB != NULL)
		g_pVB->Release();

	if (g_pMeshMaterials != NULL)
	{
		for (int loop = 0; loop < MESH_COUNT; ++loop)
			delete[] g_pMeshMaterials[loop];
	}
		
    if( g_pMeshTextures )
    {
		for (int loop = 0; loop < MESH_COUNT; ++loop)
		{
			for (DWORD i = 0; i < g_dwNumMaterials[loop]; i++)
			{
				if (g_pMeshTextures[loop][i])
					g_pMeshTextures[loop][i]->Release();
			}
		}
    }
	if (g_pMesh[0] != NULL)
		g_pMesh[0]->Release();

	if (g_pMesh[1] != NULL)
		g_pMesh[1]->Release();

    if( g_pd3dDevice != NULL )
        g_pd3dDevice->Release();

    if( g_pD3D != NULL )
        g_pD3D->Release();
}



//-----------------------------------------------------------------------------
// Name: SetupMatrices()
// Desc: Sets up the world, view, and projection transform matrices.
//-----------------------------------------------------------------------------
VOID SetupCameraMatrices()
{
	D3DXVECTOR3 vEyePt(0.0f, 3.0f, -5.0f);
	D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
	g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

	D3DXMATRIXA16 matProj;
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f);
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);
}
VOID SetupMeshesMatrices(int ObjectNum)
{
    // Set up world matrix
    D3DXMATRIXA16 matWorld;
    D3DXMatrixRotationY( &matWorld, (timeGetTime() / 1000.0f)*(ObjectNum+1) );
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
	g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0xffffffff);	
}
VOID SetupCylinderMatrices()
{
	// Set up world matrix
	D3DXMATRIXA16 matWorld;
	D3DXMatrixIdentity(&matWorld);
	D3DXMatrixRotationX(&matWorld, timeGetTime() / 1000.0f);
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
}
VOID SetupLights(int lightNum)
{
	D3DMATERIAL9 mtrl;
	ZeroMemory(&mtrl, sizeof(D3DMATERIAL9));
	mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
	mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
	mtrl.Diffuse.b = mtrl.Ambient.b = 0.0f;
	mtrl.Diffuse.a = mtrl.Ambient.a = 0.0f;
 	g_pd3dDevice->SetMaterial(&mtrl);

	D3DXVECTOR3 vecDir;
	D3DLIGHT9 light0, light1;
	ZeroMemory(&light0, sizeof(D3DLIGHT9));
	ZeroMemory(&light1, sizeof(D3DLIGHT9));

	D3DLIGHT9 *light;
	if (lightNum == 0)
		light = &light0;
	else if (lightNum == 1)
		light = &light1;

	light->Type = D3DLIGHT_DIRECTIONAL;
	light->Diffuse.r = 1.0f;
	light->Diffuse.g = 1.0f;
	light->Diffuse.b = 1.0f;
	vecDir = D3DXVECTOR3(10.f * pow(-1.f,lightNum), 1.0f, 0);
	D3DXVec3Normalize((D3DXVECTOR3*)&light->Direction, &vecDir);
	light->Range = 1000.f;
	g_pd3dDevice->SetLight(lightNum, light);
	g_pd3dDevice->LightEnable(lightNum, TRUE);
	g_pd3dDevice->LightEnable(1-lightNum, FALSE);
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);

	// Finally, turn on some ambient light.
	g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0x00202020);
}



//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Draws the scene
//-----------------------------------------------------------------------------
VOID Render()
{
    // Clear the backbuffer and the zbuffer
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_XRGB( 0, 0, 255 ), 1.0f, 0 );

    // Begin the scene
    if( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
    {
		SetupCameraMatrices();
		SetupLights((timeGetTime() / 1000) % 2);

		SetupCylinderMatrices();
		
		int TextureIndex = (timeGetTime() / 1000) % 2;

		g_pd3dDevice->SetTexture(0, g_pTexture[TextureIndex]);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

#ifdef SHOW_HOW_TO_USE_TCI
		D3DXMATRIXA16 mTextureTransform;
		D3DXMATRIXA16 mProj;
		D3DXMATRIXA16 mTrans;
		D3DXMATRIXA16 mScale;

		g_pd3dDevice->GetTransform(D3DTS_PROJECTION, &mProj);
		D3DXMatrixTranslation(&mTrans, 0.5f, 0.5f, 0.0f);
		D3DXMatrixScaling(&mScale, 0.5f, -0.5f, 1.0f);
		mTextureTransform = mProj * mScale * mTrans;

		g_pd3dDevice->SetTransform(D3DTS_TEXTURE0, &mTextureTransform);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT4 | D3DTTFF_PROJECTED);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
#endif

		g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
		g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
		g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2 * 50 - 2);

        // Meshes are divided into subsets, one for each material. Render them in
        // a loop
		for (int loop = 0; loop < MESH_COUNT; ++loop)
		{
			// Setup the world, view, and projection matrices
			SetupMeshesMatrices(loop);

			for (DWORD i = 0; i < g_dwNumMaterials[loop]; i++)
			{
				// Set the material and texture for this subset
				g_pd3dDevice->SetMaterial(&g_pMeshMaterials[loop][i]);
				g_pd3dDevice->SetTexture(0, g_pMeshTextures[loop][i]);

				// Draw the mesh subset
				g_pMesh[loop]->DrawSubset(0);
			}
		}
		
        
        // End the scene
        g_pd3dDevice->EndScene();
    }

    // Present the backbuffer contents to the display
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}




//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_DESTROY:
            Cleanup();
            PostQuitMessage( 0 );
            return 0;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
INT WINAPI wWinMain( HINSTANCE hInst, HINSTANCE, LPWSTR, INT )
{
    UNREFERENCED_PARAMETER( hInst );

    // Register the window class
    WNDCLASSEX wc =
    {
        sizeof( WNDCLASSEX ), CS_CLASSDC, MsgProc, 0L, 0L,
        GetModuleHandle( NULL ), NULL, NULL, NULL, NULL,
		L"Practice", NULL
    };
    RegisterClassEx( &wc );

    // Create the application's window
    HWND hWnd = CreateWindow( L"Practice", L"#1 131018 ±Ë¡ˆ»Ø",
                              WS_OVERLAPPEDWINDOW, 100, 100, 300, 300,
                              NULL, NULL, wc.hInstance, NULL );

    // Initialize Direct3D
    if( SUCCEEDED( InitD3D( hWnd ) ) )
    {
        // Create the scene geometry
        if( SUCCEEDED( InitGeometry() ) )
        {
            // Show the window
            ShowWindow( hWnd, SW_SHOWDEFAULT );
            UpdateWindow( hWnd );

            // Enter the message loop
            MSG msg;
            ZeroMemory( &msg, sizeof( msg ) );
            while( msg.message != WM_QUIT )
            {
                if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
                {
                    TranslateMessage( &msg );
                    DispatchMessage( &msg );
                }
                else
                    Render();
            }
        }
    }

    UnregisterClass( L"Practice", wc.hInstance );
    return 0;
}



