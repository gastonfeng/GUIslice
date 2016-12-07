//
// GUIslice Library Examples
// - Calvin Hass
// - http://www.impulseadventure.com/elec/microsdl-sdl-gui.html
// - Example 07: Sliders with dynamic color control
//               and position callback
//

#include "GUIslice.h"
#include "GUIslice_ex.h"
#include "GUIslice_drv.h"

// Define default device paths for framebuffer & touchscreen
#ifdef DRV_TYPE_SDL1
  #define GSLC_DEV_FB     "/dev/fb1"
#elif DRV_TYPE_SDL2
  #define GSLC_DEV_FB     "/dev/fb0"
#endif
#define GSLC_DEV_TOUCH  "/dev/input/touchscreen"

// Defines for resources
#define FONT_DROID_SANS "/usr/share/fonts/truetype/droid/DroidSans.ttf"

// Enumerations for pages, elements, fonts, images
enum {E_PG_MAIN};
enum {E_ELEM_BOX,E_ELEM_BTN_QUIT,E_ELEM_COLOR,
      E_SLIDER_R,E_SLIDER_G,E_SLIDER_B};
enum {E_FONT_BTN,E_FONT_TXT,E_FONT_HEAD,E_FONT_TITLE};

bool      m_bQuit = false;

// Free-running counter for display
unsigned  m_nCount = 0;

// Instantiate the GUI
#define MAX_FONT            10
gslc_tsGui                  m_gui;
gslc_tsDriver               m_drv;
gslc_tsFont                 m_asFont[MAX_FONT];
gslc_tsXSlider              m_sXSlider_R,m_sXSlider_G,m_sXSlider_B;

#define MAX_PAGE            1
#define MAX_ELEM_PG_MAIN    30
gslc_tsPage                 m_asPage[MAX_PAGE];
gslc_tsElem                 m_asPageElem[MAX_ELEM_PG_MAIN];


// Current RGB value for color box
// - Globals defined here for convenience so that callback
//   can update R,G,B components independently
int   m_nPosR = 255;
int   m_nPosG = 128;
int   m_nPosB = 0;


// Quit button callback
bool CbBtnQuit(void* pvGui,void *pvElem,gslc_teTouch eTouch,int nX,int nY)
{
  if (eTouch == GSLC_TOUCH_UP_IN) {
    m_bQuit = true;
  }
  return true;
}

// Callback function for when a slider's position has been updated
// - After a slider position has been changed, update the color box
// - Note that all three sliders use the same callback for
//   convenience. From the element's ID we can determine which
//   slider was updated.
bool CbSlidePos(void* pvGui,void* pvElem,int nPos)
{
  gslc_tsGui*     pGui    = (gslc_tsGui*)(pvGui);
  gslc_tsElem*    pElem   = (gslc_tsElem*)(pvElem);  
  //gslc_tsXSlider* pSlider = (gslc_tsXSlider*)(pElem->pXData);  
  
  // Fetch the new RGB component from the slider
  switch (pElem->nId) {
    case E_SLIDER_R:
      m_nPosR = gslc_ElemXSliderGetPos(pElem);
      break;
    case E_SLIDER_G:
      m_nPosG = gslc_ElemXSliderGetPos(pElem);      
      break;
    case E_SLIDER_B:
      m_nPosB = gslc_ElemXSliderGetPos(pElem);      
      break;
    default:
      break;
  }
  
  // Calculate the new RGB value
  gslc_Color colRGB = (gslc_Color){m_nPosR,m_nPosG,m_nPosB};
  
  // Update the color box
  gslc_tsElem* pElemColor = gslc_PageFindElemById(pGui,E_PG_MAIN,E_ELEM_COLOR);
  gslc_ElemSetCol(pElemColor,GSLC_COL_WHITE,colRGB,GSLC_COL_WHITE); 
  
  return true;
}

