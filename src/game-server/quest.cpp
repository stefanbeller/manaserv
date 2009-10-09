/*
 *  The Mana World Server
 *  Copyright 2007 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <cassert>
#include <list>
#include <map>
#include <string>

#include "game-server/quest.hpp"

#include "game-server/accountconnection.hpp"
#include "game-server/character.hpp"
#include "game-server/eventlistener.hpp"
#include "utils/logger.h"

typedef std::list< QuestCallback > QuestCallbacks;
typedef std::map< std::string, QuestCallbacks > PendingVariables;

struct PendingQuest
{
    Character *character;
    PendingVariables variables;
};

typedef std::map< int, PendingQuest > PendingQuests;

static PendingQuests pendingQuests;

bool getQuestVar(Character *ch, const std::string &name, std::string &value)
{
    std::map< std::string, std::string >::iterator
        i = ch->questCache.find(name);
    if (i == ch->questCache.end()) return false;
    value = i->second;
    return true;
}

void setQuestVar(Character *ch, const std::string &name,
                 const std::string &value)
{
    std::map< std::string, std::string >::iterator
        i = ch->questCache.lower_bound(name);
    if (i == ch->questCache.end() || i->first != name)
    {
        ch->questCache.insert(i, std::make_pair(name, value));
    }
    else if (i->second == value)
    {
        return;
    }
    else
    {
        i->second = value;
    }
    accountHandler->updateQuestVar(ch, name, value);
}

/**
 * Listener for deleting related quests when a character disappears.
 */
struct QuestDeathListener: EventDispatch
{
    static void partialRemove(const EventListener *, Thing *);

    static void fullRemove(const EventListener *, Character *);

    QuestDeathListener()
    {
        removed = &partialRemove;
        disconnected = &fullRemove;
    }
};

static QuestDeathListener questDeathDummy;
static EventListener questDeathListener(&questDeathDummy);

void QuestDeathListener::partialRemove(const EventListener *, Thing *t)
{
    int id = static_cast< Character * >(t)->getDatabaseID();
    PendingVariables &variables = pendingQuests[id].variables;
    // Remove all the callbacks, but do not remove the variable names.
    for (PendingVariables::iterator i = variables.begin(),
         i_end = variables.end(); i != i_end; ++i)
    {
        i->second.clear();
    }
    // The listener is kept in case a fullRemove is needed later.
}

void QuestDeathListener::fullRemove(const EventListener *, Character *ch)
{
    ch->removeListener(&questDeathListener);
    // Remove anything related to this character.
    pendingQuests.erase(ch->getDatabaseID());
}

void recoverQuestVar(Character *ch, const std::string &name,
                     const QuestCallback &f)
{
    assert(ch->questCache.find(name) == ch->questCache.end());
    int id = ch->getDatabaseID();
    PendingQuests::iterator i = pendingQuests.lower_bound(id);
    if (i == pendingQuests.end() || i->first != id)
    {
        i = pendingQuests.insert(i, std::make_pair(id, PendingQuest()));
        i->second.character = ch;
        /* Register a listener, because we cannot afford to get invalid
           pointers, when we finally recover the variable. */
        ch->addListener(&questDeathListener);
    }
    i->second.variables[name].push_back(f);
    accountHandler->requestQuestVar(ch, name);
}

void recoveredQuestVar(int id, const std::string &name,
                       const std::string &value)
{
    PendingQuests::iterator i = pendingQuests.find(id);
    if (i == pendingQuests.end()) return;

    Character *ch = i->second.character;
    ch->removeListener(&questDeathListener);

    PendingVariables &variables = i->second.variables;
    PendingVariables::iterator j = variables.find(name);
    if (j == variables.end())
    {
        LOG_ERROR("Account server recovered an unexpected quest variable.");
        return;
    }

    ch->questCache[name] = value;

    // Call the registered callbacks.
    for (QuestCallbacks::const_iterator k = j->second.begin(),
         k_end = j->second.end(); k != k_end; ++k)
    {
        k->handler(ch, name, value, k->data);
    }

    variables.erase(j);
    if (variables.empty())
    {
        pendingQuests.erase(i);
    }
}
