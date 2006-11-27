// kstyle.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "../plug_defs/lib.h"

#ifndef _DEBUG
	#pragma comment(lib , "d:/c++/konnekt/__libs/shared.lib")
#else
	#pragma comment(lib , "d:/c++/konnekt/__libs/shared_debug.lib")
#endif

#include "kstyle.h"
#include "konnekt/kstyle.h"
#include "include/simxml.h"
#include "include/func.h"
#include "include/fontpicker.h"
#include "konnekt/obsolete.h"
#include "resource.h"

#define URL_KATALOG_KSTYLE "http://www.konnekt.info/open.php?dirID=105"
using namespace std;

int __stdcall DllMain(void * hinstDLL, unsigned long fdwReason, void * lpvReserved)
{
        return true;
}
int Init() {
  return 1;
}

int DeInit() {
  return 1;
}


int trayBitCount;

int IStart() {
	trayBitCount = ICMessage(IMC_ISWINXP)?32 : 4;
	if (ICMessage(IMC_FINDPLUG_BYSIG , (int)"GGICO")) 
		ICMessage(IMI_INFORM , (int)"kSTYLE zawiera w sobie kod wtyczki ggIcons, powinieneœ wiêc ggIcons wy³¹czyæ!");
	GGSetIcons();
	SetTheme(true);
	return 1;
}
int IEnd() {
  return 1;
}


int GetIntVal(const char * val) {
	if (!val || !*val) return 0;
	if (val[1]=='x') // hexy
        return chtoint(val+2 , 16);
	else if (val[0]=='#')
        return chtoint(val+1 , 16);
	else return  atoi(val);

}

int BGR2RGB(int v) {
	return (v & 0x00FF00) | ((v & 0xFF) << 16) | ((v & 0xFF0000) >> 16);
}

