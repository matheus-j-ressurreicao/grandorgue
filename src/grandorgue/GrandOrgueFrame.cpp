/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "GrandOrgueFrame.h"

#include <algorithm>
#include <wx/menu.h>
#include <wx/image.h>
#include <wx/filefn.h>
#include <wx/toolbar.h>
#include <wx/icon.h>
#include <wx/config.h>
#include <wx/progdlg.h>
#include <wx/stream.h>
#include <wx/wfstream.h>
#include <wx/zstream.h>
#include <wx/html/helpctrl.h>
#include <wx/splash.h>
#include "Images.h"
#include "GOGUIPanel.h"
#include "GOrgueFrameGeneral.h"
#include "GOrgueMeter.h"
#include "GOrgueMidi.h"
#include "GOrguePipe.h"
#include "GOrgueStop.h"
#include "GOrgueManual.h"
#include "GOrgueProperties.h"
#include "GOrgueSound.h"
#include "GrandOrgueID.h"
#include "GrandOrgueFile.h"
#include "GrandOrgue.h"
#include "OrganDocument.h"
#include "OrganView.h"
#include "SettingsDialog.h"
#include "SplashScreen.h"

IMPLEMENT_CLASS(GOrgueFrame, wxDocParentFrame)
BEGIN_EVENT_TABLE(GOrgueFrame, wxDocParentFrame)
	EVT_KEY_DOWN(GOrgueFrame::OnKeyCommand)
	EVT_COMMAND(0, wxEVT_METERS, GOrgueFrame::OnMeters)
	EVT_COMMAND(0, wxEVT_LOADFILE, GOrgueFrame::OnLoadFile)
    EVT_MENU_OPEN(GOrgueFrame::OnMenuOpen)
	EVT_MENU(ID_FILE_OPEN, GOrgueFrame::OnOpen)
	EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, GOrgueFrame::OnOpen)
	EVT_MENU(ID_FILE_RELOAD, GOrgueFrame::OnReload)
	EVT_MENU(ID_FILE_REVERT, GOrgueFrame::OnRevert)
	EVT_MENU(ID_FILE_PROPERTIES, GOrgueFrame::OnProperties)
	EVT_MENU(ID_FILE_LOAD, GOrgueFrame::OnLoad)
	EVT_MENU(ID_FILE_SAVE, GOrgueFrame::OnSave)
	EVT_MENU(ID_FILE_CACHE, GOrgueFrame::OnCache)
	EVT_MENU(ID_FILE_CACHE_DELETE, GOrgueFrame::OnCacheDelete)
	EVT_MENU(ID_AUDIO_PANIC, GOrgueFrame::OnAudioPanic)
	EVT_MENU(ID_AUDIO_RECORD, GOrgueFrame::OnAudioRecord)
	EVT_MENU(ID_AUDIO_MEMSET, GOrgueFrame::OnAudioMemset)
	EVT_MENU(ID_AUDIO_SETTINGS, GOrgueFrame::OnAudioSettings)
	EVT_MENU(wxID_HELP, GOrgueFrame::OnHelp)
	EVT_MENU(wxID_ABOUT, GOrgueFrame::OnHelpAbout)
	// New events for Volume, Polyphony, Memory Level, and Transpose
	EVT_MENU(ID_VOLUME, GOrgueFrame::OnSettingsVolume)
	EVT_MENU(ID_POLYPHONY, GOrgueFrame::OnSettingsPolyphony)
	EVT_MENU(ID_MEMORY, GOrgueFrame::OnSettingsMemory)
	EVT_MENU(ID_TRANSPOSE, GOrgueFrame::OnSettingsTranspose)
	// End
	EVT_MENU_RANGE(ID_PANEL_FIRST, ID_PANEL_LAST, GOrgueFrame::OnPanel)
	EVT_UPDATE_UI(ID_PANEL_MENU, GOrgueFrame::OnUpdatePanelMenu)
	EVT_SIZE(GOrgueFrame::OnSize)

	EVT_UPDATE_UI(wxID_SAVE, GOrgueFrame::OnUpdateLoaded)
	EVT_UPDATE_UI_RANGE(ID_FILE_RELOAD, ID_AUDIO_MEMSET, GOrgueFrame::OnUpdateLoaded)
