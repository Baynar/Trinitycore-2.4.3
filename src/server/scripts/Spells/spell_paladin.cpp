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

/*
 * Scripts for spells with SPELLFAMILY_PALADIN and SPELLFAMILY_GENERIC spells used by paladin players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_pal_".
 */

#include "ScriptMgr.h"
#include "Containers.h"
#include "GameTime.h"
#include "Group.h"
#include "Player.h"
#include "Random.h"
#include "SpellAuraEffects.h"
#include "SpellHistory.h"
#include "SpellMgr.h"
#include "SpellScript.h"

enum PaladinSpells
{
    SPELL_PALADIN_HOLY_SHOCK_R1                  = 20473,
    SPELL_PALADIN_HOLY_SHOCK_R1_DAMAGE           = 25912,
    SPELL_PALADIN_HOLY_SHOCK_R1_HEALING          = 25914,
    SPELL_PALADIN_ILLUMINATION_ENERGIZE          = 20272,

    SPELL_PALADIN_BLESSING_OF_LOWER_CITY_DRUID   = 37878,
    SPELL_PALADIN_BLESSING_OF_LOWER_CITY_PALADIN = 37879,
    SPELL_PALADIN_BLESSING_OF_LOWER_CITY_PRIEST  = 37880,
    SPELL_PALADIN_BLESSING_OF_LOWER_CITY_SHAMAN  = 37881,

    SPELL_PALADIN_EYE_FOR_AN_EYE_DAMAGE          = 25997,

    SPELL_PALADIN_FORBEARANCE                    = 25771,

    SPELL_PALADIN_ITEM_HEALING_TRANCE            = 37706,

    SPELL_PALADIN_JUDGEMENT_OF_LIGHT             = 20185,

    SPELL_PALADIN_JUDGEMENT_OF_LIGHT_HEAL        = 20267,
    SPELL_PALADIN_JUDGEMENT_OF_WISDOM_MANA       = 20268,

    SPELL_PALADIN_RIGHTEOUS_DEFENSE_TAUNT        = 31790,

    SPELL_PALADIN_SEAL_OF_RIGHTEOUSNESS          = 25742,

    SPELL_PALADIN_JUDGEMENTS_OF_THE_WISE_MANA    = 31930,

    SPELL_PALADIN_HOLY_POWER_ARMOR               = 28790,
    SPELL_PALADIN_HOLY_POWER_ATTACK_POWER        = 28791,
    SPELL_PALADIN_HOLY_POWER_SPELL_POWER         = 28793,
    SPELL_PALADIN_HOLY_POWER_MP5                 = 28795,

    SPELL_PALADIN_HOLY_VENGEANCE                 = 31803,
    SPELL_PALADIN_SEAL_OF_VENGEANCE_DAMAGE       = 42463,

    SPELL_PALADIN_SPIRITUAL_ATTUNEMENT_MANA      = 31786,

    SPELL_PALADIN_ENDURING_LIGHT                 = 40471,
    SPELL_PALADIN_ENDURING_JUDGEMENT             = 40472
};

enum PaladinSpellIcons
{
    PALADIN_ICON_ID_HAMMER_OF_THE_RIGHTEOUS      = 3023
};

// 37877 - Blessing of Faith
class spell_pal_blessing_of_faith : public SpellScriptLoader
{
    public:
        spell_pal_blessing_of_faith() : SpellScriptLoader("spell_pal_blessing_of_faith") { }

        class spell_pal_blessing_of_faith_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_blessing_of_faith_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo(
                {
                    SPELL_PALADIN_BLESSING_OF_LOWER_CITY_DRUID,
                    SPELL_PALADIN_BLESSING_OF_LOWER_CITY_PALADIN,
                    SPELL_PALADIN_BLESSING_OF_LOWER_CITY_PRIEST,
                    SPELL_PALADIN_BLESSING_OF_LOWER_CITY_SHAMAN
                });
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* unitTarget = GetHitUnit())
                {
                    uint32 spell_id = 0;
                    switch (unitTarget->getClass())
                    {
                        case CLASS_DRUID:
                            spell_id = SPELL_PALADIN_BLESSING_OF_LOWER_CITY_DRUID;
                            break;
                        case CLASS_PALADIN:
                            spell_id = SPELL_PALADIN_BLESSING_OF_LOWER_CITY_PALADIN;
                            break;
                        case CLASS_PRIEST:
                            spell_id = SPELL_PALADIN_BLESSING_OF_LOWER_CITY_PRIEST;
                            break;
                        case CLASS_SHAMAN:
                            spell_id = SPELL_PALADIN_BLESSING_OF_LOWER_CITY_SHAMAN;
                            break;
                        default:
                            return; // ignore for non-healing classes
                    }
                    Unit* caster = GetCaster();
                    caster->CastSpell(caster, spell_id, true);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_pal_blessing_of_faith_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pal_blessing_of_faith_SpellScript();
        }
};

