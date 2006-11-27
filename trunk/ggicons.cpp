/* GGICONS.DLL
   Wtyczka podmieniaj¹ca ikonki statusów sieci GG na te, z zestawów z GG5.0

   (c)2002 hao | Stamina
   http://www.stamina.eu.org/konnekt/sdk/
*/ 

#include "stdafx.h"
#include "kstyle.h"
#include "konnekt/kstyle.h"
#include "konnekt/obsolete.h"
using namespace std;


bool reLoad = false;

void GGSetIcons() {
    // Ladujemy obrazki statusów
    HBITMAP bmp;
    CStdString dir = GETSTR(CFG_GGI_DIR) + string("\\") + GETSTR(CFG_GGI_SET) + string("\\");
    bmp = (HBITMAP)LoadImage(0 , (dir+"status.bmp").c_str() , IMAGE_BITMAP , 0 , 0 , LR_LOADFROMFILE);
    if (bmp)
    {
        int IDList [12] = {
            UIIcon(IT_STATUS , NET_GG , ST_ONLINE , 0)
           ,UIIcon(IT_STATUS , NET_GG , ST_OFFLINE , 0)
           ,UIIcon(IT_STATUS , NET_GG , ST_AWAY , 0)
           ,0,0,0
           ,UIIcon(IT_STATUS , NET_GG , ST_CONNECTING , 0)
           ,0,0
           ,UIIcon(IT_STATUS , NET_GG , ST_BLOCKING , 0)
           ,0
           ,UIIcon(IT_STATUS , NET_GG , ST_HIDDEN , 0)
        };
        IconRegisterList((IML_enum)(IML_16|(ICMessage(IMC_ISWINXP)*IML_ICO2)) , 12 , IDList , bmp);
        DeleteObject(bmp);
        if (!ICMessage(IMC_ISWINXP)) { // £adujemy 16 kolorowe ikonki dla tray'a
            bmp = (HBITMAP)LoadImage(0 , (dir+"status16.bmp").c_str() , IMAGE_BITMAP , 0 , 0 , LR_LOADFROMFILE);
            if (bmp) {
                IDList [6] = UIIcon(IT_STATUS , NET_GG , ST_HIDDEN , 0);
                IconRegisterList((IML_enum)(IML_ICO2) , 7 , IDList , bmp);
                DeleteObject(bmp);
            }
        }
    }
    // Ladujemy obrazki dodatkowe
    bmp = (HBITMAP)LoadImage(0 , (dir+"tray.bmp").c_str() , IMAGE_BITMAP , 0 , 0 , LR_LOADFROMFILE);
    if (bmp)
    {
        int IDList [4] = {
            UIIcon(IT_MESSAGE , NET_NONE , MT_MESSAGE , 0)
           ,0x200
           ,UIIcon(IT_MESSAGE , NET_GG , MT_FILE , 0)
           ,UIIcon(IT_MESSAGE , NET_GG , MT_SOUND , 0)
        };
        IconRegisterList((IML_enum)(IML_16|(ICMessage(IMC_ISWINXP)*IML_ICO2)) , 4 , IDList , bmp);
        DeleteObject(bmp);
        if (!ICMessage(IMC_ISWINXP)) { // £adujemy 16 kolorowe ikonki dla tray'a
            bmp = (HBITMAP)LoadImage(0 , (dir+"tray16.bmp").c_str() , IMAGE_BITMAP , 0 , 0 , LR_LOADFROMFILE);
            if (bmp) {
                IconRegisterList((IML_enum)(IML_ICO2) , 4 , IDList , bmp);
                DeleteObject(bmp);
            }
        }
    }
}

#define CFGSETCOL(i,t,d) {sSETCOL sc;sc.id=(i);sc.type=(t);sc.def=(int)(d);ICMessage(IMC_CFG_SETCOL,(int)&sc);}
void GGISetCols() {
  CFGSETCOL(CFG_GGI_DIR , DT_CT_PCHAR , "ggicons");
  CFGSETCOL(CFG_GGI_SET , DT_CT_PCHAR , "");
}