END_EVENT_TABLE()

extern GOrgueSound* g_sound;

void GOrgueFrame::AddTool(wxMenu* menu, int id, const wxString& item, const wxString& helpString)
{
	menu->Append(id, item, wxEmptyString, wxITEM_NORMAL);
}

void GOrgueFrame::AddTool(wxMenu* menu, int id, const wxString& item, const wxString& helpString, const wxBitmap& toolbarImage, wxItemKind kind)
{
	menu->Append(id, item, wxEmptyString, kind);
        GetToolBar()->AddTool(id, item, toolbarImage, helpString, kind);
}

GOrgueFrame::GOrgueFrame(wxDocManager *manager, wxFrame *frame, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, const long type) :
	wxDocParentFrame(manager, frame, id, title, pos, size, type),
	m_panel_menu(NULL)
{
#ifdef _WIN32
	SetIcon(wxIcon(wxT("#101")));
#endif

	wxMenu *file_menu = new wxMenu;

	wxMenu *recent_menu = new wxMenu;
	m_docManager->FileHistoryUseMenu(recent_menu);
	m_docManager->FileHistoryLoad(*wxConfigBase::Get());

	wxToolBar* tb = CreateToolBar(wxNO_BORDER | wxTB_HORIZONTAL | wxTB_FLAT);
	tb->SetToolBitmapSize(wxSize(16, 16));

	AddTool(file_menu, ID_FILE_OPEN, _("&Open...\tCtrl+O"), _("Open"), GetImage_open());
	file_menu->Append(wxID_ANY, _("Open &Recent"), recent_menu);
	file_menu->AppendSeparator();
	AddTool(file_menu, ID_FILE_RELOAD, _("Re&load"), _("Reload"), GetImage_reload());
	AddTool(file_menu, ID_FILE_REVERT, _("Reset to &Defaults"));
	file_menu->AppendSeparator();
	AddTool(file_menu, ID_FILE_LOAD, _("&Import Settings..."));
	AddTool(file_menu, ID_FILE_SAVE, _("&Export Settings..."));
	file_menu->AppendSeparator();
	AddTool(file_menu, ID_FILE_CACHE, _("&Update Cache..."));
	AddTool(file_menu, ID_FILE_CACHE_DELETE, _("Delete &Cache..."));
	file_menu->AppendSeparator();
	AddTool(file_menu, wxID_SAVE, _("&Save\tCtrl+S"), _("Save"), GetImage_save());
	AddTool(file_menu, wxID_CLOSE, _("&Close"));
	AddTool(file_menu, ID_FILE_PROPERTIES, _("&Properties..."), _("Properties"), GetImage_properties());
	file_menu->AppendSeparator();
	AddTool(file_menu, wxID_EXIT, _("E&xit"));
	tb->AddSeparator();

	wxMenu *audio_menu = new wxMenu;
	AddTool(audio_menu, ID_AUDIO_RECORD, _("&Record...\tCtrl+R"), _("Record"), GetImage_record(), wxITEM_CHECK);
	AddTool(audio_menu, ID_AUDIO_MEMSET, _("&Memory Set\tShift"), _("Memory Set"), GetImage_set(), wxITEM_CHECK);
	audio_menu->AppendSeparator();
	AddTool(audio_menu, ID_AUDIO_PANIC, _("&Panic\tEscape"), _("Panic"), GetImage_panic());
	AddTool(audio_menu, ID_AUDIO_SETTINGS, _("&Settings..."), _("Audio Settings"), GetImage_settings());
	//tb->AddSeparator();

	wxMenu *help_menu = new wxMenu;
	//AddTool(help_menu, wxID_HELP, _("&Help\tF1"), _("Help"), GetImage_help, sizeof(GetImage_help));
	AddTool(help_menu, wxID_HELP, _("&Help\tF1"), _("Help"));
	AddTool(help_menu, wxID_ABOUT, _("&About"));
	tb->AddSeparator();
	// Changed Text to Icons to reduce screen space - Graham Goode Nov 2009
	wxMenu *settings_menu = new wxMenu;

	m_meters[0] = new GOrgueMeter(tb, ID_METER_AUDIO_SPIN, 3);
	m_meters[1] = new GOrgueMeter(tb, ID_METER_POLY_SPIN,  2);
	m_meters[2] = new GOrgueMeter(tb, ID_METER_FRAME_SPIN, 1);
	m_meters[3] = new GOrgueMeter(tb, ID_METER_TRANSPOSE_SPIN, 0);

	AddTool(settings_menu, ID_VOLUME, _("&Volume"), _("Volume"), GetImage_volume());
	tb->AddControl(m_meters[0]);
	AddTool(settings_menu, ID_POLYPHONY, _("&Polyphony"), _("Polyphony"), GetImage_polyphony());
	tb->AddControl(m_meters[1]);
	AddTool(settings_menu, ID_MEMORY, _("&Memory Level"), _("Memory Level"), GetImage_memory());
	tb->AddControl(m_meters[2]);
	AddTool(settings_menu, ID_TRANSPOSE, _("&Transpose"), _("Transpose"), GetImage_transpose());
	tb->AddControl(m_meters[3]);


	wxMenuBar *menu_bar = new wxMenuBar;
	menu_bar->Append(file_menu, _("&File"));
	menu_bar->Append(audio_menu, _("&Audio"));
	menu_bar->Append(help_menu, _("&Help"));
	SetMenuBar(menu_bar);
	tb->Realize();

	SetClientSize(880, 495);	// default minimal size
	Center(wxBOTH);
	SetAutoLayout(true);
}