// 33695 - Exorcism and Holy Wrath Damage
class spell_pal_exorcism_and_holy_wrath_damage : public SpellScriptLoader
{
    public:
        spell_pal_exorcism_and_holy_wrath_damage() : SpellScriptLoader("spell_pal_exorcism_and_holy_wrath_damage") { }

        class spell_pal_exorcism_and_holy_wrath_damage_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_exorcism_and_holy_wrath_damage_AuraScript);

            void HandleEffectCalcSpellMod(AuraEffect const* aurEff, SpellModifier*& spellMod)
            {
                if (!spellMod)
                {
                    spellMod = new SpellModifier(aurEff->GetBase());
                    spellMod->op = SPELLMOD_DAMAGE;
                    spellMod->type = SPELLMOD_FLAT;
                    spellMod->spellId = GetId();
                    spellMod->mask[1] = 0x200002;
                }

                spellMod->value = aurEff->GetAmount();
            }

            void Register() override
            {
                DoEffectCalcSpellMod += AuraEffectCalcSpellModFn(spell_pal_exorcism_and_holy_wrath_damage_AuraScript::HandleEffectCalcSpellMod, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_exorcism_and_holy_wrath_damage_AuraScript();
        }
};

// -9799 - Eye for an Eye
class spell_pal_eye_for_an_eye : public SpellScriptLoader
{
    public:
        spell_pal_eye_for_an_eye() : SpellScriptLoader("spell_pal_eye_for_an_eye") { }

        class spell_pal_eye_for_an_eye_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_eye_for_an_eye_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_PALADIN_EYE_FOR_AN_EYE_DAMAGE });
            }

            void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();
                DamageInfo* damageInfo = eventInfo.GetDamageInfo();
                if (!damageInfo || !damageInfo->GetDamage())
                    return;

                // return damage % to attacker but < 50% own total health
                int32 damage = std::min(CalculatePct(static_cast<int32>(damageInfo->GetDamage()), aurEff->GetAmount()), static_cast<int32>(GetTarget()->GetMaxHealth()) / 2);
                CastSpellExtraArgs args(aurEff);
                args.AddSpellBP0(damage);
                GetTarget()->CastSpell(eventInfo.GetProcTarget(), SPELL_PALADIN_EYE_FOR_AN_EYE_DAMAGE, args);
            }

            void Register() override
            {
                OnEffectProc += AuraEffectProcFn(spell_pal_eye_for_an_eye_AuraScript::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_eye_for_an_eye_AuraScript();
        }
};

// -20473 - Holy Shock
class spell_pal_holy_shock : public SpellScriptLoader
{
    public:
        spell_pal_holy_shock() : SpellScriptLoader("spell_pal_holy_shock") { }

        class spell_pal_holy_shock_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_holy_shock_SpellScript);

            bool Validate(SpellInfo const* spellInfo) override
            {
                SpellInfo const* firstRankSpellInfo = sSpellMgr->GetSpellInfo(SPELL_PALADIN_HOLY_SHOCK_R1);
                if (!firstRankSpellInfo)
                    return false;

                // can't use other spell than holy shock due to spell_ranks dependency
                if (!spellInfo->IsRankOf(firstRankSpellInfo))
                    return false;

                uint8 rank = spellInfo->GetRank();
                if (!sSpellMgr->GetSpellWithRank(SPELL_PALADIN_HOLY_SHOCK_R1_DAMAGE, rank, true) || !sSpellMgr->GetSpellWithRank(SPELL_PALADIN_HOLY_SHOCK_R1_HEALING, rank, true))
                    return false;

                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Unit* unitTarget = GetHitUnit())
                {
                    uint8 rank = GetSpellInfo()->GetRank();
                    if (caster->IsFriendlyTo(unitTarget))
                        caster->CastSpell(unitTarget, sSpellMgr->GetSpellWithRank(SPELL_PALADIN_HOLY_SHOCK_R1_HEALING, rank), true);
                    else
                        caster->CastSpell(unitTarget, sSpellMgr->GetSpellWithRank(SPELL_PALADIN_HOLY_SHOCK_R1_DAMAGE, rank), true);
                }
            }

            SpellCastResult CheckCast()
            {
                Unit* caster = GetCaster();
                if (Unit* target = GetExplTargetUnit())
                {
                    if (!caster->IsFriendlyTo(target))
                    {
                        if (!caster->IsValidAttackTarget(target))
                            return SPELL_FAILED_BAD_TARGETS;

                        if (!caster->isInFront(target))
                            return SPELL_FAILED_UNIT_NOT_INFRONT;
                    }
                }
                else
                    return SPELL_FAILED_BAD_TARGETS;
                return SPELL_CAST_OK;
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_pal_holy_shock_SpellScript::CheckCast);
                OnEffectHitTarget += SpellEffectFn(spell_pal_holy_shock_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pal_holy_shock_SpellScript();
        }
};