// doIcons - czy ³aduje ikonki, czy czcionki...
void SetTheme(bool doIcons) {
	CStdString theme = doIcons? GETSTR(CFG_KSTYLE_THEME) : UIActionCfgGetValue(sUIAction(IMIG_CFG_PLUGS_KSTYLE , IMIA_CFG_PLUGS_KSTYLE_THEMELIST),0,0);
	CStdString themeDir;
	CStdString themeName = theme;
	if (theme.empty()) return;
	if (theme.Right(4).CompareNoCase(".xml")) {
		themeDir =KSTYLE_DIRNAME "\\" + theme + "\\";
		theme = KSTYLE_XMLFILE;
	} else 
		themeDir =KSTYLE_DIRNAME "\\";
	IMLOG("- Applying theme %s" , themeName.c_str());
	cXML xml;
	ICMessage(IMC_RESTORECURDIR);
	if (!xml.loadFile((themeDir + theme).c_str()))
		return;
	// Ladujemy ikonki...
	CStdString tmp;
	if (doIcons) {
		if (1/*GETINT(CFG_KSTYLE_LOADICONS)*/) {
			cXML xml2;
			xml2.loadSource(xml.getContent("theme/icons").c_str());
			while (xml2.prepareNode("ico" , true)) {
				xml2.next();
				CStdString file = xml2.getAttrib("file");
				if (file.empty() || _access(themeDir + file , 4))
					continue;
				sUIIconRegister ir;
				
				file="file://" + themeDir + file;
				ir.URL = file;
				// Wczytanie pozosta³ych parametrów
				tmp = xml2.getAttrib("iml");
				ir.target = tmp==""?IML_16 : tmp=="16"?IML_16: tmp=="32"?IML_32: tmp=="ico"?IML_ICO: tmp=="ico2"?IML_ICO2:tmp=="bmp"?IML_BMP: (IML_enum)GetIntVal(tmp);
				tmp = xml2.getAttrib("type");
				if (tmp.empty()) { // zwyk³e ID
					ir.ID = GetIntVal(xml2.getAttrib("id").c_str());
				} else {
					ir.ID = 0;
					
					tmp.MakeLower();
					int type = tmp=="logo"?IT_LOGO: tmp=="status"?IT_STATUS: tmp=="overlay"?IT_OVERLAY: tmp=="message"?IT_MESSAGE:GetIntVal(tmp);
					ir.ID = UIIcon(type , GetIntVal(xml2.getAttrib("net").c_str()) , GetIntVal(xml2.getAttrib("id").c_str()) , GetIntVal(xml2.getAttrib("frame").c_str()));
				}
				ir.iconParams.size = GetIntVal(xml2.getAttrib("size").c_str());
				tmp = xml2.getAttrib("bits");
				ir.iconParams.bits = (tmp=="tray")? trayBitCount : GetIntVal(tmp);
				// Usuwamy ew. animacjê...
/*				if ( ir.ID & UIIcon(-1 , 0 , 0 , 0) && (ir.ID & 0xF) == 0) {
					for (int i=1; i <= 0xF; i++)
						if (ICMessage(IMI_ICONEXISTS , ir.ID | i , ir.target))
							ICMessage(IMI_ICONUNREGISTER , ir.ID | i , ir.target);
				}*/
				ICMessage(IMI_ICONREGISTER , (int)&ir);
			}
		}
	} else { // doIcons
		bool loadColors = *UIActionCfgGetValue(sUIAction(IMIG_CFG_PLUGS_KSTYLE , IMIB_CFG|CFG_KSTYLE_LOADCOLORS),0,0)=='1'; 
		bool loadFonts = *UIActionCfgGetValue(sUIAction(IMIG_CFG_PLUGS_KSTYLE , IMIB_CFG|CFG_KSTYLE_LOADFONTS),0,0)=='1';
		bool loadIcons = *UIActionCfgGetValue(sUIAction(IMIG_CFG_PLUGS_KSTYLE , IMIB_CFG|CFG_KSTYLE_LOADICONS),0,0)=='1';
		if (loadIcons) {
			UIActionCfgSetValue(sUIAction(IMIG_CFG_PLUGS_KSTYLE , IMIB_CFG|CFG_KSTYLE_THEME), themeName);
			reLoad = true;
		}
		if (loadColors /*stricte color*/) {
			cXML xml2;
			xml2.loadSource(xml.getContent("theme/colors").c_str());
			while (xml2.prepareNode("color" , true)) {
				xml2.next();
				int id = GetIntVal(xml2.getAttrib("id").c_str());
				if (!id || Ctrl->DTgetPos(DTCFG , id)==-1
					|| (Ctrl->DTgetType(DTCFG , id) & 0xF) != DT_CT_INT) continue;
				// Sprawdzamy dopuszczalne wartoœci...
				sUIAction act (IMIG_CFG , IMIB_CFG|id);
				if (!ICMessage(IMI_ACTION_FINDPARENT , (int)&act , 1))
					continue; // Kontrolka nie istnieje...
				if ((UIActionGetStatus(act) & ACTM_TYPE) != ACTT_COLOR)
					continue; // To nie kolor!
				if (!ICMessage(IMI_GROUP_MAKECFG , (int)&sUIAction(0,act.parent)))
					continue; // konfiguracja jest zamkniêta!
				UIActionCfgSetValue(act , inttoch(BGR2RGB(GetIntVal(xml2.getAttrib("value").c_str()))));
//				SETINT(id , BGR2RGB(GetIntVal(xml2.getAttrib("value").c_str())));
			}
		} // kolory
		if (loadColors || loadFonts) {
			cXML xml2;
			xml2.loadSource(xml.getContent("theme/fonts").c_str());
			while (xml2.prepareNode("font" , true)) {
				xml2.next();
				int id = GetIntVal(xml2.getAttrib("id").c_str());
				if (!id || Ctrl->DTgetPos(DTCFG , id)==-1
					|| (Ctrl->DTgetType(DTCFG , id) & 0xF) != DT_CT_PCHAR) continue;
				// Sprawdzamy dopuszczalne wartoœci...
				sUIAction act (IMIG_CFG , IMIB_CFG|id);
				if (!ICMessage(IMI_ACTION_FINDPARENT , (int)&act , 1))
					continue; // Kontrolka nie istnieje...
				if ((UIActionGetStatus(act) & ACTM_TYPE) != ACTT_FONT)
					continue; // To nie font!
				if (!ICMessage(IMI_GROUP_MAKECFG , (int)&sUIAction(0,act.parent)))
					continue; // konfiguracja jest zamkniêta!
				LOGFONTEX lfe = StrToLogFont(GETSTR(id));
				lfe.active = true;
				if (loadColors) {
					tmp = xml2.getAttrib("color");
					if (!tmp.empty()) lfe.color = BGR2RGB(GetIntVal(tmp));
					tmp = xml2.getAttrib("bgcolor");
					if (!tmp.empty()) lfe.bgColor = BGR2RGB(GetIntVal(tmp));
				}
				if (loadFonts) {
					tmp = xml2.getAttrib("face");
					if (!tmp.empty()) strcpy(lfe.lf.lfFaceName , tmp);
					tmp = xml2.getAttrib("size");
					if (!tmp.empty()) lfe.lf.lfHeight = GetIntVal(tmp);
					tmp = xml2.getAttrib("charset");
					if (!tmp.empty()) lfe.lf.lfCharSet = GetIntVal(tmp);
					tmp = xml2.getAttrib("flags");
					if (!tmp.empty()) {
						lfe.lf.lfWeight = (tmp.find('b') != -1)? FW_BOLD : FW_NORMAL;
						lfe.lf.lfUnderline = (tmp.find('u') != -1);
						lfe.lf.lfItalic = (tmp.find('i') != -1);
					}
				}
				UIActionCfgSetValue(act , LogFontToStr(lfe).c_str());
//				SETSTR(id , LogFontToStr(lfe).c_str());
			}
		} // fonty
	}// doIcons

}