GOrgueFrame::~GOrgueFrame()
{
}

void GOrgueFrame::OnPanel(wxCommandEvent& event)
{
	OrganDocument* doc = (OrganDocument*)m_docManager->GetCurrentDocument();
	GrandOrgueFile* organfile = doc ? doc->GetOrganFile() : NULL;
	unsigned no = event.GetId() - ID_PANEL_FIRST + 1;
	if (!organfile || organfile->GetPanelCount() <= no)
		return;
	wxWindow* win = organfile->GetPanel(no)->GetParentWindow();
	if (win)
	{
		win->Raise();
		win->SetFocus();
	}
	else
	{
		OrganView* view = new OrganView(no);
		view->SetDocument(doc);
		view->OnCreate(doc, 0);
	}
}

void GOrgueFrame::OnUpdatePanelMenu(wxUpdateUIEvent& event)
{
	OrganDocument* doc = (OrganDocument*)m_docManager->GetCurrentDocument();
	GrandOrgueFile* organfile = doc ? doc->GetOrganFile() : NULL;
	unsigned panelcount = std::min (organfile ? organfile->GetPanelCount() - 1 : 0, (unsigned)(ID_PANEL_LAST - ID_PANEL_FIRST));
	while (m_panel_menu->GetMenuItemCount() < panelcount)
		m_panel_menu->AppendCheckItem(ID_PANEL_FIRST + m_panel_menu->GetMenuItemCount(), wxT("_"));
	
	while (m_panel_menu->GetMenuItemCount() > panelcount)
	{
		m_panel_menu->Destroy(m_panel_menu->FindItemByPosition(m_panel_menu->GetMenuItemCount() - 1));
	}

	for(unsigned i = 0; i < panelcount; i++)
	{
		wxMenuItem* item = m_panel_menu->FindItemByPosition(i);
		item->SetItemLabel(organfile->GetPanel(i + 1)->GetName());
		item->Check(organfile->GetPanel(i + 1)->GetParentWindow() ? true : false);
	}
}

void GOrgueFrame::OnSize(wxSizeEvent& event)
{
	wxWindow *child = (wxWindow *)NULL;
        for ( wxWindowList::compatibility_iterator node = GetChildren().GetFirst(); node; node = node->GetNext() )
        {
		wxWindow *win = node->GetData();
		if ( !win->IsTopLevel() && !IsOneOfBars(win) )
		{
			child = win;
		}
	}
	if (child)
		child->SetSize(0, 0, GetClientSize().GetWidth(), GetClientSize().GetHeight());
}


