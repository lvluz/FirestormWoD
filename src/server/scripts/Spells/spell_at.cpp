////////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2015 Millenium-studio SARL
///  All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Containers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"

/// Areatrigger defile - 152280
class spell_at_dk_defile : public AreaTriggerEntityScript
{
    public:
        spell_at_dk_defile() : AreaTriggerEntityScript("spell_areatrigger_dk_defile") { }

        enum eDefilebSpell
        {
            NpcDefileVisual     = 82521,
            SpellDefileDamage   = 156000,
            SpellDefile         = 152280,
            TimerDefile         = 1 * IN_MILLISECONDS
        };

        void OnUpdate(AreaTrigger* p_AreaTrigger, uint32 p_Time)
        {
            if (TimerDefile <= m_CurrentTimerDefile)
            {
                if (Unit* l_Caster = p_AreaTrigger->GetCaster())
                {
                    std::list<Unit*> l_TargetListTemp;
                    float l_Radius = p_AreaTrigger->GetFloatValue(AREATRIGGER_FIELD_EXPLICIT_SCALE) * 8.0f;

                    JadeCore::AnyUnfriendlyUnitInObjectRangeCheck l_Check(p_AreaTrigger, l_Caster, l_Radius);
                    JadeCore::UnitListSearcher<JadeCore::AnyUnfriendlyUnitInObjectRangeCheck> l_Searcher(p_AreaTrigger, l_TargetListTemp, l_Check);
                    p_AreaTrigger->VisitNearbyObject(l_Radius, l_Searcher);

                    if (!l_TargetListTemp.empty())
                    {
                        std::list<Unit*> l_TargetList;
                        /// Remove unatackable target
                        for (Unit* l_Unit : l_TargetListTemp)
                        {
                            if (l_Caster->IsValidAttackTarget(l_Unit))
                                l_TargetList.push_back(l_Unit);
                        }

                        if (!l_TargetList.empty())
                        {
                            /// Update size
                            if (SpellInfo const* l_Defile = sSpellMgr->GetSpellInfo(eDefilebSpell::SpellDefile))
                            {
                                float l_MultiplicatorVisual = 1.0f + float(l_Defile->Effects[EFFECT_1].BasePoints) / 100 / 100;

                                /// Cast damage
                                for (Unit* l_Unit : l_TargetList)
                                {
                                    /// Update damage
                                    if (SpellInfo const* l_DefileDamage = sSpellMgr->GetSpellInfo(eDefilebSpell::SpellDefileDamage))
                                    {
                                        int32 l_BasePoints = l_DefileDamage->Effects[EFFECT_0].BasePoints + m_StackDefile * float(l_Defile->Effects[EFFECT_1].BasePoints) / 100;

                                        l_Caster->CastCustomSpell(l_Unit, eDefilebSpell::SpellDefileDamage, &l_BasePoints, nullptr, nullptr, true);
                                    }
                                }

                                /// Update size
                                uint64 l_CreatureVisualGUID = p_AreaTrigger->GetGUIDCreatureVisual();
                                if (l_CreatureVisualGUID != 0)
                                {
                                    if (Creature* l_CreatureVisual = p_AreaTrigger->GetMap()->GetCreature(l_CreatureVisualGUID))
                                    {
                                        l_CreatureVisual->SetObjectScale(l_CreatureVisual->GetFloatValue(OBJECT_FIELD_SCALE) * l_MultiplicatorVisual);
                                        p_AreaTrigger->SetFloatValue(AREATRIGGER_FIELD_EXPLICIT_SCALE, p_AreaTrigger->GetFloatValue(AREATRIGGER_FIELD_EXPLICIT_SCALE) * l_MultiplicatorVisual);
                                    }
                                }

                                m_StackDefile++;
                            }
                        }
                    }
                }
                m_CurrentTimerDefile = 0;
            }
            else
                m_CurrentTimerDefile += p_Time;
        }

        AreaTriggerEntityScript* GetAI() const
        {
            return new spell_at_dk_defile();
        }

    private:
        uint32 m_CurrentTimerDefile = 1000;
        uint8 m_StackDefile = 0;
};

/// Fungal Growth - 164717
class spell_at_druid_fungal_growth : public AreaTriggerEntityScript
{
    public:
        spell_at_druid_fungal_growth() : AreaTriggerEntityScript("at_fungal_growth") { }

        enum WildMushroomSpells
        {
            SpellDruidWildMushroomFungalCloud       = 81281,
            SpellDruidAreaWildMushroomFungalCloud   = 164717,
            SpellDruidWildMushroomBalance           = 88747,
        };

        std::list<uint64> m_Targets;

