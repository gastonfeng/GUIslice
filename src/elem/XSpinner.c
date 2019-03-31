// =======================================================================
// GUIslice library (extensions)
// - Calvin Hass
// - https://www.impulseadventure.com/elec/guislice-gui.html
// - https://github.com/ImpulseAdventure/GUIslice
// =======================================================================
//
// The MIT License
//
// Copyright 2016-2019 Calvin Hass
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// =======================================================================
/// \file XSpinner.c



// GUIslice library
#include "GUIslice.h"
#include "GUIslice_drv.h"

#include "elem/XSpinner.h"

#include <stdio.h>

#if (GSLC_USE_PROGMEM)
    #include <avr/pgmspace.h>
#endif

// ----------------------------------------------------------------------------
// Error Messages
// ----------------------------------------------------------------------------

extern const char GSLC_PMEM ERRSTR_NULL[];
extern const char GSLC_PMEM ERRSTR_PXD_NULL[];


// ----------------------------------------------------------------------------
// Extended element definitions
// ----------------------------------------------------------------------------
//
// - This file extends the core GUIslice functionality with
//   additional widget types
//
// ----------------------------------------------------------------------------


#if (GSLC_FEATURE_COMPOUND)
// ============================================================================
// Extended Element: Spinner
// - Spinner element demonstrates a simple up/down counter
// - This is a compound element containing two buttons and
//   a text area to represent the current value
// ============================================================================

// Private sub Element ID definitions
static const int16_t  SPINNER_ID_BTN_INC = 100;
static const int16_t  SPINNER_ID_BTN_DEC = 101;
static const int16_t  SPINNER_ID_TXT     = 102;

