/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2018 GrandOrgue contributors (see AUTHORS)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "GOrgueGeneral.h"

#include <wx/intl.h>

GOrgueGeneral::GOrgueGeneral(GOrgueCombinationDefinition& general_template, GrandOrgueFile* organfile, bool is_setter):
	GOrguePushbutton(organfile),
	m_general(general_template, organfile, is_setter)
{
}

void GOrgueGeneral::Load(GOrgueConfigReader& cfg, wxString group)
{
	m_general.Load(cfg, group);
	GOrguePushbutton::Load(cfg, group);
}

void GOrgueGeneral::Push()
{
	m_general.Push();
}

GOrgueFrameGeneral& GOrgueGeneral::GetGeneral()
{
	return m_general;
}

wxString GOrgueGeneral::GetMidiType()
{
	return _("General");
}