void ExportGroup(ofstream & f , int parent , int type) {
	int c = ICMessage(IMI_GROUP_ACTIONSCOUNT , (int)&sUIAction(0,parent));
	for (int i=0; i<c; i++) {
		int id = ICMessage(IMI_ACTION_GETID,parent , i);
		if (!id) continue;
		int status = UIActionGetStatus(sUIAction(parent , id));
		// idziemy g³êbiej...
		if (status & ACTS_GROUP) {ExportGroup(f , id , type); continue;}
		// Sprawdzam, czy kontrolka nadaje siê na export
		if ((status & ACTM_TYPE) == type && 
			(id & IMIB_) == IMIB_CFG) {
				sUIActionInfo si;
				si.act.parent = parent;
				si.act.id = id;
				si.mask = UIAIM_P1;
				UIActionGet(si);
				if (!si.p1) continue;
				if (type == ACTT_FONT) {
					if ((Ctrl->DTgetType(DTCFG,si.p1) & 0xF)!=DT_CT_PCHAR) continue;
					LOGFONTEX lfe = StrToLogFont(GETSTR(si.p1));
					f << "		<font id=\"" << si.p1 << "\""
						<< " color=\"#" << inttoch(BGR2RGB(lfe.color),16,6,true) << "\"";
					f	<< " bgColor=\"#" << inttoch(BGR2RGB(lfe.bgColor),16,6,true) << "\""
						<< " face=\"" << lfe.lf.lfFaceName << "\""
						<< " size=\"" << lfe.lf.lfHeight << "\""
						<< " charset=\"" << (int)lfe.lf.lfCharSet << "\""
						<< " flags=\""
						;
					if (lfe.lf.lfWeight == FW_BOLD) f << 'b';
					if (lfe.lf.lfUnderline) f << 'u';
					if (lfe.lf.lfItalic) f << 'i';
					f << "\"/>" << endl;

				} else if (type == ACTT_COLOR) {
					if ((Ctrl->DTgetType(DTCFG,si.p1) & 0xF)!=DT_CT_INT) continue;
					f << "			<color id=\"" << si.p1 << "\" value=\"#" << inttoch(BGR2RGB(GETINT(si.p1)),16,6,true) << "\"/>" << endl;
				}

		}
	}
}