        void OnUpdate(AreaTrigger* p_AreaTrigger, uint32 /*p_Time*/)
        {
            auto l_SpellInfo = sSpellMgr->GetSpellInfo(WildMushroomSpells::SpellDruidWildMushroomBalance);
            auto l_AreaTriggerCaster = p_AreaTrigger->GetCaster();

            if (l_AreaTriggerCaster == nullptr || l_SpellInfo == nullptr)
                return;

            std::list<Unit*> l_TargetList;
            float l_Radius = (float)l_SpellInfo->Effects[EFFECT_0].BasePoints;

            JadeCore::AnyUnfriendlyUnitInObjectRangeCheck l_Check(p_AreaTrigger, l_AreaTriggerCaster, l_Radius);
            JadeCore::UnitListSearcher<JadeCore::AnyUnfriendlyUnitInObjectRangeCheck> l_Searcher(p_AreaTrigger, l_TargetList, l_Check);
            p_AreaTrigger->VisitNearbyObject(l_Radius, l_Searcher);

            for (Unit* l_Target : l_TargetList)
            {
                if (l_Target == nullptr)
                    return;

                if (!l_Target->HasAura(WildMushroomSpells::SpellDruidWildMushroomFungalCloud))
                {
                    l_AreaTriggerCaster->CastSpell(l_Target, WildMushroomSpells::SpellDruidWildMushroomFungalCloud, true);
                    m_Targets.push_back(l_Target->GetGUID());
                }
            }
            for (uint64 l_TargetGuid : m_Targets)
            {
                Unit* l_Target = ObjectAccessor::GetUnit(*l_AreaTriggerCaster, l_TargetGuid);

                if (l_Target == nullptr)
                    return;

                if (l_Target->HasAura(WildMushroomSpells::SpellDruidWildMushroomFungalCloud, l_AreaTriggerCaster->GetGUID()) && l_Target->GetDistance(l_AreaTriggerCaster) <= l_Radius)
                    return;

                if (l_Target->HasAura(WildMushroomSpells::SpellDruidWildMushroomFungalCloud, l_AreaTriggerCaster->GetGUID()))
                    l_Target->RemoveAurasDueToSpell(WildMushroomSpells::SpellDruidWildMushroomFungalCloud, l_AreaTriggerCaster->GetGUID());

                m_Targets.remove(l_TargetGuid);
            }
        }

        void OnRemove(AreaTrigger* p_AreaTrigger, uint32 /*p_Time*/)
        {
            auto l_AreaTriggerCaster = p_AreaTrigger->GetCaster();

            if (l_AreaTriggerCaster == nullptr)
                return;

            for (uint64 l_TargetGuid : m_Targets)
            {
                Unit* l_Target = ObjectAccessor::GetUnit(*l_AreaTriggerCaster, l_TargetGuid);

                if (l_Target == nullptr)
                    return;

                if (l_Target->HasAura(WildMushroomSpells::SpellDruidWildMushroomFungalCloud, l_AreaTriggerCaster->GetGUID()))
                    l_Target->RemoveAurasDueToSpell(WildMushroomSpells::SpellDruidWildMushroomFungalCloud, l_AreaTriggerCaster->GetGUID());

                m_Targets.remove(l_TargetGuid);
            }
        }

        AreaTriggerEntityScript* GetAI() const
        {
            return new spell_at_druid_fungal_growth();
        }
};

/// Ursol Vortex - 102793
class spell_at_druid_ursol_vortex : public AreaTriggerEntityScript
{
    public:
        spell_at_druid_ursol_vortex(): AreaTriggerEntityScript("at_ursol_vortex") { }

        void OnUpdate(AreaTrigger* p_AreaTrigger, uint32 p_Time)
        {
            std::list<Unit*> targetList;
            float l_Radius = 8.0f;
            Unit* l_Caster = p_AreaTrigger->GetCaster();

            JadeCore::NearestAttackableUnitInObjectRangeCheck u_check(p_AreaTrigger, l_Caster, l_Radius);
            JadeCore::UnitListSearcher<JadeCore::NearestAttackableUnitInObjectRangeCheck> searcher(p_AreaTrigger, targetList, u_check);
            p_AreaTrigger->VisitNearbyObject(l_Radius, searcher);

            if (!targetList.empty())
                for (auto itr : targetList)
                    if (!itr->HasAura(127797))
                        l_Caster->CastSpell(itr, 127797, true);
        }

        AreaTriggerEntityScript* GetAI() const
        {
            return new spell_at_druid_ursol_vortex();
        }
};

/// Binding Shot - 109248
class spell_at_hun_binding_shot : public AreaTriggerEntityScript
{
    public:
        spell_at_hun_binding_shot() : AreaTriggerEntityScript("spell_hun_binding_shot_areatrigger") { }

        enum eSpells
        {
            BindingShotLink         = 117405,
            BindingShotImmune       = 117553,
            BindingShotVisualLink   = 117614
        };

        uint32 m_LinkVisualTimer = 1000;

