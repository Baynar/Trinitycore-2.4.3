/*
 * Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
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

#include "scholomance.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"

enum Emotes
{
    EMOTE_FRENZY                 = 0
};

enum Spells
{
    SPELL_FLAMESTRIKE            = 18399,
    SPELL_BLAST_WAVE             = 16046,
    SPELL_FIRE_SHIELD            = 19626,
    SPELL_FRENZY                 = 8269  // 28371
};

enum Events
{
    EVENT_FIRE_SHIELD = 1,
    EVENT_BLAST_WAVE,
    EVENT_FRENZY
};

class boss_vectus : public CreatureScript
{
public:
    boss_vectus() : CreatureScript("boss_vectus") { }

    struct boss_vectusAI : public ScriptedAI
    {
        boss_vectusAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            events.Reset();
            frenzy = false;
        }

        void JustEngagedWith(Unit* /*who*/) override
        {
            events.ScheduleEvent(EVENT_FIRE_SHIELD, Seconds(2));
            events.ScheduleEvent(EVENT_BLAST_WAVE, Seconds(14));
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage) override
        {
            if (!frenzy && me->HealthBelowPctDamaged(25, damage))
            {
                frenzy = true;
                DoCastSelf(SPELL_FRENZY);
                Talk(EMOTE_FRENZY);
                events.ScheduleEvent(EVENT_FRENZY, Seconds(24));
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_FIRE_SHIELD:
                        DoCastSelf(SPELL_FIRE_SHIELD);
                        events.Repeat(Minutes(1) + Seconds(30));
                        break;
                    case EVENT_BLAST_WAVE:
                        DoCastAOE(SPELL_BLAST_WAVE);
                        events.Repeat(Seconds(12));
                        break;
                    case EVENT_FRENZY:
                        DoCastSelf(SPELL_FRENZY);
                        Talk(EMOTE_FRENZY);
                        events.Repeat(Seconds(24));
                        break;
                    default:
                        break;
                }

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;
            }

            DoMeleeAttackIfReady();
        }

        private:
            EventMap events;
            bool frenzy;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetScholomanceAI<boss_vectusAI>(creature);
    }
};

void AddSC_boss_vectus()
{
    new boss_vectus();
}