void ExportTheme() {
	sDIALOG_enter sde;
	sde.id = "kStyle.Export";
	sde.info = "Podaj nazwê schematu";
	sde.maxLength = 60;
	sde.title = "Zapisywanie aktualnych kolorów jako schemat.";
	ICMessage(IMI_DLGENTER , (int)&sde);
	if (!sde.value || !*sde.value) return;
	CStdString path = (char*)ICMessage(IMC_RESTORECURDIR);
	// Usuwamy ew. dopisek .xml
	if (stristr(sde.value , ".xml") == (sde.value + strlen(sde.value) - 4)) stristr(sde.value , ".xml")[0] = 0;
	path += KSTYLE_DIRNAME "\\";
	path += sde.value;
	bool inDir = _access(path + "\\" KSTYLE_XMLFILE , 0)==0;
	if (!inDir) 
		path+=".xml";
	if ((!inDir && _access(path , 0)) || ICMessage(IMI_CONFIRM , (int)"Nadpisaæ istniej¹cy schemat?")) {
		if (inDir) _mkdir(path);
		ofstream f;
		f.open(path  + (inDir? "\\" KSTYLE_XMLFILE : ""));
		f << "<?xml version=\"1.0\" encoding=\"Windows-1250\"?>" << endl;
		f << "<!-- Generowane automatycznie -->" << endl;
		f << "<theme>" << endl;
		f << "	<info>" << endl;
		f << "		<name>" << sde.value << "</name>" << endl;
		f << "		<author>" << (char*)ICMessage(IMC_GETPROFILE) << "</author>" << endl;
		f << "		<comment>Schemat bez ikonek</comment>" << endl;
		f << "	</info>" << endl;
		f << "	<colors>" << endl;
		//ExportGroup(f , IMIG_CFG , ACTT_COLOR);
		f << "	</colors>" << endl;
		f << "	<fonts>" << endl;
		ExportGroup(f , IMIG_CFG , ACTT_FONT);
		f << "	</fonts>" << endl;
		f << "</theme>" << endl;
		f.close();
		ICMessage(IMI_INFORM , (int)("Schemat zosta³ zapisany do pliku " + path + string(inDir? "\\" KSTYLE_XMLFILE : "") + ".\r\n").c_str());
	}
}



#define CFGSETCOL(i,t,d,n) {sSETCOL sc;sc.id=(i);sc.type=(t);sc.def=(int)(d);sc.name=n; ICMessage(IMC_CFG_SETCOL,(int)&sc);}
int ISetCols() {
	CFGSETCOL(CFG_KSTYLE_THEME , DT_CT_PCHAR , 0 , "k.Style Theme");
	CFGSETCOL(CFG_KSTYLE_LOADICONS , DT_CT_INT , 1 , "k.Style LoadIcons");
	CFGSETCOL(CFG_KSTYLE_LOADCOLORS , DT_CT_INT , 1 , "k.Style LoadColors");
	CFGSETCOL(CFG_KSTYLE_LOADFONTS , DT_CT_INT , 1 , "k.Style LoadFonts");
	GGISetCols();
    return 1;
}
void SetThemeInfo(const char * _theme = 0) {
	CStdString theme = (_theme)?_theme : GETSTR(CFG_KSTYLE_THEME);
	CStdString title;
	CStdString text;
	CStdString preview;
	CStdString icon;
	bool canImport = false;
	bool canImportIcons = false;
	if (theme.empty()) {
		text = "¯aden schemat nie jest aktywny";
		icon = "reg://IML16/" + inttostr(ICON_NO) + ".ico";
		title = "Wybierz schemat";
	} else {
		cXML xml;
		ICMessage(IMC_RESTORECURDIR);
		bool simple = !theme.Right(4).CompareNoCase(".xml");
		if (!xml.loadFile((KSTYLE_DIRNAME "\\" + theme + ( simple ? "" : "\\" KSTYLE_XMLFILE)).c_str())) {
			title = "Wybrany schemat nie istnieje!";
			icon = "reg://IML16/" + inttostr(ICON_ERROR) + ".ico";
		} else {
			xml.loadSource(xml.getContent("theme/info").c_str());
			title = theme;
			icon = "reg://IML16/" + inttostr( simple ? IDI_KSTYLE_COLORS : IDI_KSTYLE_LOGO) + ".ico";
			if (! xml.getText("author").empty())
				text += "Autor: <b>" + xml.getText("author") + "</b> ";
			if (! xml.getText("url").empty())
				text += xml.getText("url");
			if (! xml.getText("comment").empty())
				text += "<br/>" + xml.getText("comment");
			if (!simple && ! xml.getText("preview").empty()) {
				preview = "file://" KSTYLE_DIRNAME "\\" + theme + "\\" + xml.getText("preview");
			}
			canImportIcons = !simple;
			canImport = true;
		}
	}
	CStdString val;
	val = SetActParam(val, AP_TIPRICH, text);
	val = SetActParam(val, AP_TIPTITLE, title);
	val = SetActParam(val, AP_TIPICONURL, icon);
	val = SetActParam(val, AP_TIPIMAGEURL, preview);
	UIActionSetStatus(sUIAction(IMIG_CFG_PLUGS_KSTYLE , IMIA_CFG_PLUGS_KSTYLE_IMPORT), canImport ? 0 : -1 , ACTS_DISABLED);
	UIActionSetStatus(sUIAction(IMIG_CFG_PLUGS_KSTYLE , CFG_KSTYLE_LOADICONS | IMIB_CFG ), canImportIcons ? 0 : -1 , ACTS_DISABLED);
	UIActionSetText(sUIAction(IMIG_CFG_PLUGS_KSTYLE , IMIA_CFG_PLUGS_KSTYLE_THEMEINFO) , val);
}