void GOrgueFrame::OnMeters(wxCommandEvent& event)
{
	int n = event.GetInt();
	m_meters[0]->SetValue(0, n);
	m_meters[0]->SetValue(1, n >> 8);
	m_meters[1]->SetValue(0, n >> 16);
}

void GOrgueFrame::OnUpdateLoaded(wxUpdateUIEvent& event)
{
	if (g_sound)
	{
		if (event.GetId() == ID_AUDIO_RECORD)
			event.Check(g_sound->IsRecording());
		else if (event.GetId() == ID_AUDIO_MEMSET)
			event.Check(g_sound->GetMidi().SetterActive());
	}
	GrandOrgueFile* organfile = NULL;
	if (m_docManager->GetCurrentDocument())
		organfile = ((OrganDocument*)m_docManager->GetCurrentDocument())->GetOrganFile();
	event.Enable(organfile && (event.GetId() == ID_FILE_REVERT ? organfile->IsCustomized() : true));
}

void GOrgueFrame::OnLoadFile(wxCommandEvent& event)
{
    if (!IsEnabled())
        return;
    m_docManager->CreateDocument(event.GetString(), wxDOC_SILENT);
}

void GOrgueFrame::OnOpen(wxCommandEvent& event)
{
	if (event.GetId() == ID_FILE_OPEN)
	{
		wxFileName fn = wxFileName::GetCwd();
		fn.AppendDir(wxT("organs"));
		GetDocumentManager()->SetLastDirectory(wxConfig::Get()->Read(wxT("organPath"), fn.GetPath()));
		ProcessCommand(wxID_OPEN);
		if (m_docManager->GetCurrentDocument() && ((OrganDocument*)m_docManager->GetCurrentDocument())->GetOrganFile())
		{
			wxConfig::Get()->Write(wxT("organPath"), GetDocumentManager()->GetLastDirectory());
		}
	}
	else
		event.Skip();
}

void GOrgueFrame::OnLoad(wxCommandEvent& event)
{
	OrganDocument* doc = (OrganDocument*)m_docManager->GetCurrentDocument();
	if (!doc)
		return;

	wxFileDialog dlg(this, _("Import Settings"), wxConfig::Get()->Read(wxT("cmbPath"), ::wxGetApp().m_path + wxT("My Organs")), wxEmptyString, _("Settings files (*.cmb)|*.cmb"), wxOPEN | wxFILE_MUST_EXIST);
	if (dlg.ShowModal() == wxID_OK)
	{
		wxConfig::Get()->Write(wxT("cmbPath"), dlg.GetDirectory());
		wxString file = doc->GetOrganFile()->GetODFFilename();
		doc->DoOpenDocument(file, dlg.GetPath());
	}
}

void GOrgueFrame::OnSave(wxCommandEvent& event)
{
	OrganDocument* doc = (OrganDocument*)m_docManager->GetCurrentDocument();
	if (!doc)
		return;

	wxFileDialog dlg(this, _("Export Settings"), wxConfig::Get()->Read(wxT("cmbPath"), ::wxGetApp().m_path + wxT("My Organs")), wxEmptyString, _("Settings files (*.cmb)|*.cmb"), wxSAVE | wxOVERWRITE_PROMPT);
	if (dlg.ShowModal() == wxID_OK)
	{
		wxConfig::Get()->Write(wxT("cmbPath"), dlg.GetDirectory());
		doc->DoSaveDocument(dlg.GetPath());
		doc->Modify(false);
	}
}

wxString formatSize(wxLongLong& size)
{
    double n = (double)size.ToLong();
    wxChar sizes[][3] = {wxTRANSLATE("KB"), wxTRANSLATE("MB"), wxTRANSLATE("GB"), wxTRANSLATE("TB")};
    int i;

    for (i = 0; i < 3; i++)
    {
        n /= 1024.0;
        if (n < 1024.0)
            break;
    }
    return wxString::Format(wxT("%.2f %s"), n, wxGetTranslation(sizes[i]));
}

