/* Copyright (C) 2006 - 2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: Boss_Kormok
SD%Complete: 100
SDComment:
SDCategory: Scholomance
EndScriptData */

#include "scriptPCH.h"

#define SPELL_SHADOWBOLTVOLLEY      20741
#define SPELL_BONESHIELD            27688

struct boss_kormokAI : public ScriptedAI
{
    boss_kormokAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }

    uint32 ShadowVolley_Timer;
    uint32 BoneShield_Timer;
    uint32 Minion_Timer;
    uint32 Mage_Timer;
    bool Mages;

    void Reset()
    {
        ShadowVolley_Timer = Randomize(10000);
        BoneShield_Timer = 2000;
        Minion_Timer = Randomize(15000);
        Mage_Timer = 0;
        Mages = false;
    }

    void SummonMinion(Unit* victim)
    {
        if (Creature* summonedMinion = DoSpawnCreature(16119, 8.0f, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 120000))
            if (summonedMinion->AI())
                summonedMinion->AI()->AttackStart(victim);
    }

    void SummonMages(Unit* victim)
    {
        if (Creature* summonedMage = DoSpawnCreature(16120, 8.0f, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 120000))
            if (summonedMage->AI())
                summonedMage->AI()->AttackStart(victim);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        //ShadowVolley_Timer
        if (ShadowVolley_Timer < diff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_SHADOWBOLTVOLLEY) == CAST_OK)
                ShadowVolley_Timer = Randomize(15000);
        }
        else
            ShadowVolley_Timer -= diff;

        //BoneShield_Timer
        if (BoneShield_Timer < diff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_BONESHIELD) == CAST_OK)
                BoneShield_Timer = Randomize(45000);
        }
        else
            BoneShield_Timer -= diff;

        //Minion_Timer
        if (Minion_Timer < diff)
        {
            //Cast
            for (int i = 0; i < 4; ++i)
                SummonMinion(m_creature->getVictim());

            Minion_Timer = Randomize(12000);
        }
        else
            Minion_Timer -= diff;

        //Summon 2 Bone Mages
        if (!Mages && m_creature->GetHealthPercent() < 26.0f)
        {
            //Cast
            SummonMages(m_creature->getVictim());
            SummonMages(m_creature->getVictim());
            Mages = true;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_kormok(Creature* pCreature)
{
    return new boss_kormokAI(pCreature);
}

void AddSC_boss_kormok()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_kormok";
    newscript->GetAI = &GetAI_boss_kormok;
    newscript->RegisterSelf();
}