// Create page elements
bool InitOverlays()
{
  gslc_tsElem*  pElem = NULL;

  gslc_PageAdd(&m_gui,E_PG_MAIN,m_asPageElem,MAX_ELEM_PG_MAIN);
  
  // Background flat color
  gslc_SetBkgndColor(&m_gui,GSLC_COL_GRAY_DK2);

  // Create Title with offset shadow
  pElem = gslc_ElemCreateTxt(&m_gui,GSLC_ID_AUTO,E_PG_MAIN,(gslc_Rect){2,2,320,50},
    "Home Automation",E_FONT_TITLE);
  gslc_ElemSetTxtCol(pElem,(gslc_Color){32,32,60});
  gslc_ElemSetTxtAlign(pElem,GSLC_ALIGN_MID_MID);
  gslc_ElemSetFillEn(pElem,false);
  pElem = gslc_ElemCreateTxt(&m_gui,GSLC_ID_AUTO,E_PG_MAIN,(gslc_Rect){0,0,320,50},
    "Home Automation",E_FONT_TITLE);
  gslc_ElemSetTxtCol(pElem,(gslc_Color){128,128,240});
  gslc_ElemSetTxtAlign(pElem,GSLC_ALIGN_MID_MID);
  gslc_ElemSetFillEn(pElem,false);
  
  // Create background box
  pElem = gslc_ElemCreateBox(&m_gui,E_ELEM_BOX,E_PG_MAIN,(gslc_Rect){10,50,300,180});
  gslc_ElemSetCol(pElem,GSLC_COL_WHITE,GSLC_COL_BLACK,GSLC_COL_BLACK);
  
  // Create dividers
  pElem = gslc_ElemCreateBox(&m_gui,GSLC_ID_AUTO,E_PG_MAIN,(gslc_Rect){20,100,280,1});
  gslc_ElemSetCol(pElem,GSLC_COL_GRAY_DK3,GSLC_COL_BLACK,GSLC_COL_BLACK);  
  
  pElem = gslc_ElemCreateBox(&m_gui,GSLC_ID_AUTO,E_PG_MAIN,(gslc_Rect){235,60,1,35});
  gslc_ElemSetCol(pElem,GSLC_COL_GRAY_DK3,GSLC_COL_BLACK,GSLC_COL_BLACK);  
  
  
  // Create color box
  pElem = gslc_ElemCreateBox(&m_gui,E_ELEM_COLOR,E_PG_MAIN,(gslc_Rect){20,90+30,130,100});
  gslc_Color colRGB = (gslc_Color){m_nPosR,m_nPosG,m_nPosB};
  gslc_ElemSetCol(pElem,GSLC_COL_WHITE,colRGB,GSLC_COL_WHITE); 
  
  
  // Create Quit button with text label
  pElem = gslc_ElemCreateBtnTxt(&m_gui,E_ELEM_BTN_QUIT,E_PG_MAIN,
    (gslc_Rect){250,60,50,30},"SAVE",E_FONT_BTN,&CbBtnQuit);
  gslc_ElemSetCol(pElem,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK4,GSLC_COL_BLUE_DK1);    
  gslc_ElemSetTxtCol(pElem,GSLC_COL_WHITE);
  
  // Create dummy selector
  pElem = gslc_ElemCreateTxt(&m_gui,GSLC_ID_AUTO,E_PG_MAIN,(gslc_Rect){20,65,100,20},
    "Selected Room:",E_FONT_HEAD);
  gslc_ElemSetTxtCol(pElem,GSLC_COL_GRAY_LT2);

  pElem = gslc_ElemCreateBtnTxt(&m_gui,GSLC_ID_AUTO,E_PG_MAIN,
    (gslc_Rect){140,65,80,20},"Kitchen...",E_FONT_BTN,NULL);
  gslc_ElemSetCol(pElem,GSLC_COL_GRAY_DK2,GSLC_COL_GRAY_DK3,GSLC_COL_BLUE_DK1);    
  gslc_ElemSetTxtCol(pElem,GSLC_COL_WHITE);
  
  // Create sliders
  // - Define element arrangement
  int nCtrlY    = 115;
  int nCtrlGap  = 30;
  int nSlideW   = 80;
  int nSlideH   = 20;
  int nLabelX   = 160;
  int nLabelW   = 30;
  int nLabelH   = 20;
  int nSlideX   = nLabelX + nLabelW + 20;
  
  pElem = gslc_ElemCreateTxt(&m_gui,GSLC_ID_AUTO,E_PG_MAIN,(gslc_Rect){nLabelX,nCtrlY,120,20},
    "Set LED RGB:",E_FONT_HEAD);
  gslc_ElemSetTxtCol(pElem,GSLC_COL_WHITE);
  nCtrlY += 25;
  
  // Create three sliders (R,G,B) and assign callback function
  // that is invoked upon change. The common callback will update
  // the color box.
  
  pElem = gslc_ElemCreateTxt(&m_gui,GSLC_ID_AUTO,E_PG_MAIN,(gslc_Rect){nLabelX,nCtrlY,nLabelW,nLabelH},
    "Red:",E_FONT_TXT);
  gslc_ElemSetTxtCol(pElem,GSLC_COL_GRAY_LT3);  
  pElem = gslc_ElemXSliderCreate(&m_gui,E_SLIDER_R,E_PG_MAIN,&m_sXSlider_R,
          (gslc_Rect){nSlideX,nCtrlY,nSlideW,nSlideH},0,255,m_nPosR,5,false);
  gslc_ElemSetCol(pElem,GSLC_COL_RED,GSLC_COL_BLACK,GSLC_COL_BLACK);          
  gslc_ElemXSliderSetStyle(pElem,true,GSLC_COL_RED_DK4,10,5,GSLC_COL_GRAY_DK2);
  gslc_ElemXSliderSetPosFunc(pElem,&CbSlidePos);  
  nCtrlY += nCtrlGap;
  
  pElem = gslc_ElemCreateTxt(&m_gui,GSLC_ID_AUTO,E_PG_MAIN,(gslc_Rect){nLabelX,nCtrlY,nLabelW,nLabelH},
    "Green:",E_FONT_TXT);
  gslc_ElemSetTxtCol(pElem,GSLC_COL_GRAY_LT3);  
  pElem = gslc_ElemXSliderCreate(&m_gui,E_SLIDER_G,E_PG_MAIN,&m_sXSlider_G,
          (gslc_Rect){nSlideX,nCtrlY,nSlideW,nSlideH},0,255,m_nPosG,5,false);
  gslc_ElemSetCol(pElem,GSLC_COL_GREEN,GSLC_COL_BLACK,GSLC_COL_BLACK);
  gslc_ElemXSliderSetStyle(pElem,true,GSLC_COL_GREEN_DK4,10,5,GSLC_COL_GRAY_DK2);
  gslc_ElemXSliderSetPosFunc(pElem,&CbSlidePos);    
  nCtrlY += nCtrlGap;
  
  pElem = gslc_ElemCreateTxt(&m_gui,GSLC_ID_AUTO,E_PG_MAIN,(gslc_Rect){nLabelX,nCtrlY,nLabelW,nLabelH},
    "Blue:",E_FONT_TXT);
  gslc_ElemSetTxtCol(pElem,GSLC_COL_GRAY_LT3);  
  pElem = gslc_ElemXSliderCreate(&m_gui,E_SLIDER_B,E_PG_MAIN,&m_sXSlider_B,
          (gslc_Rect){nSlideX,nCtrlY,nSlideW,nSlideH},0,255,m_nPosB,5,false);
  gslc_ElemSetCol(pElem,GSLC_COL_BLUE,GSLC_COL_BLACK,GSLC_COL_BLACK);          
  gslc_ElemXSliderSetStyle(pElem,true,GSLC_COL_BLUE_DK4,10,5,GSLC_COL_GRAY_DK2);
  gslc_ElemXSliderSetPosFunc(pElem,&CbSlidePos);    
  nCtrlY += nCtrlGap;

  
  pElem = gslc_ElemCreateTxt(&m_gui,GSLC_ID_AUTO,E_PG_MAIN,(gslc_Rect){250,230,60,10},
    "GUIslice Example",E_FONT_TXT);
  gslc_ElemSetTxtCol(pElem,GSLC_COL_BLACK); 
  gslc_ElemSetTxtAlign(pElem,GSLC_ALIGN_MID_RIGHT); 
  gslc_ElemSetFillEn(pElem,false); 
  
  return true;
}