// Create a compound element
// - For now just two buttons and a text area
gslc_tsElemRef* gslc_ElemXSpinnerCreate(gslc_tsGui* pGui, int16_t nElemId, int16_t nPage, gslc_tsXSpinner* pXData,
  int16_t nX0, int16_t nY0, int16_t nMin, int16_t nMax, int16_t nVal, int16_t nIncr,
  int8_t nFontId, int8_t nButtonSz, GSLC_CB_INPUT cbInput)
{

  if ((pGui == NULL) || (pXData == NULL)) {
    static const char GSLC_PMEM FUNCSTR[] = "ElemXSpinnerCreate";
    GSLC_DEBUG_PRINT_CONST(ERRSTR_NULL, FUNCSTR);
    return NULL;
  }

  // determine size of our text box
  // first calculate number of digits required
  int nDigits, nTxtBoxW;
  char acTxtNum[SPINNER_STR_LEN];
  // FIXME: Consider replacing the sprintf() with an optimized function to
  //        conserve RAM. Potentially leverage GSLC_DEBUG_PRINT().
  snprintf(acTxtNum, SPINNER_STR_LEN, "%d", nMax);
  nDigits = strlen(acTxtNum);
  // Determine the maximum width of a digit, we will use button size for height.
  int16_t       nChOffsetX, nChOffsetY;
  uint16_t      nChSzW, nChSzH;
  char          acMono[2] = "%";
  gslc_tsFont*  pTxtFont = gslc_FontGet(pGui, nFontId);
  gslc_teTxtFlags eTxtFlags = (GSLC_TXT_DEFAULT & ~GSLC_TXT_ALLOC) | GSLC_TXT_ALLOC_EXT;
  gslc_DrvGetTxtSize(pGui, pTxtFont, (char*)&acMono, eTxtFlags, &nChOffsetX, &nChOffsetY, &nChSzW, &nChSzH);
  nTxtBoxW = nChSzW * nDigits + 10;  // add padding
  // now we can work out our rectangle  
  gslc_tsRect rElem;
  rElem.x = nX0;
  rElem.y = nY0;
  rElem.w = nTxtBoxW + (nButtonSz * 2);
  rElem.h = nButtonSz;

  // set our intial value for our text field
  snprintf(acTxtNum, SPINNER_STR_LEN - 1, "%d", nVal);

  gslc_tsElem sElem;

  // Initialize composite element
  sElem = gslc_ElemCreate(pGui, nElemId, nPage, GSLC_TYPEX_SPINNER, rElem, NULL, 0, GSLC_FONT_NONE);
  sElem.nFeatures |= GSLC_ELEM_FEA_FRAME_EN;
  sElem.nFeatures |= GSLC_ELEM_FEA_FILL_EN;
  sElem.nFeatures |= GSLC_ELEM_FEA_CLICK_EN;
  sElem.nFeatures &= ~GSLC_ELEM_FEA_GLOW_EN;  // Don't need to glow outer element
  sElem.nGroup = GSLC_GROUP_ID_NONE;

  pXData->nCounter = nVal;
  pXData->nMin = nMin;
  pXData->nMax = nMax;
  pXData->nIncr = nIncr;
  pXData->pfuncXInput = cbInput;

  // Determine the maximum number of elements that we can store
  // in the sub-element array. We do this at run-time with sizeof()
  // instead of using #define to avoid polluting the global namespace.
  int16_t nSubElemMax = sizeof(pXData->asElem) / sizeof(pXData->asElem[0]);

  // NOTE: The count parameters in CollectReset() must match the size of
  //       the asElem[] array. It is used for bounds checking when we
  //       add new elements.
  // NOTE: We only use RAM for subelement storage
  gslc_CollectReset(&pXData->sCollect, pXData->asElem, nSubElemMax, pXData->asElemRef, nSubElemMax);


  sElem.pXData = (void*)(pXData);
  // Specify the custom drawing callback
  sElem.pfuncXDraw = &gslc_ElemXSpinnerDraw;
  // Specify the custom touch tracking callback
  sElem.pfuncXTouch = &gslc_ElemXSpinnerTouch;

  sElem.colElemFill = GSLC_COL_BLACK;
  sElem.colElemFillGlow = GSLC_COL_BLACK;
  sElem.colElemFrame = GSLC_COL_GRAY;
  sElem.colElemFrameGlow = GSLC_COL_WHITE;


  // Now create the sub elements
  // - Ensure page is set to GSLC_PAGE_NONE so that
  //   we create the element struct but not add it to a specific page.
  // - When we create an element with GSLC_PAGE_NONE it is
  //   saved in the GUI's temporary element storage.
  // - When we have finished creating / styling the element, we then
  //   copy it into the permanent sub-element storage

  // - The element IDs assigned to the sub-elements are
  //   arbitrary (with local scope in the compound element),
  //   so they don't need to be unique globally across the GUI.
  gslc_tsElemRef* pElemRefTmp = NULL;
  gslc_tsElem*    pElemTmp = NULL;
  gslc_tsElemRef* pElemRef = NULL;

  // Determine offset coordinate of compound element so that we can
  // specify relative positioning during the sub-element Create() operations.
  int16_t nOffsetX = rElem.x;
  int16_t nOffsetY = rElem.y;

  #if (GSLC_LOCAL_STR)
  pElemRefTmp = gslc_ElemCreateBtnTxt(pGui, SPINNER_ID_BTN_INC, GSLC_PAGE_NONE,
    (gslc_tsRect) {
    nOffsetX + nTxtBoxW, nOffsetY, nButtonSz, nButtonSz
  }, "\030", 0, nFontId, &gslc_ElemXSpinnerClick);
  #else
  strncpy(pXData->acElemTxt[0], "\030", SPINNER_STR_LEN - 1);
  pElemRefTmp = gslc_ElemCreateBtnTxt(pGui, SPINNER_ID_BTN_INC, GSLC_PAGE_NONE,
    (gslc_tsRect) {
    nOffsetX + nTxtBoxW, nOffsetY, nButtonSz, nButtonSz
  }, pXData->acElemTxt[0], SPINNER_STR_LEN,
      nFontId, &gslc_ElemXSpinnerClick);
  #endif
  gslc_ElemSetCol(pGui, pElemRefTmp, (gslc_tsColor) { 0, 0, 192 }, (gslc_tsColor) { 0, 0, 128 }, (gslc_tsColor) { 0, 0, 224 });
  gslc_ElemSetTxtCol(pGui, pElemRefTmp, GSLC_COL_WHITE);
  pElemTmp = gslc_GetElemFromRef(pGui, pElemRefTmp);
  gslc_CollectElemAdd(pGui, &pXData->sCollect, pElemTmp, GSLC_ELEMREF_DEFAULT);

  #if (GSLC_LOCAL_STR)
  pElemRefTmp = gslc_ElemCreateBtnTxt(pGui, SPINNER_ID_BTN_DEC, GSLC_PAGE_NONE,
    (gslc_tsRect) {
    nOffsetX + nTxtBoxW + nButtonSz, nOffsetY, nButtonSz, nButtonSz
  }, "\031", 0, nFontId, &gslc_ElemXSpinnerClick);
  #else
  strncpy(pXData->acElemTxt[1], "\031", SPINNER_STR_LEN - 1);
  pElemRefTmp = gslc_ElemCreateBtnTxt(pGui, SPINNER_ID_BTN_DEC, GSLC_PAGE_NONE,
    (gslc_tsRect) {
    nOffsetX + nTxtBoxW + nButtonSz, nOffsetY, nButtonSz, nButtonSz
  }, pXData->acElemTxt[1], SPINNER_STR_LEN,
      nFontId, &gslc_ElemXSpinnerClick);
  #endif
  gslc_ElemSetCol(pGui, pElemRefTmp, (gslc_tsColor) { 0, 0, 192 }, (gslc_tsColor) { 0, 0, 128 }, (gslc_tsColor) { 0, 0, 224 });
  gslc_ElemSetTxtCol(pGui, pElemRefTmp, GSLC_COL_WHITE);
  pElemTmp = gslc_GetElemFromRef(pGui, pElemRefTmp);
  gslc_CollectElemAdd(pGui, &pXData->sCollect, pElemTmp, GSLC_ELEMREF_DEFAULT);

  #if (GSLC_LOCAL_STR)
  pElemRefTmp = gslc_ElemCreateTxt(pGui, SPINNER_ID_TXT, GSLC_PAGE_NONE,
    (gslc_tsRect) {
    nOffsetX, nOffsetY, nTxtBoxW, nButtonSz
  }, (char*)acTxtNum, 0, nFontId);
  #else
  strncpy(pXData->acElemTxt[2], acTxtNum, SPINNER_STR_LEN - 1);
  pElemRefTmp = gslc_ElemCreateTxt(pGui, SPINNER_ID_TXT, GSLC_PAGE_NONE,
    (gslc_tsRect) {
    nOffsetX, nOffsetY, nTxtBoxW, nButtonSz
  }, pXData->acElemTxt[2], SPINNER_STR_LEN, nFontId);
  #endif
  gslc_ElemSetTxtAlign(pGui, pElemRefTmp, GSLC_ALIGN_MID_MID);
  pElemTmp = gslc_GetElemFromRef(pGui, pElemRefTmp);
  gslc_CollectElemAdd(pGui, &pXData->sCollect, pElemTmp, GSLC_ELEMREF_DEFAULT);


  // Now proceed to add the compound element to the page
  if (nPage != GSLC_PAGE_NONE) {
    pElemRef = gslc_ElemAdd(pGui, nPage, &sElem, GSLC_ELEMREF_DEFAULT);

    // save our ElemRef for the callback
    pXData->pElemRef = pElemRef;

    // Now propagate the parent relationship to enable a cascade
    // of redrawing from low-level elements to the top
    gslc_CollectSetParent(pGui, &pXData->sCollect, pElemRef);

    return pElemRef;
  }
  else {
    GSLC_DEBUG_PRINT("ERROR: ElemXSpinnerCreate(%s) Compound elements inside compound elements not supported\n", "");
    return NULL;

    // TODO: For now, disable compound elements within
    // compound elements. If we want to enable this, we
    // would probably use the temporary element reference
    // the GUI.
    // Save as temporary element for further processing
    //pGui->sElemTmp = sElem;   // Need fixing
    //return &(pGui->sElemTmp); // Need fixing

  }

}