        void OnUpdate(AreaTrigger* p_AreaTrigger, uint32 p_Time)
        {
            if (Unit* l_Caster = p_AreaTrigger->GetCaster())
            {
                std::list<Unit*> l_TargetList;
                float l_Radius = sSpellMgr->GetSpellInfo(p_AreaTrigger->GetSpellId())->Effects[EFFECT_1].CalcRadius(l_Caster);

                JadeCore::AnyUnfriendlyUnitInObjectRangeCheck l_Check(p_AreaTrigger, l_Caster, l_Radius);
                JadeCore::UnitListSearcher<JadeCore::AnyUnfriendlyUnitInObjectRangeCheck> l_Searcher(p_AreaTrigger, l_TargetList, l_Check);
                p_AreaTrigger->VisitNearbyObject(l_Radius, l_Searcher);

                if (l_TargetList.empty())
                    return;

                l_TargetList.remove_if([this, l_Caster](Unit* p_Unit) -> bool
                                       {
                                           if (p_Unit == nullptr || !l_Caster->IsValidAttackTarget(p_Unit))
                                               return true;

                                           if (p_Unit->HasAura(eSpells::BindingShotImmune))
                                               return true;

                                           return false;
                                       });

                for (Unit* l_Target : l_TargetList)
                {
                    if (!l_Target->HasAura(eSpells::BindingShotLink))
                        l_Caster->CastSpell(l_Target, eSpells::BindingShotLink, true);
                }

                if (m_LinkVisualTimer <= p_Time)
                {
                    m_LinkVisualTimer = 1000;

                    for (Unit* l_Target : l_TargetList)
                        l_Target->CastSpell(p_AreaTrigger->m_positionX, p_AreaTrigger->m_positionY, p_AreaTrigger->m_positionZ, eSpells::BindingShotVisualLink, true);
                }
                else
                    m_LinkVisualTimer -= p_Time;
            }
        }
        
        AreaTriggerEntityScript* GetAI() const
        {
            return new spell_at_hun_binding_shot();
        }
};

/// Ice Trap - 13809
/// Ice Trap (Frost - Trap Launcher) - 82940
class spell_at_hun_ice_trap : public AreaTriggerEntityScript
{
    public:
        spell_at_hun_ice_trap() : AreaTriggerEntityScript("at_ice_trap") { }

        enum eSpells
        {
            SpellIceTrapEffect  = 13810,
            SpellEntrapmentAura = 19387,
            SpellEntrapment = 64803
        };

        void OnUpdate(AreaTrigger* p_AreaTrigger, uint32 p_Time)
        {
            SpellInfo const* l_CreateSpell = sSpellMgr->GetSpellInfo(p_AreaTrigger->GetSpellId());
            Unit* l_Caster = p_AreaTrigger->GetCaster();

            if (l_Caster && l_CreateSpell)
            {
                float l_Radius = 5.0f;
                Unit* l_Target = nullptr;

                JadeCore::AnyUnfriendlyNoTotemUnitInObjectRangeCheck l_Checker(p_AreaTrigger, l_Caster, l_Radius);
                JadeCore::UnitSearcher<JadeCore::AnyUnfriendlyNoTotemUnitInObjectRangeCheck> l_Searcher(p_AreaTrigger, l_Target, l_Checker);
                p_AreaTrigger->VisitNearbyGridObject(l_Radius, l_Searcher);
                if (!l_Target)
                    p_AreaTrigger->VisitNearbyWorldObject(l_Radius, l_Searcher);

                if (l_Target != nullptr)
                {
                    l_Caster->CastSpell(p_AreaTrigger->GetPositionX(), p_AreaTrigger->GetPositionY(), p_AreaTrigger->GetPositionZ(), eSpells::SpellIceTrapEffect, true);

                    if (l_Caster->HasAura(eSpells::SpellEntrapmentAura)) ///< Entrapment
                        l_Caster->CastSpell(p_AreaTrigger->GetPositionX(), p_AreaTrigger->GetPositionY(), p_AreaTrigger->GetPositionZ(), eSpells::SpellEntrapmentAura, true);
                    p_AreaTrigger->Remove(0);
                }
            }
        }

        AreaTriggerEntityScript* GetAI() const
        {
            return new spell_at_hun_ice_trap();
        }
};

/// Ice trap effect - 13810
class spell_at_hun_ice_trap_effect : public AreaTriggerEntityScript
{
    public:
        spell_at_hun_ice_trap_effect() : AreaTriggerEntityScript("at_ice_trap_effect") { }

        enum eSpells
        {
            GlyphOfBlackIce = 109263,
            BlackIceEffect = 83559,
            IceTrapEffect = 135299
        };

        void OnUpdate(AreaTrigger* p_AreaTrigger, uint32 p_Time)
        {
            std::list<Unit*> targetList;
            float l_Radius = 10.0f;
            Unit* l_Caster = p_AreaTrigger->GetCaster();

            JadeCore::NearestAttackableUnitInObjectRangeCheck u_check(p_AreaTrigger, l_Caster, l_Radius);
            JadeCore::UnitListSearcher<JadeCore::NearestAttackableUnitInObjectRangeCheck> searcher(p_AreaTrigger, targetList, u_check);
            p_AreaTrigger->VisitNearbyObject(l_Radius, searcher);

            for (auto itr : targetList)
                itr->CastSpell(itr, IceTrapEffect, true);

            // Glyph of Black Ice
            if (l_Caster->GetDistance(p_AreaTrigger) <= l_Radius && l_Caster->HasAura(GlyphOfBlackIce) && !l_Caster->HasAura(BlackIceEffect))
                l_Caster->CastSpell(l_Caster, BlackIceEffect, true);
            if (l_Caster->GetDistance(p_AreaTrigger) > l_Radius || !l_Caster->HasAura(GlyphOfBlackIce))
                l_Caster->RemoveAura(BlackIceEffect);
        }