int IPrepare() {
	IconRegister(IML_16 , IDI_KSTYLE_LOGO , Ctrl->hDll() , IDI_KSTYLE_LOGO);
	IconRegister(IML_16 , IDI_KSTYLE_COLORS , Ctrl->hDll() , IDI_KSTYLE_COLORS);
	UIGroupAdd(IMIG_CFG_PLUGS , IMIG_CFG_PLUGS_KSTYLE , 0 , "k.Style" , IDI_KSTYLE_LOGO);

	UIActionAdd(IMIG_CFG_PLUGS_KSTYLE , 0 , ACTT_GROUP , "");{
		UIActionCfgAddPluginInfoBox2(IMIG_CFG_PLUGS_KSTYLE, 
			"<b>kStyle</b> umo¿liwia zmianê wygl¹du Konnekta przez wczytywanie gotowych schematów kolorów i ikonek, które mo¿na pobraæ z \"Katalogu\" www.konnekt.info, lub z aktualizacji kUpdate." AP_TIPRICH_WIDTH "350"
			, "<br/><br/>Copyright ©2003,2004 <b>Stamina</b>"
			AP_TIPRICH_WIDTH "150", "res://dll/16000.ico", -3, 0, false);
		UIActionAdd(IMIG_CFG_PLUGS_KSTYLE , 0 , ACTT_SEPARATOR , "");

		UIActionAdd(IMIG_CFG_PLUGS_KSTYLE , IMIA_CFG_PLUGS_KSTYLE_EXPLOREDIR , ACTT_BUTTON | ACTSC_INLINE , "Katalog lokalny" AP_TIP "Schematy nale¿y wgrywaæ do katalogu Konnekt\\Themes." AP_IMGURL "res://ui/search.ico" AP_ICONSIZE "32", 0, 150, 42);
		UIActionAdd(IMIG_CFG_PLUGS_KSTYLE , IMIA_CFG_PLUGS_KSTYLE_KATALOGURL , ACTT_BUTTON , "Schematy do œci¹gniêcia" AP_TIP "Otwiera stronê www." AP_IMGURL "res://ui/url.ico" AP_ICONSIZE "32", 0, 150, 42);

		UIActionAdd(IMIG_CFG_PLUGS_KSTYLE , 0 , ACTT_GROUPEND);

		UIActionCfgAdd(IMIG_CFG_PLUGS_KSTYLE , 0 , ACTT_GROUP , "Wczytaj schemat");{
					
			UIActionCfgAdd(IMIG_CFG_PLUGS_KSTYLE , IMIA_CFG_PLUGS_KSTYLE_THEMEINFO , ACTT_TIPBUTTON | ACTSC_INLINE, "" AP_ICO ICONCH_INFO);
			UIActionCfgAdd(IMIG_CFG_PLUGS_KSTYLE , IMIA_CFG_PLUGS_KSTYLE_IMPORT , ACTT_BUTTON | ACTSC_INLINE, "" AP_IMGURL "res://ui/import.ico" AP_ICONSIZE "16" AP_TIP "Naciœnij, ¿eby za³adowaæ wybranych schemat. Zaimportowane kolory mo¿esz obejrzeæ w zak³adce ustawieñ - \"wygl¹d\".", 0, 0, 0, 24);
			UIActionCfgAdd(IMIG_CFG_PLUGS_KSTYLE , IMIA_CFG_PLUGS_KSTYLE_THEMELIST , ACTT_COMBO|ACTSCOMBO_LIST|ACTR_INIT|ACTSC_FULLWIDTH| ACTR_STATUS , "" CFGTIP "Wybierz schemat");
			//UIActionCfgAdd(IMIG_CFG_PLUGS_KSTYLE , IMIA_CFG_PLUGS_KSTYLE_INFO , ACTT_INFO , "" , 0, 0, 0, 0, -3);
			UIActionAdd(IMIG_CFG_PLUGS_KSTYLE , 0 , ACTT_COMMENT | ACTSC_INLINE , "Za³aduj: ");
			UIActionAdd(IMIG_CFG_PLUGS_KSTYLE , IMIB_CFG , ACTT_CHECK | ACTSC_INLINE , "Ikony" , CFG_KSTYLE_LOADICONS);
			UIActionAdd(IMIG_CFG_PLUGS_KSTYLE , IMIB_CFG , ACTT_CHECK | ACTSC_INLINE , "Kolory" , CFG_KSTYLE_LOADCOLORS);
			UIActionAdd(IMIG_CFG_PLUGS_KSTYLE , IMIB_CFG , ACTT_CHECK , "Czcionki" , CFG_KSTYLE_LOADFONTS);

		}UIActionCfgAdd(IMIG_CFG_PLUGS_KSTYLE , 0 , ACTT_GROUPEND);

		UIActionCfgAdd(IMIG_CFG_PLUGS_KSTYLE , 0 , ACTT_GROUP , "Schemat ikon");{
/*			if (ShowBits::checkBits(ShowBits::showInfoAdvanced)) {
				UIActionAdd(IMIG_CFG_PLUGS_KSTYLE , 0 , ACTT_TIPBUTTON | ACTSC_INLINE | ACTSBUTTON_ALIGNRIGHT, "" AP_ICO ICONCH_WARNING 
                    AP_TIPRICH "Je¿eli chcesz wczytaæ ikony, kolory i czcionki z ró¿nych schematów:"
					"<br/><b>1.</b> Odznacz <i>³aduj ikony</i>"
					"<br/><b>2.</b> Zaznacz <i>³aduj kolory/czcionki</i>"
					"<br/><b>3.</b> Wybierz schemat kolorów/czcionek z listy, mo¿esz go podejrzeæ w zak³adce \"Wygl¹d\"."
					"<br/><b>4.</b> Je¿eli chcesz dograæ inne kolory/czcionki - wróæ do punktu 2"
					"<br/><b>5.</b> Odznacz <i>³aduj kolory/czcionki</i>"
					"<br/><b>6.</b> Zaznacz <i>³aduj ikony</i>"
					"<br/><b>7.</b> Wybierz schemat ikon z listy"
					"<br/><b>8.</b> Naciœnij [Zastosuj], b¹dŸ [OK] w oknie ustawieñ i uruchom ponownie program"
					"<br/><br/><b>Pamiêtaj!</b> Schematy czcionek i kolorów wczytywane s¹ jednorazowo po wybraniu ich z listy. "
					"Schematy ikon ³adowane s¹ przy ka¿dym uruchomieniu programu. Dlatego zawsze po "
					, 0, 0, 0, 0, -3);

			}*/

			if (ShowBits::checkBits(ShowBits::showInfoAdvanced))
				UIActionAdd(IMIG_CFG_PLUGS_KSTYLE , 0 , ACTT_TIPBUTTON | ACTSC_INLINE | ACTSBUTTON_ALIGNRIGHT, "" AP_ICO ICONCH_WARNING 
					AP_TIP "Po zmianie schematu ikon mo¿e byæ konieczne ponowne uruchomienie programu!" , 0, 0, 0, 0, -3);
			UIActionCfgAdd(IMIG_CFG_PLUGS_KSTYLE , IMIB_CFG , ACTT_COMBO|ACTSCOMBO_LIST|ACTR_INIT|ACTSC_FULLWIDTH| ACTR_STATUS , "" CFGTIP "Wybierz schemat ikon" , CFG_KSTYLE_THEME);

		}UIActionCfgAdd(IMIG_CFG_PLUGS_KSTYLE , 0 , ACTT_GROUPEND);

		UIActionCfgAdd(IMIG_CFG_PLUGS_KSTYLE , 0 , ACTT_GROUP , "Zapisz");
		UIActionAdd(IMIG_CFG_PLUGS_KSTYLE , IMIA_CFG_PLUGS_KSTYLE_EXPORT , ACTT_BUTTON | ACTSC_FULLWIDTH , "Zapisz aktualne czcionki i kolory jako schemat" AP_IMGURL "res://ui/export.ico" AP_ICONSIZE "32", 0, 0, 42);
		UIActionCfgAdd(IMIG_CFG_PLUGS_KSTYLE , 0 , ACTT_GROUPEND);
	}


    GGIPrepare();
    return 1;
}