// Redraw the compound element
// - When drawing a compound element, we clear the background
//   and then redraw the sub-element collection.
bool gslc_ElemXSpinnerDraw(void* pvGui,void* pvElemRef,gslc_teRedrawType eRedraw)
{
  gslc_tsGui* pGui = (gslc_tsGui*)(pvGui);
  gslc_tsElemRef* pElemRef = (gslc_tsElemRef*)(pvElemRef);
  gslc_tsElem* pElem = gslc_GetElemFromRef(pGui,pElemRef);
  gslc_tsXSpinner* pSpinner = (gslc_tsXSpinner*)gslc_GetXDataFromRef(pGui, pElemRef, GSLC_TYPEX_SPINNER, __LINE__);
  if (!pSpinner) return false;

  bool bGlow = (pElem->nFeatures & GSLC_ELEM_FEA_GLOW_EN) && gslc_ElemGetGlow(pGui,pElemRef);

  // Draw the compound element fill (background)
  // - Should only need to do this in full redraw
  if (eRedraw == GSLC_REDRAW_FULL) {
    gslc_DrawFillRect(pGui,pElem->rElem,(bGlow)?pElem->colElemFillGlow:pElem->colElemFill);
  }

  // Draw the sub-elements
  // - For now, force redraw of entire compound element
  gslc_tsCollect* pCollect = &pSpinner->sCollect;

  gslc_tsEvent  sEvent = gslc_EventCreate(pGui,GSLC_EVT_DRAW,GSLC_EVTSUB_DRAW_FORCE,(void*)(pCollect),NULL);
  gslc_CollectEvent(pGui,sEvent);

  // Optionally, draw a frame around the compound element
  // - This could instead be done by creating a sub-element
  //   of type box.
  // - We don't need to show any glowing of the compound element

  gslc_DrawFrameRect(pGui,pElem->rElem,(bGlow)?pElem->colElemFrameGlow:pElem->colElemFrame);

  // Clear the redraw flag
  gslc_ElemSetRedraw(pGui,pElemRef,GSLC_REDRAW_NONE);

  // Mark page as needing flip
  gslc_PageFlipSet(pGui,true);

  return true;
}