// -20210 - Illumination
class spell_pal_illumination : public SpellScriptLoader
{
public:
    spell_pal_illumination() : SpellScriptLoader("spell_pal_illumination") { }

    class spell_pal_illumination_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pal_illumination_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo(
            {
                SPELL_PALADIN_HOLY_SHOCK_R1_HEALING,
                SPELL_PALADIN_ILLUMINATION_ENERGIZE,
                SPELL_PALADIN_HOLY_SHOCK_R1
            });
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();

            // this script is valid only for the Holy Shock procs of illumination
            if (eventInfo.GetHealInfo() && eventInfo.GetHealInfo()->GetSpellInfo())
            {
                SpellInfo const* originalSpell = nullptr;

                // if proc comes from the Holy Shock heal, need to get mana cost of original spell - else it's the original heal itself
                if (eventInfo.GetHealInfo()->GetSpellInfo()->SpellFamilyFlags[1] & 0x00010000)
                    originalSpell = sSpellMgr->GetSpellInfo(sSpellMgr->GetSpellWithRank(SPELL_PALADIN_HOLY_SHOCK_R1, eventInfo.GetHealInfo()->GetSpellInfo()->GetRank()));
                else
                    originalSpell = eventInfo.GetHealInfo()->GetSpellInfo();

                if (originalSpell && aurEff->GetSpellInfo())
                {
                    Unit* target = eventInfo.GetActor(); // Paladin is the target of the energize
                    uint32 bp = CalculatePct(originalSpell->CalcPowerCost(target, originalSpell->GetSchoolMask()), aurEff->GetSpellInfo()->Effects[EFFECT_1].CalcValue());
                    CastSpellExtraArgs args(aurEff);
                    args.AddSpellBP0(bp);
                    target->CastSpell(target, SPELL_PALADIN_ILLUMINATION_ENERGIZE, args);
                }
            }
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_pal_illumination_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_pal_illumination_AuraScript();
    }
};

//   498 - Divine Protection
//   642 - Divine Shield
// -1022 - Hand of Protection
class spell_pal_immunities : public SpellScript
{
    PrepareSpellScript(spell_pal_immunities);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_FORBEARANCE });
    }

    SpellCastResult CheckCast()
    {
        Unit* caster = GetCaster();

        // for HoP
        Unit* target = GetExplTargetUnit();
        if (!target)
            target = caster;

        // "Cannot be used within $61987d. of using Avenging Wrath."
        if (target->HasAura(SPELL_PALADIN_FORBEARANCE))
            return SPELL_FAILED_TARGET_AURASTATE;

        return SPELL_CAST_OK;
    }

    void TriggerDebuffs()
    {
        // Blizz seems to just apply aura without bothering to cast
        if (Unit* target = GetHitUnit())
            GetCaster()->AddAura(SPELL_PALADIN_FORBEARANCE, target);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_pal_immunities::CheckCast);
        AfterHit += SpellHitFn(spell_pal_immunities::TriggerDebuffs);
    }
};

// -20234 - Improved Lay on Hands
class spell_pal_improved_lay_of_hands : public SpellScriptLoader
{
    public:
        spell_pal_improved_lay_of_hands() : SpellScriptLoader("spell_pal_improved_lay_of_hands") { }