void GOrgueFrame::OnCache(wxCommandEvent& event)
{
	OrganDocument* doc = (OrganDocument*)m_docManager->GetCurrentDocument();
	if (!doc)
		return;
	GrandOrgueFile* organfile = doc->GetOrganFile();

	/* Figure out how many pipes there are */
	unsigned nb_pipes = 0;
	unsigned nb_saved_pipes = 0;
	for (unsigned i = organfile->GetFirstManualIndex(); i <= organfile->GetManualAndPedalCount(); i++)
		for (unsigned j = 0; j < organfile->GetManual(i)->GetStopCount(); j++)
			nb_pipes += organfile->GetManual(i)->GetStop(j)->GetPipeCount();

	wxString filename = organfile->GetODFFilename() + wxT(".cache");
	wxFileOutputStream file(filename);
	wxZlibOutputStream zout(file);

	wxProgressDialog dlg(_("Creating sample cache"), wxEmptyString, 32768, 0, wxPD_AUTO_HIDE | wxPD_CAN_ABORT | wxPD_APP_MODAL | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME);

	/* Save pipes to cache */
	bool cache_save_ok = true;
	int magic = GRANDORGUE_CACHE_MAGIC;
	zout.Write(&magic, sizeof(magic));
	if (zout.LastWrite() != sizeof(magic))
		cache_save_ok = false;
	
	unsigned char hash[20];
	organfile->GenerateCacheHash(hash);
	zout.Write(hash, sizeof(hash));
	if (zout.LastWrite() != sizeof(hash))
		cache_save_ok = false;

	for (unsigned i = organfile->GetFirstManualIndex(); cache_save_ok && i <= organfile->GetManualAndPedalCount(); i++)
		for (unsigned j = 0; cache_save_ok && j < organfile->GetManual(i)->GetStopCount(); j++)
			for (unsigned k = 0; cache_save_ok && k < organfile->GetManual(i)->GetStop(j)->GetPipeCount(); k++)
			{
				GOrguePipe* pipe = organfile->GetManual(i)->GetStop(j)->GetPipe(k);
				if (!pipe->SaveCache(&zout))
				{
					cache_save_ok = false;
					wxLogError(_("Save of %s to the cache failed"), pipe->GetFilename().c_str());
				}
				nb_saved_pipes++;
				dlg.Update
					((nb_saved_pipes << 15) / (nb_pipes + 1)
					,pipe->GetFilename()
					);
			}
	zout.Close();
	file.Close();
	if (!cache_save_ok)
	{
		wxLogError(_("Creating the cache failed"));
		wxMessageBox(_("Creating the cache failed"), _("Error"), wxOK | wxICON_ERROR, NULL);
		wxRemoveFile(filename);
	}
}

void GOrgueFrame::OnCacheDelete(wxCommandEvent& event)
{
	OrganDocument* doc = (OrganDocument*)m_docManager->GetCurrentDocument();
	if (!doc)
		return;

	wxString filename = doc->GetOrganFile()->GetODFFilename() + wxT(".cache");
	wxRemoveFile(filename);
}

void GOrgueFrame::OnReload(wxCommandEvent& event)
{
	ProcessCommand(wxID_FILE1);
}

void GOrgueFrame::OnRevert(wxCommandEvent& event)
{
	OrganDocument* doc = (OrganDocument*)m_docManager->GetCurrentDocument();
	if (doc && doc->GetOrganFile() && ::wxMessageBox(_("Any customizations you have saved to this\norgan definition file will be lost!\n\nReset to defaults and reload?"), wxT(APP_NAME), wxYES_NO | wxICON_EXCLAMATION, this) == wxYES)
	{
		wxLog::EnableLogging(false);
		wxFileConfig cfg(wxEmptyString, wxEmptyString, doc->GetOrganFile()->GetODFFilename(), wxEmptyString, wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_NO_ESCAPE_CHARACTERS);
		wxLog::EnableLogging(true);
		m_docManager->GetCurrentDocument()->Modify(false);
		doc->GetOrganFile()->Revert(cfg);
		ProcessCommand(wxID_FILE1);
	}
}