        void OnRemove(AreaTrigger* p_AreaTrigger, uint32 /*p_Time*/)
        {
            Unit* l_Caster = p_AreaTrigger->GetCaster();

            if (l_Caster == nullptr)
                return;

            if (l_Caster->HasAura(BlackIceEffect))
                l_Caster->RemoveAura(BlackIceEffect);
        }

        AreaTriggerEntityScript* GetAI() const
        {
            return new spell_at_hun_ice_trap_effect();
        }
};

/// Freezing Trap - 1499
/// Freezing Trap (Frost - Trap Launcher) - 60202
class spell_at_hun_freezing_trap : public AreaTriggerEntityScript
{
    public:
        spell_at_hun_freezing_trap() : AreaTriggerEntityScript("at_freezing_trap") { }

        enum class HunterFreezingTrap : uint32
        {
            SpellIncapacitate         = 3355,
            SpellGlyphOfSolace        = 119407,
            HunterWodPvp2PBonus       = 166005,
            HunterWodPvp2PBonusEffect = 166009
        };

        void OnUpdate(AreaTrigger* p_AreaTrigger, uint32 p_Time)
        {
            auto l_CreateSpell       = sSpellMgr->GetSpellInfo(p_AreaTrigger->GetSpellId());
            auto l_AreaTriggerCaster = p_AreaTrigger->GetCaster();

            if (l_AreaTriggerCaster && l_CreateSpell)
            {
                float l_Radius = 2.0f;
                Unit* l_Target = nullptr;

                JadeCore::AnyUnfriendlyNoTotemUnitInObjectRangeCheck l_Checker(p_AreaTrigger, l_AreaTriggerCaster, l_Radius);
                JadeCore::UnitSearcher<JadeCore::AnyUnfriendlyNoTotemUnitInObjectRangeCheck> l_Searcher(p_AreaTrigger, l_Target, l_Checker);
                p_AreaTrigger->VisitNearbyGridObject(l_Radius, l_Searcher);
                if (!l_Target)
                    p_AreaTrigger->VisitNearbyWorldObject(l_Radius, l_Searcher);

                if (l_Target != nullptr)
                {
                    if (l_AreaTriggerCaster->HasAura((uint32)HunterFreezingTrap::SpellGlyphOfSolace)) ///< Your Freezing Trap also removes all damage over time effects from its target.
                        l_Target->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE, l_AreaTriggerCaster->GetGUID());
                    l_AreaTriggerCaster->CastSpell(l_Target, (uint32)HunterFreezingTrap::SpellIncapacitate, true);
                    p_AreaTrigger->Remove(0);

                    /// Item - Hunter WoD PvP 2P Bonus
                    if (l_AreaTriggerCaster->HasAura((uint32)HunterFreezingTrap::HunterWodPvp2PBonus))
                        l_AreaTriggerCaster->CastSpell(l_AreaTriggerCaster, (uint32)HunterFreezingTrap::HunterWodPvp2PBonusEffect, true);
                }
            }
        }

        AreaTriggerEntityScript* GetAI() const
        {
            return new spell_at_hun_freezing_trap();
        }
};

/// Explosive Trap - 13813
/// Explosive Trap (Fire - Trap Launcher) - 82938
class spell_at_hun_explosive_trap : public AreaTriggerEntityScript
{
    public:
        spell_at_hun_explosive_trap() : AreaTriggerEntityScript("at_explosive_trap") { }

        enum HunterExplosiveTrap
        {
            SpellExplosiveEffect = 13812
        };

        void OnUpdate(AreaTrigger* p_AreaTrigger, uint32 p_Time)
        {
            auto l_CreateSpell = sSpellMgr->GetSpellInfo(p_AreaTrigger->GetSpellId());
            auto l_AreaTriggerCaster = p_AreaTrigger->GetCaster();

            if (l_AreaTriggerCaster && l_CreateSpell)
            {
                float l_Radius = 5.0f;
                Unit* l_Target = nullptr;

                JadeCore::AnyUnfriendlyNoTotemUnitInObjectRangeCheck l_Checker(p_AreaTrigger, l_AreaTriggerCaster, l_Radius);
                JadeCore::UnitSearcher<JadeCore::AnyUnfriendlyNoTotemUnitInObjectRangeCheck> l_Searcher(p_AreaTrigger, l_Target, l_Checker);
                p_AreaTrigger->VisitNearbyGridObject(l_Radius, l_Searcher);
                if (!l_Target)
                    p_AreaTrigger->VisitNearbyWorldObject(l_Radius, l_Searcher);

                if (l_Target != nullptr)
                {
                    l_AreaTriggerCaster->CastSpell(p_AreaTrigger->GetPositionX(), p_AreaTrigger->GetPositionY(), p_AreaTrigger->GetPositionZ(), HunterExplosiveTrap::SpellExplosiveEffect, true);
                    p_AreaTrigger->Remove(0);
                }
            }
        }

        AreaTriggerEntityScript* GetAI() const
        {
            return new spell_at_hun_explosive_trap();
        }
};

/// Item - Mage WoD PvP Frost 2P Bonus - 180723
class spell_at_mage_wod_frost_2p_bonus : public AreaTriggerEntityScript
{
    public:
        spell_at_mage_wod_frost_2p_bonus() : AreaTriggerEntityScript("at_mage_wod_frost_2p_bonus") { }

        enum eSpells
        {
            SlickIce = 180724
        };