int main( int argc, char* args[] )
{
  bool                bOk = true;

  // -----------------------------------
  // Initialize

  gslc_InitEnv(GSLC_DEV_FB,GSLC_DEV_TOUCH);

  if (!gslc_Init(&m_gui,&m_drv,m_asPage,MAX_PAGE,m_asFont,MAX_FONT,NULL,0)) { exit(1); }  

  gslc_InitTs(&m_gui,GSLC_DEV_TOUCH);

  // Load Fonts
  // - Normally we would select a number of different fonts
  bOk = gslc_FontAdd(&m_gui,E_FONT_BTN,FONT_DROID_SANS,14);
  if (!bOk) { fprintf(stderr,"ERROR: FontAdd failed\n"); exit(1); }
  bOk = gslc_FontAdd(&m_gui,E_FONT_TXT,FONT_DROID_SANS,10);
  if (!bOk) { fprintf(stderr,"ERROR: FontAdd failed\n"); exit(1); }
  bOk = gslc_FontAdd(&m_gui,E_FONT_HEAD,FONT_DROID_SANS,14);
  if (!bOk) { fprintf(stderr,"ERROR: FontAdd failed\n"); exit(1); }
  bOk = gslc_FontAdd(&m_gui,E_FONT_TITLE,FONT_DROID_SANS,36);
  if (!bOk) { fprintf(stderr,"ERROR: FontAdd failed\n"); exit(1); }

  // -----------------------------------
  // Start display
  InitOverlays();

  // Start up display on main page
  gslc_SetPageCur(&m_gui,E_PG_MAIN);
 
  // -----------------------------------
  // Main event loop

  m_bQuit = false;
  while (!m_bQuit) {

    // General counter
    m_nCount++;

    // Periodically call GUIslice update function
    gslc_Update(&m_gui);
    
  } // bQuit


  // -----------------------------------
  // Close down display

  gslc_Quit(&m_gui);

  return 0;
}