CStdString GetThemesComboValue(bool allowSimple) {
    CStdString dirs= "brak" CFGICO "116" CFGVALUE "\n";
    CStdString dir=(char*)ICMessage(IMC_RESTORECURDIR);
	dir+= KSTYLE_DIRNAME;
	dir+= "\\";
    WIN32_FIND_DATA fd;
    HANDLE hFile;
    BOOL found;
	found = ((hFile = FindFirstFile((dir + "*.*").c_str(),&fd))!=INVALID_HANDLE_VALUE);
    while (found)
    {
		if (strcmp("." , fd.cFileName) && strcmp(".." , fd.cFileName)) {
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
				&& !_access((dir + fd.cFileName + string("\\" KSTYLE_XMLFILE)).c_str() , 04)
				) {
					dirs+=string(fd.cFileName) + CFGICO "16000\n";
			}
			if (allowSimple && !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
				&& CStdString(fd.cFileName).Right(4).ToLower() == ".xml") {
					dirs+=string(fd.cFileName) + CFGICO "16001\n";
			}
		}
        found = FindNextFile(hFile , &fd);
    }
    FindClose(hFile); 
	return dirs;
}

int ActionCfgProc(sUIActionNotify_base * anBase) {
	sUIActionNotify_2params * an = (anBase->s_size>=sizeof(sUIActionNotify_2params))?static_cast<sUIActionNotify_2params*>(anBase):0;
	switch (anBase->act.id & ~IMIB_CFG) {
	case CFG_KSTYLE_THEME:
        if (an->code == ACTN_ACTION || an->code == ACTN_STATUS) {
			if (an->code == ACTN_ACTION) reLoad = true;

			//SetThemeInfo(UIActionCfgGetValue(an->act,0,0));
			//if (an->code == ACTN_ACTION) SetTheme(false);
		}
        else if (an->code == ACTN_DROP || an->code == ACTN_CREATE) {

            UIActionSetText(an->act , GetThemesComboValue(false));
        }
	    
	}
	GGIActionCfgProc(anBase);
	return 0;
}