// Fetch the current value of the element's counter
int gslc_ElemXSpinnerGetCounter(gslc_tsGui* pGui,gslc_tsXSpinner* pSpinner)
{
  if ((pGui == NULL) || (pSpinner == NULL)) {
    static const char GSLC_PMEM FUNCSTR[] = "ElemXSpinnerGetCounter";
    GSLC_DEBUG_PRINT_CONST(ERRSTR_NULL,FUNCSTR);
    return 0;
  }
  return pSpinner->nCounter;
}


void gslc_ElemXSpinnerSetCounter(gslc_tsGui* pGui,gslc_tsXSpinner* pSpinner,int16_t nCount)
{
  if (pSpinner == NULL) {
    static const char GSLC_PMEM FUNCSTR[] = "ElemXSpinnerSetCounter";
    GSLC_DEBUG_PRINT_CONST(ERRSTR_NULL,FUNCSTR);
    return;
  }
  pSpinner->nCounter = nCount;

  // Determine new counter text
  // FIXME: Consider replacing the printf() with an optimized function to
  //        conserve RAM. Potentially leverage GSLC_DEBUG_PRINT().
  char  acStrNew[GSLC_LOCAL_STR_LEN];
  snprintf(acStrNew,GSLC_LOCAL_STR_LEN,"%hd",pSpinner->nCounter);

  // Update the element
  gslc_tsElemRef* pElemRef = gslc_CollectFindElemById(pGui,&pSpinner->sCollect,SPINNER_ID_TXT);
  gslc_ElemSetTxtStr(pGui,pElemRef,acStrNew);

}