        class spell_pal_improved_lay_of_hands_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_improved_lay_of_hands_AuraScript);

            bool Validate(SpellInfo const* spellInfo) override
            {
                return ValidateSpellInfo({ spellInfo->Effects[EFFECT_0].TriggerSpell });
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();
                eventInfo.GetActionTarget()->CastSpell(eventInfo.GetActionTarget(), GetSpellInfo()->Effects[EFFECT_0].TriggerSpell, { aurEff, GetTarget()->GetGUID() });
            }

            void Register() override
            {
                OnEffectProc += AuraEffectProcFn(spell_pal_improved_lay_of_hands_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_improved_lay_of_hands_AuraScript();
        }
};

// 37705 - Healing Discount
class spell_pal_item_healing_discount : public SpellScriptLoader
{
    public:
        spell_pal_item_healing_discount() : SpellScriptLoader("spell_pal_item_healing_discount") { }

        class spell_pal_item_healing_discount_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_item_healing_discount_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_PALADIN_ITEM_HEALING_TRANCE });
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
            {
                PreventDefaultAction();
                GetTarget()->CastSpell(GetTarget(), SPELL_PALADIN_ITEM_HEALING_TRANCE, aurEff);
            }

            void Register() override
            {
                OnEffectProc += AuraEffectProcFn(spell_pal_item_healing_discount_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_item_healing_discount_AuraScript();
        }
};

// 40470 - Paladin Tier 6 Trinket
class spell_pal_item_t6_trinket : public SpellScriptLoader
{
    public:
        spell_pal_item_t6_trinket() : SpellScriptLoader("spell_pal_item_t6_trinket") { }

        class spell_pal_item_t6_trinket_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_item_t6_trinket_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo(
                {
                    SPELL_PALADIN_ENDURING_LIGHT,
                    SPELL_PALADIN_ENDURING_JUDGEMENT
                });
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();
                SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
                if (!spellInfo)
                    return;

                uint32 spellId;
                int32 chance;

                // Holy Light & Flash of Light
                if (spellInfo->SpellFamilyFlags[0] & 0xC0000000)
                {
                    spellId = SPELL_PALADIN_ENDURING_LIGHT;
                    chance = 15;
                }
                // Judgements
                else if (spellInfo->SpellFamilyFlags[0] & 0x00800000)
                {
                    spellId = SPELL_PALADIN_ENDURING_JUDGEMENT;
                    chance = 50;
                }
                else
                    return;

                if (roll_chance_i(chance))
                    eventInfo.GetActor()->CastSpell(eventInfo.GetProcTarget(), spellId, aurEff);
            }

            void Register() override
            {
                OnEffectProc += AuraEffectProcFn(spell_pal_item_t6_trinket_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_item_t6_trinket_AuraScript();
        }
};

// 20271 - Judgement of Light
class spell_pal_judgement : public SpellScriptLoader
{
    public:
        spell_pal_judgement(char const* scriptName, uint32 spellId) : SpellScriptLoader(scriptName), _spellId(spellId) { }

        class spell_pal_judgement_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_judgement_SpellScript);

        public:
            spell_pal_judgement_SpellScript(uint32 spellId) : SpellScript(), _spellId(spellId) { }

        private:
            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ _spellId });
            }

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                uint32 spellId2 = 0;

                // some seals have SPELL_AURA_DUMMY in EFFECT_2
                Unit::AuraEffectList const& auras = GetCaster()->GetAuraEffectsByType(SPELL_AURA_DUMMY);
                for (Unit::AuraEffectList::const_iterator i = auras.begin(); i != auras.end(); ++i)
                {
                    if ((*i)->GetSpellInfo()->GetSpellSpecific() == SPELL_SPECIFIC_SEAL && (*i)->GetEffIndex() == EFFECT_2)
                    {
                        if (sSpellMgr->GetSpellInfo((*i)->GetAmount()))
                        {
                            spellId2 = (*i)->GetAmount();
                            break;
                        }
                    }
                }

                GetCaster()->CastSpell(GetHitUnit(), _spellId, true);
                if (spellId2)
                    GetCaster()->CastSpell(GetHitUnit(), spellId2, true);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_pal_judgement_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }

            uint32 const _spellId;
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pal_judgement_SpellScript(_spellId);
        }

    private:
        uint32 const _spellId;
};