void GOrgueFrame::OnProperties(wxCommandEvent& event)
{
	OrganDocument* doc = (OrganDocument*)m_docManager->GetCurrentDocument();
	GOrgueProperties dlg(doc->GetOrganFile(), this);
	dlg.ShowModal();
}

void GOrgueFrame::OnAudioPanic(wxCommandEvent& WXUNUSED(event))
{
	if (!g_sound)
		return;
	g_sound->ResetSound();
}

void GOrgueFrame::OnAudioRecord(wxCommandEvent& WXUNUSED(event))
{
	if (!g_sound)
		return;

	if (g_sound->IsRecording())
		g_sound->StopRecording();
	else
		g_sound->StartRecording();
}

void GOrgueFrame::OnAudioMemset(wxCommandEvent& WXUNUSED(event))
{
	if (!GetDocumentManager()->GetCurrentDocument() || !g_sound)
		return;
	g_sound->GetMidi().ToggleSetter();
}

void GOrgueFrame::OnAudioSettings(wxCommandEvent& WXUNUSED(event))
{
	::wxLogDebug(_("settingsdialog.."));
	SettingsDialog dialog(this);
	::wxLogDebug(_("success"));
	dialog.ShowModal();
	g_sound->GetMidi().UpdateOrganMIDI();
}

void GOrgueFrame::OnHelp(wxCommandEvent& event)
{
    ::wxGetApp().m_help->Display(_("User Interface"));
}

void GOrgueFrame::OnSettingsVolume(wxCommandEvent& event)
{
	//
}

void GOrgueFrame::OnSettingsPolyphony(wxCommandEvent& event)
{
	//
}

void GOrgueFrame::OnSettingsMemory(wxCommandEvent& event)
{
	m_meters[2]->OnFrame(event);
}

void GOrgueFrame::OnSettingsTranspose(wxCommandEvent& event)
{
	m_meters[0]->OnTranspose(event);
}

void GOrgueFrame::OnHelpAbout(wxCommandEvent& event)
{
	DoSplash(false);
}

void GOrgueFrame::DoSplash(bool timeout)
{
	wxSplashScreenModal* splash = new wxSplashScreenModal(GetImage_Splash(), timeout ? wxSPLASH_CENTRE_ON_SCREEN | wxSPLASH_TIMEOUT : wxSPLASH_CENTRE_ON_SCREEN | wxSPLASH_NO_TIMEOUT, 3000, this, wxID_ANY);
	if (!timeout)
		splash->ShowModal();
}


void GOrgueFrame::OnMenuOpen(wxMenuEvent& event)
{
    DoMenuUpdates(event.GetMenu());
    event.Skip();
}

void GOrgueFrame::ChangeSetter(unsigned position)
{
	OrganDocument* doc = (OrganDocument*)m_docManager->GetCurrentDocument();
	if (doc && doc->GetOrganFile())
		doc->GetOrganFile()->GetFrameGeneral(position)->Push();
}

void GOrgueFrame::OnKeyCommand(wxKeyEvent& event)
{
	if (g_sound && g_sound->GetMidi().SetterActive() ^ event.ShiftDown())
	{
		ProcessCommand(ID_AUDIO_MEMSET);
		UpdateWindowUI();
	}

	int k = event.GetKeyCode();
	if ( !event.AltDown())
	{

		GOrgueMeter* meter = m_meters[2];
		switch(k)
		{
			case WXK_ESCAPE:
			{
				ProcessCommand(ID_AUDIO_PANIC);
				break;
			}
			case WXK_LEFT:
			{
				meter->SetValue(meter->GetValue() - 1);
				break;
			}
			case WXK_DOWN:
			{
				ChangeSetter(meter->GetValue() - 1);
				break;
			}
			case WXK_RIGHT:
			{
				meter->SetValue(meter->GetValue() + 1);
				break;
			}
		}
	}
	event.Skip();
}