// Handle the compound element main functionality
// - This routine is called by gslc_ElemEvent() to handle
//   any click events that resulted from the touch tracking process.
// - The code here will generally represent the core
//   functionality of the compound element and any communication
//   between sub-elements.
// - pvElemRef is a void pointer to the element ref being tracked. From
//   the pElemRefParent member we can get the parent/compound element
//   data structures.
bool gslc_ElemXSpinnerClick(void* pvGui,void *pvElemRef,gslc_teTouch eTouch,int16_t nX,int16_t nY)
{
#if defined(DRV_TOUCH_NONE)
  return false;
#else

  gslc_tsGui* pGui = (gslc_tsGui*)(pvGui);
  gslc_tsElemRef* pElemRef = (gslc_tsElemRef*)(pvElemRef);
  gslc_tsElem* pElem = gslc_GetElemFromRefD(pGui, pElemRef, __LINE__);
  if (!pElem) return false;

  // Fetch the parent of the clicked element which is the compound
  // element itself. This enables us to access the extra control data.
  gslc_tsElemRef*    pElemRefParent = pElem->pElemRefParent;
  if (pElemRefParent == NULL) {
    GSLC_DEBUG_PRINT("ERROR: ElemXSpinnerClick(%s) parent ElemRef ptr NULL\n","");
    return false;
  }

  gslc_tsElem*    pElemParent = gslc_GetElemFromRef(pGui,pElemRefParent);
  gslc_tsXSpinner* pSpinner     = (gslc_tsXSpinner*)(pElemParent->pXData);
  if (pSpinner == NULL) {
    GSLC_DEBUG_PRINT("ERROR: ElemXSpinnerClick() element (ID=%d) has NULL pXData\n",pElem->nId);
    return false;
  }

  // Begin the core compound element functionality
  int nCounter  = pSpinner->nCounter;

  // Handle the various button presses
  GSLC_CB_INPUT pfuncXInput = NULL;
  if (eTouch == GSLC_TOUCH_UP_IN) {

    // Get the tracked element ID
    gslc_tsElemRef* pElemRefTracked = pSpinner->sCollect.pElemRefTracked;
    gslc_tsElem*    pElemTracked    = gslc_GetElemFromRef(pGui,pElemRefTracked);
    int nSubElemId = pElemTracked->nId;

    if (nSubElemId == SPINNER_ID_BTN_INC) {
      // Increment button
      if (nCounter < pSpinner->nMax) {
        nCounter += pSpinner->nIncr;
      }
      gslc_ElemXSpinnerSetCounter(pGui,pSpinner,nCounter);

    } else if (nSubElemId == SPINNER_ID_BTN_DEC) {
      // Decrement button
      if (nCounter > pSpinner->nMin) {
        nCounter -= pSpinner->nIncr;
      }
      gslc_ElemXSpinnerSetCounter(pGui,pSpinner,nCounter);

    }

    // Invoke the callback function
    pfuncXInput = pSpinner->pfuncXInput;
    if (pfuncXInput != NULL) {
      (*pfuncXInput)(pvGui, pSpinner->pElemRef);
    }

  } // eTouch

  return true;
#endif // !DRV_TOUCH_NONE
}

bool gslc_ElemXSpinnerTouch(void* pvGui,void* pvElemRef,gslc_teTouch eTouch,int16_t nRelX,int16_t nRelY)
{
#if defined(DRV_TOUCH_NONE)
  return false;
#else
  gslc_tsGui* pGui = (gslc_tsGui*)(pvGui);
  gslc_tsElemRef* pElemRef = (gslc_tsElemRef*)(pvElemRef);
  gslc_tsXSpinner* pSpinner = (gslc_tsXSpinner*)gslc_GetXDataFromRef(pGui, pElemRef, GSLC_TYPEX_SPINNER, __LINE__);
  if (!pSpinner) return false;

  // Handle any compound element operations
  switch(eTouch) {
    case GSLC_TOUCH_DOWN_IN:
    case GSLC_TOUCH_MOVE_IN:
      gslc_ElemSetGlow(pGui,pElemRef,true);
      break;
    case GSLC_TOUCH_MOVE_OUT:
    case GSLC_TOUCH_UP_IN:
    case GSLC_TOUCH_UP_OUT:
    default:
      gslc_ElemSetGlow(pGui,pElemRef,false);
      break;
  }

  // Handle any sub-element operations

  // Get Collection
  gslc_tsCollect* pCollect = &pSpinner->sCollect;

  return gslc_CollectTouchCompound(pvGui, pvElemRef, eTouch, nRelX, nRelY, pCollect);
  #endif // !DRV_TOUCH_NONE
}
#endif // GSLC_FEATURE_COMPOUND


// ============================================================================