// 20425 - Judgement of Command
class spell_pal_judgement_of_command : public SpellScriptLoader
{
    public:
        spell_pal_judgement_of_command() : SpellScriptLoader("spell_pal_judgement_of_command") { }

        class spell_pal_judgement_of_command_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_judgement_of_command_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* unitTarget = GetHitUnit())
                    GetCaster()->CastSpell(unitTarget, GetEffectValue(), true);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_pal_judgement_of_command_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pal_judgement_of_command_SpellScript();
        }
};

// 20185 - Judgement of Light
class spell_pal_judgement_of_light_heal : public SpellScriptLoader
{
    public:
        spell_pal_judgement_of_light_heal() : SpellScriptLoader("spell_pal_judgement_of_light_heal") { }

        class spell_pal_judgement_of_light_heal_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_judgement_of_light_heal_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_PALADIN_JUDGEMENT_OF_LIGHT_HEAL });
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                Unit* caster = eventInfo.GetProcTarget();

                CastSpellExtraArgs args(aurEff);
                args.OriginalCaster = GetCasterGUID();
                args.AddSpellBP0(caster->CountPctFromMaxHealth(aurEff->GetAmount()));
                caster->CastSpell(nullptr, SPELL_PALADIN_JUDGEMENT_OF_LIGHT_HEAL, args);
            }

            void Register() override
            {
                OnEffectProc += AuraEffectProcFn(spell_pal_judgement_of_light_heal_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_judgement_of_light_heal_AuraScript();
        }
};

// -20186 - Judgement of Wisdom
class spell_pal_judgement_of_wisdom_mana : public SpellScriptLoader
{
    public:
        spell_pal_judgement_of_wisdom_mana() : SpellScriptLoader("spell_pal_judgement_of_wisdom_mana") { }

        class spell_pal_judgement_of_wisdom_mana_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_judgement_of_wisdom_mana_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_PALADIN_JUDGEMENT_OF_WISDOM_MANA });
            }

            bool CheckProc(ProcEventInfo& eventInfo)
            {
                return eventInfo.GetProcTarget()->GetPowerType() == POWER_MANA;
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                SpellInfo const* spellInfo = sSpellMgr->AssertSpellInfo(SPELL_PALADIN_JUDGEMENT_OF_WISDOM_MANA);

                Unit* caster = eventInfo.GetProcTarget();
                int32 const amount = CalculatePct(static_cast<int32>(caster->GetCreateMana()), spellInfo->Effects[EFFECT_0].CalcValue());
                CastSpellExtraArgs args(aurEff);
                args.OriginalCaster = GetCasterGUID();
                args.AddSpellBP0(amount);
                caster->CastSpell(nullptr, spellInfo->Id, args);
            }

            void Register() override
            {
                DoCheckProc += AuraCheckProcFn(spell_pal_judgement_of_wisdom_mana_AuraScript::CheckProc);
                OnEffectProc += AuraEffectProcFn(spell_pal_judgement_of_wisdom_mana_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_judgement_of_wisdom_mana_AuraScript();
        }
};

// -31876 - Judgements of the Wise
class spell_pal_judgements_of_the_wise : public SpellScriptLoader
{
    public:
        spell_pal_judgements_of_the_wise() : SpellScriptLoader("spell_pal_judgements_of_the_wise") { }

        class spell_pal_judgements_of_the_wise_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_judgements_of_the_wise_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_PALADIN_JUDGEMENTS_OF_THE_WISE_MANA });
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                Unit* caster = eventInfo.GetActor();
                caster->CastSpell(nullptr, SPELL_PALADIN_JUDGEMENTS_OF_THE_WISE_MANA, aurEff);
            }

            void Register() override
            {
                OnEffectProc += AuraEffectProcFn(spell_pal_judgements_of_the_wise_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_judgements_of_the_wise_AuraScript();
        }
};

// -633 - Lay on Hands
class spell_pal_lay_on_hands : public SpellScriptLoader
{
    public:
        spell_pal_lay_on_hands() : SpellScriptLoader("spell_pal_lay_on_hands") { }

