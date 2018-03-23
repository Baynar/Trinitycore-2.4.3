/*
 * Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/* ScriptData
SDName: Item_Scripts
SD%Complete: 100
SDComment: Items for a range of different items. See content below (in script)
SDCategory: Items
EndScriptData */

/* ContentData
item_nether_wraith_beacon(i31742)   Summons creatures for quest Becoming a Spellfire Tailor (q10832)
item_flying_machine(i34060, i34061)  Engineering crafted flying machines
item_gor_dreks_ointment(i30175)     Protecting Our Own(q10488)
item_only_for_flight                Items which should only useable while flying
EndContentData */

#include "ScriptMgr.h"
#include "GameObject.h"
#include "Item.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "Spell.h"
#include "SpellMgr.h"
#include "TemporarySummon.h"

/*#####
# item_only_for_flight
#####*/

enum OnlyForFlight
{
    SPELL_ARCANE_CHARGES    = 45072
};

class item_only_for_flight : public ItemScript
{
    public:
        item_only_for_flight() : ItemScript("item_only_for_flight") { }

        bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
        {
            uint32 itemId = item->GetEntry();
            bool disabled = false;

            //for special scripts
            switch (itemId)
            {
                case 24538:
                    if (player->GetAreaId() != 3628)
                        disabled = true;
                    break;
                case 34489:
                    if (player->GetZoneId() != 4080)
                        disabled = true;
                    break;
                case 34475:
                    if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(SPELL_ARCANE_CHARGES))
                        Spell::SendCastResult(player, spellInfo, 1, SPELL_FAILED_NOT_ON_GROUND);
                    break;
            }

            // allow use in flight only
            if (player->IsInFlight() && !disabled)
                return false;

            // error
            player->SendEquipError(EQUIP_ERR_CANT_DO_RIGHT_NOW, item, nullptr);
            return true;
        }
};

/*#####
# item_nether_wraith_beacon
#####*/

class item_nether_wraith_beacon : public ItemScript
{
    public:
        item_nether_wraith_beacon() : ItemScript("item_nether_wraith_beacon") { }

        bool OnUse(Player* player, Item* /*item*/, SpellCastTargets const& /*targets*/) override
        {
            if (player->GetQuestStatus(10832) == QUEST_STATUS_INCOMPLETE)
            {
                if (Creature* nether = player->SummonCreature(22408, player->GetPositionX(), player->GetPositionY()+20, player->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 180000))
                    nether->AI()->AttackStart(player);

                if (Creature* nether = player->SummonCreature(22408, player->GetPositionX(), player->GetPositionY()-20, player->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 180000))
                    nether->AI()->AttackStart(player);
            }
            return false;
        }
};

/*#####
# item_gor_dreks_ointment
#####*/

class item_gor_dreks_ointment : public ItemScript
{
    public:
        item_gor_dreks_ointment() : ItemScript("item_gor_dreks_ointment") { }

        bool OnUse(Player* player, Item* item, SpellCastTargets const& targets) override
        {
            if (targets.GetUnitTarget() && targets.GetUnitTarget()->GetTypeId() == TYPEID_UNIT &&
                targets.GetUnitTarget()->GetEntry() == 20748 && !targets.GetUnitTarget()->HasAura(32578))
                return false;

            player->SendEquipError(EQUIP_ERR_CANT_DO_RIGHT_NOW, item, nullptr);
            return true;
        }
};

// Only used currently for
// 19169: Nightfall
class item_generic_limit_chance_above_60 : public ItemScript
{
    public:
        item_generic_limit_chance_above_60() : ItemScript("item_generic_limit_chance_above_60") { }

        bool OnCastItemCombatSpell(Player* /*player*/, Unit* victim, SpellInfo const* /*spellInfo*/, Item* /*item*/) override
        {
            // spell proc chance gets severely reduced on victims > 60 (formula unknown)
            if (victim->getLevel() > 60)
            {
                // gives ~0.1% proc chance at lvl 70
                float const lvlPenaltyFactor = 9.93f;
                float const failureChance = (victim->getLevel() - 60) * lvlPenaltyFactor;

                // base ppm chance was already rolled, only roll success chance
                return !roll_chance_f(failureChance);
            }

            return true;
        }
};

void AddSC_item_scripts()
{
    new item_only_for_flight();
    new item_nether_wraith_beacon();
    new item_gor_dreks_ointment();
    new item_generic_limit_chance_above_60();
}