ActionProc(sUIActionNotify_base * anBase) {
	sUIActionNotify_2params * an = (anBase->s_size>=sizeof(sUIActionNotify_2params))?static_cast<sUIActionNotify_2params*>(anBase):0;

	if ((anBase->act.id & IMIB_) == IMIB_CFG) return ActionCfgProc(anBase); 
	switch (anBase->act.id) {
		case IMIA_CFG_PLUGS_KSTYLE_EXPORT: 
			ACTIONONLY(an);
			ExportTheme();
			break;
		case IMIA_CFG_PLUGS_KSTYLE_EXPLOREDIR:
			ACTIONONLY(an);
			ShellExecute(0, "open", CStdString((char*)ICMessage(IMC_KONNEKTDIR)) + KSTYLE_DIRNAME, "", "", SW_SHOW);
			break;
		case IMIA_CFG_PLUGS_KSTYLE_KATALOGURL:
			ACTIONONLY(an);
			ShellExecute(0, "open", URL_KATALOG_KSTYLE, "", "", SW_SHOW);
			break;
		case IMIA_CFG_PLUGS_KSTYLE_IMPORT:
			ACTIONONLY(an);
			SetTheme(false);
			break;
		case IMIA_CFG_PLUGS_KSTYLE_THEMELIST:
			if (an->code == ACTN_ACTION || an->code == ACTN_STATUS || an->code == ACTN_CREATE) {
				SetThemeInfo(UIActionCfgGetValue(an->act,0,0));
			}
			if (an->code == ACTN_DROP || an->code == ACTN_CREATE) {
				UIActionSetText(an->act , GetThemesComboValue(true));
			}


	}
	return 0;
}



