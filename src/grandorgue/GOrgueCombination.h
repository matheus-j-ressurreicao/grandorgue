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

#ifndef GORGUECOMBINATION_H
#define GORGUECOMBINATION_H

#include <vector>

class GOrgueCombinationDefinition;
class GrandOrgueFile;

class GOrgueCombination
{
private:
	GrandOrgueFile* m_OrganFile;
protected:
	GOrgueCombinationDefinition& m_Template;
	std::vector<int> m_State;
	bool m_Protected;

	void UpdateState();
	virtual bool PushLocal();
public:
	GOrgueCombination(GOrgueCombinationDefinition& combination_template, GrandOrgueFile* organfile);
	virtual ~GOrgueCombination();
	int GetState(unsigned no);
	void SetState(unsigned no, int value);
	void Copy(GOrgueCombination* combination);
	void Clear();
	GOrgueCombinationDefinition* GetTemplate();
};

#endif
