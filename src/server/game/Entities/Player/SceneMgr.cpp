/*
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
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

#include "SceneMgr.h"
#include "Chat.h"
#include "Language.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"

SceneMgr::SceneMgr(Player* player) : _player(player)
{
    _standaloneSceneInstanceID = 0;
}

uint32 SceneMgr::PlayScene(uint32 sceneId, Position const* position /*= nullptr*/)
{
    SceneTemplate const* sceneTemplate = sObjectMgr->GetSceneTemplate(sceneId);
    return PlaySceneByTemplate(sceneTemplate, position);
}

uint32 SceneMgr::PlaySceneByTemplate(SceneTemplate const* sceneTemplate, Position const* position /*= nullptr*/)
{
    if (!sceneTemplate)
        return 0;

    SceneScriptPackageEntry const* entry = sSceneScriptPackageStore.LookupEntry(sceneTemplate->ScenePackageId);
    if (!entry)
        return 0;

    // By default, take player position
    if (!position)
        position = GetPlayer();

    uint32 sceneInstanceID = GetNewStandaloneSceneInstanceID();

    WorldPacket data(SMSG_PLAY_SCENE, 4 + 4 + 4 + 4 + 8 + 4 + 4 + 4 + 4);
    data << int32(sceneTemplate->SceneId);
    data << int32(sceneTemplate->PlaybackFlags);
    data << int32(sceneInstanceID);
    data << int32(sceneTemplate->ScenePackageId);
    data << GetPlayer()->GetTransGUID();
    Position pos = *position;
    data << pos.PositionXYZOStream();

    GetPlayer()->GetSession()->SendPacket(&data);

    AddInstanceIdToSceneMap(sceneInstanceID, sceneTemplate);

    return sceneInstanceID;
}

uint32 SceneMgr::PlaySceneByPackageId(uint32 sceneScriptPackageId, uint32 playbackflags /*= SCENEFLAG_UNK16*/, Position const* position /*= nullptr*/)
{
    SceneTemplate sceneTemplate;
    sceneTemplate.SceneId           = 0;
    sceneTemplate.ScenePackageId    = sceneScriptPackageId;
    sceneTemplate.PlaybackFlags     = playbackflags;

    return PlaySceneByTemplate(&sceneTemplate, position);
}

void SceneMgr::CancelScene(uint32 sceneInstanceID, bool removeFromMap /*= true*/)
{
    if (removeFromMap)
        RemoveSceneInstanceId(sceneInstanceID);

    /*WorldPackets::Scenes::CancelScene cancelScene;
    cancelScene.SceneInstanceID = sceneInstanceID;
    GetPlayer()->GetSession()->SendPacket(cancelScene.Write(), true);*/
}

void SceneMgr::OnSceneTrigger(uint32 sceneInstanceID, std::string const& triggerName)
{
    if (!HasScene(sceneInstanceID))
        return;

    //SceneTemplate const* sceneTemplate = GetSceneTemplateFromInstanceId(sceneInstanceID);
}

void SceneMgr::OnSceneCancel(uint32 sceneInstanceID)
{
    if (!HasScene(sceneInstanceID))
        return;

    SceneTemplate const* sceneTemplate = GetSceneTemplateFromInstanceId(sceneInstanceID);

    // Must be done before removing aura
    RemoveSceneInstanceId(sceneInstanceID);

    if (sceneTemplate->SceneId != 0)
        RemoveAurasDueToSceneId(sceneTemplate->SceneId);
}

void SceneMgr::OnSceneComplete(uint32 sceneInstanceID)
{
    if (!HasScene(sceneInstanceID))
        return;

    SceneTemplate const* sceneTemplate = GetSceneTemplateFromInstanceId(sceneInstanceID);

    // Must be done before removing aura
    RemoveSceneInstanceId(sceneInstanceID);

    if (sceneTemplate->SceneId != 0)
        RemoveAurasDueToSceneId(sceneTemplate->SceneId);
}

bool SceneMgr::HasScene(uint32 sceneInstanceID, uint32 sceneScriptPackageId /*= 0*/) const
{
    auto itr = _scenesByInstance.find(sceneInstanceID);

    if (itr != _scenesByInstance.end())
        return !sceneScriptPackageId || sceneScriptPackageId == itr->second->ScenePackageId;

    return false;
}

void SceneMgr::AddInstanceIdToSceneMap(uint32 sceneInstanceID, SceneTemplate const* sceneTemplate)
{
    _scenesByInstance[sceneInstanceID] = sceneTemplate;
}

void SceneMgr::CancelSceneByPackageId(uint32 sceneScriptPackageId)
{
    std::vector<uint32> instancesIds;

    for (auto itr : _scenesByInstance)
        if (itr.second->ScenePackageId == sceneScriptPackageId)
            instancesIds.push_back(itr.first);

    for (uint32 sceneInstanceID : instancesIds)
        CancelScene(sceneInstanceID);
}

void SceneMgr::RemoveSceneInstanceId(uint32 sceneInstanceID)
{
    _scenesByInstance.erase(sceneInstanceID);
}

void SceneMgr::RemoveAurasDueToSceneId(uint32 sceneId)
{
    Player::AuraEffectList const& scenePlayAuras = GetPlayer()->GetAuraEffectsByType(SPELL_AURA_PLAY_SCENE);
    for (AuraEffectPtr scenePlayAura : scenePlayAuras)
    {
        if (uint32(scenePlayAura->GetMiscValue()) == sceneId)
        {
            GetPlayer()->RemoveAura(scenePlayAura->GetBase());
            break;
        }
    }
}

SceneTemplate const* SceneMgr::GetSceneTemplateFromInstanceId(uint32 sceneInstanceID)
{
    auto itr = _scenesByInstance.find(sceneInstanceID);

    if (itr != _scenesByInstance.end())
        return itr->second;

    return nullptr;
}

uint32 SceneMgr::GetActiveSceneCount(uint32 sceneScriptPackageId /*= 0*/)
{
    uint32 activeSceneCount = 0;

    for (auto itr : _scenesByInstance)
        if (!sceneScriptPackageId || itr.second->ScenePackageId == sceneScriptPackageId)
            ++activeSceneCount;

    return activeSceneCount;
}