int __stdcall IMessageProc(sIMessage_base * msgBase) {
    sIMessage_2params * msg = (msgBase->s_size>=sizeof(sIMessage_2params))?static_cast<sIMessage_2params*>(msgBase):0;
    switch (msgBase->id) {
    case IM_PLUG_NET:        return NET_KSTYLE; // Zwracamy wartoœæ NET, która MUSI byæ ró¿na od 0 i UNIKATOWA!
	case IM_PLUG_TYPE:       return IMT_UI|IMT_CONFIG; // Zwracamy jakiego typu jest nasza wtyczka (które wiadomoœci bêdziemy obs³ugiwaæ)
    case IM_PLUG_VERSION:    return 0; // Wersja wtyczki tekstowo major.minor.release.build ...
    case IM_PLUG_SDKVERSION: return KONNEKT_SDK_V;  // Ta linijka jest wymagana!
    case IM_PLUG_SIG:        return (int)"kStyle"; // Sygnaturka wtyczki (krótka, kilkuliterowa nazwa)
    case IM_PLUG_CORE_V:     return (int)"W98"; // Wymagana wersja rdzenia
    case IM_PLUG_UI_V:       return 0; // Wymagana wersja UI
    case IM_PLUG_NAME:       return (int)"kStyle"; // Pe³na nazwa wtyczki
    case IM_PLUG_NETNAME:    return 0; // Nazwa obs³ugiwanej sieci (o ile jak¹œ sieæ obs³uguje)
    case IM_PLUG_INIT:       Plug_Init(msg->p1,msg->p2);return Init();
    case IM_PLUG_DEINIT:     Plug_Deinit(msg->p1,msg->p2);return DeInit();

    case IM_SETCOLS:     return ISetCols();

    case IM_UI_PREPARE:      return IPrepare();
    case IM_START:           return IStart();
    case IM_END:             return IEnd();

    case IM_UIACTION:        return ActionProc((sUIActionNotify_base*)msg->p1);

    /* Tutaj obs³ugujemy wszystkie pozosta³e wiadomoœci */
	
	case IM_CFG_CHANGED: 	
		if (reLoad) {
			GGSetIcons();
			SetTheme(true);
			reLoad = false;
		}
		return 0;
    default:
        if (Ctrl) Ctrl->setError(IMERROR_NORESULT);
        return 0;

 }
 return 0;
}

