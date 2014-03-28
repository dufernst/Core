/*Copyright (C) 2014 SkyMist Project.
*
* This file is NOT free software. Third-party users can NOT redistribute it or modify it :). 
* If you find it, you are either hacking something, or very lucky (presuming someone else managed to hack it).
*/

#include "ScriptPCH.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "ScriptedCreature.h"
#include "Map.h"
#include "PoolMgr.h"
#include "AccountMgr.h"

#include "terrace_of_endless_spring.h"

class instance_terrace_of_endless_spring : public InstanceMapScript
{
    public:
        instance_terrace_of_endless_spring() : InstanceMapScript("instance_terrace_of_endless_spring", 36) { }

        struct instance_terrace_of_endless_spring_InstanceMapScript : public InstanceScript
        {
            instance_terrace_of_endless_spring_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
            {
                Initialize();
            }

            // Bosses.
            uint64 ProtectorKaolanGUID;
            uint64 ElderAsanjGUID;
            uint64 ElderRegailGUID;
            uint64 TsulongGUID;
            uint64 LeiShiGUID;
            uint64 ShaofFearGUID;

            void Initialize()
            {
                SetBossNumber(MAX_ENCOUNTER);

                // Bosses.
                ProtectorKaolanGUID          = 0;
                ElderAsanjGUID               = 0;
                ElderRegailGUID              = 0;
                TsulongGUID                  = 0;
                LeiShiGUID                   = 0;
                ShaofFearGUID                = 0;

                for (uint32 i = 0; i < MAX_ENCOUNTER; ++i)
                    SetBossState(i, NOT_STARTED);
            }

            bool IsEncounterInProgress() const
            {
                for (uint32 i = 0; i < MAX_ENCOUNTER; ++i)
                    if (GetBossState(i) == IN_PROGRESS)
                        return true;
            
                return false;
            }

            void OnCreatureCreate(Creature* creature)
            {
                switch (creature->GetEntry())
                {
                    // Bosses.
                    case BOSS_PROTECTOR_KAOLAN:
                        ProtectorKaolanGUID = creature->GetGUID();
                        break;
                    case BOSS_ELDER_ASANJ:
                        ElderAsanjGUID = creature->GetGUID();
                        break;
                    case BOSS_ELDER_REGAIL:
                        ElderRegailGUID = creature->GetGUID();
                        break;
                    case BOSS_TSULONG:
                        TsulongGUID = creature->GetGUID();
                        break;
                    case BOSS_LEI_SHI:
                        LeiShiGUID = creature->GetGUID();
                        break;
                    case BOSS_SHA_OF_FEAR:
                        ShaofFearGUID = creature->GetGUID();
                        break;

                    default: break;
                }
            }

/*
            void OnUnitDeath(Unit* killed)
            {
                if (killed->GetTypeId() == TYPEID_PLAYER) return;
            
                switch(killed->ToCreature()->GetEntry())
                {
                }
            }

            void OnGameObjectCreate(GameObject* go)
            {
                switch (go->GetEntry())
                {
                }
            }

            void OnGameObjectRemove(GameObject* go)
            {
                switch (go->GetEntry())
                {
                }
            }
*/

            void SetData(uint32 type, uint32 data)
            {
                SetBossState(type, EncounterState(data));

                if (data == DONE)
                    SaveToDB();
            }

            uint32 GetData(uint32 type)
            {
                return GetBossState(type);
            }

            uint64 GetData64(uint32 data)
            {
                switch (data)
                {
                    case DATA_PROTECTOR_KAOLAN:  return ProtectorKaolanGUID;    break;
                    case DATA_ELDER_ASANJ:       return ElderAsanjGUID;         break;
                    case DATA_ELDER_REGAIL:      return ElderRegailGUID;        break;
                    case DATA_TSULONG:           return TsulongGUID;            break;
                    case DATA_LEI_SHI:           return LeiShiGUID;             break;
                    case DATA_SHA_OF_FEAR:       return ShaofFearGUID;          break;

                    default:                     return 0;                      break;
                }
            }

            bool SetBossState(uint32 data, EncounterState state)
            {
                if (!InstanceScript::SetBossState(data, state))
                    return false;

                if (state == DONE)
                {
                    switch(data)
                    {
                        case DATA_PROTECTOR_KAOLAN_EVENT:
						case DATA_ELDER_ASANJ_EVENT:
						case DATA_ELDER_REGAIL_EVENT:
						case DATA_TSULONG_EVENT:
						case DATA_LEI_SHI_EVENT:
						case DATA_SHA_OF_FEAR_EVENT:
                        break;
                    }
                }

                return true;
            }

            std::string GetSaveData()
            {
                OUT_SAVE_INST_DATA;

                std::ostringstream saveStream;
                saveStream << "D M " << GetBossSaveData();

                OUT_SAVE_INST_DATA_COMPLETE;
                return saveStream.str();
            }

            void Load(const char* in)
            {
                if (!in)
                {
                    OUT_LOAD_INST_DATA_FAIL;
                    return;
                }

                OUT_LOAD_INST_DATA(in);

                char dataHead1, dataHead2;

                std::istringstream loadStream(in);
                loadStream >> dataHead1 >> dataHead2;

                if (dataHead1 == 'D' && dataHead2 == 'M')
                {
                    for (uint32 i = 0; i < MAX_ENCOUNTER; ++i)
                    {
                        uint32 tmpState;
                        loadStream >> tmpState;

                        if (tmpState == IN_PROGRESS || tmpState > SPECIAL)
                            tmpState = NOT_STARTED;

                        // Below makes the player on-instance-entry display of bosses killed shit work (SMSG_RAID_INSTANCE_INFO).
                        // Like, say an unbound player joins the party and he tries to enter the dungeon / raid.
                        // This makes sure binding-to-instance-on-entrance confirmation box will properly display bosses defeated / available.
                        SetBossState(i, EncounterState(tmpState));
                    }

                } else OUT_LOAD_INST_DATA_FAIL;

                OUT_LOAD_INST_DATA_COMPLETE;
            }
        };

        InstanceScript* GetInstanceScript(InstanceMap* map) const
        {
            return new instance_terrace_of_endless_spring_InstanceMapScript(map);
        }
};

void AddSC_instance_terrace_of_endless_spring()
{
    new instance_terrace_of_endless_spring();
}