        class spell_pal_lay_on_hands_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_lay_on_hands_SpellScript);

            bool Validate(SpellInfo const* /*spell*/) override
            {
                return ValidateSpellInfo({ SPELL_PALADIN_FORBEARANCE });
            }

            SpellCastResult CheckCast()
            {
                Unit* caster = GetCaster();
                if (Unit* target = GetExplTargetUnit())
                    if (caster == target)
                        if (target->HasAura(SPELL_PALADIN_FORBEARANCE))
                            return SPELL_FAILED_TARGET_AURASTATE;

                return SPELL_CAST_OK;
            }

            void HandleScript()
            {
                Unit* caster = GetCaster();
                if (caster == GetHitUnit())
                    caster->CastSpell(caster, SPELL_PALADIN_FORBEARANCE, true);
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_pal_lay_on_hands_SpellScript::CheckCast);
                AfterHit += SpellHitFn(spell_pal_lay_on_hands_SpellScript::HandleScript);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pal_lay_on_hands_SpellScript();
        }
};

// 31789 - Righteous Defense
class spell_pal_righteous_defense : public SpellScript
{
    PrepareSpellScript(spell_pal_righteous_defense);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PALADIN_RIGHTEOUS_DEFENSE_TAUNT });
    }

    SpellCastResult CheckCast()
    {
        Unit* caster = GetCaster();
        if (caster->GetTypeId() != TYPEID_PLAYER)
            return SPELL_FAILED_DONT_REPORT;

        if (Unit* target = GetExplTargetUnit())
        {
            if (!target->IsFriendlyTo(caster) || target == caster || target->getAttackers().empty())
                return SPELL_FAILED_BAD_TARGETS;
        }
        else
            return SPELL_FAILED_BAD_TARGETS;

        return SPELL_CAST_OK;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (Unit* target = GetHitUnit())
        {
            auto const& attackers = target->getAttackers();

            std::vector<Unit*> list(attackers.cbegin(), attackers.cend());
            Trinity::Containers::RandomResize(list, 3);

            for (Unit* attacker : list)
                GetCaster()->CastSpell(attacker, SPELL_PALADIN_RIGHTEOUS_DEFENSE_TAUNT, TRIGGERED_FULL_MASK);
        }
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_pal_righteous_defense::CheckCast);
        OnEffectHitTarget += SpellEffectFn(spell_pal_righteous_defense::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 20154, 21084 - Seal of Righteousness - melee proc dummy (addition ${$MWS*(0.022*$AP+0.044*$SPH)} damage)
class spell_pal_seal_of_righteousness : public SpellScriptLoader
{
    public:
        spell_pal_seal_of_righteousness() : SpellScriptLoader("spell_pal_seal_of_righteousness") { }

        class spell_pal_seal_of_righteousness_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_seal_of_righteousness_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_PALADIN_SEAL_OF_RIGHTEOUSNESS });
            }

            bool CheckProc(ProcEventInfo& eventInfo)
            {
                return eventInfo.GetProcTarget() != nullptr;
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                Unit* victim = eventInfo.GetProcTarget();

                float ap = GetTarget()->GetTotalAttackPowerValue(BASE_ATTACK);
                ap += victim->GetTotalAuraModifier(SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS);

                int32 sph = GetTarget()->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_HOLY);
                sph += victim->GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_DAMAGE_TAKEN, SPELL_SCHOOL_MASK_HOLY);

                float mws = GetTarget()->GetAttackTime(BASE_ATTACK);
                mws /= 1000.0f;

                int32 bp = int32(mws * (0.022f * ap + 0.044f * sph));
                CastSpellExtraArgs args(aurEff);
                args.AddSpellBP0(bp);
                GetTarget()->CastSpell(victim, SPELL_PALADIN_SEAL_OF_RIGHTEOUSNESS, args);
            }

            void Register() override
            {
                DoCheckProc += AuraCheckProcFn(spell_pal_seal_of_righteousness_AuraScript::CheckProc);
                OnEffectProc += AuraEffectProcFn(spell_pal_seal_of_righteousness_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_seal_of_righteousness_AuraScript();
        }
};

// 31801 - Seal of Vengeance
template <uint32 DoTSpellId, uint32 DamageSpellId>
class spell_pal_seal_of_vengeance : public SpellScriptLoader
{
    public:
        spell_pal_seal_of_vengeance(char const* ScriptName) : SpellScriptLoader(ScriptName) { }