        void OnUpdate(AreaTrigger* p_AreaTrigger, uint32 p_Time)
        {
            std::list<Unit*> targetList;
            float l_Radius = 20.0f;
            Unit* l_Caster = p_AreaTrigger->GetCaster();

            JadeCore::NearestAttackableUnitInObjectRangeCheck u_check(p_AreaTrigger, l_Caster, l_Radius);
            JadeCore::UnitListSearcher<JadeCore::NearestAttackableUnitInObjectRangeCheck> searcher(p_AreaTrigger, targetList, u_check);
            p_AreaTrigger->VisitNearbyObject(l_Radius, searcher);

            for (auto itr : targetList)
            {
                if (itr->GetDistance(p_AreaTrigger) <= 6.0f)
                    l_Caster->CastSpell(itr, eSpells::SlickIce, true);
                else
                    itr->RemoveAura(eSpells::SlickIce, l_Caster->GetGUID());
            }
        }

        void OnRemove(AreaTrigger* p_AreaTrigger, uint32 /*p_Time*/)
        {
            std::list<Unit*> targetList;
            float l_Radius = 10.0f;
            Unit* l_Caster = p_AreaTrigger->GetCaster();

            JadeCore::NearestAttackableUnitInObjectRangeCheck u_check(p_AreaTrigger, l_Caster, l_Radius);
            JadeCore::UnitListSearcher<JadeCore::NearestAttackableUnitInObjectRangeCheck> searcher(p_AreaTrigger, targetList, u_check);
            p_AreaTrigger->VisitNearbyObject(l_Radius, searcher);

            for (auto itr : targetList)
            {
                if (itr->HasAura(eSpells::SlickIce, l_Caster->GetGUID()))
                    itr->RemoveAura(eSpells::SlickIce, l_Caster->GetGUID());
            }
        }

        AreaTriggerEntityScript* GetAI() const
        {
            return new spell_at_mage_wod_frost_2p_bonus();
        }
};

/// Arcane Orb - 153626
class spell_at_mage_arcane_orb : public AreaTriggerEntityScript
{
    public:
        spell_at_mage_arcane_orb() : AreaTriggerEntityScript("spell_areatrigger_arcane_orb") { }

        enum eArcaneOrbSpell
        {
            ArcaneChrage = 36032,
            ArcaneOrbDamage = 153640
        };

        void OnCreate(AreaTrigger* p_AreaTrigger)
        {
            if (Unit* l_Caster = p_AreaTrigger->GetCaster())
                l_Caster->CastSpell(l_Caster, eArcaneOrbSpell::ArcaneChrage, true);
        }

        void OnUpdate(AreaTrigger* p_AreaTrigger, uint32 p_Time)
        {
            if (Unit* l_Caster = p_AreaTrigger->GetCaster())
            {
                std::list<Unit*> l_TargetList;
                float l_Radius = 2.0f;

                JadeCore::AnyUnfriendlyUnitInObjectRangeCheck l_Check(p_AreaTrigger, l_Caster, l_Radius);
                JadeCore::UnitListSearcher<JadeCore::AnyUnfriendlyUnitInObjectRangeCheck> l_Searcher(p_AreaTrigger, l_TargetList, l_Check);
                p_AreaTrigger->VisitNearbyObject(l_Radius, l_Searcher);

                for (Unit* l_Unit : l_TargetList)
                    l_Caster->CastSpell(l_Unit, eArcaneOrbSpell::ArcaneOrbDamage, true);
            }
        }

        AreaTriggerEntityScript* GetAI() const
        {
            return new spell_at_mage_arcane_orb();
        }
};

/// Rune of Power - 116011
class spell_at_mage_rune_of_power : public AreaTriggerEntityScript
{
    public:
        spell_at_mage_rune_of_power() : AreaTriggerEntityScript("at_rune_of_power") { }

        void OnUpdate(AreaTrigger* p_AreaTrigger, uint32 p_Time)
        {
            std::list<Unit*> targetList;
            float l_Radius = 5.0f;
            Unit* l_Caster = p_AreaTrigger->GetCaster();

            if (l_Caster->IsWithinDistInMap(p_AreaTrigger, 5.0f))
            {
                if (!l_Caster->HasAura(116014))
                    l_Caster->CastSpell(l_Caster, 116014, true);
                else if (AuraPtr runeOfPower = l_Caster->GetAura(116014))
                    runeOfPower->RefreshDuration();

                if (l_Caster->ToPlayer())
                    l_Caster->ToPlayer()->UpdateManaRegen();
            }
        }

        void OnRemove(AreaTrigger* p_AreaTrigger, uint32(p_time))
        {
            Unit* l_Caster = p_AreaTrigger->GetCaster();
            if (l_Caster && l_Caster->HasAura(116014))
                l_Caster->RemoveAura(116014);
        }

        AreaTriggerEntityScript* GetAI() const
        {
            return new spell_at_mage_rune_of_power();
        }
};

/// Meteor Burn - 175396
class spell_at_mage_meteor_burn : public AreaTriggerEntityScript
{
    public:
        spell_at_mage_meteor_burn() : AreaTriggerEntityScript("spell_areatrigger_meteor_burn") { }

