#include "EditorUtils.h"

_NAMESPACE_BEGIN

BOOL CMFCUtils::LoadTrueColorImageList( CImageList &imageList,UINT nIDResource,
									   int nIconWidth,COLORREF colMaskColor )
{
	CBitmap bitmap;
	BITMAP bmBitmap;
	if (!bitmap.Attach(LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(nIDResource),IMAGE_BITMAP, 0, 0,LR_DEFAULTSIZE|LR_CREATEDIBSECTION)))
		return FALSE;
	if (!bitmap.GetBitmap(&bmBitmap))
		return FALSE;
	CSize		cSize(bmBitmap.bmWidth, bmBitmap.bmHeight); 
	RGBTRIPLE*	rgb		= (RGBTRIPLE*)(bmBitmap.bmBits);
	int	nCount	= cSize.cx/nIconWidth;
	if (!imageList)
	{
		if (!imageList.Create(nIconWidth, cSize.cy, ILC_COLOR32/*ILC_COLOR24*/|ILC_MASK, nCount, 0))
			return FALSE;
	}

	if (imageList.Add(&bitmap,colMaskColor) == -1)
		return FALSE;
	return TRUE;
}

void CMFCUtils::TransparentBlt( HDC hdcDest, int nXDest, int nYDest, int nWidth,int nHeight,
							   HBITMAP hBitmap, int nXSrc, int nYSrc, COLORREF colorTransparent )
{
	CDC dc, memDC, maskDC, tempDC;
	dc.Attach( hdcDest );
	maskDC.CreateCompatibleDC(&dc);
	CBitmap maskBitmap;

	CBitmap* pOldMemBmp = NULL;
	CBitmap* pOldMaskBmp = NULL;
	HBITMAP hOldTempBmp = NULL;

	memDC.CreateCompatibleDC(&dc);
	tempDC.CreateCompatibleDC(&dc);
	CBitmap bmpImage;
	bmpImage.CreateCompatibleBitmap( &dc, nWidth, nHeight );
	pOldMemBmp = memDC.SelectObject( &bmpImage );

	hOldTempBmp = (HBITMAP) ::SelectObject( tempDC.m_hDC, hBitmap );

	memDC.BitBlt( 0,0,nWidth, nHeight, &tempDC, nXSrc, nYSrc, SRCCOPY );

	maskBitmap.CreateBitmap( nWidth, nHeight, 1, 1, NULL );
	pOldMaskBmp = maskDC.SelectObject( &maskBitmap );
	memDC.SetBkColor( colorTransparent );

	maskDC.BitBlt( 0, 0, nWidth, nHeight, &memDC,
		0, 0, SRCCOPY );

	memDC.SetBkColor(RGB(0,0,0));
	memDC.SetTextColor(RGB(255,255,255));
	memDC.BitBlt(0, 0, nWidth, nHeight, &maskDC, 0, 0, SRCAND);

	dc.SetBkColor(RGB(255,255,255));
	dc.SetTextColor(RGB(0,0,0));
	dc.BitBlt(nXDest, nYDest, nWidth, nHeight, &maskDC, 0, 0, SRCAND);

	dc.BitBlt(nXDest, nYDest, nWidth, nHeight, &memDC,
		0, 0, SRCPAINT);


	if (hOldTempBmp)
	{
		::SelectObject( tempDC.m_hDC, hOldTempBmp);
	}
	if (pOldMaskBmp)
	{
		maskDC.SelectObject( pOldMaskBmp );
	}
	if (pOldMemBmp)
	{
		memDC.SelectObject( pOldMemBmp );
	}

	dc.Detach();
}


void HeapCheck::Check( const char *file,int line )
{
#ifdef _DEBUG

	_ASSERTE(_CrtCheckMemory());
	
   int heapstatus = _heapchk();
   switch( heapstatus )
   {
   case _HEAPOK:
      break;
   case _HEAPEMPTY:
      break;
   case _HEAPBADBEGIN:
			{
				CString str;
				str.Format( "Bad Start of Heap, at file %s line:%d",file,line );
				MessageBox( NULL,str,"Heap Check",MB_OK );
			}
      break;
   case _HEAPBADNODE:
			{
				CString str;
				str.Format( "Bad Node in Heap, at file %s line:%d",file,line );
				MessageBox( NULL,str,"Heap Check",MB_OK );
			}
      break;
   }
	 
#endif
}

_NAMESPACE_END