        template <uint32 DoTSpell, uint32 DamageSpell>
        class spell_pal_seal_of_vengeance_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_seal_of_vengeance_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo(
                {
                    DoTSpell,
                    DamageSpell
                });
            }

            /*
            When an auto-attack lands (does not dodge/parry/miss) that can proc a seal the of the following things happen independently of each other (see 2 roll system).

            1) A "hidden strike" which uses melee combat mechanics occurs. If it lands it refreshes/stacks SoV DoT. Only white swings can trigger a refresh or stack. (This hidden strike mechanic can also proc things like berserking..)
            2) A weapon damage based proc will occur if you used a special (CS/DS/judge) or if you have a 5 stack (from auto attacks). This attack can not be avoided.

            Remember #2 happens regardless of #1 landing, it just requires the initial attack (autos, cs, etc) to land.

            Stack Number    % of Weapon Damage  % with SotP
            0               0%                  0%
            1               6.6%                7.6%
            2               13.2%               15.2%
            3               19.8%               22.8%
            4               26.4%               30.4%
            5               33%                 38%
            */

            void HandleApplyDoT(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                if (!(eventInfo.GetTypeMask() & PROC_FLAG_DONE_MELEE_AUTO_ATTACK))
                {
                    // Patch 3.2.0 Notes: Only auto-attacks and Hammer of the Righteous can place the debuff on the paladin's current target(s).
                    SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
                    if (!spellInfo || spellInfo->SpellIconID != PALADIN_ICON_ID_HAMMER_OF_THE_RIGHTEOUS)
                        return;
                }

                // don't cast triggered, spell already has SPELL_ATTR4_CAN_CAST_WHILE_CASTING attr
                eventInfo.GetActor()->CastSpell(eventInfo.GetProcTarget(), DoTSpell, CastSpellExtraArgs(TRIGGERED_DONT_RESET_PERIODIC_TIMER).SetTriggeringAura(aurEff));
            }

            void HandleSeal(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                Unit* caster = eventInfo.GetActor();
                Unit* target = eventInfo.GetProcTarget();

                // get current aura on target, if any
                AuraEffect const* sealDot = target->GetAuraEffectByFamilyFlags(SPELL_AURA_PERIODIC_DAMAGE, SPELLFAMILY_PALADIN, 0x00000000, 0x00000800, caster->GetGUID());
                if (!sealDot)
                    return;

                uint8 const stacks = sealDot->GetBase()->GetStackAmount();
                uint8 const maxStacks = sealDot->GetSpellInfo()->StackAmount;

                if (stacks < maxStacks && !(eventInfo.GetTypeMask() & PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS))
                    return;

                SpellInfo const* spellInfo = sSpellMgr->AssertSpellInfo(DamageSpell);
                int32 amount = spellInfo->Effects[EFFECT_0].CalcValue();
                amount *= stacks;
                amount /= maxStacks;

                CastSpellExtraArgs args(aurEff);
                args.AddSpellBP0(amount);
                caster->CastSpell(target, DamageSpell, args);
            }

            void Register() override
            {
                OnEffectProc += AuraEffectProcFn(spell_pal_seal_of_vengeance_AuraScript::HandleApplyDoT, EFFECT_0, SPELL_AURA_DUMMY);
                OnEffectProc += AuraEffectProcFn(spell_pal_seal_of_vengeance_AuraScript::HandleSeal, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_seal_of_vengeance_AuraScript<DoTSpellId, DamageSpellId>();
        }
};

// 20375 - Seal of Command
// 21084 - Seal of Righteousness
// 31801 - Seal of Vengeance
// 31892 - Seal of Blood
// 33127 - Seal of Command
// 38008 - Seal of Blood
// 41459 - Seal of Blood
// 53720 - Seal of the Martyr
// 53736 - Seal of Corruption
class spell_pal_seals : public SpellScriptLoader
{
    public:
        spell_pal_seals() : SpellScriptLoader("spell_pal_seals") { }

        class spell_pal_seals_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_seals_AuraScript);

            // Effect 2 is used by Judgement code, we prevent the proc to avoid console logging of unknown spell trigger
            bool CheckDummyProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
            {
                return false;
            }

            void Register() override
            {
                DoCheckEffectProc += AuraCheckEffectProcFn(spell_pal_seals_AuraScript::CheckDummyProc, EFFECT_2, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_seals_AuraScript();
        }
};

// -31785 - Spiritual Attunement
class spell_pal_spiritual_attunement : public SpellScriptLoader
{
    public:
        spell_pal_spiritual_attunement() : SpellScriptLoader("spell_pal_spiritual_attunement") { }