        enum eMeteorSpell
        {
            MeteorDoT = 155158,
            VisualID  = 45326
        };

        void OnCreate(AreaTrigger* p_AreaTrigger)
        {
            /// VisualID of 175396 is not the same of his AreaTrigger
            p_AreaTrigger->SetUInt32Value(EAreaTriggerFields::AREATRIGGER_FIELD_SPELL_VISUAL_ID, eMeteorSpell::VisualID);
        }

        void OnUpdate(AreaTrigger* p_AreaTrigger, uint32 p_Time)
        {
            if (Unit* l_Caster = p_AreaTrigger->GetCaster())
            {
                std::list<Unit*> l_TargetList;
                float l_Radius = 8.0f;

                JadeCore::AnyUnfriendlyUnitInObjectRangeCheck l_Check(p_AreaTrigger, l_Caster, l_Radius);
                JadeCore::UnitListSearcher<JadeCore::AnyUnfriendlyUnitInObjectRangeCheck> l_Searcher(p_AreaTrigger, l_TargetList, l_Check);
                p_AreaTrigger->VisitNearbyObject(l_Radius, l_Searcher);

                for (Unit* l_Unit : l_TargetList)
                    l_Caster->CastSpell(l_Unit, eMeteorSpell::MeteorDoT, true);
            }
        }

        AreaTriggerEntityScript* GetAI() const
        {
            return new spell_at_mage_meteor_burn();
        }
};

/// Meteor - 177345
class spell_at_mage_meteor_timestamp : public AreaTriggerEntityScript
{
    public:
        spell_at_mage_meteor_timestamp() : AreaTriggerEntityScript("spell_areatrigger_meteor_timestamp") {}

        enum eSpells
        {
            MeteorDamage = 153564
        };

        void OnRemove(AreaTrigger* p_AreaTrigger, uint32 /*p_Time*/)
        {
            Unit* l_Caster = p_AreaTrigger->GetCaster();

            if (l_Caster == nullptr)
                return;

            l_Caster->CastSpell(p_AreaTrigger->m_positionX, p_AreaTrigger->m_positionY, p_AreaTrigger->m_positionZ, eSpells::MeteorDamage, true);
        }

        AreaTriggerEntityScript* GetAI() const
        {
            return new spell_at_mage_meteor_timestamp();
        }
};

/// Gift of the Serpent (healing sphere) - 119031
class spell_at_monk_healing_sphere : public AreaTriggerEntityScript
{
    public:
        spell_at_monk_healing_sphere() : AreaTriggerEntityScript("at_healing_sphere") { }

        enum eGiftOfTheSerpent
        {
            SpellHealingSphere = 124041
        };

        void OnUpdate(AreaTrigger* p_AreaTrigger, uint32 p_Time)
        {
            SpellInfo const* l_CreateSpell = sSpellMgr->GetSpellInfo(p_AreaTrigger->GetSpellId());
            Unit* l_AreaTriggerCaster = p_AreaTrigger->GetCaster();

            if (l_AreaTriggerCaster && l_CreateSpell)
            {
                float l_Radius = 1.0f;
                Unit* l_Target = nullptr;

                JadeCore::AnyFriendlyUnitInObjectRangeCheck l_Checker(p_AreaTrigger, l_AreaTriggerCaster, l_Radius);
                JadeCore::UnitSearcher<JadeCore::AnyFriendlyUnitInObjectRangeCheck> l_Searcher(p_AreaTrigger, l_Target, l_Checker);
                p_AreaTrigger->VisitNearbyGridObject(l_Radius, l_Searcher);
                if (!l_Target)
                    p_AreaTrigger->VisitNearbyWorldObject(l_Radius, l_Searcher);

                if (l_Target != nullptr)
                {
                    l_AreaTriggerCaster->CastSpell(l_Target, eGiftOfTheSerpent::SpellHealingSphere, true);
                    p_AreaTrigger->Remove(0);
                }
            }
        }

        AreaTriggerEntityScript* GetAI() const
        {
            return new spell_at_monk_healing_sphere();
        }
};

/// Chi Sphere - 121286
class spell_at_monk_chi_sphere_afterlife : public AreaTriggerEntityScript
{
    public:
        spell_at_monk_chi_sphere_afterlife() : AreaTriggerEntityScript("at_chi_sphere_afterlife") { }

        void OnUpdate(AreaTrigger* p_AreaTrigger, uint32 p_Time)
        {
            std::list<Unit*> targetList;
            float l_Radius = 1.0f;
            Unit* l_Caster = p_AreaTrigger->GetCaster();

            JadeCore::AnyFriendlyUnitInObjectRangeCheck u_check(p_AreaTrigger, l_Caster, l_Radius);
            JadeCore::UnitListSearcher<JadeCore::AnyFriendlyUnitInObjectRangeCheck> searcher(p_AreaTrigger, targetList, u_check);
            p_AreaTrigger->VisitNearbyObject(l_Radius, searcher);

            if (!targetList.empty())
            {
                for (auto itr : targetList)
                {
                    if (itr->GetGUID() == l_Caster->GetGUID())
                    {
                        l_Caster->CastSpell(itr, 121283, true); // Restore 1 Chi
                        p_AreaTrigger->SetDuration(0);
                        return;
                    }
                }
            }
        }