int GGIPrepare() {
	UIActionCfgAdd(IMIG_CFG_PLUGS_KSTYLE , 0 , ACTT_GROUP , "Schemat ikon z GG5.0 [ggIcons]" , 0 , 0 , 20);{
		if (ShowBits::checkBits(ShowBits::showInfoNormal)) {
			UIActionCfgAdd(IMIG_CFG_PLUGS_KSTYLE , 0 , ACTT_TIPBUTTON | ACTSC_INLINE | ACTSBUTTON_ALIGNRIGHT , "" 
				AP_TIP "Pobrane z internetu zestawy ikon GG wgraj do katalogu Konnekt\\GGIcons. Mo¿esz ich u¿ywaæ jednoczeœnie ze schematami ikon kStyle."
				"\r\n\r\n¯eby powróciæ do domyœlnych ikonek, ustaw zestaw na pusty i uruchom program ponownie.");
		}
		if (ShowBits::checkLevel(ShowBits::levelNormal)) {
			UIActionCfgAdd(IMIG_CFG_PLUGS_KSTYLE , IMIB_CFG , ACTT_DIR , "" AP_TIP "Katalog ze schematami" , CFG_GGI_DIR);
		}
		UIActionCfgAdd(IMIG_CFG_PLUGS_KSTYLE , IMIB_CFG , ACTT_COMBO|ACTSCOMBO_LIST|ACTR_INIT|ACTSC_INLINE|ACTR_STATUS , "" CFGTIP "Wybierz schemat" , CFG_GGI_SET);
		UIActionCfgAdd(IMIG_CFG_PLUGS_KSTYLE , IMIA_CFG_PLUGS_GGI_PREVIEW , ACTT_IMAGE , "");
        
	}UIActionCfgAdd(IMIG_CFG_PLUGS_KSTYLE , 0 , ACTT_GROUPEND);
  return 1;
}

int GGIActionCfgProc(sUIActionNotify_base * anBase) {
  sUIActionNotify_2params * an = static_cast<sUIActionNotify_2params*>(anBase);
  switch (an->act.id & ~IMIB_CFG) {
    case CFG_GGI_SET:
        if (an->code == ACTN_ACTION || an->code == ACTN_STATUS) {
            CStdString dir="";
            CStdString set="";
            UIActionCfgGetValue(sUIAction(an->act.parent , CFG_GGI_DIR|IMIB_CFG) , dir.GetBuffer(255) , 255);
            dir.ReleaseBuffer();
			if (dir.empty()) dir = GETSTR(CFG_GGI_DIR);
            UIActionCfgGetValue(sUIAction(an->act.parent , CFG_GGI_SET|IMIB_CFG) , set.GetBuffer(255) , 255);
            set.ReleaseBuffer();
            UIActionSetText(sUIAction(an->act.parent , IMIA_CFG_PLUGS_GGI_PREVIEW) , "file://" + dir + "\\" + set + "\\status.bmp");
        }
        else if (an->code == ACTN_DROP || an->code == ACTN_CREATE) {
            reLoad = true;
            CStdString dirs="brak" CFGICO "116" CFGVALUE "116\n";
            CStdString dir="";
            UIActionCfgGetValue(sUIAction(an->act.parent , CFG_GGI_DIR|IMIB_CFG) , dir.GetBuffer(255) , 255);
            dir.ReleaseBuffer();
			if (dir.empty()) dir = GETSTR(CFG_GGI_DIR);
            dir+="\\";
            WIN32_FIND_DATA fd;
            HANDLE hFile;
            BOOL found;
            found = ((hFile = FindFirstFile((dir+"*.*").c_str(),&fd))!=INVALID_HANDLE_VALUE);
            while (found)
            {
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
                    && strcmp("." , fd.cFileName) && strcmp(".." , fd.cFileName)
                    && !_access((dir + fd.cFileName + string("\\status.bmp")).c_str() , 04)
                    ) {
                    dirs+=string(fd.cFileName) + CFGICO "3\n";
                }
                found = FindNextFile(hFile , &fd);
            }
            FindClose(hFile);   

            UIActionSetText(an->act , dirs.c_str());
        }
        break;
  }
  return 0;
}