        class spell_pal_spiritual_attunement_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_spiritual_attunement_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo({ SPELL_PALADIN_SPIRITUAL_ATTUNEMENT_MANA });
            }

            bool CheckProc(ProcEventInfo& eventInfo)
            {
                // "when healed by other friendly targets' spells"
                if (eventInfo.GetProcTarget() == eventInfo.GetActionTarget())
                    return false;

                return eventInfo.GetHealInfo() && eventInfo.GetHealInfo()->GetEffectiveHeal();
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();
                HealInfo* healInfo = eventInfo.GetHealInfo();
                int32 amount = CalculatePct(static_cast<int32>(healInfo->GetEffectiveHeal()), aurEff->GetAmount());

                CastSpellExtraArgs args(aurEff);
                args.AddSpellBP0(amount);
                eventInfo.GetActionTarget()->CastSpell(nullptr, SPELL_PALADIN_SPIRITUAL_ATTUNEMENT_MANA, args);
            }

            void Register() override
            {
                DoCheckProc += AuraCheckProcFn(spell_pal_spiritual_attunement_AuraScript::CheckProc);
                OnEffectProc += AuraEffectProcFn(spell_pal_spiritual_attunement_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_spiritual_attunement_AuraScript();
        }
};

// 28789 - Holy Power
class spell_pal_t3_6p_bonus : public SpellScriptLoader
{
    public:
        spell_pal_t3_6p_bonus() : SpellScriptLoader("spell_pal_t3_6p_bonus") { }

        class spell_pal_t3_6p_bonus_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_t3_6p_bonus_AuraScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return ValidateSpellInfo(
                {
                    SPELL_PALADIN_HOLY_POWER_ARMOR,
                    SPELL_PALADIN_HOLY_POWER_ATTACK_POWER,
                    SPELL_PALADIN_HOLY_POWER_SPELL_POWER,
                    SPELL_PALADIN_HOLY_POWER_MP5
                });
            }

            void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                uint32 spellId;
                Unit* caster = eventInfo.GetActor();
                Unit* target = eventInfo.GetProcTarget();

                switch (target->getClass())
                {
                    case CLASS_PALADIN:
                    case CLASS_PRIEST:
                    case CLASS_SHAMAN:
                    case CLASS_DRUID:
                        spellId = SPELL_PALADIN_HOLY_POWER_MP5;
                        break;
                    case CLASS_MAGE:
                    case CLASS_WARLOCK:
                        spellId = SPELL_PALADIN_HOLY_POWER_SPELL_POWER;
                        break;
                    case CLASS_HUNTER:
                    case CLASS_ROGUE:
                        spellId = SPELL_PALADIN_HOLY_POWER_ATTACK_POWER;
                        break;
                    case CLASS_WARRIOR:
                        spellId = SPELL_PALADIN_HOLY_POWER_ARMOR;
                        break;
                    default:
                        return;
                }

                caster->CastSpell(target, spellId, aurEff);
            }

            void Register() override
            {
                OnEffectProc += AuraEffectProcFn(spell_pal_t3_6p_bonus_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_t3_6p_bonus_AuraScript();
        }
};

void AddSC_paladin_spell_scripts()
{
    new spell_pal_blessing_of_faith();
    new spell_pal_exorcism_and_holy_wrath_damage();
    new spell_pal_eye_for_an_eye();
    new spell_pal_holy_shock();
    new spell_pal_illumination();
    RegisterSpellScript(spell_pal_immunities);
    new spell_pal_improved_lay_of_hands();
    new spell_pal_item_healing_discount();
    new spell_pal_item_t6_trinket();
    new spell_pal_judgement("spell_pal_judgement_of_light", SPELL_PALADIN_JUDGEMENT_OF_LIGHT);
    new spell_pal_judgement_of_command();
    new spell_pal_judgement_of_light_heal();
    new spell_pal_judgement_of_wisdom_mana();
    new spell_pal_judgements_of_the_wise();
    new spell_pal_lay_on_hands();
    RegisterSpellScript(spell_pal_righteous_defense);
    new spell_pal_seal_of_righteousness();
    new spell_pal_seal_of_vengeance<SPELL_PALADIN_HOLY_VENGEANCE, SPELL_PALADIN_SEAL_OF_VENGEANCE_DAMAGE>("spell_pal_seal_of_vengeance");
    new spell_pal_seals();
    new spell_pal_spiritual_attunement();
    new spell_pal_t3_6p_bonus();
}