        AreaTriggerEntityScript* GetAI() const
        {
            return new spell_at_monk_chi_sphere_afterlife();
        }
};

// Gift of the Ox 124503 124506
class spell_at_monk_gift_of_the_ox : public AreaTriggerEntityScript
{
    public:
        spell_at_monk_gift_of_the_ox()  : AreaTriggerEntityScript("at_gift_of_the_ox") { }

        void OnUpdate(AreaTrigger* p_AreaTrigger, uint32 p_Time)
        {
            std::list<Unit*> targetList;
            float l_Radius = 1.0f;
            Unit* l_Caster = p_AreaTrigger->GetCaster();

            JadeCore::AnyFriendlyUnitInObjectRangeCheck u_check(p_AreaTrigger, l_Caster, l_Radius);
            JadeCore::UnitListSearcher<JadeCore::AnyFriendlyUnitInObjectRangeCheck> searcher(p_AreaTrigger, targetList, u_check);
            p_AreaTrigger->VisitNearbyObject(l_Radius, searcher);

            for (auto itr : targetList)
            {
                if (itr->GetGUID() != l_Caster->GetGUID())
                    continue;

                l_Caster->CastSpell(itr, 124507, true); // Gift of the Ox - Heal
                p_AreaTrigger->SetDuration(0);
                return;
            }
        }

        AreaTriggerEntityScript* GetAI() const
        {
            return new spell_at_monk_gift_of_the_ox();
        }
};

class spell_at_pri_divine_star : public AreaTriggerEntityScript
{
    public:
        spell_at_pri_divine_star() : AreaTriggerEntityScript("at_pri_divine_star") { }

        enum AreaTriggerSpells
        {
            SPELL_DIVINE_STAR_HOLY = 110744,
            SPELL_DIVINE_STAR_HEAL = 110745,
            SPELL_DIVINE_STAR_DAMAGE = 122128,
        };

        std::map<uint64, uint32> m_Cooldows;

        void OnSetCreatePosition(AreaTrigger* p_AreaTrigger, Unit* p_Caster, Position& p_SourcePosition, Position& p_DestinationPosition, std::list<Position>& p_PathToLinearDestination)
        {
            Position l_Position;
            float l_Dist = 24.f; // Hardcoded in the tooltip;

            l_Position.m_positionX = p_SourcePosition.m_positionX + (l_Dist * cos(p_Caster->GetOrientation()));
            l_Position.m_positionY = p_SourcePosition.m_positionY + (l_Dist * sin(p_Caster->GetOrientation()));
            l_Position.m_positionZ = p_SourcePosition.m_positionZ;
            p_Caster->UpdateGroundPositionZ(l_Position.m_positionX, l_Position.m_positionY, l_Position.m_positionZ);

            p_PathToLinearDestination.push_back(l_Position);
            p_DestinationPosition = p_SourcePosition; // Return back
        }

        void OnUpdate(AreaTrigger* p_AreaTrigger, uint32 p_Time)
        {
            Unit* l_Caster = p_AreaTrigger->GetCaster();

            if (!l_Caster)
                return;

            std::list<Unit*> l_TargetList;
            float l_Radius = 3.f;
            bool friendly = p_AreaTrigger->GetSpellId() == SPELL_DIVINE_STAR_HOLY;
            uint32 l_SpellID = friendly ? SPELL_DIVINE_STAR_HEAL : SPELL_DIVINE_STAR_DAMAGE;

            for (std::map<uint64, uint32>::iterator iter = m_Cooldows.begin(); iter != m_Cooldows.end();)
            {
                if (iter->second < p_Time)
                    iter = m_Cooldows.erase(iter);
                else
                {
                    iter->second -= p_Time;
                    iter++;
                }
            }

            if (friendly)
            {
                JadeCore::AnyFriendlyUnitInObjectRangeCheck l_Check(p_AreaTrigger, p_AreaTrigger->GetCaster(), l_Radius);
                JadeCore::UnitListSearcher<JadeCore::AnyFriendlyUnitInObjectRangeCheck> l_Searcher(p_AreaTrigger, l_TargetList, l_Check);
                p_AreaTrigger->VisitNearbyObject(l_Radius, l_Searcher);
            }
            else
            {
                JadeCore::NearestAttackableUnitInObjectRangeCheck l_Check(p_AreaTrigger, p_AreaTrigger->GetCaster(), l_Radius);
                JadeCore::UnitListSearcher<JadeCore::NearestAttackableUnitInObjectRangeCheck> l_Searcher(p_AreaTrigger, l_TargetList, l_Check);
                p_AreaTrigger->VisitNearbyObject(l_Radius, l_Searcher);
            }

            for (auto l_Unit : l_TargetList)
            {
                if (m_Cooldows.find(l_Unit->GetGUID()) != m_Cooldows.end())
                    continue;

                m_Cooldows.insert({ l_Unit->GetGUID(), 500 });
                l_Caster->CastSpell(l_Unit, l_SpellID, true);
            }
        }

        AreaTriggerEntityScript* GetAI() const
        {
            return new spell_at_pri_divine_star();
        }
};

/// Power Word: Barrier - 62618
class spell_at_pri_power_word_barrier : public AreaTriggerEntityScript
{
    public:
        spell_at_pri_power_word_barrier() : AreaTriggerEntityScript("spell_areatrigger_power_word_barrier") { }

        enum ePowerWordBarrierSpell
        {
            PowerWordBarrierAura = 81782
        };

        void OnUpdate(AreaTrigger* p_AreaTrigger, uint32 p_Time)
        {
            if (Unit* l_Caster = p_AreaTrigger->GetCaster())
            {
                std::list<Unit*> l_FriendListInRadius;
                float l_Radius = 6.5f;

                JadeCore::AnyFriendlyUnitInObjectRangeCheck l_Check(p_AreaTrigger, l_Caster, l_Radius);
                JadeCore::UnitListSearcher<JadeCore::AnyFriendlyUnitInObjectRangeCheck> l_Searcher(p_AreaTrigger, l_FriendListInRadius, l_Check);
                p_AreaTrigger->VisitNearbyObject(l_Radius, l_Searcher);

                if (!l_FriendListInRadius.empty())
                {
                    for (Unit* l_Unit : l_FriendListInRadius)
                    {
                        if (l_Caster->IsValidAssistTarget(l_Unit))
                            l_Caster->CastSpell(l_Unit, ePowerWordBarrierSpell::PowerWordBarrierAura, true);
                    }
                }
            }
        }

        AreaTriggerEntityScript* GetAI() const
        {
            return new spell_at_pri_power_word_barrier();
        }
};

/// Angelic Feather - 121536
class spell_at_pri_angelic_feather : public AreaTriggerEntityScript
{
    public:
        spell_at_pri_angelic_feather() : AreaTriggerEntityScript("at_angelic_feather") { }

        void OnUpdate(AreaTrigger* p_AreaTrigger, uint32 p_Time)
        {
            std::list<Unit*> targetList;
            float l_Radius = 1.0f;
            Unit* l_Caster = p_AreaTrigger->GetCaster();

            JadeCore::AnyFriendlyUnitInObjectRangeCheck u_check(p_AreaTrigger, l_Caster, l_Radius);
            JadeCore::UnitListSearcher<JadeCore::AnyFriendlyUnitInObjectRangeCheck> searcher(p_AreaTrigger, targetList, u_check);
            p_AreaTrigger->VisitNearbyObject(l_Radius, searcher);

            if (!targetList.empty())
            {
                for (auto itr : targetList)
                {
                    l_Caster->CastSpell(itr, 121557, true); // Angelic Feather increase speed
                    p_AreaTrigger->SetDuration(0);
                    return;
                }
            }
        }

        AreaTriggerEntityScript* GetAI() const
        {
            return new spell_at_pri_angelic_feather();
        }
};

/// Smoke Bomb - 76577
class spell_at_rogue_smoke_bomb : public AreaTriggerEntityScript
{
    public:
        spell_at_rogue_smoke_bomb() : AreaTriggerEntityScript("spell_areatrigger_smoke_bomb") { }

        enum eSmokeSpells
        {
            SmokeBombAura = 88611
        };

        void OnUpdate(AreaTrigger* p_AreaTrigger, uint32 p_Time)
        {
            if (Unit* l_Caster = p_AreaTrigger->GetCaster())
            {
                std::list<Unit*> l_TargetList;
                float l_Radius = 8.0f;

                JadeCore::AnyFriendlyUnitInObjectRangeCheck l_Check(p_AreaTrigger, l_Caster, l_Radius);
                JadeCore::UnitListSearcher<JadeCore::AnyFriendlyUnitInObjectRangeCheck> l_Searcher(p_AreaTrigger, l_TargetList, l_Check);
                p_AreaTrigger->VisitNearbyObject(l_Radius, l_Searcher);

                for (Unit* l_Unit : l_TargetList)
                    l_Caster->CastSpell(l_Unit, eSmokeSpells::SmokeBombAura, true);
            }
        }

        AreaTriggerEntityScript* GetAI() const
        {
            return new spell_at_rogue_smoke_bomb();
        }
};

void AddSC_areatrigger_spell_scripts()
{
    /// Deathknight Area Trigger
    new spell_at_dk_defile();

    /// Druid Area Trigger
    new spell_at_druid_fungal_growth();
    new spell_at_druid_ursol_vortex();

    /// Hunter Area Trigger
    new spell_at_hun_binding_shot();
    new spell_at_hun_ice_trap();
    new spell_at_hun_ice_trap_effect();
    new spell_at_hun_freezing_trap();
    new spell_at_hun_explosive_trap();

    /// Mage Area Trigger
    new spell_at_mage_wod_frost_2p_bonus();
    new spell_at_mage_arcane_orb();
    new spell_at_mage_meteor_burn();
    new spell_at_mage_meteor_timestamp();
    new spell_at_mage_rune_of_power();

    /// Monk Area Trigger
    new spell_at_monk_healing_sphere();
    new spell_at_monk_chi_sphere_afterlife();
    new spell_at_monk_gift_of_the_ox();

    /// Priest Area Trigger
    new spell_at_pri_divine_star();
    new spell_at_pri_power_word_barrier();
    new spell_at_pri_angelic_feather();

    /// Rogue Area Trigger
    new spell_at_rogue_smoke_bomb();
}